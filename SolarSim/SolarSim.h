#pragma once

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <time.h>
#include <tchar.h>

#include <vector>
#include <string>
#include <algorithm>
#include <execution>

#define _USE_MATH_DEFINES
#include <math.h>

#include <gdiplus.h>

using namespace Gdiplus;
using namespace std;

///////////////////////////////////////////////////

//#define	GDIPLUS_VIEW		// GDI+を使ったスタンダードな描画
//#define	DIRECT2D_VIEW		// Direct2Dを使った描画
#define	DIRECT3D_SWAP			// Direct3D/2Dを使った描画でCUDAが使えるなら使う

//#define	ASTEROID_SHADOW		// 小惑星の影を惑星に映す
//#define	SATERITE_SHADOW		// 惑星の影を小惑星に映す

#define	RK4_TEST				// 4次ルンゲ＝クッタ法を使った軌道計算
//#define	CUDA_KERNEL			// CUDAを使った軌道計算と3D描画支援
//#define	NOT_CUDA_CALC		// 軌道計算にCUDAを使用しない
//#define	MULTI_THREAD		// CPUでのマルチスレッド軌道計算
#define	ONIDLE_CALC				// OnIdle時に軌道計算する

//#define	ROCKET_VIEW			// 月を回るロケット
//#define	SATELITE			// 各惑星の衛星を表示
//#define	ASTEROID_BELT		// 火星と木星間のアステロイドベルトを表示
//#define	TROJAN_ASTEROIDS	// 木星軌道のアステロイドベルトを表示
//#define	SATURN_RINGS		// 土星の環を表示

//#define	ASTEROID_VIEW		// 小惑星を描画する
//#define	RING_TEXTURE		// 木星の環をテクスチャで描画

#define	POSFILENAME		L"D:\\Temp\\SolarSim.dat"

///////////////////////////////////////////////////

#ifndef SATELITE
	#if defined(ROCKET_VIEW) || defined(SATURN_RINGS) || (!defined(ASTEROID_BELT) && !defined(TROJAN_ASTEROIDS))
		#define	SATELITE
	#endif
#endif

#if	defined(DIRECT3D_SWAP)
	#if !defined(CUDA_KERNEL)
		#define	CUDA_KERNEL
	#endif
	#if	!defined(DIRECT2D_VIEW)
		#define	DIRECT2D_VIEW
	#endif
#endif

#if (defined(ASTEROID_BELT) || defined(TROJAN_ASTEROIDS) || defined(SATURN_RINGS))
	#if !defined(MULTI_THREAD)
		#define	MULTI_THREAD
	#endif
	#if !defined(ASTEROID_VIEW)
		#define	ASTEROID_VIEW
	#endif
#else
	#if !defined(RING_TEXTURE)
		#define	RING_TEXTURE
	#endif
#endif

#if defined(DIRECT2D_VIEW)
	#ifdef GDIPLUS_VIEW
		#undef GDIPLUS_VIEW
	#endif
#else
	#ifndef GDIPLUS_VIEW
		#define GDIPLUS_VIEW
	#endif
#endif

#if defined(ROCKET_VIEW) && defined(RK4_TEST)
	#undef	RK4_TEST
#endif

#ifdef SATURN_RINGS
	//#define	ASTEROID_SHADOW
	#define	SATERITE_SHADOW
	//#define RING_TEXTURE
#endif

///////////////////////////////////////////////////

#ifdef	CUDA_KERNEL
	#pragma warning(disable : 4819)

	#include <cuda_runtime.h>
	#include <device_launch_parameters.h>
	#include <cuda_d3d11_interop.h>

	#pragma comment(lib, "cudart.lib")
#endif	// CUDA_KERNEL

#ifdef	DIRECT2D_VIEW
	#ifdef	DIRECT3D_SWAP
		#include <d2d1_1.h>
		#include <d2d1helper.h>
		#include <dwrite.h>
		#include <d3d11.h>
		#include <dxgi1_2.h>
		#include <d3dcompiler.h>
		#include<directxmath.h>

		#pragma comment(lib, "d2d1.lib")
		#pragma comment(lib, "dwrite.lib")
		#pragma comment(lib, "d3d11.lib")
		#pragma comment(lib, "dxgi.lib")
		#pragma comment(lib, "D3DCompiler.lib")

		//	#include <wrl/client.h>
		//	using Microsoft::WRL::ComPtr;
		using namespace DirectX;
	#else
		#include <d2d1.h>
		#include <d2d1helper.h>
		#include <dwrite.h>

		#pragma comment(lib, "d2d1.lib")
		#pragma comment(lib, "dwrite.lib")
	#endif	// DIRECT3D_SWAP
