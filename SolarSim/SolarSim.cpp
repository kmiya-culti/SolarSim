
#include "SolarSim.h"

#if		defined(ROCKET_VIEW)
	double baseTimeStep = 3600.0 * 0.05;
	double viewZoom = 1000000.0;
	int targetIdx = 3;
	double rotateX = 0.0;
	double rotateY = 0.0;
	int FramePerSec = 600;	// Frame / sec * 10
#elif	defined(ASTEROID_BELT) || defined(TROJAN_ASTEROIDS)
	double baseTimeStep = 3600.0 * 100.0;
	double viewZoom = 100.0;
	int targetIdx = 0;
	double rotateX = -0.35;
	double rotateY = 0.0;
	int FramePerSec = 100;
#elif	defined(SATURN_RINGS)
	double baseTimeStep = 3600.0 * 0.05;
	double viewZoom = 300000.0;
	int targetIdx = 6;
	double rotateX = -0.35;
	double rotateY = 0.0;
	int FramePerSec = 600;
#else
	double baseTimeStep = 3600.0 * 2.0;
	double viewZoom = 300.0;
	int targetIdx = 0;
	double rotateX = -0.35;
	double rotateY = 0.0;
	int FramePerSec = 600;
#endif

// --- グローバル変数 ---

vector<Vector3D> posV, velV;
vector<double> massV, radiusV;
vector<uint32_t> colorV;
float ViewProjMap[32];

int globalSubSteps = 20;
double baseTimeDivs = 1.0;
bool targetLock = false;
double TotalTime = 0.0;
int TotalYear = 0;
int InitTexture = 0;
double worldZoom = 1.0;
bool earthEyes = false;

vector<Body> bodies;
ULONG_PTR gdiplusToken;

float SaveCx = 0;
float SaveCy = 0;
double SaveZoom = 300.0;
double SaveRotateX = 0.0;
double SaveRotateY = 0.0;
int SaveIdx = 0;

Vector3D CenterPos = { 0, 0, 0 };
Vector3D CenterVel = { 0, 0, 0 };

int RocketNum = 0;
double TotalPhysicsMsec = 0.0;
double TotalPhysicsStep = 0.0;

clock_t FrameViewClock = 0;
int FrameUpdateCount = 0;
int FrameParSec = 0;

bool MouseDown = false;
POINT MousePos = { 0, 0 };
double MouseRoll = 0.0;
double MouseScll = 0.0;

#ifdef	CUDA_KERNEL
	extern "C" void launchCudaPhysics(void* pos, void* vel, double* mass, double* radius, int numBodies, double dt);
	extern "C" void CudaUpdateOrbits(void* pos, void* vel, double* mass, double* radius, int numBodies, double dt);

	#ifdef	DIRECT3D_SWAP
		struct Vec3 {
			double x, y, z;
		};

		extern "C" void mapAndWriteVertices(void* d_pos, void* d_vel, void* d_mass, void* d_color, int numBodies, float* d_m, Vec3 center);
	#endif	// DIRECT3D_SWAP

	Vector3D *d_pos = nullptr, *d_vel = nullptr;
	double *d_mass = nullptr, *d_radius = nullptr;
	uint32_t *d_color = nullptr;
	float* d_viewProj = nullptr;
	int CudaDeviceCount = 0;
#endif	// CUDA_KERNEL

#ifdef	DIRECT2D_VIEW
#ifdef	DIRECT3D_SWAP
	ID2D1Factory1* pD2DFactory = nullptr;
	ID2D1DeviceContext* pRT = nullptr;
	ID2D1SolidColorBrush* pBrush = nullptr;
	IDWriteFactory* pDWriteFactory = nullptr;
	IDWriteTextFormat* pTextFormat = nullptr;
	IDWriteTextFormat* pPlanetFormat = nullptr;

	IDXGISwapChain1* pSwapChain = nullptr;
	ID3D11RenderTargetView* pBackBufferRT = nullptr;

	ID2D1Device* pD2DDevice = nullptr;
	IDXGIDevice* pDxgiDevice = nullptr;
	ID3D11Device* pD3DDevice = nullptr;
	ID3D11DeviceContext* pD3DContext = nullptr;

	ID3D11VertexShader* pAsteroidShadowVS = nullptr;
	ID3D11PixelShader* pAsteroidShadowPS = nullptr;
	ID3D11VertexShader* pAsteroidVS = nullptr;
	ID3D11PixelShader* pAsteroidPS = nullptr;
	ID3D11InputLayout* pInputLayout = nullptr;
	ID3D11BlendState* pBlendState = nullptr;
	ID3D11BlendState* pLabelBlendState = nullptr;

	struct Vertex {
		float4 pos;
		float4 spos;
		uint32_t color;
	};

	int ScreenWidth = 0;
	int ScreenHeight = 0;
	ID3D11Buffer* pVertexBuffer = nullptr;
	cudaGraphicsResource_t cudaVBResource = nullptr; // CUDAと共有するためのハンドル

	struct PlanetVertex {
		XMFLOAT3 Pos;    // 位置
		XMFLOAT3 Normal; // 法線（三日月表現に必須）
		XMFLOAT2 Tex;
	};

	struct RingVertex {
		XMFLOAT3 Pos;
		XMFLOAT2 Tex; // 内側を0.0、外側を1.0とするUV（テクスチャ貼る用）
	};

	ID3D11PixelShader* pSimplePS = nullptr;
	ID3D11PixelShader* pShadowPS = nullptr;
	ID3D11VertexShader* pPlanetVS = nullptr;
	ID3D11PixelShader* pPlanetPS = nullptr;
	ID3D11InputLayout* pPlanetInputLayout = nullptr;

	ID3D11Buffer* pPlanetVB = nullptr;
	ID3D11Buffer* pPlanetIB = nullptr;
	UINT sphereIndexCount = 0;
	ID3D11Buffer* pConstantBuffer = nullptr;

	// planet.hlsl内のcbuffer ConstantBufferと同じになるように注意
	struct PlanetConstantBuffer {
		XMMATRIX World;
		XMMATRIX DispViewProj;
		XMMATRIX LightViewProj;
		XMFLOAT3 SunPos;
		float padding1;
		XMFLOAT3 CameraPos;
		float padding2;
		XMFLOAT3 PlanetColor;
		float padding3;
	};

	ID3D11Texture2D* pDepthStencilBuffer = nullptr;
	ID3D11DepthStencilView* pDepthStencilView = nullptr;
	ID3D11DepthStencilState* pDSState_Normal = nullptr;
	ID3D11DepthStencilState* pDSState_NoWrite = nullptr;
	ID3D11DepthStencilState* pDSState_None = nullptr;

	ID3D11Texture2D* pShadowMapBuffer = nullptr;
	ID3D11DepthStencilView* pShadowDepthView = nullptr;
	ID3D11ShaderResourceView* pShadowResourceView = nullptr;
	ID3D11SamplerState* pSamplerState = nullptr;
	ID3D11SamplerState* pSamplerStateLinear = nullptr;
	ID3D11SamplerState* pSamplerState2DLinear = nullptr;

	ID3D11RasterizerState* pDrawingRenderState = nullptr;
	ID3D11RasterizerState* pShadowRenderState = nullptr;
	ID3D11RasterizerState* pNomalRenderState = nullptr;

	ID3D11Texture1D* pUranusRingTexture = nullptr;
	ID3D11ShaderResourceView* pUranusRingSRV = nullptr;

	ID3D11Buffer* pUranusRingVB = nullptr;
	ID3D11Buffer* pUranusRingIB = nullptr;
	UINT UranusRingIndexCount = 0;

	ID3D11Texture1D* pRingTexture = nullptr;
	ID3D11ShaderResourceView* pRingSRV = nullptr;

	ID3D11Buffer* pRingVB = nullptr;
	ID3D11Buffer* pRingIB = nullptr;
	UINT ringIndexCount = 0;

	ID3D11PixelShader* pRingShadowPS = nullptr;
	ID3D11PixelShader* pRingPS = nullptr;
	ID3D11VertexShader* pRingVS = nullptr;
	ID3D11InputLayout* pRingInputLayout = nullptr;

	ID2D1DeviceContext* pLabelRT = nullptr;
	ID3D11Buffer* pLabelVB = nullptr;
	ID3D11Buffer* pLabelIB = nullptr;

	ID3D11PixelShader* pLabelPS = nullptr;
	ID3D11VertexShader* pLabelVS = nullptr;
	ID3D11InputLayout* pLabelInputLayout = nullptr;

	ID3D11Texture2D* pEarthTexture[12] = {};
	ID3D11ShaderResourceView* pEarthSRV[12] = {};

	ID3D11VertexShader* pDebugVS = nullptr;
	ID3D11PixelShader* pDebugPS = nullptr;
#else
	ID2D1Factory* pD2DFactory = nullptr;
	ID2D1HwndRenderTarget* pRT = nullptr;
	ID2D1SolidColorBrush* pBrush = nullptr;
	IDWriteFactory* pDWriteFactory = nullptr;
	IDWriteTextFormat* pTextFormat = nullptr;
	IDWriteTextFormat* pPlanetFormat = nullptr;
#endif	// DIRECT3D_SWAP
#endif	// DIRECT2D_VIEW

#ifdef	DIRECT2D_VIEW
// --- Dicrect2D init ---
HRESULT InitDirect2D(HWND hWnd) {
	HRESULT hr = 0;

	if (pD2DFactory == nullptr) {
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
		if (FAILED(hr))	return hr;
	}

#ifdef	DIRECT3D_SWAP
	// 3. デバイスコンテキスト（新しいレンダーターゲット）の作成
	hr = pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pRT);
	if (FAILED(hr)) return hr;

	// 4. スワップチェーンのバックバッファをD2Dのビットマップとして紐付け
	IDXGISurface* pDxgiSurface = nullptr;
	hr = pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&pDxgiSurface);
	if (FAILED(hr)) return hr;

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	ID2D1Bitmap1* pD2DBackBuffer = nullptr;
	hr = pRT->CreateBitmapFromDxgiSurface(pDxgiSurface, &bitmapProperties, &pD2DBackBuffer);
	if (FAILED(hr)) return hr;

	// 5. 描画先としてセット
	pRT->SetTarget(pD2DBackBuffer);

	pD2DBackBuffer->Release();
	pDxgiSurface->Release();

#else
	RECT rc;
	GetClientRect(hWnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	// レンダーターゲットの作成
	hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &pRT);
	if (FAILED(hr))	return hr;
#endif

	// デフォルトのブラシ（白）を作成
	hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
	if (FAILED(hr))	return hr;

	// DirectWrite の初期化
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if (FAILED(hr))	return hr;

	// テキストフォントの作成
	hr = pDWriteFactory->CreateTextFormat(
		L"Consolas",                // フォント名
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		14.0f,                      // フォントサイズ
		L"ja-jp",
		&pTextFormat
	);
	if (FAILED(hr))	return hr;

	// 衛星名フォントの作成
	hr = pDWriteFactory->CreateTextFormat(
		L"Comic Sans MS",                // フォント名
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		9.0f,                      // フォントサイズ
		L"ja-jp",
		&pPlanetFormat
	);

	return hr;
}
void ClearDirect2D() {
	if (pBrush) pBrush->Release();
	if (pTextFormat) pTextFormat->Release();
	if (pPlanetFormat) pPlanetFormat->Release();
	if (pDWriteFactory) pDWriteFactory->Release();
	if (pRT) pRT->Release();
	if (pD2DFactory) pD2DFactory->Release();
	pD2DFactory = nullptr;
}

#ifdef	DIRECT3D_SWAP

extern	HRESULT CreatePlanetTexture(ID3D11Device* pDevice, Body& b);

