
//////////////////////////////////////////////
// シャドウマップの定義

Texture2D<float> ShadowMap			: register(t0); // シャドウマップ
SamplerComparisonState ShadowSampler: register(s0);	// 比較関数付のサンプラー

Texture1D<float4> RingTexture		: register(t1);	// スロット1にリングテクスチャをバインド
SamplerState RingSampler			: register(s1);	// 通常の線形サンプラー

Texture2D<float4> LabelTexture		: register(t2);	// スロット2に文字テクスチャをバインド
SamplerState LabelSampler			: register(s2);	// 通常の線形サンプラー

Texture2D<float4> PlanetTexture		: register(t3);	// スロット3に惑星テクスチャをバインド
SamplerState PlanetSampler			: register(s3);	// 通常の線形サンプラー

//////////////////////////////////////////////
// 小惑星のVS/PS

struct VS_ASTEROID_INPUT {
	float4 Pos : POSITION;
	float4 SPos : SPOSITION;
	float4 Color : COLOR; // uint から float4 に変更
};

struct PS_ASTEROID_INPUT {
	float4 Pos : SV_POSITION;
	float4 SPos : SPOSITION;
	float4 Color : COLOR;
};

// 頂点シェーダーのエントリポイント
PS_ASTEROID_INPUT VS_Asteroid(VS_ASTEROID_INPUT input) {
	PS_ASTEROID_INPUT output;
	output.Pos = input.Pos;
	output.SPos = input.SPos;
	output.Color = input.Color; // そのまま渡す
	return output;
}

// ピクセルシェーダーのエントリポイント
float4 PS_Asteroid(PS_ASTEROID_INPUT input) : SV_Target {
	return input.Color;
}

// Shadow頂点シェーダーのエントリポイント
PS_ASTEROID_INPUT VS_AsteroidShadow(VS_ASTEROID_INPUT input) {
	PS_ASTEROID_INPUT output;
	output.Pos = input.SPos;
	output.SPos = input.Pos;
	output.Color = input.Color; // そのまま渡す
	return output;
}

// Shadowピクセルシェーダーのエントリポイント
float4 PS_AsteroidShadow(PS_ASTEROID_INPUT input) : SV_Target {

	// 同次座標系から正規化デバイス座標へ
	float4 sPos = input.SPos;
	sPos.xyz /= sPos.w;

	// NDC (-1～1) から テクスチャ座標 (0～1) への変換
	float2 texCoord = float2(sPos.x * 0.5f + 0.5f, -sPos.y * 0.5f + 0.5f);

	// 2. SampleCmpLevelZero による自動比較とフィルタリング
	// 引数：(サンプラー, UV座標, 比較したい深度)
	// 戻り値：0.0 (完全に影) ～ 1.0 (完全に光) の間の連続値
	float shadowFactor = ShadowMap.SampleCmpLevelZero(ShadowSampler, texCoord, sPos.z) * 0.5f + 0.5f;

	// アルファを影に合わせる
	input.Color.w *= shadowFactor;

	return input.Color;
}

//////////////////////////////////////////////
// 惑星のVS/PS

// 定数バッファ：C++から更新するデータ
cbuffer ConstantBuffer : register(b0) {
	matrix World;			// 惑星のワールド行列（位置・回転・拡大）
	matrix DispViewProj;	// カメラ投影行列
	matrix LightViewProj;	// 太陽視点の行列
	float3 SunPos;			// 太陽の座標（ワールド空間）
	float padding1;			// アラインメント調整用
	float3 CameraPos;		// カメラの座標（ワールド空間）
	float padding2;			// アラインメント調整用
	float3 PlanetColor;		// 基本色
	float padding3;			// アラインメント調整用
};

struct VS_PLANET_INPUT {
	float3 Pos : POSITION;
	float3 Normal : NORMAL;		// 球体生成時に作る法線データ
	float2 Tex : TEXTURE;		// 球体表面のテクスチャ座標
};

struct PS_PLANET_INPUT {
	float4 Pos : SV_POSITION;
	float3 WPos : TEXCOORD0;	// ワールド空間での座標
	float3 WNormal : TEXCOORD1; // ワールド空間での法線
	float2 Tex : TEXCOORD2;		// テクスチャ座標
};

