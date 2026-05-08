#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm> // 用于 std::min

int main() {
    const int N = 1024;
    const int BLOCK_SIZE = 32; // 分块大小，通常选择能放入 L2 缓存的数值
    
    std::vector<std::vector<float>> A(N, std::vector<float>(N));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = (float)rand() / RAND_MAX;
        }
        A[i][i] += N;
    }

    std::cout << "开始【仅 Cache 分块优化】版本 (N=" << N << ")..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (int k = 0; k < N; k++) {
        // --- 除法部分 (针对当前行) ---
        float pivot = A[k][k];
        for (int j = k + 1; j < N; j++) {
            A[k][j] /= pivot;
        }
        A[k][k] = 1.0f;

        // --- 消去部分：使用 Tiling (分块) 技巧 ---
        // 将剩余的矩阵划分为 BLOCK_SIZE x BLOCK_SIZE 的小块
        for (int i_block = k + 1; i_block < N; i_block += BLOCK_SIZE) {
            for (int j_block = k + 1; j_block < N; j_block += BLOCK_SIZE) {
                
                // 处理当前小块内部的计算
                for (int i = i_block; i < std::min(i_block + BLOCK_SIZE, N); i++) {
                    float factor = A[i][k];
                    for (int j = j_block; j < std::min(j_block + BLOCK_SIZE, N); j++) {
                        // 这里的访问在空间上更集中，有助于提高缓存命中率
                        A[i][j] -= A[k][j] * factor;
                    }
                }
            }
        }
        
        // 将主元下方的列置零
        for (int i = k + 1; i < N; i++) A[i][k] = 0;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1000>> elapsed = end - start;

    std::cout << "计算完成。" << std::endl;
    std::cout << "average latency  : " << elapsed.count() << " (ms)" << std::endl;

    return 0;
}