#endif	// DIRECT2D_VIEW

#ifdef GDIPLUS_VIEW
	#pragma comment (lib, "gdiplus.lib")
#endif	// GDIPLUS_VIEW

#ifdef _DEBUG
#   define TRACE( str, ... ) \
      { \
        TCHAR c[512]; \
        _stprintf_s( c, str, __VA_ARGS__ ); \
        OutputDebugString( c ); \
      }
#else
#    define TRACE( str, ... ) // 空実装
#endif	// _DEBUG

#define	G		6.67430e-11
#define	AU		1.495978707e11		// 1 au ＝ 149,597,870,700メートル

// --- 物理演算用構造体 ---

struct Vector3D {
	double x, y, z;

	Vector3D operator+(const Vector3D& b) const { return { x + b.x, y + b.y, z + b.z }; }
	Vector3D operator-(const Vector3D& b) const { return { x - b.x, y - b.y, z - b.z }; }
	Vector3D operator*(double b) const { return { x * b, y * b, z * b }; }
	Vector3D operator/(double b) const { return { x / b, y / b, z / b }; }
	Vector3D& operator+=(const Vector3D& b) { x += b.x; y += b.y; z += b.z;  return *this; }
	Vector3D& operator-=(const Vector3D& b) { x -= b.x; y -= b.y; z -= b.z;  return *this; }
	Vector3D& operator*=(double b) { x *= b; y *= b; z *= b;  return *this; }
	Vector3D& operator/=(double b) { x /= b; y /= b; z *= b;  return *this; }
	double length() const { return sqrt(x * x + y * y + z * z); }
	void Normalize() { *this = *this * (1.0 / length()); }
	Vector3D rotateX(double a) { return { x, y * cos(a) - z * sin(a), y * sin(a) + z * cos(a) }; }
	Vector3D rotateY(double b) { return { x * cos(b) + z * sin(b), y, -x * sin(b) + z * cos(b) }; }
	Vector3D rotateZ(double r) { return { x * cos(r) - y * sin(r), x * sin(r) + y * cos(r), z }; }
	void Angle(double a, double b, double r) { *this = rotateZ(r * M_PI / 180.0); *this = rotateY(b * M_PI / 180.0); *this = rotateX(a * M_PI / 180.0); }
};

struct Booster {
	double time;
	int target;
	double z, a;
	double speed;
};

#define	TYPE_PLANET		0
#define	TYPE_ROKCET		1
#define	TYPE_SATELITE	2
#define	TYPE_SATE_NON	3
#define	TYPE_ASTEROID	4

#define	ColorToRGBA(c)		(uint32_t)(c.GetAlpha() << 24 | c.GetBlue() << 16 | c.GetGreen() << 8 | c.GetRed())
#define	GetRByte(c)			((c) & 0xFF)
#define	GetGByte(c)			(((c) >>  8) & 0xFF)
#define	GetBByte(c)			(((c) >> 16) & 0xFF)
#define	GetAByte(c)			(((c) >> 24) & 0xFF)

#define PATH_MAX			500

struct Body {
	wstring name;
	size_t i;
	int type;
	Color color;
	int base;
	double obliquity;
	double rotation;
	double obli_rot;
	double period;

#ifdef	DIRECT3D_SWAP
	int vlen;
	ID3D11Buffer* vertex;
	XMVECTOR* vpos = nullptr;
	ID3D11Texture2D* pLabelTexture = nullptr;
	ID3D11ShaderResourceView* pLabelSRV = nullptr;
	ID2D1Bitmap1* pLabelBitmap = nullptr;
	ID3D11Texture2D* pPlanetTexture = nullptr;
	ID3D11ShaderResourceView* pPlanetSRV = nullptr;
#else
	int plen;
	PointF* path = nullptr;
#endif // DIRECT3D_SWAP

	vector<size_t> satelite;
	vector<Booster> boost;

	//Vector3D & pos() { return posV[i]; }
	//Vector3D & vel() { return velV[i]; }
	//double & mass() { return massV[i]; }
	//double & radius() { return radiusV[i]; }
	//uint32_t & color() { return colorV[i]; }
};