// 頂点シェーダー
PS_PLANET_INPUT VS_Planet(VS_PLANET_INPUT input) {
	PS_PLANET_INPUT output;
	// 座標変換
	float4 worldPos = mul(float4(input.Pos, 1.0f), World);

	output.WPos = worldPos.xyz;
	output.Pos = mul(worldPos, DispViewProj);

	// 法線をワールド空間へ変換（回転のみ考慮）
	output.WNormal = normalize(mul(input.Normal, (float3x3)World));

	output.Tex = input.Tex;

	return output;
}

//////////////////////////////////////////////
// 単色の惑星のPS

float4 PS_Simple(PS_PLANET_INPUT input) : SV_Target {
	//float3 lightDir = normalize(SunPos - input.WPos);
	//float3 viewDir = normalize(CameraPos - input.WPos); // カメラ方向

	//// 1. シャドウマップ比較
	//// ワールド座標を太陽視点の座標に変換
	//float4 sPos = mul(float4(input.WPos, 1.0f), LightViewProj);
	//sPos.xyz /= sPos.w; // 同次座標系から正規化デバイス座標へ

	//// NDC (-1～1) から テクスチャ座標 (0～1) への変換
	//float2 texCoord = float2(sPos.x * 0.5f + 0.5f, -sPos.y * 0.5f + 0.5f);

	//// 2. SampleCmpLevelZero による自動比較とフィルタリング
	//// 引数：(サンプラー, UV座標, 比較したい深度)
	//// 戻り値：0.0 (完全に影) ～ 1.0 (完全に光) の間の連続値
	//float shadowFactor = ShadowMap.SampleCmpLevelZero(ShadowSampler, texCoord, sPos.z);

	//// 1. 基本のランバート反射
	//float diff = max(0.0, dot(input.WNormal, lightDir)) * shadowFactor;

	//// 2. 環境光（影の部分も真っ暗にしない）
	//float3 ambient = PlanetColor * 0.2f;

	//// 3. 拡散反射（光が当たっている部分の色）
	//float3 diffuse = PlanetColor * diff;

	//// 4. スペキュラ（光沢：水星や氷の衛星などに効果的）
	//float3 halfVec = normalize(lightDir + viewDir);
	//float spec = pow(max(0.0, dot(input.WNormal, halfVec)), 32.0);
	//float3 specular = float3(0.3, 0.3, 0.3) * spec * diff;

	//return float4(ambient + diffuse + specular, 1.0f);

	return float4(PlanetColor, 1.0f);
}

//////////////////////////////////////////////
// テクスチャ付きの惑星のPS

float4 PS_Planet(PS_PLANET_INPUT input) : SV_Target {
	float3 lightDir = normalize(SunPos - input.WPos);
	float3 viewDir = normalize(CameraPos - input.WPos); // カメラ方向

	// 1. シャドウマップ比較
	// ワールド座標を太陽視点の座標に変換
	float4 sPos = mul(float4(input.WPos, 1.0f), LightViewProj);
	sPos.xyz /= sPos.w; // 同次座標系から正規化デバイス座標へ

	// NDC (-1～1) から テクスチャ座標 (0～1) への変換
	float2 texCoord = float2(sPos.x * 0.5f + 0.5f, -sPos.y * 0.5f + 0.5f);

	// 2. SampleCmpLevelZero による自動比較とフィルタリング
	// 引数：(サンプラー, UV座標, 比較したい深度)
	// 戻り値：0.0 (完全に影) ～ 1.0 (完全に光) の間の連続値
	float shadowFactor = ShadowMap.SampleCmpLevelZero(ShadowSampler, texCoord, sPos.z);

	// 1. 基本のランバート反射
	float diff = max(0.0, dot(input.WNormal, lightDir)) * shadowFactor;

	// 惑星のテクスチャからアルファを外して設定
	float4 texColor = PlanetTexture.Sample(PlanetSampler, input.Tex);

	float3 baseColor = texColor.xyz;
	//float3 baseColor = PlanetColor;

	// 2. 環境光（影の部分も真っ暗にしない）
	float3 ambient = baseColor * 0.2f;

	// 3. 拡散反射（光が当たっている部分の色）
	float3 diffuse = baseColor * diff;

	// 4. スペキュラ（光沢：水星や氷の衛星などに効果的）
	float3 halfVec = normalize(lightDir + viewDir);
	float spec = pow(max(0.0, dot(input.WNormal, halfVec)), 32.0);
	float3 specular = float3(0.3, 0.3, 0.3) * spec * diff;

	return float4(ambient + diffuse + specular, 1.0f);
}

//////////////////////////////////////////////
// リムライト付きの惑星のPS

