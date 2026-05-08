#include <iostream>
#include <chrono>
#include <arm_neon.h> // ARM 平台 SIMD
#include <stdlib.h>

int main() {
    const int N = 1024;
    // 使用 aligned_alloc 确保内存对齐，防止 vld 指令崩溃
    float* A = (float*)aligned_alloc(64, N * N * sizeof(float));

    // 初始化
    for (int i = 0; i < N * N; i++) A[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < N; i++) A[i * N + i] += N;

    std::cout << "开始【仅消去部分 SIMD 优化】版本 (N=" << N << ")..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    for (int k = 0; k < N; k++) {
        // ======= 1. 除法部分：普通 C++ 循环 (无优化) =======
        float pivot = A[k * N + k];
        for (int j = k + 1; j < N; j++) {
            A[k * N + j] /= pivot;
        }
        A[k * N + k] = 1.0f;

        // ======= 2. 消去部分：使用 SIMD (NEON) 优化 =======
        for (int i = k + 1; i < N; i++) {
            // 将 A[i][k] 广播到向量中，方便后续乘法
            float32x4_t vaik = vdupq_n_f32(A[i * N + k]);
            
            int jj = k + 1;
            // 向量化执行：A[i][jj] = A[i][jj] - A[k][jj] * A[i][k]
            for (; jj + 4 <= N; jj += 4) {
                float32x4_t vakj = vld1q_f32(&A[k * N + jj]);
                float32x4_t vaij = vld1q_f32(&A[i * N + jj]);
                
                // 乘法并减法：vaij = vaij - (vakj * vaik)
                float32x4_t res = vmlsq_f32(vaij, vakj, vaik);
                
                vst1q_f32(&A[i * N + jj], res);
            }
            
            // 处理末尾剩余元素
            for (; jj < N; jj++) {
                A[i * N + jj] -= A[k * N + jj] * A[i * N + k];
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
