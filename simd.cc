#include <iostream>
#include <vector>
#include <chrono>
#include <arm_neon.h> // ARM 平台 SIMD 头文件
#include <stdlib.h>

int main() {
    const int N = 1024;
    size_t size = N * N * sizeof(float);
    
    float* A;
    if (posix_memalign((void**)&A, 64, size) != 0) {
        return -1;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = (float)rand() / RAND_MAX;
        }
        A[i * N + i] += N; // 保证主元不为 0，防止除以 0 崩溃
    }

    std::cout << "开始 SIMD 优化版高斯消元 (N=" << N << ")..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

   for (int k = 0; k < N; k++) {
        // --- 除法部分 ---
        float32x4_t vt = vdupq_n_f32(A[k * N + k]);
        int j = k + 1;
        // SIMD 加速
        for (; j + 4 <= N; j += 4) {
            float32x4_t va = vld1q_f32(&A[k * N + j]);
            va = vdivq_f32(va, vt);
            vst1q_f32(&A[k * N + j], va);
        }
        // 处理不满足 4 个的剩余元素
        for (; j < N; j++) {
            A[k * N + j] /= A[k * N + k];
        }
        A[k * N + k] = 1.0f;

        // --- 消去部分 ---
        for (int i = k + 1; i < N; i++) {
            float32x4_t vaik = vdupq_n_f32(A[i * N + k]);
            int jj = k + 1;
            // SIMD 加速
            for (; jj + 4 <= N; jj += 4) {
                float32x4_t vakj = vld1q_f32(&A[k * N + jj]);
                float32x4_t vaij = vld1q_f32(&A[i * N + jj]);
                // Fused Multiply-Subtract: vaij = vaij - (vakj * vaik)
                float32x4_t res = vmlsq_f32(vaij, vakj, vaik);
                vst1q_f32(&A[i * N + jj], res);
            }
            // 处理剩余元素
            for (; jj < N; jj++) {
                A[i * N + jj] -= A[k * N + jj] * A[i * N + k];
            }
            A[i * N + k] = 0;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1000>> elapsed = end - start;

    std::cout << "耗时: " << elapsed.count() << " (ms)" << std::endl;

    free(A);
    return 0;
}