float4 PS_Shadow(PS_PLANET_INPUT input) : SV_Target {
	float3 lightDir = normalize(SunPos - input.WPos);
	float3 viewDir = normalize(CameraPos - input.WPos); // カメラ方向

	// 1. シャドウマップ比較
	// ワールド座標を太陽視点の座標に変換
	float4 sPos = mul(float4(input.WPos, 1.0f), LightViewProj);
	sPos.xyz /= sPos.w; // 同次座標系から正規化デバイス座標へ

	// NDC (-1～1) から テクスチャ座標 (0～1) への変換
	float2 texCoord = float2(sPos.x * 0.5f + 0.5f, -sPos.y * 0.5f + 0.5f);

	// 2. SampleCmpLevelZero による自動比較とフィルタリング
	// 引数：(サンプラー, UV座標, 比較したい深度)
	// 戻り値：0.0 (完全に影) ～ 1.0 (完全に光) の間の連続値
	float shadowFactor = ShadowMap.SampleCmpLevelZero(ShadowSampler, texCoord, sPos.z);

	// 1. 基本のランバート反射
	float diff = max(0.0, dot(input.WNormal, lightDir)) * shadowFactor;

	// 惑星のテクスチャからアルファを外して設定
	float4 texColor = PlanetTexture.Sample(PlanetSampler, input.Tex);

	float3 baseColor = texColor.xyz;
	//float3 baseColor = PlanetColor;

	// 2. 環境光（影の部分も真っ暗にしない）
	float3 ambient = baseColor * 0.2f;

	// 3. 拡散反射（光が当たっている部分の色）
	float3 diffuse = baseColor * diff;

	// =========================================================
	// ✨ 追加：大気レイヤー（リムライト）の計算
	// =========================================================

	float3 normal = normalize(input.WNormal);

	// 1. 視線方向と法線の関係から「球体のフチ（エッジ）」を検出する
	// 正面を向いているときは dot は 1 に近く、真横（フチ）のときは 0 に近くなる
	float vDotN = max(0.0f, dot(viewDir, normal));

	// 反転させて「フチほど 1.0 に近づく」数値を作る
	float rim = 1.0f - vDotN;

	// フチのアルファを落としてぼかす
	float alpha = 1.0f - pow(rim, 8.0f) * 0.7f;

	// 指数関数（pow）を使って、フチの極限だけが鋭く光るように形を整える
	// 4.0〜5.0 くらいにすると、大気の「薄い層」の雰囲気が出ます
	rim = pow(rim, 5.0f);

	// 2. 太陽の光が当たっている側（昼の面）だけ大気を光らせる
	// 影の側（夜の面）は大気も暗くなるように、ライト方向との関係（dot）をマイルドに掛ける
	float atmosphereLight = max(0.0f, dot(normal, lightDir) * 0.5f + 0.5f);

	// 3. 地球の大気の色（美しいシアン・ブルー）を設定
	float3 atmosphereColor = float3(0.2f, 0.6f, 1.0f); // 好みで脚色してください

	// 最終的な大気の輝き
	float3 atmosphere = atmosphereColor * rim * atmosphereLight * 0.8f; // 1.5fは輝度調整


	// 4. スペキュラ（光沢：水星や氷の衛星などに効果的）
	float3 halfVec = normalize(lightDir + viewDir);
	float spec = pow(max(0.0, dot(input.WNormal, halfVec)), 32.0);
	float3 specular = float3(0.3, 0.3, 0.3) * spec * diff;

	//return float4(ambient + diffuse + specular, 1.0f);
	return float4(ambient + diffuse + specular + atmosphere, alpha);
}

//////////////////////////////////////////////
// 土星の環のVS/PS

struct VS_RING_INPUT {
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD0; // 1Dテクスチャ位置
};

struct PS_RING_INPUT {
	float4 Pos : SV_POSITION;
	float3 WPos : TEXCOORD0;  // ワールド空間での座標
	float2 Tex : TEXCOORD1; // 1Dテクスチャ位置
};

// 頂点シェーダー
PS_RING_INPUT VS_Ring(VS_RING_INPUT input) {
	PS_RING_INPUT output;
	// 座標変換
	float4 worldPos = mul(float4(input.Pos, 1.0f), World);

	output.WPos = worldPos.xyz;
	output.Pos = mul(worldPos, DispViewProj);
	output.Tex = input.Tex;

	return output;
}

