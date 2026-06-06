
#include "SolarSim.h"

#ifdef CUDA_KERNEL

// 物理演算用構造体 (GPUでも使用可能なように調整)
struct Vec3 {
    double x, y, z;
};

__device__ Vec3 operator+(Vec3 a, Vec3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
__device__ Vec3 operator-(Vec3 a, Vec3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
__device__ Vec3 operator*(Vec3 a, double b) { return { a.x * b, a.y * b, a.z * b }; }

// 加速度計算カーネル (N-Bodyシミュレーション)
__global__ void computeGravityKernel(Vec3* pos, Vec3* vel, double* mass, double* radius, int numBodies, double dt) 
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= numBodies) return;

    Vec3 acc = { 0, 0, 0 };
    Vec3 p_i = pos[i];

    for (int j = 0; j < numBodies; j++) {
        if (i == j) continue;
        
        // 質量ゼロ（環の粒子など）は他者に重力を与えない
        if (mass[j] <= 0.0) break;

        Vec3 diff = pos[j] - p_i;
        double distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
        double dist = sqrt(distSq);

        // 衝突判定（簡易）
        double minDist = radius[i] + radius[j];
        if (dist < minDist) dist = minDist;

        double force = (G * mass[j]) / (dist * dist * dist);
        acc = acc + (diff * force);
    }

    // 速度と位置の更新 (オイラー法)
    vel[i].x += acc.x * dt;
    vel[i].y += acc.y * dt;
    vel[i].z += acc.z * dt;

    pos[i].x += vel[i].x * dt;
    pos[i].y += vel[i].y * dt;
    pos[i].z += vel[i].z * dt;
}
extern "C" void launchCudaPhysics(void* pos, void* vel, double* mass, double* radius, int numBodies, double dt) 
{
    int threadsPerBlock = 256;
    int blocksPerGrid = (numBodies + threadsPerBlock - 1) / threadsPerBlock;

    computeGravityKernel<<<blocksPerGrid, threadsPerBlock>>>(
        (Vec3*)pos, (Vec3*)vel, mass, radius, numBodies, dt);
    
    cudaDeviceSynchronize();
}

