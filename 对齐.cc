#include <iostream>
#include <chrono>
#include <stdlib.h>

int main() {
    const int N = 1024;
    const size_t alignment = 16; // Align to 64-byte Cache Line

    float** A = new float*[N];

    // Size (N * 4 bytes) must be a multiple of alignment (64)
    // 1024 * 4 = 4096, which is a multiple of 64.
    size_t row_size = N * sizeof(float);

    for (int i = 0; i < N; i++) {
        A[i] = (float*)aligned_alloc(alignment, row_size);
        if (A[i] == nullptr) {
            std::cerr << "Memory allocation failed at row " << i << std::endl;
            return -1;
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = (float)rand() / RAND_MAX;
        }
        A[i][i] += N; // Prevent pivot from being zero
    }

    std::cout << "Starting [Aligned with aligned_alloc] version (N=" << N << ")..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (int k = 0; k < N; k++) {
        // 除法部分
        for (int j = k + 1; j < N; j++) {
            A[k][j] = A[k][j] / A[k][k];
        }
        A[k][k] = 1.0f;

        // 消去部分
        for (int i = k + 1; i < N; i++) {
            for (int j = k + 1; j < N; j++) {
                // 核心计算：每一行减去 A[k] 行的倍数
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }
            A[i][k] = 0;
        }
    }


    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1000>> elapsed = end - start;

    std::cout << "Calculation finished." << std::endl;
    std::cout << "average latency  : " << elapsed.count() << " (ms)" << std::endl;
    for (int i = 0; i < N; i++) {
        free(A[i]); // Memory allocated with aligned_alloc must be freed with free()
    }
    delete[] A;

    return 0;
}