HRESULT CreateLabelTexture(Body &b, int width, int height) {

		// 惑星ごとに小さなD3Dテクスチャを作る (サイズは 256x64 程度で十分です)
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc = { 1, 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		HRESULT hr = pD3DDevice->CreateTexture2D(&desc, nullptr, &b.pLabelTexture);
		if (FAILED(hr)) return hr;

		// SRVを作る
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		hr = pD3DDevice->CreateShaderResourceView(b.pLabelTexture, &srvDesc, &b.pLabelSRV);
		if (FAILED(hr)) return hr;

		// 先ほどの安全な方法で D2Dビットマップ(b.pLabelBitmap) を作成
		IDXGISurface* pDxgiSurface = nullptr;
		b.pLabelTexture->QueryInterface(__uuidof(IDXGISurface), (void**)&pDxgiSurface);
		D2D1_BITMAP_PROPERTIES1 prop = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
		hr = pLabelRT->CreateBitmapFromDxgiSurface(pDxgiSurface, prop, &b.pLabelBitmap);
		pDxgiSurface->Release();
		if (FAILED(hr)) return hr;

		// 共通デバイスコンテキスト(pLabelRT)に対象をセットして、文字を「1回だけ」書き込む！
		pLabelRT->SetTarget(b.pLabelBitmap);

		pLabelRT->BeginDraw();
		pLabelRT->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f)); // 背景透明

		ID2D1SolidColorBrush* pBr = nullptr;
		pLabelRT->CreateSolidColorBrush(D2D1::ColorF(GetRByte(colorV[b.i]) / 255.f, GetGByte(colorV[b.i]) / 255.f, GetBByte(colorV[b.i]) / 255.f, GetAByte(colorV[b.i]) / 255.f), &pBr);

		IDWriteTextFormat* pFontFormat = nullptr;
		pDWriteFactory->CreateTextFormat(L"Comic Sans MS", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, (float) height * 8.0f / 10.0f, L"ja-jp", &pFontFormat);

		// ここで各惑星の名前を書き込む
		pLabelRT->DrawText(b.name.c_str(), (UINT32)b.name.length(), pFontFormat, D2D1::RectF(0, 0, 256.f, 64.f), pBr);

		pBr->Release();
		pLabelRT->EndDraw();

		return hr;
}
HRESULT CreateUranusRingTexture(ID3D11Device* pDevice) {
	vector<uint32_t> pixels(1024);

	ZeroMemory(pixels.data(), sizeof(uint32_t) * 1024);

	//   0- 139	輝度:低　コンストラクト:少
	for (int i = 0; i < 139; i++) {
		float alpha = sinf((float)i * 0.02f) * 0.2f + 0.2f;
		pixels[i] = 0x00FFFFDD | ((int)(alpha * 255.0f) << 24);
	}
	// 139- 262	輝度:高　コンストラクト:高
	for (int i = 139; i < 262; i++) {
		float alpha = sinf((float)(i - 139) * 0.03f) * 0.6f + 0.3f;
		pixels[i] = 0x00FFFFDD | ((int)(alpha * 255.0f) << 24);
	}
	// 472- 520	輝度:低　コンストラクト:少
	for (int i = 472; i < 520; i++) {
		float alpha = sinf((float)(i - 472) * 0.1f) * 0.2f + 0.2f;
		pixels[i] = 0x00FFFFDD | ((int)(alpha * 255.0f) << 24);
	}
	// 808-1024	輝度:低　コンストラクト:少
	for (int i = 808; i < 1024; i++) {
		float alpha = sinf((float)(i - 808) * 0.02f) * 0.2f + 0.3f;
		pixels[i] = 0x00FFFFDD | ((int)(alpha * 255.0f) << 24);
	}

	// --- Direct3D 11 テクスチャの構築 ---
	D3D11_TEXTURE1D_DESC desc = {};
	desc.Width = (UINT)pixels.size();
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage = D3D11_USAGE_IMMUTABLE; // 変更しないので高速なメモリに配置
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pixels.data();

	HRESULT hr = pDevice->CreateTexture1D(&desc, &initData, &pUranusRingTexture);
	if (FAILED(hr)) return hr;

	// シェーダーリソースビュー (SRV) の作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srvDesc.Texture1D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(pUranusRingTexture, &srvDesc, &pUranusRingSRV);
	return hr;
}
HRESULT CreateSaturnRingTexture(ID3D11Device* pDevice) {
	const int textureSize = 1024; // テクスチャの解像度（縦の細かさ）
	vector<uint32_t> pixels(textureSize);

	// 土星本体の半径を 1.0 としたときの、環の各エリアの開始・終了比率（科学的データ準拠）
	const float ringMin = 1.11f; // D環の内界
	const float ringMax = 2.30f; // A環（またはF環）の外界
	const float totalWidth = ringMax - ringMin;

	for (int i = 0; i < textureSize; ++i) {
		// テクスチャの0.0（内側）～1.0（外側）に対応する、土星半径比の距離を割り出す
		float t = (float)i / (float)(textureSize - 1);
		float r = ringMin + t * totalWidth;

		uint8_t r_col = 0, g_col = 0, b_col = 0, a_col = 0;

		// --- 各環の構造に応じた色と透明度(Alpha)の割り当て ---
		if (r < 1.24f) {
			// 【D環】非常に薄い灰色の環
			r_col = 140; g_col = 140; b_col = 140; a_col = 60;
		}
		else if (r < 1.53f) {
			// 【C環】やや半透明で暗い灰色の環
			r_col = 150; g_col = 145; b_col = 140; a_col = 100;
		}
		else if (r < 1.95f) {
			// 【B環】最も明るく、密度が濃い主環（ほぼ不透明）
			// 内部のグラデーション（縞模様）を擬似的にノイズで表現
			float stripe = 0.85f + 0.15f * sinf(r * 10.0f);
			r_col = (uint8_t)(215 * stripe);
			g_col = (uint8_t)(205 * stripe);
			b_col = (uint8_t)(190 * stripe);
			a_col = 240; // ほぼ不透明
		}
		else if (r < 2.02f) {
			// 【カッシーニの間隙】完全に透明な隙間（わずかに物質があるが視覚的には0）
			r_col = 0; g_col = 0; b_col = 0; a_col = 0;
		}
		else if (r < 2.21f) {
			// 【A環】中程度の明るさを持つ主環
			float stripe = 0.85f + 0.15f * sinf(r * 20.0f);

			// エンケの間隙 (半径 2.21 付近にある細い隙間) のシミュレート
			if (r > 2.195f && r < 2.200f) {
				r_col = 0; g_col = 0; b_col = 0; a_col = 0;
			} else {
				r_col = (uint8_t)(180 * stripe);
				g_col = (uint8_t)(175 * stripe);
				b_col = (uint8_t)(165 * stripe);
				a_col = 160; // 半透明
			}
		}
		else if (r < 2.30f) {
			// 【F環】外側の一際細い環
			if (r > 2.26f && r < 2.27f) {
				r_col = 160; g_col = 155; b_col = 150; a_col = 90;
			} else {
				r_col = 0; g_col = 0; b_col = 0; a_col = 0; // 隙間
			}
		}

#ifdef SATURN_RINGS
		a_col /= 4;
#endif
		// Direct3D11の標準フォーマット（DXGI_FORMAT_R8G8B8A8_UNORM）に合わせてパック
		// バイナリ上の並び：0xAABBGGRR
		pixels[i] = (a_col << 24) | (b_col << 16) | (g_col << 8) | r_col;
	}

	// --- Direct3D 11 テクスチャの構築 ---
	D3D11_TEXTURE1D_DESC desc = {};
	desc.Width = textureSize;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage = D3D11_USAGE_IMMUTABLE; // 変更しないので高速なメモリに配置
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pixels.data();

	HRESULT hr = pDevice->CreateTexture1D(&desc, &initData, &pRingTexture);
	if (FAILED(hr)) return hr;

	// シェーダーリソースビュー (SRV) の作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srvDesc.Texture1D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(pRingTexture, &srvDesc, &pRingSRV);
	return hr;
}
void CreateRing(float innerRadius, float outerRadius, int segments, vector<RingVertex>& vertices, vector<UINT>& indices) {
	// シンプルな円環（リング）メッシュの生成例
	// vector<RingVertex> RingVertexs;
	// vector<UINT> RingIndices;
	// CreateRing(1.5f, 2.3f, 128, 0x80808080, RingVertexs, RingIndexs);

	for (int i = 0; i <= segments; ++i) {
		float angle = (float)i / segments * XM_2PI;
		float cosA = cosf(angle);
		float sinA = sinf(angle);

		// 内側の頂点 (XZ平面上に平らに配置)
		vertices.push_back({ XMFLOAT3(innerRadius * cosA, 0.0f, innerRadius * sinA), XMFLOAT2(0.0f, (float)i / segments) });
		// 外側の頂点
		vertices.push_back({ XMFLOAT3(outerRadius * cosA, 0.0f, outerRadius * sinA), XMFLOAT2(1.0f, (float)i / segments) });
	}

	// 2. インデックスバッファの生成（D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST用）
	// 1ステップずつ（i++）隣の分割位置と結んでいく
	for (int i = 0; i < segments; ++i) {
		// 現在の分割位置の「内・外」の頂点番号
		UINT currentInner = (i * 2);
		UINT currentOuter = (i * 2 + 1);

		// 次の分割位置の「内・外」の頂点番号
		UINT nextInner    = ((i + 1) * 2);
		UINT nextOuter    = ((i + 1) * 2 + 1);

		// 三角形1: [現在の内] -> [次の内] -> [現在の外] (反時計回り = 上向き面)
		indices.push_back(currentInner);
		indices.push_back(nextInner);
		indices.push_back(currentOuter);

		// 三角形2: [現在の外] -> [次の内] -> [次の外] (反時計回り = 上向き面)
		indices.push_back(currentOuter);
		indices.push_back(nextInner);
		indices.push_back(nextOuter);
	}
}
void CreateSphere(float radius, UINT slices, UINT stacks, std::vector<PlanetVertex>& vertices, std::vector<UINT>& indices) {
	vertices.clear();
	indices.clear();
	vertices.reserve((slices + 1) * (stacks + 1));
	indices.reserve(6 * slices * stacks);

	for (UINT i = 0; i <= stacks; ++i) {
		float phi = XM_PI * (float)i / stacks;

		// z を「極（てっぺんと底）」にする
		float z = radius * cosf(phi); 
		float r = radius * sinf(phi);

		for (UINT j = 0; j <= slices; ++j) {
			float theta = 2.0f * XM_PI * (float)j / slices;

			// 残りの円周を X と Y に割り当てる（XY平面が赤道になる）
			float x = r * cosf(theta);
			float y = r * sinf(theta);

			PlanetVertex v;
			v.Pos = { x, y, z };

			// 法線の計算
			XMStoreFloat3(&v.Normal, XMVector3Normalize(XMLoadFloat3(&v.Pos)));

			// テクスチャUVでは、-Zが北極、+Zが南極で巻き付きます）
			v.Tex = XMFLOAT2((float)j / (float)slices, (float)(stacks - i) / (float)stacks);

			vertices.push_back(v);
		}
	}

	// インデックス生成は元のままで完全に正しいので変更不要です
	for (UINT i = 0; i < stacks; ++i) {
		for (UINT j = 0; j < slices; ++j) {
			UINT first = i * (slices + 1) + j;
			UINT second = first + slices + 1;

			indices.push_back(first);
			indices.push_back(second);
			indices.push_back(first + 1);

			indices.push_back(second);
			indices.push_back(second + 1);
			indices.push_back(first + 1);
		}
	}
}
HRESULT InitD3DAndSwapChain(HWND hWnd) {
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(hWnd, &rc);
	ScreenWidth  = rc.right - rc.left;
	ScreenHeight = rc.bottom - rc.top;

	if (ScreenWidth <= 0)
		ScreenWidth = 10;
	if (ScreenHeight <= 0)
		ScreenHeight = 10;

	// 1. デバイスとコンテキストの作成
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	hr = D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG, // Direct2D共存に必須
		featureLevels, 1, D3D11_SDK_VERSION,
		&pD3DDevice, nullptr, &pD3DContext
	);
	if (FAILED(hr)) return hr;

	// 1. D3DデバイスからDXGIデバイスを取得
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDxgiDevice);
	if (FAILED(hr)) return hr;

	if (pD2DFactory == nullptr) {
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
		if (FAILED(hr))	return hr;
	}

	// 2. Direct2Dデバイスの作成
	hr = pD2DFactory->CreateDevice(pDxgiDevice, &pD2DDevice);
	if (FAILED(hr)) return hr;

	IDXGIAdapter* pAdapter = nullptr;
	hr = pDxgiDevice->GetAdapter(&pAdapter);
	if (FAILED(hr)) return hr;

	IDXGIFactory2* pDXGIFactory = nullptr;
	hr = pAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pDXGIFactory);
	if (FAILED(hr)) return hr;

	// 3. スワップチェーンの設定
	DXGI_SWAP_CHAIN_DESC1 sd = { 0 };
	sd.Width  = ScreenWidth;
	sd.Height = ScreenHeight;
	sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Direct2Dと共通
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	hr = pDXGIFactory->CreateSwapChainForHwnd(pD3DDevice, hWnd, &sd, nullptr, nullptr, &pSwapChain);
	if (FAILED(hr)) return hr;

	// 4. バックバッファの作成
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	hr = pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pBackBufferRT);
	if (FAILED(hr)) return hr;

#ifdef _DEBUG
	ID3DBlob* pBlob = nullptr;

	////////////////////////////////////////////
	// 小惑星の頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"VS_Asteroid", // ここがエントリポイント名（関数名と一致させる）
		"vs_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pAsteroidVS);
	if (FAILED(hr)) return hr;

	// 入力レイアウトの作成 (C++のVertex構造体とHLSLを繋ぐ)
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SPOSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = pD3DDevice->CreateInputLayout(layoutDesc, 3, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	// Shadow頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"VS_AsteroidShadow", // ここがエントリポイント名（関数名と一致させる）
		"vs_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pAsteroidShadowVS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// 小惑星のピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"PS_Asteroid", // ここがエントリポイント名
		"ps_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pAsteroidPS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	// Shadow小惑星のピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"PS_AsteroidShadow", // ここがエントリポイント名
		"ps_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pAsteroidShadowPS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// 惑星の頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"VS_Planet", // ここがエントリポイント名（関数名と一致させる）
		"vs_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPlanetVS);
	if (FAILED(hr)) return hr;

	// 入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC planetLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// 入力レイアウトの作成 (C++のVertex構造体とHLSLを繋ぐ)
	hr = pD3DDevice->CreateInputLayout(planetLayoutDesc, 3, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pPlanetInputLayout);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// 単色の惑星のピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"PS_Simple", // ここがエントリポイント名
		"ps_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pSimplePS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// テクスチャ・影付きの惑星ピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"PS_Planet", // ここがエントリポイント名
		"ps_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPlanetPS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// リムライト・テクスチャ・影付きの惑星ピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"PS_Shadow", // ここがエントリポイント名
		"ps_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShadowPS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// 土星の環の頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"VS_Ring", // ここがエントリポイント名（関数名と一致させる）
		"vs_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pRingVS);
	if (FAILED(hr)) return hr;

	// 入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC ringLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// 入力レイアウトの作成 (C++のVertex構造体とHLSLを繋ぐ)
	hr = pD3DDevice->CreateInputLayout(ringLayoutDesc, 2, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pRingInputLayout);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// 土星の環のピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"PS_Ring", // ここがエントリポイント名
		"ps_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pRingPS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// 土星の環の影のピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"PS_RingShadow", // ここがエントリポイント名
		"ps_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pRingShadowPS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// ラベルの頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"VS_Label", // ここがエントリポイント名（関数名と一致させる）
		"vs_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pLabelVS);
	if (FAILED(hr)) return hr;

	// 入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC labelLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// 入力レイアウトの作成 (C++のVertex構造体とHLSLを繋ぐ)
	hr = pD3DDevice->CreateInputLayout(labelLayoutDesc, 2, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pLabelInputLayout);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// ラベルのピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"PS_Label", // ここがエントリポイント名
		"ps_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pLabelPS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// デバッグ頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"VS_Debug", // ここがエントリポイント名（関数名と一致させる）
		"vs_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pDebugVS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

	////////////////////////////////////////////
	// デバッグピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"planet.hlsl", nullptr, nullptr, 
		"PS_Debug", // ここがエントリポイント名
		"ps_5_0",  // シェーダーモデル
		0, 0, &pBlob, nullptr);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pDebugPS);
	if (FAILED(hr)) return hr;
	pBlob->Release();

#else	// !_DEBUG
	////////////////////////////////////////////
	// コンパイル済み小惑星の頂点・ピクセルシェーダーの読み込み
	#include "vs_shader.h"		// fxc.exe /T vs_5_0 /E VS_Asteroid /Vn VS_Asteroid /Fh vs_shader.h planet.hlsl
	#include "ps_shader.h"		// fxc.exe /T ps_5_0 /E PS_Asteroid /Vn PS_Asteroid /Fh ps_shader.h planet.hlsl

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(VS_Asteroid, sizeof(VS_Asteroid), nullptr, &pAsteroidVS);
	if (FAILED(hr)) return hr;

	// 入力レイアウトの作成 (C++のVertex構造体とHLSLを繋ぐ)
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SPOSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = pD3DDevice->CreateInputLayout(layoutDesc, 3, VS_Asteroid, sizeof(VS_Asteroid), &pInputLayout);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(PS_Asteroid, sizeof(PS_Asteroid), nullptr, &pAsteroidPS);
	if (FAILED(hr)) return hr;

	////////////////////////////////////////////
	// コンパイル済み小惑星の影付頂点・ピクセルシェーダーの読み込み
	#include "vs_asteroid.h"	// fxc.exe /T vs_5_0 /E VS_AsteroidShadow /Vn VS_AsteroidShadow /Fh vs_asteroid.h planet.hlsl
	#include "ps_asteroid.h"	// fxc.exe /T ps_5_0 /E PS_AsteroidShadow /Vn PS_AsteroidShadow /Fh ps_asteroid.h planet.hlsl

	// Shadow頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(VS_AsteroidShadow, sizeof(VS_AsteroidShadow), nullptr, &pAsteroidShadowVS);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(PS_AsteroidShadow, sizeof(PS_AsteroidShadow), nullptr, &pAsteroidShadowPS);
	if (FAILED(hr)) return hr;

	////////////////////////////////////////////
	// コンパイル済み惑星の頂点・ピクセルシェーダーの読み込み
	#include "vs_planet.h"		// fxc.exe /T vs_5_0 /E VS_Planet /Vn VS_Planet /Fh vs_planet.h planet.hlsl
	#include "ps_planet.h"		// fxc.exe /T ps_5_0 /E PS_Planet /Vn PS_Planet /Fh ps_planet.h planet.hlsl
	#include "ps_shadow.h"		// fxc.exe /T ps_5_0 /E PS_Shadow /Vn PS_Shadow /Fh ps_shadow.h planet.hlsl
	#include "ps_simple.h"		// fxc.exe /T ps_5_0 /E PS_Simple /Vn PS_Simple /Fh ps_simple.h planet.hlsl

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(VS_Planet, sizeof(VS_Planet), nullptr, &pPlanetVS);
	if (FAILED(hr)) return hr;

	// 入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC planetLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// vsBlobは球体用シェーダーをコンパイルした際のものを使用
	hr = pD3DDevice->CreateInputLayout(planetLayoutDesc, 3, VS_Planet, sizeof(VS_Planet), &pPlanetInputLayout);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(PS_Planet, sizeof(PS_Planet), nullptr, &pPlanetPS);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(PS_Shadow, sizeof(PS_Shadow), nullptr, &pShadowPS);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(PS_Simple, sizeof(PS_Simple), nullptr, &pSimplePS);
	if (FAILED(hr)) return hr;

	////////////////////////////////////////////
	// コンパイル済み木星の環の頂点・ピクセルシェーダーの読み込み
	#include "vs_ring.h"		// fxc.exe /T vs_5_0 /E VS_Ring /Vn VS_Ring /Fh vs_ring.h planet.hlsl
	#include "ps_ring.h"		// fxc.exe /T ps_5_0 /E PS_Ring /Vn PS_Ring /Fh ps_ring.h planet.hlsl
	#include "ps_ringshadow.h"	// fxc.exe /T ps_5_0 /E PS_RingShadow /Vn PS_RingShadow /Fh ps_ringshadow.h planet.hlsl

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(VS_Ring, sizeof(VS_Ring), nullptr, &pRingVS);
	if (FAILED(hr)) return hr;

	// 入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC ringLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// vsBlobは球体用シェーダーをコンパイルした際のものを使用
	hr = pD3DDevice->CreateInputLayout(ringLayoutDesc, 2, VS_Ring, sizeof(VS_Ring), &pRingInputLayout);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(PS_Ring, sizeof(PS_Ring), nullptr, &pRingPS);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(PS_RingShadow, sizeof(PS_RingShadow), nullptr, &pRingShadowPS);
	if (FAILED(hr)) return hr;

	////////////////////////////////////////////
	// コンパイル済みラベルの頂点・ピクセルシェーダーの読み込み
	#include "vs_label.h"		// fxc.exe /T vs_5_0 /E VS_Label /Vn VS_Label /Fh vs_label.h planet.hlsl
	#include "ps_label.h"		// fxc.exe /T ps_5_0 /E PS_Label /Vn PS_Label /Fh ps_label.h planet.hlsl

	// 頂点シェーダーの作成
	hr = pD3DDevice->CreateVertexShader(VS_Label, sizeof(VS_Label), nullptr, &pLabelVS);
	if (FAILED(hr)) return hr;

	// 入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC labelLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// vsBlobは球体用シェーダーをコンパイルした際のものを使用
	hr = pD3DDevice->CreateInputLayout(labelLayoutDesc, 2, VS_Label, sizeof(VS_Label), &pLabelInputLayout);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーの作成
	hr = pD3DDevice->CreatePixelShader(PS_Label, sizeof(PS_Label), nullptr, &pLabelPS);
	if (FAILED(hr)) return hr;