// 4次ルンゲ＝クッタ法
__global__ void CudaAccsK1(Vec3* pos, Vec3* vel, double* mass, double* radius, int numBodies, double dt, Vec3* k1v, Vec3* k1a, Vec3* k2v, Vec3* k2p)  {
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i >= numBodies) return;

	k1v[i] = vel[i];
	k1a[i] = { 0, 0, 0 };

	for (int n = 0 ; n < numBodies ; n++ ) {
		if (i == n ) continue;
		if (mass[n] <= 0.0) break;

		Vec3 diff = pos[n] - pos[i];
		double distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		double dist = sqrt(distSq);

		if (dist < (radius[i] + radius[n])) {
			dist = radius[i] + radius[n];
			distSq = dist * dist;
		}

		double force = (G * mass[n]) / (distSq * dist);
		k1a[i] = k1a[i] + (diff * force);
	}

	k2p[i] = pos[i] + (k1v[i] * (dt * 0.5));
	k2v[i] = vel[i] + (k1a[i] * (dt * 0.5));
}
__global__ void CudaAccsK2(Vec3* pos, Vec3* vel, double* mass, double* radius, int numBodies, double dt, Vec3* k2v, Vec3* k2a, Vec3* k2p, Vec3* k3v, Vec3* k3p)  {
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i >= numBodies) return;

	k2a[i] = { 0, 0, 0 };

	for (int n = 0 ; n < numBodies ; n++ ) {
		if (i == n ) continue;
		if (mass[n] <= 0.0) break;

		Vec3 diff = k2p[n] - k2p[i];
		double distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		double dist = sqrt(distSq);

		if (dist < (radius[i] + radius[n])) {
			dist = radius[i] + radius[n];
			distSq = dist * dist;
		}

		double force = (G * mass[n]) / (distSq * dist);
		k2a[i] = k2a[i] + (diff * force);
	}

	k3p[i] = pos[i] + (k2v[i] * (dt * 0.5));
	k3v[i] = vel[i] + (k2a[i] * (dt * 0.5));
}
__global__ void CudaAccsK3(Vec3* pos, Vec3* vel, double* mass, double* radius, int numBodies, double dt, Vec3* k3v, Vec3* k3a, Vec3* k3p, Vec3* k4v, Vec3* k4p)  {
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i >= numBodies) return;

	k3a[i] = { 0, 0, 0 };

	for (int n = 0 ; n < numBodies ; n++ ) {
		if (i == n ) continue;
		if (mass[n] <= 0.0) break;

		Vec3 diff = k3p[n] - k3p[i];
		double distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		double dist = sqrt(distSq);

		if (dist < (radius[i] + radius[n])) {
			dist = radius[i] + radius[n];
			distSq = dist * dist;
		}

		double force = (G * mass[n]) / (distSq * dist);
		k3a[i] = k3a[i] + (diff * force);
	}

	k4p[i] = pos[i] + (k3v[i] * dt);
	k4v[i] = vel[i] + (k3a[i] * dt);
}
__global__ void CudaAccsK4(Vec3* pos, Vec3* vel, double* mass, double* radius, int numBodies, double dt, Vec3* k4v, Vec3* k4a, Vec3* k4p, Vec3* k1v, Vec3* k2v, Vec3* k3v, Vec3* k1a, Vec3* k2a, Vec3* k3a)  {
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i >= numBodies) return;

	k4a[i] = { 0, 0, 0 };

	for (int n = 0 ; n < numBodies ; n++ ) {
		if (i == n ) continue;
		if (mass[n] <= 0.0) break;

		Vec3 diff = k4p[n] - k4p[i];
		double distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		double dist = sqrt(distSq);

		if (dist < (radius[i] + radius[n])) {
			dist = radius[i] + radius[n];
			distSq = dist * dist;
		}

		double force = (G * mass[n]) / (distSq * dist);
		k4a[i] = k4a[i] + (diff * force);
	}

	pos[i] = pos[i] + (k1v[i] + k2v[i] * 2.0 + k3v[i] * 2.0 + k4v[i]) * (dt / 6.0);
	vel[i] = vel[i] + (k1a[i] + k2a[i] * 2.0 + k3a[i] * 2.0 + k4a[i]) * (dt / 6.0);
}
extern "C" void CudaUpdateOrbits(void* pos, void* vel, double* mass, double* radius, int numBodies, double dt) {
	static Vec3 *k1v = nullptr, *k1a = nullptr, *k2v = nullptr, *k2a = nullptr, *k2p = nullptr, *k3v = nullptr, *k3a = nullptr, *k3p = nullptr, *k4v = nullptr, *k4a = nullptr, *k4p = nullptr;

	if (k1v == nullptr) {
		cudaMalloc(&k1v, numBodies * sizeof(Vec3));
		cudaMalloc(&k1a, numBodies * sizeof(Vec3));
		cudaMalloc(&k2v, numBodies * sizeof(Vec3));
		cudaMalloc(&k2a, numBodies * sizeof(Vec3));
		cudaMalloc(&k2p, numBodies * sizeof(Vec3));
		cudaMalloc(&k3v, numBodies * sizeof(Vec3));
		cudaMalloc(&k3a, numBodies * sizeof(Vec3));
		cudaMalloc(&k3p, numBodies * sizeof(Vec3));
		cudaMalloc(&k4v, numBodies * sizeof(Vec3));
		cudaMalloc(&k4a, numBodies * sizeof(Vec3));
		cudaMalloc(&k4p, numBodies * sizeof(Vec3));
	}

	int threadsPerBlock = 256;
	int blocksPerGrid = (numBodies + threadsPerBlock - 1) / threadsPerBlock;

	CudaAccsK1<<<blocksPerGrid, threadsPerBlock>>>((Vec3*)pos, (Vec3*)vel, mass, radius, numBodies, dt, k1v, k1a, k2v, k2p);
	CudaAccsK2<<<blocksPerGrid, threadsPerBlock>>>((Vec3*)pos, (Vec3*)vel, mass, radius, numBodies, dt, k2v, k2a, k2p, k3v, k3p);
	CudaAccsK3<<<blocksPerGrid, threadsPerBlock>>>((Vec3*)pos, (Vec3*)vel, mass, radius, numBodies, dt, k3v, k3a, k3p, k4v, k4p);
	CudaAccsK4<<<blocksPerGrid, threadsPerBlock>>>((Vec3*)pos, (Vec3*)vel, mass, radius, numBodies, dt, k4v, k4a, k4p, k1v, k2v, k3v, k1a, k2a, k3a);

	cudaDeviceSynchronize();
}
#endif	// CUDA_KERNEL

