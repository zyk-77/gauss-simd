#include <iostream>
#include <vector>
#include <chrono>
#include <random>
using namespace std;
int main() {
    const int N = 1024; // 矩阵规模
    vector<vector<float>> A(N, vector<float>(N));
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dis(1.0f, 10.0f);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = dis(gen);
        }
    }

    cout << "开始计算规模为 " << N << "x" << N << " 的普通高斯消元..." << endl;
    auto start = chrono::high_resolution_clock::now();

    // --- 标准高斯消元算法 (无优化版) ---
    for (int k = 0; k < N; k++) {
        // 除法部分：将当前行的主元归一
        for (int j = k + 1; j < N; j++) {
            A[k][j] = A[k][j] / A[k][k];
        }
        A[k][k] = 1.0f;

        // 消去部分：将其下方所有行的第 k 列变为 0
        for (int i = k + 1; i < N; i++) {
            for (int j = k + 1; j < N; j++) {
                // 核心计算：每一行减去 A[k] 行的倍数
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }
            A[i][k] = 0;
        }
    }
    // --- 算法结束 ---
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, ratio<1, 1000>> elapsed = end - start;

    cout << "计算完成。" << endl;
    cout << "average latency  : " << elapsed.count() << " (ms) " << endl;

    return 0;
}