#endif	// _DEBUG
	////////////////////////////////////////////
	// 球体のモデリング
	vector<PlanetVertex> vertices;
	vector<UINT> indices;
	CreateSphere(1.0f, 32, 32, vertices, indices); // 半径1.0の「単位球」を作る
	sphereIndexCount = (UINT)indices.size();

	// 頂点バッファ作成
	D3D11_BUFFER_DESC vbd = { sizeof(PlanetVertex) * (UINT)vertices.size(), D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	D3D11_SUBRESOURCE_DATA vsd = { vertices.data() };
	hr = pD3DDevice->CreateBuffer(&vbd, &vsd, &pPlanetVB);
	if (FAILED(hr)) return hr;

	// インデックスバッファ作成
	D3D11_BUFFER_DESC ibd = { sizeof(UINT) * sphereIndexCount, D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
	D3D11_SUBRESOURCE_DATA isd = { indices.data() };
	hr = pD3DDevice->CreateBuffer(&ibd, &isd, &pPlanetIB);
	if (FAILED(hr)) return hr;

	////////////////////////////////////////////
	// 土星の環のモデリング
	vector<RingVertex> ringVertices;
	vector<UINT> ringIndices;
	CreateRing(1.5f, 2.3f, 128, ringVertices, ringIndices);
	ringIndexCount = (UINT)ringIndices.size();

	// 頂点バッファ作成
	vbd = { (UINT)sizeof(RingVertex) * (UINT)ringVertices.size(), D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	vsd = { ringVertices.data() };
	hr = pD3DDevice->CreateBuffer(&vbd, &vsd, &pRingVB);
	if (FAILED(hr)) return hr;

	// インデックスバッファ作成
	ibd = { (UINT)sizeof(UINT) * (UINT)ringIndexCount, D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
	isd = { ringIndices.data() };
	hr = pD3DDevice->CreateBuffer(&ibd, &isd, &pRingIB);
	if (FAILED(hr)) return hr;

	// 土星の環の1Dテクスチャの作成
	hr = CreateSaturnRingTexture(pD3DDevice);
	if (FAILED(hr)) return hr;

	////////////////////////////////////////////
	// 天王星の環のモデリング
	ringVertices.clear();
	ringIndices.clear();
	CreateRing(1.2f, 4.36f, 128, ringVertices, ringIndices);
	UranusRingIndexCount = (UINT)ringIndices.size();

	// 頂点バッファ作成
	vbd = { (UINT)sizeof(RingVertex) * (UINT)ringVertices.size(), D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	vsd = { ringVertices.data() };
	hr = pD3DDevice->CreateBuffer(&vbd, &vsd, &pUranusRingVB);
	if (FAILED(hr)) return hr;

	// インデックスバッファ作成
	ibd = { (UINT)sizeof(UINT) * (UINT)UranusRingIndexCount, D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
	isd = { ringIndices.data() };
	hr = pD3DDevice->CreateBuffer(&ibd, &isd, &pUranusRingIB);
	if (FAILED(hr)) return hr;

	// 天王星の環のテクスチャの作成
	hr = CreateUranusRingTexture(pD3DDevice);
	if (FAILED(hr)) return hr;

	////////////////////////////////////////////
	// ラベルのモデリング

	// DirectWrite用のテクスチャに描画するレンダーターゲットの作成
	if (pD2DFactory == nullptr) {
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
		if (FAILED(hr))	return hr;
	}

	// デバイスコンテキスト（新しいレンダーターゲット）の作成
	hr = pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pLabelRT);
	if (FAILED(hr)) return hr;

	// 頂点バッファ作成
	vbd = { (UINT)sizeof(RingVertex) * 4, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
	hr = pD3DDevice->CreateBuffer(&vbd, nullptr, &pLabelVB);
	if (FAILED(hr)) return hr;

	// インデックスバッファ作成
	UINT labelIndex[] = { 3, 2, 0, 0, 2, 1 };
	ibd = { (UINT)sizeof(labelIndex), D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
	isd = { labelIndex };
	hr = pD3DDevice->CreateBuffer(&ibd, &isd, &pLabelIB);
	if (FAILED(hr)) return hr;

	////////////////////////////////////////////
	// 定数バッファの作成
	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(PlanetConstantBuffer); // 16の倍数であること
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	hr = pD3DDevice->CreateBuffer(&cbd, nullptr, &pConstantBuffer);
	if (FAILED(hr)) return hr;

	// アルファ・ブレンディングの設定
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
//	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;		// 重なり方を変更
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	// アルファ値自体の合成設定
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = pD3DDevice->CreateBlendState(&blendDesc, &pBlendState);
	if (FAILED(hr)) return hr;

	// 惑星名ラベル専用のブレンドステートを作成
	// すでにRGBにアルファが乗算されているため、ソースのブレンド係数は「1 (ONE)」にする
	blendDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE; 
	blendDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	// アルファチャンネル自体の合成規則
	blendDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	hr = pD3DDevice->CreateBlendState(&blendDesc, &pLabelBlendState);
	if (FAILED(hr)) return hr;

	// 1. 深度バッファ（テクスチャ）の設定
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = ScreenWidth;
	depthDesc.Height = ScreenHeight;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24bit深度、8bitステンシル
	depthDesc.SampleDesc.Count = 1;                   // マルチサンプルなし
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;   // 深度ステンシルとして使う
	hr = pD3DDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthStencilBuffer);
	if (FAILED(hr)) return hr;

	// 2. 深度ステンシルビューの設定
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = depthDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = pD3DDevice->CreateDepthStencilView(pDepthStencilBuffer, &dsvDesc, &pDepthStencilView);
	if (FAILED(hr)) return hr;

	// 深度テスト初期化
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS; // 手前のものほど合格

	// 惑星用：深度テストON、書き込みON
	hr = pD3DDevice->CreateDepthStencilState(&dsDesc, &pDSState_Normal);
	if (FAILED(hr)) return hr;

	// リング用：深度テストON、書き込みOFF
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = pD3DDevice->CreateDepthStencilState(&dsDesc, &pDSState_NoWrite);
	if (FAILED(hr)) return hr;

	// テスト自体をしない
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 書き込まない
	hr = pD3DDevice->CreateDepthStencilState(&dsDesc, &pDSState_None);
	if (FAILED(hr)) return hr;

	// シャドウマップの作成
	D3D11_TEXTURE2D_DESC shadowMapDesc = {};
	shadowMapDesc.Width = ScreenWidth;
	shadowMapDesc.Height = ScreenHeight;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	hr = pD3DDevice->CreateTexture2D(&shadowMapDesc, nullptr, &pShadowMapBuffer);
	if (FAILED(hr)) return hr;

	// シャドウマップステンシルビューの設定
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = pD3DDevice->CreateDepthStencilView(pShadowMapBuffer, &depthStencilViewDesc, &pShadowDepthView);
	if (FAILED(hr)) return hr;

	// シャドウマップリソースビューの設定
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = pD3DDevice->CreateShaderResourceView(pShadowMapBuffer, &shaderResourceViewDesc, &pShadowResourceView);
	if (FAILED(hr)) return hr;

	// サンプラーの設定
	D3D11_SAMPLER_DESC comparisonSamplerDesc = {};
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.0f;
	comparisonSamplerDesc.BorderColor[1] = 1.0f;
	comparisonSamplerDesc.BorderColor[2] = 1.0f;
	comparisonSamplerDesc.BorderColor[3] = 1.0f;
	comparisonSamplerDesc.MinLOD = 0.f;
	comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	comparisonSamplerDesc.MipLODBias = 0.f;
	comparisonSamplerDesc.MaxAnisotropy = 0;

	// 比較関数ShadowMap.SampleCmpLevelZero(...)を使用する場合
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	hr = pD3DDevice->CreateSamplerState(&comparisonSamplerDesc,	&pSamplerState);
	if (FAILED(hr)) return hr;

	// 比較関数は使用しないため NEVER にするか、記述自体を省略
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	comparisonSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// D3D11_FILTER_MIN_MAG_MIP_POINT;
	hr = pD3DDevice->CreateSamplerState(&comparisonSamplerDesc,	&pSamplerStateLinear);
	if (FAILED(hr)) return hr;

	// 2Dサンプラーの設定
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	comparisonSamplerDesc.BorderColor[0] = 0.0f;
	comparisonSamplerDesc.BorderColor[1] = 0.0f;
	comparisonSamplerDesc.BorderColor[2] = 0.0f;
	comparisonSamplerDesc.BorderColor[3] = 0.0f;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	comparisonSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	hr = pD3DDevice->CreateSamplerState(&comparisonSamplerDesc,	&pSamplerState2DLinear);
	if (FAILED(hr)) return hr;

	// ラスタライザーの設定
	D3D11_RASTERIZER_DESC RenderStateDesc = {};
	RenderStateDesc.CullMode = D3D11_CULL_BACK;
	RenderStateDesc.FillMode = D3D11_FILL_SOLID;
	RenderStateDesc.DepthClipEnable = true; // Feature level 9_1 requires DepthClipEnable == true
	RenderStateDesc.MultisampleEnable = FALSE;    // これが FALSE である必要があります
	RenderStateDesc.AntialiasedLineEnable = TRUE; // 線用アンチエイリアスを有効化
	hr = pD3DDevice->CreateRasterizerState(&RenderStateDesc, &pDrawingRenderState);
	if (FAILED(hr)) return hr;

	D3D11_RASTERIZER_DESC shadowRenderStateDesc = {};
	RenderStateDesc.CullMode = D3D11_CULL_FRONT;
	hr = pD3DDevice->CreateRasterizerState(&RenderStateDesc, &pShadowRenderState);
	if (FAILED(hr)) return hr;

	RenderStateDesc.CullMode = D3D11_CULL_NONE;
	hr = pD3DDevice->CreateRasterizerState(&RenderStateDesc, &pNomalRenderState);
	if (FAILED(hr)) return hr;

	// 後片付け
	if (pBackBuffer) pBackBuffer->Release();
	if (pDXGIFactory) pDXGIFactory->Release();
	if (pAdapter) pAdapter->Release();

	return hr;
}
void InitVertexResources(int numBodies) {
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * numBodies;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	if (CudaDeviceCount <= 0) {
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	// Direct3D側にバッファを作成
	pD3DDevice->CreateBuffer(&bd, nullptr, &pVertexBuffer);

	// ★重要：このバッファをCUDAで扱えるように登録する
	if (CudaDeviceCount > 0)
		cudaGraphicsD3D11RegisterResource(&cudaVBResource, pVertexBuffer, cudaGraphicsRegisterFlagsNone);
}
void ClearDirect3D()
{
	for (auto& b : bodies) {
		if (b.vertex != nullptr) {
			b.vertex->Release();
			b.vertex = nullptr;
		}
		if (b.pLabelBitmap) {
			b.pLabelBitmap->Release();
			b.pLabelBitmap = nullptr;
		}
		if (b.pLabelSRV) {
			b.pLabelSRV->Release();
			b.pLabelSRV = nullptr;
		}
		if (b.pLabelTexture) {
			b.pLabelTexture->Release();
			b.pLabelTexture = nullptr;
		}
		if (b.pPlanetTexture) {
			b.pPlanetTexture->Release();
			b.pPlanetTexture = nullptr;
		}
		if (b.pPlanetSRV) {
			b.pPlanetSRV->Release();
			b.pPlanetSRV = nullptr;
		}
	}
	InitTexture = 0;

	if(cudaVBResource) cudaGraphicsUnregisterResource(cudaVBResource);

	if (pLabelRT) pLabelRT->Release();
	if (pLabelVB) pLabelVB->Release();
	if (pLabelIB) pLabelIB->Release();

	if (pLabelVS) pLabelVS->Release();
	if (pLabelPS) pLabelPS->Release();
	if (pLabelInputLayout) pLabelInputLayout->Release();

	if (pUranusRingTexture) pUranusRingTexture->Release();
	if (pUranusRingSRV) pUranusRingSRV->Release();

	if (pUranusRingVB) pUranusRingVB->Release();
	if (pUranusRingIB) pUranusRingIB->Release();

	if (pRingTexture) pRingTexture->Release();
	if (pRingSRV) pRingSRV->Release();

	if (pRingVB) pRingVB->Release();
	if (pRingIB) pRingIB->Release();

	if (pRingShadowPS) pRingShadowPS->Release();
	if (pRingPS) pRingPS->Release();
	if (pRingVS) pRingVS->Release();
	if (pRingInputLayout) pRingInputLayout->Release();

	if (pNomalRenderState) pNomalRenderState->Release();
	if (pShadowRenderState) pShadowRenderState->Release();
	if (pDrawingRenderState) pDrawingRenderState->Release();
	if (pSamplerState) pSamplerState->Release();
	if (pSamplerStateLinear) pSamplerStateLinear->Release();
	if (pSamplerState2DLinear) pSamplerState2DLinear->Release();
	if (pShadowResourceView) pShadowResourceView->Release();
	if (pShadowDepthView) pShadowDepthView->Release();
	if (pShadowMapBuffer) pShadowMapBuffer->Release();

	if (pDSState_None) pDSState_None->Release();
	if (pDSState_NoWrite) pDSState_NoWrite->Release();
	if (pDSState_Normal) pDSState_Normal->Release();
	if (pDepthStencilBuffer) pDepthStencilBuffer->Release();
	if (pDepthStencilView) pDepthStencilView->Release();

	if (pBlendState) pBlendState->Release();
	if (pLabelBlendState) pLabelBlendState->Release();

	if (pDebugVS) pDebugVS->Release();
	if (pDebugPS) pDebugPS->Release();

	if (pConstantBuffer) pConstantBuffer->Release();
	if (pPlanetIB) pPlanetIB->Release();
	if (pPlanetVB) pPlanetVB->Release();

	if (pSimplePS) pSimplePS->Release();
	if (pShadowPS) pShadowPS->Release();
	if (pPlanetPS) pPlanetPS->Release();
	if (pPlanetVS) pPlanetVS->Release();
	if (pPlanetInputLayout) pPlanetInputLayout->Release();

	if (pVertexBuffer) pVertexBuffer->Release();
	if (pInputLayout) pInputLayout->Release();
	if (pAsteroidShadowVS) pAsteroidShadowVS->Release();
	if (pAsteroidShadowPS) pAsteroidShadowPS->Release();
	if (pAsteroidVS) pAsteroidVS->Release();
	if (pAsteroidPS) pAsteroidPS->Release();

	if (pBackBufferRT) pBackBufferRT->Release();
	if (pSwapChain) pSwapChain->Release();

	if (pD2DDevice) pD2DDevice->Release();
	if (pDxgiDevice) pDxgiDevice->Release();
	if (pD3DContext) pD3DContext->Release();
	if (pD3DDevice) pD3DDevice->Release();
}
#endif	// DIRECT3D_SWAP
#endif	// DIRECT2D_VIEW

// --- 物理演算ロジック ---

void BodyReserve(size_t size)
{
	size += bodies.size();

	posV.reserve(size);
	velV.reserve(size);
	massV.reserve(size);
	radiusV.reserve(size);
	colorV.reserve(size);
	bodies.reserve(size);
}
void BodyPushBask(wstring name, double mass, Vector3D pos, Vector3D vel, double radius, Color color, int type, int base, double obliquity = 0.0, double rotation = 0.0, double obli_rot = 0.0, double period = 0.0)
{
	size_t i = bodies.size();

	posV.push_back(pos);
	velV.push_back(vel);
	massV.push_back(mass);
	radiusV.push_back(radius);
	colorV.push_back(ColorToRGBA(color));

#ifdef	DIRECT3D_SWAP
	bodies.push_back({ name, i, type, color, base, obliquity, rotation, obli_rot, period, 0, nullptr });
#else
	bodies.push_back({ name, i, type, color, base, obliquity, rotation, obli_rot, period, 0 });
#endif	// DIRECT3D_SWAP
}

void AddAsteroidBelt(int count) {

	BodyReserve(count);

	for (int i = 0; i < count; i++) {
		// 2.1au ～ 3.3au の間にランダムに距離を設定
		double dist = (2.1 + (double)rand() / RAND_MAX * 1.2) * AU;
		double angle = (double)rand() / RAND_MAX * 2.0 * M_PI;

		// 円軌道の速度 v = sqrt(G * M_sun / r)
		double speed = sqrt(G * 1.9885e30 / dist);

		Vector3D pos = { cos(angle) * dist, sin(angle) * dist, ((double)rand()/RAND_MAX - 0.5) * 0.1 * AU };
		Vector3D vel = { -sin(angle) * speed, cos(angle) * speed, 0 };

		// 名前を "Asteroid" にして質量を極小(1.0)にする
		BodyPushBask(L"Asteroid", 0.0, pos, vel, 1.0e3, Color(100, 200, 200, 200), TYPE_ASTEROID, 0);
	}
}
void AddTrojanAsteroids(int countPerGroup) {
	// 木星を探す
	int jupiterIdx = -1;
	for (int i = 0; i < (int)bodies.size(); i++) {
		if (bodies[i].name == L"Jupiter") {
			jupiterIdx = i;
			break;
		}
	}
	if (jupiterIdx == -1) return;

	// 木星の現在の位相（角度）を算出
	double jupiterAngle = atan2(posV[bodies[jupiterIdx].i].y, posV[bodies[jupiterIdx].i].x);
	double jupiterDist  = posV[bodies[jupiterIdx].i].length();
	double jupiterSpeed = velV[bodies[jupiterIdx].i].length();

	// L4 (前方60度) と L5 (後方60度) の相対角度
	double offsets[] = { M_PI / 3.0, -M_PI / 3.0 };

	BodyReserve(countPerGroup * 2);

	for (double offset : offsets) {
		for (int i = 0; i < countPerGroup; i++) {
			// ラグランジュ点付近に少し散らす (±10度、距離±5%)
			double angle = jupiterAngle + offset + ((double)rand() / RAND_MAX - 0.5) * (M_PI / 18.0);
			double dist = jupiterDist * (0.95 + (double)rand() / RAND_MAX * 0.1);

			// 速度は木星の公転速度をベースに調整
			double speed = jupiterSpeed * sqrt(jupiterDist / dist);

			Vector3D pos = { cos(angle) * dist, sin(angle) * dist, ((double)rand() / RAND_MAX - 0.5) * 0.05 * AU };
			Vector3D vel = { -sin(angle) * speed, cos(angle) * speed, 0 };

			// 質量 1.0 (実質ゼロ) にすることで惑星への影響を排除
			BodyPushBask( L"Trojan", 0.0, pos, vel, 1.0e3, Color(30, 200, 200, 200), TYPE_ASTEROID, 0);
		}
	}
}
void AddSaturnRings(int particleCount) {
	int saturnIdx = -1;
	for (int i = 0; i < (int)bodies.size(); i++) {
		if (bodies[i].name == L"Saturn") {
			saturnIdx = i;
			break;
		}
	}
	if (saturnIdx == -1) return;

	BodyReserve(particleCount);

	BYTE alpha = particleCount >= 1000000 ? 10 : (particleCount >= 100000 ? 30 : (particleCount >= 10000 ? 90 : 120));

	for (int i = 0; i < particleCount; i++) {
		double dist = (1.5 + (double)rand() / RAND_MAX * 0.8) * radiusV[bodies[saturnIdx].i]; 
		double angle = (double)rand() / RAND_MAX * 2.0 * M_PI;
		double speed = sqrt(G * massV[bodies[saturnIdx].i] / dist);

		Vector3D pos = { cos(angle) * dist, sin(angle) * dist, 0 };
		Vector3D vel = { -sin(angle) * speed, cos(angle) * speed, 0 };

		pos = pos.rotateY(bodies[saturnIdx].obliquity * M_PI / 180.0); 
		vel = vel.rotateY(bodies[saturnIdx].obliquity * M_PI / 180.0);

		pos = pos.rotateZ(bodies[saturnIdx].obli_rot * M_PI / 180.0); 
		vel = vel.rotateZ(bodies[saturnIdx].obli_rot * M_PI / 180.0);

		BodyPushBask( L"RingParticle", 0.0, posV[bodies[saturnIdx].i] + pos, velV[bodies[saturnIdx].i] + vel, 1.0e3, Color(alpha, 200, 200, 150), TYPE_ASTEROID, saturnIdx);
	}
}
void AddRocket(wstring base, wstring name, double mass, double ofs, double speed, double radius, double z, double a, double q, Color color) {
	for (auto& b : bodies) if (b.name.find(base) != wstring::npos) {
		z = z * M_PI / 180.0;
		a = a * M_PI / 180.0;
		q = -q * M_PI / 180.0;

		// z=天頂角 a=方位角 q=打上角度(z=0)
		Vector3D pos = { sin(z) * cos(a), sin(z) * sin(a), cos(z) };
		Vector3D vel = pos.rotateZ(q);

		BodyPushBask(name, mass, posV[b.i] + pos * ofs, velV[b.i] + vel * speed, radius, color, TYPE_ROKCET, (int)b.i);
		break;
	}
}
void AddBooster(double time, double z, double a, double speed) {
	bodies[bodies.size() - 1].boost.push_back({ time, (-1), z, a, speed });
}
void AddTarget(double time, wstring target, double z, double a, double speed) {
	for (int n = 0; n < (int)bodies.size(); n++) {
		if (bodies[n].name.find(target) != wstring::npos) {
			bodies[bodies.size() - 1].boost.push_back({ time, n, z, a, speed });
			break;
		}
	}
}
void AddSate(wstring base, wstring name, double mass, double dist, double speed, double a, double i, double radius, Color color, int type = TYPE_SATELITE) {
	for (auto& b : bodies) if (b.name.find(base) != wstring::npos) {

		// 軌道傾斜角は、主星の赤道傾斜角を基準に黄道からの軌道傾斜角にする
		// 衛星の赤道傾斜角は,0度で黄道からの軌道傾斜角にする
		double obliquity = b.obliquity + i;
		double obli_rot = b.obli_rot + a;

		a = a * M_PI / 180.0;	// 方位角		rotateZ(a)
		i = i * M_PI / 180.0;	// 軌道傾斜角	rotateY(i)
		Vector3D rp = { dist * cos(a) * cos(i), dist * sin(a), -dist * cos(a) * sin(i) };
		Vector3D rv = { -speed * sin(a) * cos(i), speed * cos(a), speed * sin(a) * sin(i) };

		rp = rp.rotateY(b.obliquity * M_PI / 180.0); 
		rv = rv.rotateY(b.obliquity * M_PI / 180.0);

		rp = rp.rotateZ(b.obli_rot * M_PI / 180.0); 
		rv = rv.rotateZ(b.obli_rot * M_PI / 180.0);

		// 惑星に自身と衛星のインデックスを登録
		if (b.satelite.empty()) b.satelite.push_back(b.i);
		b.satelite.push_back(bodies.size());

		// 自転時間は、公転時間と同じ潮汐ロック
		double rotation = (2.0 * M_PI * dist) / speed / (24.0 * 60.0 * 60.0);	// 2 * 公転半径(m) * PI =　距離 / 速度(m/s) = 時間(s) / (24 * 60 * 60) = 日

		BodyPushBask(name, mass, posV[b.i] + rp, velV[b.i] + rv, radius, color, type, (int)b.i, obliquity, rotation, obli_rot, rotation);
		break;
	}
}
void AddBody(wstring name, double mass, Vector3D pos, Vector3D vel, double radius, Color color, double obliquity = 0.0, double rotation = 0.0, double obli_rot = 0.0) {

	//Ecliptic at the standard reference epoch
	// 
	//	Reference epoch: J2000.0
	//	X-Y plane: adopted Earth orbital plane at the reference epoch
	//	Note: IAU76 obliquity of 84381.448 arcseconds wrt ICRF X-Y plane
	//	X-axis   : ICRF
	//	Z-axis   : perpendicular to the X-Y plane in the directional (+ or -) sense of Earth's north pole at the reference epoch.

	// 公転時間を計算
	double period = (2.0 * M_PI * pos.length()) / vel.length() / (24.0 * 60.0 * 60.0);	// 2 * 公転半径(m) * PI =　距離 / 速度(m/s) = 時間(s) / (24 * 60 * 60) = 日

	// NASAのデータをDirect3Dの左手座標に合わせる為に-zとし距離の単位をkmからm(*1.0e3)にした

	BodyPushBask(name, mass, { pos.x * 1.0e3, pos.y * 1.0e3, pos.z * -1.0e3 }, { vel.x * 1.0e3, vel.y * 1.0e3, vel.z * -1.0e3 }, radius, color, TYPE_PLANET, 0, obliquity, rotation, obli_rot, period);
}

void InitSystem() {

	// このプログラムのワールド座標は、Direct3Dに合わせて左手座標とし中心を太陽として
	// XY平面に地球が公転し地球の北極が-Zで距離の単位は、メートルです。

	// NASA Horizons System 2026-Jun-02 00:00:00.0000 TDB
	//                  kg             x, y, z (km)                                                                Vx, Vy, Vz (km/s)														半径(m)			R8G8B8A8		軌道傾斜角+赤道傾斜角(°)	自転周期(day)	公転夏至(°)
	AddBody(L"Sun",     1.9885e30, {  0.000000000000000E+00,  0.000000000000000E+00,  0.000000000000000E+00 }, {  0.000000000000000E+00,  0.000000000000000E+00,  0.000000000000000E+00 },  6.9551e8,		Color::Yellow,		  0.0     +   7.25,			  27.275					);
	AddBody(L"Mercury", 3.3011e23, { -5.043178824874716E+07,  1.829301002423413E+07,  6.120433004324793E+06 }, { -2.667072750366245E+01, -4.371957238776655E+01, -1.126747470911024E+00 },  2.4397e6,		Color::Gray,		 -7.00487 +   0.027,		  58.65						);
	AddBody(L"Venus",   4.8675e24, { -1.012199387670795E+08,  3.584262635743252E+07,  6.332748008583985E+06 }, { -1.185551684519481E+01, -3.317405094941296E+01,  2.282814346104356E-01 },  6.0518e6,		Color::Bisque,		 -3.39471 + 177.36,			-243.0187					);
	AddBody(L"Earth",   5.9722e24, { -4.906126712964878E+07, -1.435453387214010E+08,  9.278623139135540E+03 }, {  2.769280404071511E+01, -9.745085334271074E+00, -2.740082723828863E-05 },	6.3567e6,		Color::DeepSkyBlue,	  0.0     +  23.44,			   0.997271,	-90.701484	);
	AddBody(L"Mars",    6.4171e23, {  2.004588149811580E+08,  6.685506045885199E+07, -3.514339793238159E+06 }, { -6.736319253000222E+00,  2.505571392453404E+01,  6.902633266834197E-01 },  3.3962e6,		Color::Tomato,		 -1.85061 +  25.19,			   1.02595					);
	AddBody(L"Jupiter", 1.8982e27, { -4.104985204530647E+08,  6.723756275890380E+08,  6.391276184205055E+06 }, { -1.131307252205394E+01, -6.203903932227547E+00,  2.788898710368501E-01 },  6.9911e7,		Color::Orange,		 -1.3028  +   3.13,			   0.4135					);
	AddBody(L"Saturn",  5.6834e26, {  1.406102801822374E+09,  1.647539524117132E+08, -5.884049469022842E+07 }, { -1.661681641910072E+00,  9.571745617027879E+00, -1.011479095216030E-01 },  6.0268e7,		Color::Khaki,		 -2.48446 +  25.33,			   0.4264,		180.0		);
	AddBody(L"Uranus",  8.6810e25, {  1.399493380515447E+09,  2.553225021990701E+09, -8.663861299519062E+06 }, { -6.033131912324926E+00,  2.953181080017963E+00,  8.911971352708137E-02 },  2.5559e7,		Color::LightCyan,	 -0.76986 +  97.77,			  -0.7181					);
	AddBody(L"Neptune", 1.0241e26, {  4.466377375936104E+09,  1.493915637914833E+08, -1.060016753449846E+08 }, { -2.275978011375479E-01,  5.461182050842329E+00, -1.080175868093851E-01 },  2.4764e7,		Color::RoyalBlue,	 -1.76917 +  28.32,			   0.6712					);
	AddBody(L"Pluto",   1.3030e22, {  2.937955096655423E+09, -4.412669840620298E+09, -3.774928164391334E+08 }, {  4.658231044779534E+00,  1.800823316293785E+00, -1.523134215050819E+00 },  1.1850e6,		Color::Tan,			-17.089   + 112.78,			  -6.3872					);

	AddBody(L"Ceres",   9.3930e20, {  2.211027986517447E+08,  3.495459045850214E+08, -2.966644148353477E+07 }, { -1.547883230498215E+01,  8.366935256278254E+00,  3.116537613908606E+00 },  4.7300e5,		Color::LightGray,	-10.585   +   4.0,			   0.3781					);
	AddBody(L"Halley",  2.2000e14, { -2.905353615126613E+09,  4.106833397050663E+09, -1.476443505594707E+09 }, {  9.193971372604495E-01,  2.482961378288616E-01,  2.136583687742188E-01 },  7.2200e3,		Color::White,	    162.26    +   0.0										);

#ifdef SATELITE
    //      base        name			質量(kg)	軌道半径(m)		速度(m/s)	方位角(°)	軌道傾斜角(°)	半径(m)
    AddSate(L"Earth",   L"Moon",		7.347e22,	   3.844e8,		  1022.0,	115.0,		 5.10 - 23.44,	1737.9e3,	Color::LightGray);		// 月の軌道傾斜角のみ黄道

	AddSate(L"Mars",    L"Phobos",		1.260e22,	    9376e3,		  2138.1,	115.0,		 1.02,			  11.1e3,	Color::LightGray);
	AddSate(L"Mars",    L"Deimos",		1.800e22,	   23758e3,		  1351.3,	115.0,		 1.79,			   6.2e3,	Color::LightGray);

	AddSate(L"Jupiter", L"Io",			8.900e22,	  421700e3,		 17333.8,	220.0,		 0.04,			1830.0e3,	Color::LightGray);
    AddSate(L"Jupiter", L"Europa",		4.800e22,	  671034e3,		 13741.2,	 45.0,		 0.47,			1560.8e3,	Color::LightGray);
    AddSate(L"Jupiter", L"Ganymede",	1.500e23,	 1070412e3,		 10879.8,	310.0,		 0.20,			2631.2e3,	Color::LightGray);
    AddSate(L"Jupiter", L"Callisto",	1.100e23,	 1882709e3,		  8203.6,	120.0,		 0.28,			2410.3e3,	Color::LightGray);

//#ifdef SATURN_RINGS
	AddSate(L"Saturn",  L"Pan",			5.000e15,	  133584e3,		 16892.8,	180.0,		 0.04,			  14.1e3,	Color::LightGray,	TYPE_SATE_NON);
	AddSate(L"Saturn",  L"Daphnis",		7.700e14,	  136505e3,		 16709.3,	160.0,		 0.00,			   3.8e3,	Color::LightGray,	TYPE_SATE_NON);
	AddSate(L"Saturn",  L"Atlas",		6.600e15,	  137670e3,		 16638.7,	120.0,		 0.00,			  15.1e3,	Color::LightGray,	TYPE_SATE_NON);
	AddSate(L"Saturn",  L"Prometheus",	1.595e17,	  139380e3,		 16534.9,	 90.0,		 0.00,			  43.1e3,	Color::LightGray,	TYPE_SATE_NON);
	AddSate(L"Saturn",  L"Pandora",		1.371e17,	  141720e3,		 16397.6,	 60.0,		 0.05,			  40.7e3,	Color::LightGray,	TYPE_SATE_NON);
//#endif // SATURN_RINGS

	AddSate(L"Saturn",  L"Mimas",	    3.749e19,	  185404e3,		 14306.3,	  0.0,		 1.57,			 198.2e3,	Color::LightGray);
	AddSate(L"Saturn",  L"Enceladus",   1.080e20,	  237950e3,		 12628.8,	  0.0,		 0.01,			 252.1e3,	Color::LightGray);
	AddSate(L"Saturn",  L"Dione",	    1.095e21,	  377396e3,		 10027.4,	  0.0,		 0.00,			 561.4e3,	Color::LightGray);
    AddSate(L"Saturn",  L"Rhea",	    2.307e21,	  527108e3,		  8483.7,	  0.0,		 0.33,			 763.8e3,	Color::LightGray);
    AddSate(L"Saturn",  L"Titan",	    1.345e23,	 1221930e3,		  5572.7,	 15.0,		 0.35,			2574.9e3,	Color::LightGray);
    AddSate(L"Saturn",  L"Iapetus",	    1.806e21,	 3560850e3,		  3264.5,	  0.0,		15.47,			 734.3e3,	Color::LightGray);

	AddSate(L"Uranus",  L"Miranda",	    6.590e19,	  129900e3,		  6683.0,	  0.0,		 0.00,			 235.8e3,	Color::LightGray);
	AddSate(L"Uranus",  L"Ariel",	    1.353e21,	  190900e3,		  5508.8,	  0.0,		 0.04,			 578.9e3,	Color::LightGray);
	AddSate(L"Uranus",  L"Umbriel",	    1.172e21,	  266000e3,		  4667.8,	  0.0,		 0.00,			 584.7e3,	Color::LightGray);
	AddSate(L"Uranus",  L"Titania",	    3.530e21,	  436300e3,		  3644.4,	  0.0,		 0.08,			 788.4e3,	Color::LightGray);
	AddSate(L"Uranus",  L"Oberon",	    3.010e21,	  583520e3,		  3151.8,	  0.0,		 0.06,			 761.4e3,	Color::LightGray);

	AddSate(L"Neptune", L"Triton",		2.140e21,	  354789e3,		 -4390.0,	  0.0,		 0.00,			1350.0e3,	Color::LightGray);
	AddSate(L"Neptune", L"Nereid",		3.100e21,	 5513787e3,		  1113.4,	  0.0,		32.55,			 170.0e3,	Color::LightGray);

	AddSate(L"Pluto",   L"Charon",		1.586e21,	 19571.4e3,		   222.8,	  0.0,		 0.001,			 606.0e3,	Color::LightGray);
#endif	// SATELITE

#if 0
	AddRocket(L"Earth", L"Rocket" + to_wstring(++RocketNum), 1.000e3, AR * 1.1, 11300, 100, 90, -90, 0, Color::Gold);
	//AddBooster(21 * (24 * 60 * 60), 0, 0, 10000);
	//AddTarget(30 * (24 * 60 * 60), L"Venus", 0, 0, 10000);
#endif

#ifdef ROCKET_VIEW
	// 月を周回して帰ってくる軌道
	AddRocket(L"Earth", L"Rocket" + to_wstring(++RocketNum), 1.000e3, AR * 1.1, 6000.0, 0.00001, 97.2, 35, 0, Color::Gold);
	AddBooster(  1400, 120.0,   0.0, 6000);
	AddBooster(  8100,  92.5,   6.2, 4000);
	AddBooster( 73600, -58.0,  -5.0, 3900);
	AddBooster( 17600, -40.0,  -1.0, 4000);
	AddBooster( 72000, 170.0,   0.0, 4620);
#endif

	// TYPE_ASTEROIDは、計算の簡略化の為にすべての衛星の後に登録すること。

#ifdef	ASTEROID_BELT
	AddAsteroidBelt(10000);
#endif

#ifdef	TROJAN_ASTEROIDS
	AddTrojanAsteroids(2000);
#endif

#ifdef SATURN_RINGS
	AddSaturnRings(10000);
#endif

	// 簡略化の為に表示の中心衛星にtargetIdxを数値で指定
	CenterPos = posV[bodies[targetIdx].i];
	CenterVel = velV[bodies[targetIdx].i];

#ifdef	CUDA_KERNEL
	// すべての衛星データを登録後にCUDAのメモリを設定コピーを行う。
	// 後からの追加は、考慮していない
	int num = (int)bodies.size();

	if (num >= 1000 && cudaGetDeviceCount(&CudaDeviceCount) == cudaSuccess && CudaDeviceCount > 0) {
		if (cudaMalloc(&d_pos, num * sizeof(Vector3D)) == cudaSuccess)
			cudaMemcpy(d_pos, posV.data(), num * sizeof(Vector3D), cudaMemcpyHostToDevice);
		if (cudaMalloc(&d_vel, num * sizeof(Vector3D)) == cudaSuccess)
			cudaMemcpy(d_vel, velV.data(), num * sizeof(Vector3D), cudaMemcpyHostToDevice);
		if (cudaMalloc(&d_mass, num * sizeof(double)) == cudaSuccess)
			cudaMemcpy(d_mass, massV.data(), num * sizeof(double), cudaMemcpyHostToDevice);
		if (cudaMalloc(&d_radius, num * sizeof(double)) == cudaSuccess)
			cudaMemcpy(d_radius, radiusV.data(), num * sizeof(double), cudaMemcpyHostToDevice);
		if (cudaMalloc(&d_color, num * sizeof(COLORREF)) == cudaSuccess)
			cudaMemcpy(d_color, colorV.data(), num * sizeof(COLORREF), cudaMemcpyHostToDevice);

		if (cudaMalloc(&d_viewProj, sizeof(ViewProjMap)) == cudaSuccess)
			cudaMemcpy(d_viewProj, ViewProjMap, sizeof(ViewProjMap), cudaMemcpyHostToDevice);
	} else
		CudaDeviceCount = 0;
#endif	// CUDA_KERNEL
}

#ifdef RK4_TEST
// 加速度の計算 (a = GM/r^2)
Vector3D computeAcceleration(size_t target, std::vector<Vector3D>& pos, std::vector<Vector3D>& vel) {
	Vector3D acc = {0, 0, 0};
	for (size_t n = 0 ; n < bodies.size() ; n++ ) {
		if (target == n || bodies[n].type == TYPE_ROKCET)
			continue;
		else if (bodies[n].type == TYPE_ASTEROID)
			break;

		Vector3D diff = pos[n] - pos[target];
		double distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		double dist = sqrt(distSq);

		if (dist < (radiusV[target] + radiusV[n])) {
			dist = radiusV[target] + radiusV[n];
			distSq = dist * dist;
		}

		double force = (G * massV[n]) / (distSq * dist);
		acc = acc + (diff * force);
	}
	return acc;
}
// RK4法による1ステップ更新
void updateOrbits(std::vector<Body>& bodies, double dt) {
	size_t n = bodies.size();
	static std::vector<Vector3D> k1v(n), k1a(n), k2v(n), k2a(n), k2p(n), k3v(n), k3a(n), k3p(n), k4v(n), k4a(n), k4p(n);

	// k1
#ifdef	MULTI_THREAD
	std::for_each(std::execution::par, bodies.begin(), bodies.end(), [&](Body& b_i) {
		size_t i = b_i.i;
#else
	for (size_t i = 0; i < n; i++) {
#endif
		k1v[i] = velV[i];
		k1a[i] = computeAcceleration(i, posV, velV);

		k2p[i] = posV[i] + (k1v[i] * (dt * 0.5));
		k2v[i] = velV[i] + (k1a[i] * (dt * 0.5));
	}
	// k2
#ifdef	MULTI_THREAD
	);
	std::for_each(std::execution::par, bodies.begin(), bodies.end(), [&](Body& b_i) {
		size_t i = b_i.i;
#else
	for (size_t i = 0; i < n; i++) {
#endif	// MULTI_THREAD
		k2a[i] = computeAcceleration(i, k2p, k2v);

		k3p[i] = posV[i] + (k2v[i] * (dt * 0.5));
		k3v[i] = velV[i] + (k2a[i] * (dt * 0.5));
	}
	// k3
#ifdef	MULTI_THREAD
	);
	std::for_each(std::execution::par, bodies.begin(), bodies.end(), [&](Body& b_i) {
		size_t i = b_i.i;
#else
	for (size_t i = 0; i < n; i++) {
#endif	// MULTI_THREAD
		k3a[i] = computeAcceleration(i, k3p, k3v);

		k4p[i] = posV[i] + (k3v[i] * dt);
		k4v[i] = velV[i] + (k3a[i] * dt);
	}
	// k4
#ifdef	MULTI_THREAD
	);
	std::for_each(std::execution::par, bodies.begin(), bodies.end(), [&](Body& b_i) {
		size_t i = b_i.i;
#else
	for (size_t i = 0; i < n; i++) {
#endif	// MULTI_THREAD
		k4a[i] = computeAcceleration(i, k4p, k4v);
		// 最終的な位置と速度の更新
		posV[i] = posV[i] + (k1v[i] + k2v[i] * 2.0 + k3v[i] * 2.0 + k4v[i]) * (dt / 6.0);
		velV[i] = velV[i] + (k1a[i] + k2a[i] * 2.0 + k3a[i] * 2.0 + k4a[i]) * (dt / 6.0);
	}
#ifdef	MULTI_THREAD
	);
#endif
}
#endif	// RK4_TEST

void UpdatePhysics() {
    // F = G * (m1 * m2) / (r * r)
    // a = F / r / m1
    // a = G * m2 / (r * r * r)
    // vel += a * t
    // pos += vel * t
	clock_t now = clock();

    for (int g = 0; g < globalSubSteps; g++) {
#ifdef ROCKET_VIEW
		double min = 1.0;
		double dt = baseTimeStep * baseTimeDivs / globalSubSteps;
#else
		double dt = baseTimeStep / globalSubSteps;
#endif // ROCKET_VIEW

#if		defined(RK4_TEST)
		// 4次ルンゲ＝クッタ法
		#if	defined(CUDA_KERNEL) && !defined(NOT_CUDA_CALC)
			if (CudaDeviceCount > 0)
				CudaUpdateOrbits(d_pos, d_vel, d_mass, d_radius, (int)bodies.size(), dt);
			else
		#endif	// CUDA_KERNEL
			{
				updateOrbits(bodies, dt);
			}
#else
	#if	defined(CUDA_KERNEL) && !defined(NOT_CUDA_CALC)
		// CUDAを利用
		if (CudaDeviceCount > 0)
			launchCudaPhysics(d_pos, d_vel, d_mass, d_radius, (int)bodies.size(), dt);
		else {
	#else
		{
	#endif	// CUDA_KERNEL
	#if defined(MULTI_THREAD)
			// 各天体の加速度計算を並列化
			std::for_each(std::execution::par, bodies.begin(), bodies.end(), [&](Body& b_i) {
	#else
			for (auto& b_i : bodies) {
	#endif	// MULTI_THREAD
				size_t i = b_i.i;
				Vector3D acc = { 0, 0, 0 };

				for (size_t n = 0; n < bodies.size(); n++) {
					if (i == n || bodies[n].type == TYPE_ROKCET)
						continue;
					else if (bodies[n].type == TYPE_ASTEROID)
						break;
					Vector3D diff = posV[n] - posV[i];
					double r = diff.length();
					if (r < (radiusV[i] + radiusV[n])) {
	#ifdef ROCKET_VIEW
						if (b_i.type == TYPE_ROKCET) {
							velV[i] = velV[n];
							colorV[i] = 0xFF0000FF;
						}
	#endif // ROCKET_VIEW
						r = radiusV[i] + radiusV[n];
					}
	#ifdef ROCKET_VIEW
					else if (b_i.type == TYPE_ROKCET) {
						double f = 2e8 / (massV[n] / (r * r)) * globalSubSteps / baseTimeStep;
						if (f < min)
							min = f;
					}
	#endif // ROCKET_VIEW
					acc += diff * (G * massV[n] / (r * r * r));
				}
				velV[i] += acc * dt;

	#ifdef ROCKET_VIEW
				if (b_i.boost.size() > 0) {
					if ((b_i.boost[0].time -= dt) < 0.0) {
						double z = b_i.boost[0].z * M_PI / 180.0;
						double a = b_i.boost[0].a * M_PI / 180.0;
						if (b_i.boost[0].target >= 0) {
							acc = posV[bodies[b_i.boost[0].target].i] - posV[b_i.i];
							acc = acc * (1.0 / acc.length());
							acc = acc.rotateZ(z);
							acc = acc.rotateX(a);
							velV[b_i.i] = acc * b_i.boost[0].speed;
						}
						else {
							// z=回転(x/y) a=(z,y)
							acc = { cos(z) * cos(a), sin(z) * cos(a), sin(a) };
							velV[b_i.i] += acc * b_i.boost[0].speed;
						}
						b_i.boost.erase(b_i.boost.begin());
					}
				}
	#endif // ROCKET_VIEW
			}
	#if defined(MULTI_THREAD)
			);
	#endif	// MULTI_THREAD

			// 位置の更新
			for (size_t i = 0; i < bodies.size(); i++)
				posV[i] += velV[i] * dt;
		}

#endif	// RK4_TEST

		if (targetLock) {
            Vector3D diff = posV[bodies[0].i] - CenterPos;
			double r = max(diff.length(), radiusV[bodies[0].i]);
            diff = diff * (G * massV[bodies[0].i] / (r * r * r));
            CenterVel += diff * dt;
            CenterPos += CenterVel * dt;
        }

        TotalTime += dt;
#ifdef ROCKET_VIEW
		baseTimeDivs = min;
#endif // ROCKET_VIEW
    }

#if defined(CUDA_KERNEL) && !defined(NOT_CUDA_CALC)
	if (d_pos != nullptr)
		cudaMemcpy(posV.data(), d_pos, (int)posV.size() * sizeof(Vector3D), cudaMemcpyDeviceToHost);
#endif	// CUDA_KERNEL

	// フレームレートに合うようにglobalSubStepsを計算
	clock_t msec = (clock() - now) * 1000 / CLOCKS_PER_SEC;

	TotalPhysicsMsec += (double)msec;
	TotalPhysicsStep += (double)globalSubSteps;

	if (TotalPhysicsMsec > 1e20) {
		TotalPhysicsMsec /= 1e10;
		TotalPhysicsStep /= 1e10;
	}
	
#ifdef ONIDLE_CALC
	#define	CLOCKS_BASE_MSEC	9900.0		// CLOCKS_PAR_SEC * 10.0 * 0.99
#else
	#define	CLOCKS_BASE_MSEC	8400.0		// CLOCKS_PAR_SEC * 10.0 * 0.84
#endif // ONIDLE_CALC

	if (TotalPhysicsMsec <= 0.0)
		globalSubSteps *= 2;
	else if ((globalSubSteps = (int)(CLOCKS_BASE_MSEC * TotalPhysicsStep / (double)FramePerSec / TotalPhysicsMsec)) < 1)
		globalSubSteps = 1;

	if (globalSubSteps < 10 && FramePerSec > 5)
		FramePerSec = FramePerSec * globalSubSteps / 10;
}

void LoadFilePos(HWND hWnd) {
	FILE* fp = NULL;
	size_t size = 0;
	OPENFILENAME ofn;       // 構造体
	wchar_t szFile[260];    // 結果を格納するバッファ

	_tcscpy_s(szFile, 260, POSFILENAME);

	// 構造体の初期化
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
	ofn.lpstrFilter = L"Data Files\0*.dat\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; // フラグ設定

	if (GetOpenFileName(&ofn) == FALSE)
		return;

	if (_tfopen_s(&fp, szFile, L"rb"))
		return;

	if (fread((void*)&size, sizeof(size), 1, fp) != 1)
		goto ENDOF;

	if (size != posV.size())
		goto ENDOF;

	if (fread((void*)&TotalYear, sizeof(TotalYear), 1, fp) != 1)
		goto ENDOF;

	if (fread((void*)&TotalTime, sizeof(TotalTime), 1, fp) != 1)
		goto ENDOF;

	if (fread((void*)posV.data(), sizeof(Vector3D), size, fp) != size)
		goto ENDOF;

	if (fread((void*)velV.data(), sizeof(Vector3D), size, fp) != size)
		goto ENDOF;

ENDOF:
	fclose(fp);

	SaveIdx = (-1);
	CenterPos = posV[bodies[targetIdx].i];
	CenterVel = velV[bodies[targetIdx].i];

#if defined(CUDA_KERNEL) && !defined(NOT_CUDA_CALC)
	if (d_pos)
		cudaMemcpy(d_pos, posV.data(), size * sizeof(Vector3D), cudaMemcpyHostToDevice);
	if (d_vel)
		cudaMemcpy(d_vel, velV.data(), size * sizeof(Vector3D), cudaMemcpyHostToDevice);
#endif	// CUDA_KERNEL
}
void SaveFilePos(HWND hWnd) {
	FILE* fp = NULL;
	size_t size = posV.size();
	OPENFILENAME ofn;       // 構造体
	wchar_t szFile[260];    // 結果を格納するバッファ

	_tcscpy_s(szFile, 260, POSFILENAME);

	// 構造体の初期化
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
	ofn.lpstrFilter = L"Data Files\0*.dat\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; // フラグ設定

	if (GetSaveFileName(&ofn) == FALSE)
		return;

#if defined(CUDA_KERNEL) && !defined(NOT_CUDA_CALC)
	if (d_pos)
		cudaMemcpy(posV.data(), d_pos, (int)posV.size() * sizeof(Vector3D), cudaMemcpyDeviceToHost);
	if (d_vel)
		cudaMemcpy(velV.data(), d_vel, (int)velV.size() * sizeof(Vector3D), cudaMemcpyDeviceToHost);
#endif

	if (_tfopen_s(&fp, szFile, L"wb"))
		return;

	if (fwrite((void*)&size, sizeof(size), 1, fp) != 1)
		goto ENDOF;

	if (fwrite((void*)&TotalYear, sizeof(TotalYear), 1, fp) != 1)
		goto ENDOF;

	if (fwrite((void*)&TotalTime, sizeof(TotalTime), 1, fp) != 1)
		goto ENDOF;

	if (fwrite((void*)posV.data(), sizeof(Vector3D), size, fp) != size)
		goto ENDOF;

	if (fwrite((void*)velV.data(), sizeof(Vector3D), size, fp) != size)
		goto ENDOF;

ENDOF:
	fclose(fp);
}

#ifdef DIRECT3D_SWAP
void UpdateVertexBuffer(XMMATRIX* pViewProj, XMMATRIX* pLightViewProj)
{
	if ( pViewProj != nullptr)
		XMStoreFloat4x4((XMFLOAT4X4*)(ViewProjMap +  0), *pViewProj);
	if ( pLightViewProj != nullptr)
		XMStoreFloat4x4((XMFLOAT4X4*)(ViewProjMap + 16), *pLightViewProj);

	if (pVertexBuffer == nullptr)
		InitVertexResources((int)bodies.size());

	if (CudaDeviceCount > 0) {
#if defined(NOT_CUDA_CALC)
		if (d_pos)
			cudaMemcpy(d_pos, posV.data(), posV.size() * sizeof(Vector3D), cudaMemcpyHostToDevice);
		if (d_vel)
			cudaMemcpy(d_vel, velV.data(), velV.size() * sizeof(Vector3D), cudaMemcpyHostToDevice);
#endif
		if (d_viewProj)
			cudaMemcpy(d_viewProj, ViewProjMap, sizeof(ViewProjMap), cudaMemcpyHostToDevice);

		mapAndWriteVertices(d_pos, d_vel, d_mass, d_color, (int)bodies.size(), d_viewProj, { CenterPos.x, CenterPos.y, CenterPos.z });
	}
	else {
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		if (SUCCEEDED(pD3DContext->Map(pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
			Vertex* verV = (Vertex*)mappedResource.pData;

#if defined(MULTI_THREAD)
			std::for_each(std::execution::par, bodies.begin(), bodies.end(), [&](Body& b) {
#else
			for (auto& b : bodies) {
#endif	// MULTI_THREAD
				size_t i = b.i;

				XMVECTOR worldPos = { (float)((posV[i].x - CenterPos.x) / AU), (float)((posV[i].y - CenterPos.y) / AU), (float)((posV[i].z - CenterPos.z) / AU), 1.0f };
				XMVECTOR clipPos = {};
				
				if (pViewProj)
					clipPos = XMVector3Transform(worldPos, *pViewProj);

				if (clipPos.m128_f32[3] > 0.001f && massV[i] <= 0.0)
					XMStoreFloat4((XMFLOAT4 *)&(verV[i].pos), clipPos);
				else
					verV[i].pos.z = 2.0f;

				if (pLightViewProj)
					clipPos = XMVector3Transform(worldPos, *pLightViewProj);

				XMStoreFloat4((XMFLOAT4 *)&(verV[i].spos), clipPos);
				verV[i].color = colorV[i];
			}
#if defined(MULTI_THREAD)
			);
#endif	// MULTI_THREAD
			pD3DContext->Unmap(pVertexBuffer, 0);
		}
	}
}
void OnPaint(HWND hWnd) {

	if (pRT == nullptr || pD3DContext == nullptr)
		return;

	// 画面サイズ取得
	RECT rc;
	GetClientRect(hWnd, &rc);

	float screenX = (float)(rc.right - rc.left);
	float screenY = (float)(rc.bottom - rc.top);

	if (screenX <= 0.0f || screenY <= 0.0f) {
		SaveIdx = (-1);
		return;
	}

	float centerX = screenX / 2.0f;
	float centerY = screenY / 2.0f;
	double ky = cos(M_PI * rotateX);
	double kz = sin(M_PI * rotateX);

	ScreenWidth = (int)screenX;
	ScreenHeight = (int)screenY;

	if (!targetLock) {
		CenterPos = posV[bodies[targetIdx].i];
		CenterVel = velV[bodies[targetIdx].i];
	}

	bool bClear = (centerX != SaveCx || centerY != SaveCy || viewZoom != SaveZoom || rotateX != SaveRotateX || rotateY != SaveRotateY || targetIdx != SaveIdx) ? true : false;

	SaveCx = centerX;
	SaveCy = centerY;
	SaveZoom = viewZoom;
	SaveRotateX = rotateX;
	SaveRotateY = rotateY;
	SaveIdx = targetIdx;

	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	pD3DContext->ClearRenderTargetView(pBackBufferRT, clearColor);
	pD3DContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	pD3DContext->ClearDepthStencilView(pShadowDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Direct3Dは、左手座標でモニターに映した場合(XY平面)に右が+X、上が+Y、奥+Zになり
	// 惑星などの北極が-Zになるので注意

	// ターゲット惑星を中心に太陽系全景を見た場合のビュー
	float distance = (float)(1000.0 / viewZoom); // viewZoomの値に合わせて調整
	XMVECTOR targetPos = XMVectorSet((float)(CenterPos.x / AU), (float)(CenterPos.y / AU), (float)(CenterPos.z / AU), 1.0f);
	XMVECTOR toffset = XMVectorSet(0.0f, 0.0f, -distance, 0.0f); // ターゲットから後ろに下がる

	if (earthEyes && targetIdx != 3) {
		Vector3D vec = posV[3] - CenterPos; vec.Normalize(); vec *= distance;
		toffset = XMVectorSet((float)vec.x, (float)vec.y, (float)vec.z, 0.0f);
	}

	// rotateX（回転）を行列として適用
	XMMATRIX rotation = XMMatrixRotationX(XM_PI * (float)rotateX) * XMMatrixRotationY(XM_PI * (float)rotateY);
	toffset = XMVector3Transform(toffset, rotation);

	XMVECTOR eyePos = targetPos + toffset;
	XMVECTOR localCamera = toffset;	// eyePos - targetPos
	XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	if (earthEyes && targetIdx != 3)
		upDir = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);

	XMMATRIX viewMat = XMMatrixLookAtLH(localCamera, {0.0f, 0.0f, 0.0f, 1.0f}, upDir);

	// プロジェクション行列（レンズの設定）
	// 画角45度、アスペクト比、手前0.1、奥1000.0（描画スケールに合わせて調整）
	XMMATRIX projMat = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)ScreenWidth / (float)ScreenHeight, distance * 0.1f, 10000.0f);
	XMMATRIX ViewProj = XMMatrixMultiply(viewMat, projMat);

	////////////////////////////////////
	// 影のビューを作成

	// 太陽からターゲット惑星を見るビュー行列
	XMVECTOR lightCamera = XMVector3Normalize(XMVectorSet((float)((posV[0].x - CenterPos.x) / AU), (float)((posV[0].y - CenterPos.y) / AU), (float)((posV[0].z - CenterPos.z) / AU), 1.0f));

	// ターゲットが太陽の場合の暫定処置（実際は、影の計算をしない）
	if (targetIdx == 0)
		lightCamera = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMVECTOR lightUpDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightCamera, {0.0f, 0.0f, 0.0f, 1.0f}, lightUpDir);

	// ターゲット惑星周辺だけに絞ったプロジェクション行列
	// 遠近感のない平行投影（Ortho）を使うと、影の歪みが少なくなります
	XMMATRIX lightProj = XMMatrixOrthographicLH((float)(screenX / viewZoom), (float)(screenY / viewZoom), 0.0001f, 200.0f);
	XMMATRIX lightViewProj = XMMatrixMultiply(lightView, lightProj); // lightView * lightProj;

	////////////////////////////////////
	// 現在の状態などを画面左上に表示

	pRT->BeginDraw();

	// Year = 365.2425 * 24 * 60 * 60
	int year = (int)(TotalTime / (365.2425 * 24.0 * 60.0 * 60.0));
	if (year > 0) {
		TotalYear += year;
		TotalTime -= (double)year * (365.2425 * 24.0 * 60.0 * 60.0);
	}

	if (pTextFormat) {
		wstring info;
		info = L"Zoom: " + to_wstring(viewZoom  * worldZoom/ 300.0).substr(0, 5) + L"x";
		info += L" | Step: " + to_wstring(baseTimeStep * baseTimeDivs / 3600.0).substr(0, 5) + L"h";
		info += L" / " + to_wstring(globalSubSteps);
		info += L" | Fps: " + to_wstring(FrameParSec / 10) + L"." + to_wstring(FrameParSec % 10);
		info += L" | Year: " + to_wstring(TotalYear);
		info += L" | Days: " + to_wstring((int)(TotalTime / (24 * 60 * 60)));
		info += L" | Time: " + to_wstring((int)(TotalTime));

		pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
		pRT->DrawText(info.c_str(), (UINT32)info.length(), pTextFormat, D2D1::RectF(10, 10, screenX, 50), pBrush);
	}

	HRESULT hr = pRT->EndDraw();

	////////////////////////////////////
	// 惑星のテクスチャを事前に作成

	if (InitTexture < (int)bodies.size()) {
		if (bodies[InitTexture].type == TYPE_ASTEROID) {
			InitTexture = (int)bodies.size();
		} else {
			if (bodies[InitTexture].pLabelSRV == nullptr)
				CreateLabelTexture(bodies[InitTexture], (int)((float)screenX * 0.08f), (int)((float)screenY * 0.03f));
			else if (bodies[InitTexture].pPlanetSRV == nullptr)
				CreatePlanetTexture(pD3DDevice, bodies[InitTexture]);
			else
				InitTexture++;
		}
	}

	////////////////////////////////////
	// ビューポートのセット（これを忘れると座標がズレて見えません）
	D3D11_VIEWPORT vp = { 0, 0, (float)ScreenWidth, (float)ScreenHeight, 0, 1 };
	pD3DContext->RSSetViewports(1, &vp);

	////////////////////////////////////////////
	// パス1	惑星をシャドウマップに描画

	// レンダーターゲットを解除し、シャドウマップステンシルビューをセット
	ID3D11RenderTargetView* nullRT = nullptr;
	pD3DContext->OMSetRenderTargets(1, &nullRT, pShadowDepthView);

	// 惑星用シェーダーとバッファをセット
	pD3DContext->IASetInputLayout(pPlanetInputLayout);
	pD3DContext->VSSetShader(pPlanetVS, nullptr, 0);
	pD3DContext->PSSetShader(nullptr, nullptr, 0);

	UINT pstride = sizeof(PlanetVertex);
	UINT poffset = 0;
	pD3DContext->IASetVertexBuffers(0, 1, &pPlanetVB, &pstride, &poffset);
	pD3DContext->IASetIndexBuffer(pPlanetIB, DXGI_FORMAT_R32_UINT, 0);
	pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画対象を設定
	//pD3DContext->RSSetState(pDrawingRenderState);
	pD3DContext->RSSetState(pShadowRenderState);
	//pD3DContext->RSSetState(pNomalRenderState);

	pD3DContext->OMSetDepthStencilState(pDSState_Normal, 1);

	// 衛星を持つ惑星がターゲットの場合のみに限定
	// bodies.sateliteには、最初に自身の惑星を登録済み
	int index = (bodies[targetIdx].base > 0 ? bodies[targetIdx].base : targetIdx);
	for (auto& i : bodies[index].satelite ) {

		float r = (float)(radiusV[i] / AU * worldZoom);

		// ワールド行列の作成（サイズを半径、位置をシミュレーション座標に）
		// Viewの回転に合わせて放線を-rで強制した
		XMMATRIX worldMat = XMMatrixScaling(r, r, r) * XMMatrixTranslation((float)((posV[i].x - CenterPos.x) * worldZoom / AU), (float)((posV[i].y - CenterPos.y) * worldZoom / AU), (float)((posV[i].z - CenterPos.z) * worldZoom / AU));

		// 定数バッファの更新
		PlanetConstantBuffer cb = {};
		cb.World = XMMatrixTranspose(worldMat); // HLSLは列優先のため転置
		cb.DispViewProj = XMMatrixTranspose(lightViewProj);

		// 定数バッファの設定
		pD3DContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb, 0, 0);
		pD3DContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
		pD3DContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);

		// 描画実行！
		pD3DContext->DrawIndexed(sphereIndexCount, 0, 0);
	}

#ifdef ASTEROID_VIEW
	// 小惑星の頂点シェーダーバッファをここで更新
	// 簡略化の為にパス１・２の頂点シューだーを作成
	UpdateVertexBuffer(&ViewProj, &lightViewProj);

  #ifdef ASTEROID_SHADOW
	////////////////////////////////////////////
	// パス2	小惑星をシャドウマップに描画

	// プリミティブ（描画形式）のセット
	pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// 入力レイアウトのセット
	pD3DContext->IASetInputLayout(pInputLayout);
	pD3DContext->VSSetShader(pAsteroidShadowVS, nullptr, 0);
	pD3DContext->PSSetShader(nullptr, nullptr, 0);

	// 頂点バッファをセット
	UINT vstride = sizeof(Vertex); // 前に定義したVertex構造体のサイズ
	UINT voffset = 0;
	pD3DContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &vstride, &voffset);

	pD3DContext->Draw((UINT)bodies.size(), 0);
  #endif	// ASTEROID_SHADOW
#endif	// ASTEROID_VIEW

	////////////////////////////////////////////
	// パス3	惑星をバックバッファに描画

	// 3. D3Dのレンダーターゲットをセット
	pD3DContext->OMSetRenderTargets(1, &pBackBufferRT, pDepthStencilView);

	// リソースビューとサンプラーを設定
	pD3DContext->PSSetShaderResources(0, 1, &pShadowResourceView);
	pD3DContext->PSSetSamplers(0, 1, &pSamplerState);

	// 2. 惑星用シェーダーとバッファをセット
	pD3DContext->IASetInputLayout(pPlanetInputLayout);
	pD3DContext->VSSetShader(pPlanetVS, nullptr, 0);
	pD3DContext->PSSetShader(pPlanetPS, nullptr, 0);

	//UINT pstride = sizeof(PlanetVertex);
	//UINT poffset = 0;
	pD3DContext->IASetVertexBuffers(0, 1, &pPlanetVB, &pstride, &poffset);
	pD3DContext->IASetIndexBuffer(pPlanetIB, DXGI_FORMAT_R32_UINT, 0);

	pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 深度書き込みをONにする
	pD3DContext->OMSetDepthStencilState(pDSState_Normal, 1);

	// 描画対象を設定
	pD3DContext->RSSetState(pDrawingRenderState);
	//pD3DContext->RSSetState(pShadowRenderState);
	//pD3DContext->RSSetState(pNomalRenderState);

	// アルファブレンドの設定
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pD3DContext->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);

	// 3. 各衛星（または惑星）ごとに描画
	for (size_t i = 0; i < bodies.size(); i++) {
		if (bodies[i].type == TYPE_ASTEROID)
			break;
		else if (i != targetIdx && bodies[i].base != 0 && (bodies[i].base != targetIdx && bodies[i].base != bodies[targetIdx].base))	// 衛星は、ターゲットのみ
			continue;

		float distValue = XMVectorGetX(XMVector3Length(XMVectorSet((float)(posV[i].x / AU), (float)(posV[i].y / AU), (float)(posV[i].z / AU), 1.0f) - eyePos));
		float pixelRadius = ((float)(radiusV[i] / AU) / distValue) * projMat.r[0].m128_f32[0] * (screenX * 0.5f);
#ifdef PLANET_SIMPLE
		float scaleFactor = (pixelRadius < 1.0f ? (1.0f / pixelRadius) : 1.0f);
#else
		float scaleFactor = (pixelRadius < 1.5f ? (1.5f / pixelRadius) : 1.0f);
#endif	// PLANET_SIMPLE
		float r = (float)(radiusV[i] * worldZoom / AU) * scaleFactor;

		// XMMatrixRotationYが90以上なら逆回転になるので注意
		double sign = (bodies[i].obliquity >= 90.0 ? -1.0 : 1.0);
		float period = (float)(bodies[i].rotation * sign * 24.0 * 60.0 * 60.0);		// 自転周期(日) -> 秒
		float rotation = 0.0f;
		if (period != 0.0)
			rotation = 2.0f * XM_PI * (float)fmod(TotalTime, period) / period;

		// ワールド行列の作成（サイズを半径、位置をシミュレーション座標に）
		XMMATRIX worldMat = XMMatrixScaling(r, r, r) * 
			XMMatrixRotationZ(rotation) * XMMatrixRotationY((float)bodies[i].obliquity * XM_PI / 180.0f) * XMMatrixRotationZ((float)bodies[i].obli_rot * XM_PI / 180.0f) *
			XMMatrixTranslation((float)((posV[i].x - CenterPos.x) * worldZoom / AU), (float)((posV[i].y - CenterPos.y) * worldZoom / AU), (float)((posV[i].z - CenterPos.z) * worldZoom / AU));

		// 定数バッファの更新
		PlanetConstantBuffer cb;
		cb.World = XMMatrixTranspose(worldMat); // HLSLは列優先のため転置

		cb.DispViewProj = XMMatrixTranspose(ViewProj);
		cb.LightViewProj = XMMatrixTranspose(lightViewProj);

		cb.PlanetColor = { (float)bodies[i].color.GetRed() / 255.f, (float)bodies[i].color.GetGreen() / 255.f, (float)bodies[i].color.GetBlue() / 255.f };
		cb.CameraPos = { localCamera.m128_f32[0], localCamera.m128_f32[1], localCamera.m128_f32[2] };

		if (i == 0)
			cb.SunPos = cb.CameraPos;
		else
			cb.SunPos = { (float)((posV[0].x - CenterPos.x) / AU), (float)((posV[0].y - CenterPos.y) / AU), (float)((posV[0].z - CenterPos.z) / AU) }; // 太陽を原点とする場合

		pD3DContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb, 0, 0);
		pD3DContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
		pD3DContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);

		if (bodies[i].pPlanetSRV == nullptr)
			CreatePlanetTexture(pD3DDevice, bodies[i]);

		pD3DContext->PSSetShaderResources(3, 1, &bodies[i].pPlanetSRV); // t3 レジスタにセット
		pD3DContext->PSSetSamplers(3, 1, &pSamplerState2DLinear); // 通常用サンプラー

		// 地球3のみリムライト付きPS
#ifdef PLANET_SIMPLE
		pD3DContext->PSSetShader(i == 3 && viewZoom > 300000.0 ? pShadowPS : (scaleFactor == 1.0f ? pPlanetPS : pSimplePS), nullptr, 0);
#else
		pD3DContext->PSSetShader(i == 3 && viewZoom > 300000.0 ? pShadowPS : pPlanetPS, nullptr, 0);
#endif // PLANET_SIMPLE

		// 描画実行！
		pD3DContext->DrawIndexed(sphereIndexCount, 0, 0);
	}

	////////////////////////////////////////////
	// 以下のパスの共有設定

	// 深度書き込みをOFFにして描画（惑星の裏側だけ自動的に消える）
	pD3DContext->OMSetDepthStencilState(pDSState_NoWrite, 1);

	// 描画対象を設定
	//pD3DContext->RSSetState(pDrawingRenderState);
	//pD3DContext->RSSetState(pShadowRenderState);
	pD3DContext->RSSetState(pNomalRenderState);


#ifdef RING_TEXTURE
	////////////////////////////////////////////
	// パス4	土星の環をバックバッファに描画

	if (targetIdx == 6) {	// 土星6のみ描画
		// リソースビューとサンプラーを設定
		pD3DContext->PSSetShaderResources(1, 1, &pRingSRV); // t1 レジスタにセット
		pD3DContext->PSSetSamplers(1, 1, &pSamplerStateLinear); // 通常用サンプラー

		// 2. 惑星用シェーダーとバッファをセット
		pD3DContext->IASetInputLayout(pRingInputLayout);
		pD3DContext->VSSetShader(pRingVS, nullptr, 0);
		pD3DContext->PSSetShader(pRingShadowPS, nullptr, 0);

		UINT rstride = sizeof(RingVertex);
		UINT roffset = 0;
		pD3DContext->IASetVertexBuffers(0, 1, &pRingVB, &rstride, &roffset);
		pD3DContext->IASetIndexBuffer(pRingIB, DXGI_FORMAT_R32_UINT, 0);
		pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		size_t i = 6;
		float r = (float)(radiusV[i] / AU);
		XMMATRIX worldMat = XMMatrixScaling(r, 1.0f, r) * 
			XMMatrixRotationX(XM_PI / 2.0f) * XMMatrixRotationY((float)bodies[i].obliquity * XM_PI / 180.0f) * XMMatrixRotationZ((float)bodies[i].obli_rot * XM_PI / 180.0f) *
			XMMatrixTranslation((float)((posV[i].x - CenterPos.x) / AU), (float)((posV[i].y - CenterPos.y) / AU), (float)((posV[i].z - CenterPos.z) / AU));

		PlanetConstantBuffer cb;
		cb.World = XMMatrixTranspose(worldMat); // HLSLは列優先のため転置
		cb.DispViewProj = XMMatrixTranspose(ViewProj);
		cb.LightViewProj = XMMatrixTranspose(lightViewProj);

		cb.PlanetColor = { (float)bodies[i].color.GetRed() / 255.f, (float)bodies[i].color.GetGreen() / 255.f, (float)bodies[i].color.GetBlue() / 255.f };
		cb.CameraPos = { localCamera.m128_f32[0], localCamera.m128_f32[1], localCamera.m128_f32[2] };
		cb.SunPos = { (float)((posV[0].x - CenterPos.x) / AU), (float)((posV[0].y - CenterPos.y) / AU), (float)((posV[0].z - CenterPos.z) / AU) }; // 太陽を原点とする場合

		pD3DContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb, 0, 0);
		pD3DContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
		pD3DContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);

		pD3DContext->DrawIndexed(ringIndexCount, 0, 0);
	}

	if (targetIdx == 7) {	// 天王星7のみ描画
		// リソースビューとサンプラーを設定
		pD3DContext->PSSetShaderResources(1, 1, &pUranusRingSRV); // t1 レジスタにセット
		pD3DContext->PSSetSamplers(1, 1, &pSamplerStateLinear); // 通常用サンプラー

		// 2. 惑星用シェーダーとバッファをセット
		pD3DContext->IASetInputLayout(pRingInputLayout);
		pD3DContext->VSSetShader(pRingVS, nullptr, 0);
		pD3DContext->PSSetShader(pRingShadowPS, nullptr, 0);

		UINT rstride = sizeof(RingVertex);
		UINT roffset = 0;
		pD3DContext->IASetVertexBuffers(0, 1, &pUranusRingVB, &rstride, &roffset);
		pD3DContext->IASetIndexBuffer(pUranusRingIB, DXGI_FORMAT_R32_UINT, 0);
		pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		size_t i = 7;
		float r = (float)(radiusV[i] / AU);
		XMMATRIX worldMat = XMMatrixScaling(r, 1.0f, r) * 
			XMMatrixRotationX(XM_PI / -2.0f) * XMMatrixRotationY((float)bodies[i].obliquity * XM_PI / 180.0f) * XMMatrixRotationZ((float)bodies[i].obli_rot * XM_PI / 180.0f) *
			XMMatrixTranslation((float)((posV[i].x - CenterPos.x) / AU), (float)((posV[i].y - CenterPos.y) / AU), (float)((posV[i].z - CenterPos.z) / AU));

		PlanetConstantBuffer cb;
		cb.World = XMMatrixTranspose(worldMat); // HLSLは列優先のため転置
		cb.DispViewProj = XMMatrixTranspose(ViewProj);
		cb.LightViewProj = XMMatrixTranspose(lightViewProj);

		cb.PlanetColor = { (float)bodies[i].color.GetRed() / 255.f, (float)bodies[i].color.GetGreen() / 255.f, (float)bodies[i].color.GetBlue() / 255.f };
		cb.CameraPos = { localCamera.m128_f32[0], localCamera.m128_f32[1], localCamera.m128_f32[2] };
		cb.SunPos = { (float)((posV[0].x - CenterPos.x) / AU), (float)((posV[0].y - CenterPos.y) / AU), (float)((posV[0].z - CenterPos.z) / AU) }; // 太陽を原点とする場合

		pD3DContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb, 0, 0);
		pD3DContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
		pD3DContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);

		pD3DContext->DrawIndexed(UranusRingIndexCount, 0, 0);
	}
#endif // RING_TEXTURE

	////////////////////////////////////////////
	// パス6	小惑星をバックバッファに描画

#ifdef ASTEROID_VIEW
	// シャドウマップ作成時は、同時に計算済み
	// 小惑星の頂点シェーダーバッファを更新
	//XMMATRIX viewProj = XMMatrixMultiply(viewMat, projMat);
	//UpdateVertexBuffer(&viewProj, NULL);

	// プリミティブ（描画形式）のセット
	pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// 入力レイアウトのセット
	pD3DContext->IASetInputLayout(pInputLayout);

	// 頂点シェーダーとピクセルシェーダーをセット
	pD3DContext->VSSetShader(pAsteroidVS, nullptr, 0);

  #ifdef SATERITE_SHADOW
	pD3DContext->PSSetShader(pAsteroidShadowPS, nullptr, 0);
  #else
	pD3DContext->PSSetShader(pAsteroidPS, nullptr, 0);
  #endif // SATERITE_SHADOW

	// 4. 頂点バッファをセット
#ifndef ASTEROID_SHADOW
	UINT vstride = sizeof(Vertex);
	UINT voffset = 0;
#endif
	pD3DContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &vstride, &voffset);

	// 小惑星を描画
	pD3DContext->Draw((UINT)bodies.size(), 0);
#endif // ASTEROID_VIEW

	////////////////////////////////////////////
	// パス5 惑星の軌跡を描画

	pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	pD3DContext->IASetInputLayout(pInputLayout);
	pD3DContext->VSSetShader(pAsteroidVS, nullptr, 0);
	pD3DContext->PSSetShader(pAsteroidPS, nullptr, 0);

	// 軌跡のバーテックスを計算
	for (auto& b : bodies) {
		if (b.type == TYPE_ASTEROID)
			break;
		else if (b.type == TYPE_SATE_NON)
			continue;
		else if (b.i != targetIdx && b.base != 0 && (b.base != targetIdx && b.base != bodies[targetIdx].base))	// 衛星は、ターゲットのみ
			continue;

		int max = PATH_MAX;
		XMVECTOR wpos = XMVectorSet((float)((posV[b.i].x - CenterPos.x) * worldZoom / AU), (float)((posV[b.i].y - CenterPos.y) * worldZoom / AU), (float)((posV[b.i].z - CenterPos.z) * worldZoom / AU), 1.0f);
		XMVECTOR vpos = XMVector3Transform(wpos, ViewProj);

		if (bClear)
			b.vlen = 0;

		if (b.vpos == nullptr)
			b.vpos = new XMVECTOR[PATH_MAX];

		// 衛星の場合は、自転周期＝公転周期
		if (b.period > 0.0) {
			if ((max = (int)(b.period * (24 * 60 * 60) / baseTimeStep)) < 5)
				max = 5;
			else if (max > PATH_MAX)
				max = PATH_MAX;
		}

		if (b.vlen >= max) {
			int n = b.vlen - max + 1;
			memmove(b.vpos + 0, b.vpos + n, sizeof(XMVECTOR) * (b.vlen - n));
			b.vlen -= n;
		}
		b.vpos[b.vlen++] = vpos;

		if (b.vertex != nullptr) {
			D3D11_BUFFER_DESC bd = {};
			b.vertex->GetDesc(&bd);
			if (bd.ByteWidth != (sizeof(Vertex) * b.vlen)) {
				b.vertex->Release();
				b.vertex = nullptr;
			}
		}

		if (b.vertex == nullptr) {
			D3D11_BUFFER_DESC bd = {};
			bd.ByteWidth = sizeof(Vertex) * b.vlen;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			if (FAILED(pD3DDevice->CreateBuffer(&bd, nullptr, &b.vertex)))
				continue;
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		if (FAILED(pD3DContext->Map(b.vertex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			continue;
		Vertex* verV = (Vertex*)mappedResource.pData;

		for (int n = 0; n < b.vlen; n++) {
			XMStoreFloat4((XMFLOAT4 *)&(verV[n].pos), b.vpos[n]);
			verV[n].spos = verV[n].pos;
			verV[n].color = colorV[b.i] & 0x00FFFFFF;
			verV[n].color |= (((uint32_t)n * 128 / (uint32_t)b.vlen) << 24);
		}

		pD3DContext->Unmap(b.vertex, 0);

		if (b.vertex == nullptr || b.vlen < 2)
			continue;

		UINT vstride = sizeof(Vertex);
		UINT voffset = 0;
		pD3DContext->IASetVertexBuffers(0, 1, &b.vertex, &vstride, &voffset);
		pD3DContext->Draw(b.vlen, 0);
	}

	////////////////////////////////////////////
	// パス7	惑星名を惑星の横に表示

	// アルファブレンドを変更
	// pD3DContext->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);
	pD3DContext->OMSetBlendState(pLabelBlendState, blendFactor, 0xffffffff);

	pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pD3DContext->IASetInputLayout(pLabelInputLayout);
	pD3DContext->VSSetShader(pLabelVS, nullptr, 0);
	pD3DContext->PSSetShader(pLabelPS, nullptr, 0);

	// 定数バッファの更新
	PlanetConstantBuffer labelCb = {};
	labelCb.DispViewProj = XMMatrixTranspose(ViewProj);
	pD3DContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &labelCb, 0, 0);
	pD3DContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	pD3DContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);

	for (auto& b : bodies) {
		if (b.type == TYPE_ASTEROID)
			break;
		
		if (viewZoom < 3000.0 && (b.type == TYPE_SATELITE || b.type == TYPE_SATE_NON))
			continue;

		if (b.i != targetIdx && b.base != 0 && (b.base != targetIdx && b.base != bodies[targetIdx].base))	// 衛星は、ターゲットのみ
			continue;

		XMVECTOR wpos = XMVectorSet((float)(((posV[b.i].x + radiusV[b.i] * 1.1) - CenterPos.x) * worldZoom / AU), (float)((posV[b.i].y - CenterPos.y) * worldZoom / AU), (float)((posV[b.i].z - CenterPos.z) * worldZoom / AU), 1.0f);
		XMVECTOR vpos = XMVector3Transform(wpos, ViewProj);

		float baseX = vpos.m128_f32[0] / vpos.m128_f32[3];
		float baseY = vpos.m128_f32[1] / vpos.m128_f32[3];
		float baseZ = vpos.m128_f32[2] / vpos.m128_f32[3];

		if (baseX < -1.0f || baseX > 1.0f || baseY < -1.0f || baseY > 1.0f)
			continue;

		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		if (FAILED(pD3DContext->Map(pLabelVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			continue;
		RingVertex* verV = (RingVertex*)mappedResource.pData;

		float labelWidth  = 0.08f; 
		float labelHeight = 0.03f;

		// 頂点0: 左下
		verV[0].Pos = XMFLOAT3(baseX, baseY - labelHeight, baseZ);
		verV[0].Tex = XMFLOAT2(0.0f, 1.0f);

		// 頂点1: 右下
		verV[1].Pos = XMFLOAT3(baseX + labelWidth, baseY - labelHeight, baseZ);
		verV[1].Tex = XMFLOAT2(1.0f, 1.0f);

		// 頂点2: 右上
		verV[2].Pos = XMFLOAT3(baseX + labelWidth, baseY, baseZ);
		verV[2].Tex = XMFLOAT2(1.0f, 0.0f);

		// 頂点3: 左上
		verV[3].Pos = XMFLOAT3(baseX, baseY, baseZ);
		verV[3].Tex = XMFLOAT2(0.0f, 0.0f);

		pD3DContext->Unmap(pLabelVB, 0);

		// インデックスの並び { 3, 2, 0, 0, 2, 1 } に合わせて、反時計回り（上・手前向き）になるように結合
		pstride = sizeof(RingVertex);
		poffset = 0;
		pD3DContext->IASetVertexBuffers(0, 1, &pLabelVB, &pstride, &poffset);
		pD3DContext->IASetIndexBuffer(pLabelIB, DXGI_FORMAT_R32_UINT, 0);

		if (b.pLabelSRV == nullptr)
			CreateLabelTexture(b, (int)((float)screenX * labelWidth), (int)((float)screenY * labelHeight));

		pD3DContext->PSSetShaderResources(2, 1, &b.pLabelSRV); // t2 レジスタにセット
		pD3DContext->PSSetSamplers(2, 1, &pSamplerState2DLinear); // 通常用サンプラー

		pD3DContext->DrawIndexed(6, 0, 0);
	}

	////////////////////////////////////////////
	// パス8	画面の隅にシャドウマップを表示

#if defined(_DEBUG)
	// 1. 通常の画面（バックブッファ）をターゲットに再設定
	pD3DContext->OMSetRenderTargets(1, &pBackBufferRT, pDepthStencilView);

	// 3. デバッグ用のシェーダーをセット
	pD3DContext->VSSetShader(pDebugVS, nullptr, 0);
	pD3DContext->PSSetShader(pDebugPS, nullptr, 0);

	// 4. シャドウマップ（SRV）と「通常」のサンプラーをスロット0にセット
	// ※ 比較用サンプラー(SampleCmp用)ではなく、通常のSample用サンプラーを渡します
	pD3DContext->PSSetShaderResources(0, 1, &pShadowResourceView);
	pD3DContext->PSSetSamplers(0, 1, &pSamplerState); // 既存の通常のサンプラー

	// 5. 深度テストを無効にする、または「常に描画」にする（一番手前に強制表示するため）
	pD3DContext->OMSetDepthStencilState(pDSState_None, 1); // 既存の書き込みなしステート

	// 6. 頂点バッファなしで、4つの頂点（Triangle Strip）を描画！
	pD3DContext->IASetInputLayout(nullptr);
	pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	pD3DContext->Draw(4, 0);
#endif // _DEBUG

	////////////////////////////////////////////
	// Direct3Dの後始末

	// リソースビューとサンプラーを解除
	ID3D11ShaderResourceView* nullSRV = nullptr;
	pD3DContext->PSSetShaderResources(0, 1, &nullSRV);
	ID3D11SamplerState* nullSS = nullptr;
	pD3DContext->PSSetSamplers(0, 1, &nullSS);

	// 6. D3Dのターゲットを解除（Direct2Dに渡す準備）
	//ID3D11RenderTargetView* nullRT = nullptr;
	pD3DContext->OMSetRenderTargets(1, &nullRT, nullptr);

	// 垂直同期(VSync)を待機して表示
	pSwapChain->Present(1, 0);

	// ウィンドウサイズが変更された場合などのリカバリ
	if (hr == D2DERR_RECREATE_TARGET) {
		ClearDirect2D();
		InitDirect2D(hWnd);
	}

	// 実際のフレームレートを計算
	FrameUpdateCount++;
	int msec = clock() - FrameViewClock;

	if (msec > 0)
		FrameParSec = FrameUpdateCount * 10000 / msec;

	if (msec > 3000) {
		FrameUpdateCount = FrameParSec / 10;
		FrameViewClock = clock() - 1000;
	}
}
#endif	// DIRECT3D_SWAP

#if defined(DIRECT2D_VIEW) && !defined(DIRECT3D_SWAP)
void OnPaint(HWND hWnd) {

	if (pRT == nullptr)
		return;

	// 画面サイズ取得
	RECT rc;
	GetClientRect(hWnd, &rc);

	float screenX = (float)(rc.right - rc.left);
	float screenY = (float)(rc.bottom - rc.top);
	float centerX = screenX / 2.0f;
	float centerY = screenY / 2.0f;
	double ky = cos(M_PI * rotateX);
	double kz = sin(M_PI * rotateX);

	if (!targetLock) {
		CenterPos = posV[bodies[targetIdx].i];
		CenterVel = velV[bodies[targetIdx].i];
	}

	bool bClear = (centerX != SaveCx || centerY != SaveCy || viewZoom != SaveZoom || rotateX != SaveRotateX || rotateY != SaveRotateY || targetIdx != SaveIdx) ? true : false;

	SaveCx = centerX;
	SaveCy = centerY;
	SaveZoom = viewZoom;
	SaveRotateX = rotateX;
	SaveRotateY = rotateY;
	SaveIdx = targetIdx;

	pRT->BeginDraw();
	pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black)); // 背景を黒でクリア

	// 天体の描画ループ
	for (size_t i = 0; i < bodies.size(); i++) {
		if (viewZoom < 3000.0 && (bodies[i].type == TYPE_SATELITE || bodies[i].type == TYPE_SATE_NON))
			continue;

		double tx = (posV[i].x - CenterPos.x) * viewZoom / AU;
		double ty = (posV[i].y - CenterPos.y) * viewZoom / AU;
		double tz = (posV[i].z - CenterPos.z) * viewZoom / AU;
		double tr = radiusV[i] * viewZoom / AU;

		ty = ty * ky - tz * kz;

		float x = centerX + (float)tx;
		float y = centerY + (float)ty;
		float r = (float)(tr);

		if (bodies[i].type != TYPE_ASTEROID && bodies[i].type != TYPE_SATE_NON) {
			// 軌道の描画
			if (bClear)
				bodies[i].plen = 0;

			if (bodies[i].path == nullptr)
				bodies[i].path = new PointF[PATH_MAX];

			bodies[i].path[bodies[i].plen++] = {x, y};

			if (bodies[i].plen >= PATH_MAX) {
				memmove(bodies[i].path + 0, bodies[i].path + 1, sizeof(PointF) * (bodies[i].plen - 1));
				bodies[i].plen--;
			}

			if (bodies[i].plen > 1) {
				D2D1::ColorF col = { GetRByte(colorV[i]) / 511.f, GetGByte(colorV[i]) / 511.f, GetBByte(colorV[i]) / 511.f, GetAByte(colorV[i]) / 255.f };
				for (size_t n = 1; n < bodies[i].plen; n++) {
					if (bodies[i].path[n - 1].X > screenX || bodies[i].path[n - 1].Y > screenY || bodies[i].path[n].X < 0 || bodies[i].path[n].Y < 0)
						continue;
					col.a = (float)n / (float)bodies[i].plen;
					pBrush->SetColor(col);
					pRT->DrawLine(D2D1_POINT_2F(bodies[i].path[n - 1].X, bodies[i].path[n - 1].Y), D2D1_POINT_2F(bodies[i].path[n].X, bodies[i].path[n].Y), pBrush);
				}
			}
		}

		// 画面内判定
		if (x + r < 0 || x - r > screenX || y + r < 0 || y - r > screenY)
			continue;

		// 色の設定（bodiesVから取得、または種類に応じて）
		pBrush->SetColor(D2D1::ColorF(GetRByte(colorV[i]) / 255.f, GetGByte(colorV[i]) / 255.f, GetBByte(colorV[i]) / 255.f, GetAByte(colorV[i]) / 255.f));

		if (r < 1.0f)
			r = 1.0f;

		if (bodies[i].type == TYPE_ASTEROID) {
			// 非常に小さい場合は点として描画（高速）
			//pRT->DrawRectangle(D2D1::RectF(x, y, x + r, y + r), pBrush);
			pRT->FillRectangle(D2D1::RectF(x, y, x + 1.f, y + 1.f), pBrush);
		}
		else {
			// 円の描画
			pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), r, r), pBrush);
		}

		if (pPlanetFormat && bodies[i].type <= TYPE_SATE_NON) {
			pRT->DrawText(
				bodies[i].name.c_str(),
				(UINT32)bodies[i].name.length(),
				pPlanetFormat,
				D2D1::RectF(x + r, y, x + r + 200.f, y + 50.f),
				pBrush
			);
		}
	}

	// Year = 365.2425 * 24 * 60 * 60
	int year = (int)(TotalTime / (365.2425 * 24.0 * 60.0 * 60.0));
	if (year > 0) {
		TotalYear += year;
		TotalTime -= (double)year * (365.2425 * 24.0 * 60.0 * 60.0);
	}

	if (pTextFormat) {
		wstring info;
		info = L"Zoom: " + to_wstring(viewZoom / 300.0).substr(0, 4) + L"x";
		info += L" | Step: " + to_wstring(baseTimeStep * baseTimeDivs / 3600.0).substr(0, 5) + L"h";
		info += L" / " + to_wstring(globalSubSteps);
		info += L" | Fps: " + to_wstring(FrameParSec / 10) + L"." + to_wstring(FrameParSec % 10);
		info += L" | Year: " + to_wstring(TotalYear);
		info += L" | Days: " + to_wstring((int)(TotalTime / (24 * 60 * 60)));
		info += L" | Time: " + to_wstring((int)(TotalTime));

		pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
		pRT->DrawText(
			info.c_str(),
			(UINT32)info.length(),
			pTextFormat,
			D2D1::RectF(10, 10, screenX, 50),
			pBrush
		);
	}

	HRESULT hr = pRT->EndDraw();

	// ウィンドウサイズが変更された場合などのリカバリ
	if (hr == D2DERR_RECREATE_TARGET) {
		ClearDirect2D();
		InitDirect2D(hWnd);
	}

	// 実際のフレームレートを計算
	FrameUpdateCount++;
	int msec = clock() - FrameViewClock;

	if (msec > 0)
		FrameParSec = FrameUpdateCount * 10000 / msec;

	if (msec > 3000) {
		FrameUpdateCount = FrameParSec / 10;
		FrameViewClock = clock() - 1000;
	}
}
#endif	// DIRECT2D_VIEW

#ifdef GDIPLUS_VIEW
// --- ダブルバッファリング描画 ---
void OnPaint(HDC hdc, RECT rect) {
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;

    // メモリDCとビットマップの作成
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(memDC, memBM);

    {
        Graphics g(memDC);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.Clear(Color(255, 5, 5, 16)); // 宇宙の暗い紺色

        if ( !targetLock ) {
            CenterPos = posV[bodies[targetIdx].i];
            CenterVel = velV[bodies[targetIdx].i];
        }

        float centerX = w / 2.0f;
        float centerY = h / 2.0f;

		double cx = CenterPos.x;
		double cy = CenterPos.y * cos(M_PI * (1.0 - rotateX)) - CenterPos.z * sin(M_PI * (1.0 - rotateX));

		double offX = centerX - (cx * viewZoom / AU);
		double offY = centerY - (cy * viewZoom / AU);
        bool bClear = (centerX != SaveCx || centerY != SaveCy || viewZoom != SaveZoom || rotateX != SaveRotateX || rotateY != SaveRotateY || targetIdx != SaveIdx) ? true : false;

        SaveCx = centerX;
        SaveCy = centerY;
        SaveZoom = viewZoom;
        SaveRotateX = rotateX;
		SaveRotateY = rotateY;
        SaveIdx = targetIdx;

        for (auto& b : bodies) {

            if (viewZoom < 1000.0 && (b.type == TYPE_SATELITE || b.type == TYPE_SATE_NON))
                continue;

			double dx, dy;
			dx = posV[b.i].x;
			dy = posV[b.i].y * cos(M_PI * (1.0 - rotateX)) - posV[b.i].z * sin(M_PI * (1.0 - rotateX));

            float x = (float)(dx * viewZoom / AU + offX);
            float y = (float)(dy * viewZoom / AU + offY);

			if (b.type != TYPE_ASTEROID && b.type != TYPE_SATE_NON) {
				// 軌道の描画
				if (bClear)
					b.plen = 0;

				if (b.path == nullptr)
					b.path = new PointF[PATH_MAX];

				b.path[b.plen++] = {x, y};

				if (b.plen >= PATH_MAX) {
					memmove(b.path + 0, b.path + 1, sizeof(PointF) * (b.plen - 1));
					b.plen--;
				}
				if (b.plen > 1) {
					Color col(b.color.GetAlpha(), b.color.GetRed() / 2, b.color.GetGreen() / 2, b.color.GetBlue() / 2);
					Pen p(col, 1);
					g.DrawLines(&p, b.path, (int)b.plen);
				}
			}

            float radius = (float)(radiusV[b.i] * viewZoom / AU);

            if (radius < 1.0)
                radius = 1.0;

			if (b.type == TYPE_ASTEROID) {
#if 1
				#define		ALPHACOL(a, b, c)		(((int)(a) * (int)(c) + (int)(b) * (255 - (int)(c))) / 255)
				COLORREF c = GetPixel(memDC, (int)x, (int)y);
				BYTE a = b.color.GetAlpha();
				c = RGB(ALPHACOL(b.color.GetRed(), GetRValue(c), a), ALPHACOL(b.color.GetGreen(), GetGValue(c), a), ALPHACOL(b.color.GetBlue(), GetBValue(c), a));
				SetPixelV(memDC, (int)x, (int)y, c);
#else
				SolidBrush br(b.color);
				g.FillRectangle(&br, x, y, 1.0, 1.0);
#endif
			} else {
				// 天体
				SolidBrush br(b.color);
				g.FillEllipse(&br, x - radius, y - radius, radius * 2, radius * 2);

				// ラベル
				Font font(L"Comic Sans MS", 9);
				g.DrawString(b.name.c_str(), -1, &font, PointF(x + radius, y), &br);
			}
        }

        // Year = 365.2425 * 24 * 60 * 60
        int year = (int)(TotalTime / (365.2425 * 24.0 * 60.0 * 60.0));
        if (year > 0) {
            TotalYear += year;
            TotalTime -= (double)year * (365.2425 * 24.0 * 60.0 * 60.0);
        }

        // 情報表示
        SolidBrush infoBr(Color::Lime);
        Font infoFont(L"Consolas", 12);
        wstring info;
        info  = L"Zoom: "    + to_wstring(viewZoom / 300.0).substr(0, 4) + L"x";
        info += L" | Step: " + to_wstring(baseTimeStep * baseTimeDivs / 3600.0).substr(0, 5) + L"h";
		info += L" / "       + to_wstring(globalSubSteps);
		info += L" | Fps: "  + to_wstring(FrameParSec / 10) + L"." + to_wstring(FrameParSec % 10);
		info += L" | Year: " + to_wstring(TotalYear);
        info += L" | Days: " + to_wstring((int)(TotalTime / (24 * 60 * 60)));
		info += L" | Time: " + to_wstring((int)(TotalTime));
		g.DrawString(info.c_str(), -1, &infoFont, PointF(10, 10), &infoBr);
    }

    // 表画面に転送
    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

    // 解放
    DeleteObject(memBM);
    DeleteDC(memDC);

	// 実際のフレームレートを計算
	FrameUpdateCount++;
	int msec = clock() - FrameViewClock;

	if (msec > 0)
		FrameParSec = FrameUpdateCount * 10000 / msec;

	if (msec > 3000) {
		FrameUpdateCount = FrameParSec / 10;
		FrameViewClock = clock() - 1000;
	}
}
#endif	// GDIPLUS_VIEW

// --- ウィンドウプロシージャ ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		InitSystem();
#ifndef ONIDLE_CALC
		SetTimer(hWnd, 1, 10000 / FramePerSec, NULL); // 約60fps
#endif // !ONIDLE_CALC
		return 0;
	case WM_TIMER:
		UpdatePhysics();
		InvalidateRect(hWnd, NULL, FALSE); // 再描画要求
		return 0;
	case WM_KEYDOWN:
		switch (wParam) {
#if 0
		case VK_RETURN:
			AddRocket(L"Earth", L"Rocket" + to_wstring(++RocketNum), 1.000e3, AR, 12000, 0.00001, 90, -90, 0, Color::Gold);
			break;
#endif
		case VK_SPACE:
			targetLock = targetLock ? false : true;
			SaveIdx = (-1);
			CenterPos = posV[bodies[targetIdx].i];
			CenterVel = velV[bodies[targetIdx].i];
			break;
		case VK_LEFT:
			if ((rotateY += 0.01) >= 0.5)
				rotateY = 0.49;
			break;
		case VK_RIGHT:
			if ((rotateY -= 0.01) <= -0.5)
				rotateY = -0.49;
			break;
		case VK_UP:
			if ((rotateX += 0.01) >= 0.5)
				rotateX = 0.49;
			break;
		case VK_DOWN:
			if ((rotateX -= 0.01) <= -0.5)
				rotateX = -0.49;
			break;
		case 'N':
			if (++targetIdx >= (int)bodies.size())
				targetIdx = (int)bodies.size() - 1;
			targetLock = false;
			SaveIdx = (-1);
			CenterPos = posV[bodies[targetIdx].i];
			CenterVel = velV[bodies[targetIdx].i];
			break;
		case 'B':
			if (--targetIdx < 0)
				targetIdx = 0;
			targetLock = false;
			SaveIdx = (-1);
			CenterPos = posV[bodies[targetIdx].i];
			CenterVel = velV[bodies[targetIdx].i];
			break;
		case 'G':
			baseTimeStep /= 1.2;
			break;
		case 'T':
			baseTimeStep *= 1.2;
			break;
#if 0
		case 'W': // 前進加速
			for (auto& b : bodies) if (b.type == TYPE_ROKCET) {
				Vector3D thrust = velV[b.i];
				thrust = thrust * (1.0 / thrust.length());
				velV[b.i] += thrust * 100.0; // 100m/s 加速
			}
			break;
#endif
		case 'L':
			LoadFilePos(hWnd);
			break;
		case 'S':
			SaveFilePos(hWnd);
			break;

		case VK_ADD:
		case VK_OEM_PLUS:
			if (worldZoom > 1.0) {
				worldZoom *= 1.1; 
				if (worldZoom > 1.0e2)
					worldZoom = 1.0e2;
				SaveIdx = (-1);
			} else {
				viewZoom *= 1.1; 
				if (viewZoom > 3.0e7) {
					viewZoom = 3.0e7;
					worldZoom *= 1.1;
				}
			}
			break;
		case VK_SUBTRACT:
		case VK_OEM_MINUS:
			if (worldZoom > 1.0) {
				worldZoom /= 1.1;
				if (worldZoom < 1.0)
					worldZoom = 1.0;
				SaveIdx = (-1);
			} else {
				viewZoom /= 1.1;
				if (viewZoom < 3.0)
					viewZoom = 3.0;
			}
			break;
		case '0':
		case VK_NUMPAD0:
			rotateX = -0.35;
			rotateY = 0.0;
			earthEyes = false;
			break;
		case 'E':
			rotateX = 0.0;
			rotateY = 0.0;
			earthEyes = true;
			break;
		}
		return 0;
	case WM_MOUSEWHEEL:
		if (worldZoom > 1.0) {
			if ((short)HIWORD(wParam) > 0)
				worldZoom *= 1.1; 
			else 
				worldZoom /= 1.1;
			if (worldZoom > 1.0e2)
				worldZoom = 1.0e2;
			else if (worldZoom < 1.0)
				worldZoom = 1.0;
			// 軌跡を更新
			SaveIdx = (-1);
		} else {
			if ((short)HIWORD(wParam) > 0)
				viewZoom *= 1.1; 
			else 
				viewZoom /= 1.1;
			if (viewZoom > 3.0e7) {
				viewZoom = 3.0e7;
				worldZoom *= 1.1;
			} else if (viewZoom < 3.0)
				viewZoom = 3.0;
		}
		return 0;

	case WM_LBUTTONDOWN:
		MouseDown = true;
		MousePos.x = GET_X_LPARAM(lParam);
		MousePos.y = GET_Y_LPARAM(lParam);
		MouseRoll = rotateX;
		MouseScll = rotateY;
		break;
	case WM_LBUTTONUP:
		MouseDown = false;
		break;
	case WM_MOUSEMOVE:
		if (MouseDown) {
			rotateY = MouseScll + (double)(MousePos.x - GET_X_LPARAM(lParam)) * 0.002;
			rotateX = MouseRoll + (double)(MousePos.y - GET_Y_LPARAM(lParam)) * 0.002;

			if (rotateY <= -0.5)
				rotateY = -0.49;
			else if (rotateY >= 0.5)
				rotateY = 0.49;

			if (rotateX <= -0.5)
				rotateX = -0.49;
			else if (rotateX >= 0.5)
				rotateX = 0.49;
		}
		break;

	case WM_PAINT:
#ifdef	DIRECT2D_VIEW
		OnPaint(hWnd);
		ValidateRect(hWnd, NULL);
#else
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT rect;
			GetClientRect(hWnd, &rect);
			OnPaint(hdc, rect); // ダブルバッファリング描画へ
			EndPaint(hWnd, &ps);
		} 
