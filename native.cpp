#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <windows.h> // Windows 高精度计时器

// 为了防止栈溢出，将大数组定义在全局区域
const int N = 1024;
float m[N][N];

void reset_matrix() {
    // 简单的随机初始化
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            m[i][j] = (float)rand() / RAND_MAX;
        }
        m[i][i] += 10.0f; // 保证主元非 0
    }
}

int main() {
    // 1. 初始化矩阵
    reset_matrix();
    printf("Matrix initialized (N=%d). Starting computation...\n", N);

    // 2. 准备 Windows 计时器
    LARGE_INTEGER freq;
    LARGE_INTEGER start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    // 3. 标准高斯消元 (无任何优化的串行版本)
    for (int k = 0; k < N; k++) {
        // 除法部分
        for (int j = k + 1; j < N; j++) {
            m[k][j] = m[k][j] / m[k][k];
        }
        m[k][k] = 1.0f;

        // 消去部分
        for (int i = k + 1; i < N; i++) {
            for (int j = k + 1; j < N; j++) {
                m[i][j] = m[i][j] - m[i][k] * m[k][j];
            }
            m[i][k] = 0.0f;
        }
    }

    // 4. 计时结束
    QueryPerformanceCounter(&end);
    double msec = (double)(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;

    printf("Computation finished.\n");
    printf("average latency  : %f (ms)\n", msec);

    // 防止命令行窗口一闪而过
    system("pause");

    return 0;
}