#ifdef DIRECT3D_SWAP

struct Vertex {
	float4 pos;
	float4 spos;
	uint32_t color;
};

extern cudaGraphicsResource_t cudaVBResource;

__global__ void convertToVertexKernel(Vec3* pos, Vec3* vel, double* mass, uint32_t* col, Vertex* vbo, int n, float* m, Vec3 center) {
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i >= n) return;

	float x = (float)((pos[i].x - center.x) / AU);
	float y = (float)((pos[i].y - center.y) / AU);
	float z = (float)((pos[i].z - center.z) / AU);

	float4 clipPos = {
		m[0] * x + m[4] * y + m[8]  * z + m[12],
		m[1] * x + m[5] * y + m[9]  * z + m[13],
		m[2] * x + m[6] * y + m[10] * z + m[14],
		m[3] * x + m[7] * y + m[11] * z + m[15]
	};

	// カメラ後方のチェック
	if (clipPos.w > 0.001f && mass[i] == 0.0)
		vbo[i].pos = clipPos;
	else
		vbo[i].pos.z = 2.0f;	// 画面外（カメラ後方）に飛ばして描画されないようにする

	clipPos = {
		m[16 + 0] * x + m[16 + 4] * y + m[16 + 8]  * z + m[16 + 12],
		m[16 + 1] * x + m[16 + 5] * y + m[16 + 9]  * z + m[16 + 13],
		m[16 + 2] * x + m[16 + 6] * y + m[16 + 10] * z + m[16 + 14],
		m[16 + 3] * x + m[16 + 7] * y + m[16 + 11] * z + m[16 + 15]
	};
	vbo[i].spos = clipPos;

	vbo[i].color = col[i];
}
extern "C" void mapAndWriteVertices(void* d_pos, void* d_vel, void* d_mass, void* d_color, int numBodies, float* d_m, Vec3 center) {

	Vertex* d_vbo_ptr = nullptr;
	size_t size;

	// CUDAでバッファをロック（マップ）
	if (cudaGraphicsMapResources(1, &cudaVBResource, 0) == cudaSuccess) {
		if (cudaGraphicsResourceGetMappedPointer((void**)&d_vbo_ptr, &size, cudaVBResource) == cudaSuccess) {
			int threadsPerBlock = 256;
			int blocksPerGrid = (numBodies + threadsPerBlock - 1) / threadsPerBlock;

			convertToVertexKernel << <blocksPerGrid, threadsPerBlock >> > ((Vec3*)d_pos, (Vec3*)d_vel, (double*)d_mass, (uint32_t*)d_color, d_vbo_ptr, numBodies, d_m, center);
		}
		// アンマップ（Direct3Dへ返す）
		cudaGraphicsUnmapResources(1, &cudaVBResource, 0);
	}
}
#endif	// DIRECT3D_SWAP