#endif	// DIRECT2D_VIEW
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

#if defined(DIRECT3D_SWAP)
	case WM_SIZE:
		if (pD3DContext == nullptr)
			break;
		DefWindowProc(hWnd, message, wParam, lParam);
		ClearDirect2D();
		ClearDirect3D();
		InitD3DAndSwapChain(hWnd);
		InitDirect2D(hWnd);
		InitVertexResources((int)bodies.size());
		return 0;
#elif defined(DIRECT2D_VIEW)
	case WM_SIZE:
		if (pRT) {
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			pRT->Resize(D2D1::SizeU(width, height));
		}
		break;
#endif	// DIRECT3D_SWAP
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

#ifdef ONIDLE_CALC
bool OnIdle(HWND hWnd, int count)
{
	UpdatePhysics();
	InvalidateRect(hWnd, NULL, FALSE); // 再描画要求
	return false;
}
#endif	// ONIDLE_CALC

// --- メイン関数 ---
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {

#ifdef	GDIPLUS_VIEW
	GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif	// GDIPLUS_VIEW

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"SolarSystemClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

	MSG msg = { nullptr, 0, 0, 0 };
    HWND hWnd = CreateWindow(wc.lpszClassName, L"Cute Solar System C++ 2026", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 800, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nShowCmd);

#ifdef	DIRECT2D_VIEW
#ifdef	DIRECT3D_SWAP
	if (FAILED(InitD3DAndSwapChain(hWnd)))
		goto ENDOF;
#endif	// DIRECT3D_SWAP
	if (FAILED(InitDirect2D(hWnd)))
		goto ENDOF;
#endif	// DIRECT2D_VIEW

	FrameViewClock = clock();
	FrameUpdateCount = 0;

#ifdef	ONIDLE_CALC
	for ( ; ; ) {
		bool bIdle = false;
		while ( PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE) ) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_PAINT)
				bIdle = true;
			else if (msg.message == WM_QUIT)
				goto ENDOF;
		}

		for ( int idlecount = 0 ; bIdle || !PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE) ; idlecount++ ) {
			bIdle = FALSE;
			if ( !OnIdle(hWnd, idlecount) )
				break;
		}
	}
#else
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#endif	// ONIDLE_CALC

ENDOF:

#ifdef	DIRECT2D_VIEW
	ClearDirect2D();
#ifdef DIRECT3D_SWAP
	ClearDirect3D();
#endif	// DIRECT3D_SWAP
#endif	// DIRECT2D_VIEW

#ifdef	CUDA_KERNEL
	if (d_pos) cudaFree(d_pos);
	if (d_vel) cudaFree(d_vel);
	if (d_mass) cudaFree(d_mass);
	if (d_radius) cudaFree(d_radius);
	if (d_color) cudaFree(d_color);
	if (d_viewProj) cudaFree(d_viewProj);
#endif	// CUDA_KERNEL

#ifdef	GDIPLUS_VIEW
	GdiplusShutdown(gdiplusToken);
#endif	// GDIPLUS_VIEW

	for (auto& b : bodies) {
#ifdef DIRECT3D_SWAP
		if (b.vpos)
			delete[] b.vpos;
#else
		if (b.path)
			delete[] b.path;
#endif // DIRECT3D_SWAP
	}

	return (int)msg.wParam;
}