// リング描画用ピクセルシェーダー
float4 PS_Ring(PS_RING_INPUT input) : SV_Target {

	// 頂点シェーダーから送られてきた UV の X成分（0.0=内側、1.0=外側）を利用して1Dテクスチャを引きにいく
	float4 ringColor = RingTexture.Sample(RingSampler, input.Tex.x);

	// 【魔法の1行】カッシーニの間隙やエンケの間隙など、アルファ（透明度）が0のピクセルは完全に破棄
	if (ringColor.a < 0.05f) {
		discard; 
	}

	return ringColor;
}

// リング描画用影付ピクセルシェーダー
float4 PS_RingShadow(PS_RING_INPUT input) : SV_Target {

	// 頂点シェーダーから送られてきた UV の X成分（0.0=内側、1.0=外側）を利用して1Dテクスチャを引きにいく
	float4 ringColor = RingTexture.Sample(RingSampler, input.Tex.x);

	// 【魔法の1行】カッシーニの間隙やエンケの間隙など、アルファ（透明度）が0のピクセルは完全に破棄
	if (ringColor.a < 0.05f) {
		discard; 
	}

	// 太陽からのライティング計算（現在の惑星のライティングをそのまま適用）
	float3 lightDir = normalize(SunPos - input.WPos);
	float diff = max(0.0, dot(float3(0, 1, 0), lightDir)); // リングの法線は真上(Y軸)向き

	// 1. シャドウマップ比較
	// ワールド座標を太陽視点の座標に変換
	float4 sPos = mul(float4(input.WPos, 1.0f), LightViewProj);
	sPos.xyz /= sPos.w; // 同次座標系から正規化デバイス座標へ

	// NDC (-1～1) から テクスチャ座標 (0～1) への変換
	float2 texCoord = float2(sPos.x * 0.5f + 0.5f, -sPos.y * 0.5f + 0.5f);

	// 影の計算 (SampleCmpLevelZero) 
	float shadowFactor = ShadowMap.SampleCmpLevelZero(ShadowSampler, texCoord, sPos.z);

	// 最終カラーの出力
	float3 finalColor = ringColor.rgb * (diff * shadowFactor * 0.8f + 0.2f);
	return float4(finalColor, ringColor.a);
}

//////////////////////////////////////////////
// 文字ラベルのVS/PS

struct VS_LABEL_INPUT {
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD0; // 1Dテクスチャ位置
};

struct PS_LABEL_INPUT {
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD1; // 1Dテクスチャ位置
};

// 頂点シェーダー
PS_LABEL_INPUT VS_Label(VS_LABEL_INPUT input) {
	PS_LABEL_INPUT output;

	output.Pos = float4(input.Pos, 1.0f);
	output.Tex = input.Tex;
	return output;
}

// 描画用ピクセルシェーダー
float4 PS_Label(PS_LABEL_INPUT input) : SV_Target{
	// 2Dテクスチャの色を引きにいく
	float4 col = LabelTexture.Sample(LabelSampler, input.Tex);
	return col;
}

//////////////////////////////////////////////
// デバッグのVS/PS

struct VS_DEBUG_OUTPUT {
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

// 頂点バッファなしで、画面の隅に四角形を作るトリッキーで便利なVS
VS_DEBUG_OUTPUT VS_Debug(uint vertexID : SV_VertexID) {
	VS_DEBUG_OUTPUT output;

	// 画面の右下に配置する4頂点の座標 (NDC座標系: -1 ～ 1)
	// X: 0.4 ～ 0.95 (右側), Y: -0.95 ～ -0.4 (下側)
	float2 positions[4] = {
		float2(0.4f,  -0.4f),  // 右上
		float2(0.95f, -0.4f),  // 左上
		float2(0.4f,  -0.95f), // 右下
		float2(0.95f, -0.95f)  // 左下
	};

	float2 texCoords[4] = {
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f),
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f)
	};

	output.Pos = float4(positions[vertexID], 0.0f, 1.0f);
	output.Tex = texCoords[vertexID];
	return output;
}

float4 PS_Debug(VS_DEBUG_OUTPUT input) : SV_Target {
	// Sample ではなく SampleCmpLevelZero を使う
	// 第3引数に 0.99f を渡すことで、深度が 0.99 より手前にある天体が「黒(0.0)」、宇宙の虚空が「白(1.0)」で映ります
	float shadowVis = ShadowMap.SampleCmpLevelZero(ShadowSampler, input.Tex, 0.99f);
	//float shadowVis = DebugShadowMap.Sample(DefaultSampler, input.Tex);

	return float4(shadowVis, shadowVis, shadowVis, 1.0f);
}