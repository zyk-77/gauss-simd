#include <iostream>
#include <vector>
#include <chrono>
#include <arm_neon.h> // 仅用于除法部分
#include <stdlib.h>

int main() {
    const int N = 1024;
    // 使用连续的一维数组，方便除法部分加载向量
    float* A = (float*)malloc(N * N * sizeof(float));

    // 随机初始化
    for (int i = 0; i < N * N; i++) {
        A[i] = (float)rand() / RAND_MAX;
    }
    for (int i = 0; i < N; i++) A[i * N + i] += N;

    std::cout << "开始【仅除法部分 SIMD 优化】版本 (N=" << N << ")..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    for (int k = 0; k < N; k++) {
        // ======= 1. 除法部分：使用 SIMD (NEON) 优化 =======
        float pivot = A[k * N + k];
        float32x4_t vt = vdupq_n_f32(pivot); // 将主元广播到向量
        
        int j = k + 1;
        // 向量化执行 A[k][j] / pivot
        for (; j + 4 <= N; j += 4) {
            float32x4_t va = vld1q_f32(&A[k * N + j]);
            va = vdivq_f32(va, vt);
            vst1q_f32(&A[k * N + j], va);
        }
        // 处理末尾剩余元素
        for (; j < N; j++) {
            A[k * N + j] /= pivot;
        }
        A[k * N + k] = 1.0f;

        // ======= 2. 消去部分：保持原始 C++ 循环 (无优化) =======
        for (int i = k + 1; i < N; i++) {
            float factor = A[i * N + k];
            for (int jj = k + 1; jj < N; jj++) {
                // 这里不使用 SIMD，逐个计算
                A[i * N + jj] -= A[k * N + jj] * factor;
            }
            A[i * N + k] = 0;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1000>> elapsed = end - start;

    std::cout << "计算完成。" << std::endl;
    std::cout << "average latency  : " << elapsed.count() << " (ms)" << std::endl;

    free(A);
    return 0;
}
