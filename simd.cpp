#include <iostream>
#include <immintrin.h> // SSE 指令集
#include <stdlib.h>    // malloc/free
#include <windows.h>   // Windows 高精度计时

int main() {
    const int N = 1024;
    
    // Windows 下专用的对齐内存申请 (对齐到 16 字节)
    float* A = (float*)_aligned_malloc(N * N * sizeof(float), 16);
    if (A == NULL) return -1;

    // 初始化
    for (int i = 0; i < N * N; i++) A[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < N; i++) A[i * N + i] += N;

    // --- Windows 高精度计时开始 ---
    LARGE_INTEGER nFreq, nBeginTime, nEndTime;
    QueryPerformanceFrequency(&nFreq);
    QueryPerformanceCounter(&nBeginTime);

    // --- SSE 核心逻辑 ---
    for (int k = 0; k < N; k++) {
        __m128 vt = _mm_set1_ps(A[k * N + k]);
        int j = k + 1;
        for (; j + 4 <= N; j += 4) {
            __m128 va = _mm_loadu_ps(&A[k * N + j]);
            va = _mm_div_ps(va, vt);
            _mm_storeu_ps(&A[k * N + j], va);
        }
        for (; j < N; j++) A[k * N + j] /= A[k * N + k];
        A[k * N + k] = 1.0f;

        for (int i = k + 1; i < N; i++) {
            __m128 vaik = _mm_set1_ps(A[i * N + k]);
            int jj = k + 1;
            for (; jj + 4 <= N; jj += 4) {
                __m128 vakj = _mm_loadu_ps(&A[k * N + jj]);
                __m128 vaij = _mm_loadu_ps(&A[i * N + jj]);
                __m128 vx = _mm_mul_ps(vakj, vaik);
                vaij = _mm_sub_ps(vaij, vx);
                _mm_storeu_ps(&A[i * N + jj], vaij);
            }
            for (; jj < N; jj++) A[i * N + jj] -= A[k * N + jj] * A[i * N + k];
            A[i * N + k] = 0.0f;
        }
    }

    // --- 计时结束 ---
    QueryPerformanceCounter(&nEndTime);
    double msec = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) * 1000 / nFreq.QuadPart;

    std::cout << "Average Latency: " << msec << " (ms)" << std::endl;

    _aligned_free(A); // Windows 专用释放
    return 0;
}
