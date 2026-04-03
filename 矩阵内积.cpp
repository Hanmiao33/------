#include <iostream>   // C++ 标准输入输出
#include <vector>     // C++ 动态数组（替代C语言裸指针）
#include <cmath>      // 浮点绝对值函数
#include <windows.h>  // Windows 高精度计时（必需保留）

// 纯C++：使用标准命名空间，简化代码
using namespace std;

// Windows QueryPerformance 高精度计时（C++封装，返回秒）
double get_time() {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return static_cast<double>(count.QuadPart) / freq.QuadPart;
}

// 生成测试数据：C++ vector传引用，纯C++风格
void generate_data(vector<double>& A, vector<double>& a, int n) {
    for (int j = 0; j < n; j++) {
        a[j] = static_cast<double>(j);
        for (int i = 0; i < n; i++) {
            A[j * n + i] = static_cast<double>(j + i);
        }
    }
}

// 1. 平凡算法：逐列访问，Cache不友好
void naive_inner_product(const vector<double>& A, const vector<double>& a, vector<double>& sum, int n) {
    for (int i = 0; i < n; i++) {
        sum[i] = 0.0;
        for (int j = 0; j < n; j++) {
            sum[i] += A[j * n + i] * a[j];
        }
    }
}

// 2. Cache优化算法：逐行访问，空间局部性优
void optimized_inner_product(const vector<double>& A, const vector<double>& a, vector<double>& sum, int n) {
    fill(sum.begin(), sum.end(), 0.0);  // C++ 标准初始化向量

    for (int j = 0; j < n; j++) {
        double a_j = a[j];
        for (int i = 0; i < n; i++) {
            sum[i] += A[j * n + i] * a_j;
        }
    }
}

// 结果正确性校验：纯C++风格
bool verify_result(const vector<double>& sum_naive, const vector<double>& sum_opt) {
    for (size_t i = 0; i < sum_naive.size(); i++) {
        if (fabs(sum_naive[i] - sum_opt[i]) > 1e-10) {
            return false;
        }
    }
    return true;
}

int main() {
    // ========== 第一步：算法正确性测试 ==========
    const int test_n = 4;
    vector<double> test_A(test_n * test_n);  // C++ vector，自动管理内存
    vector<double> test_a(test_n);
    vector<double> sum_naive_test(test_n);
    vector<double> sum_opt_test(test_n);

    generate_data(test_A, test_a, test_n);
    naive_inner_product(test_A, test_a, sum_naive_test, test_n);
    optimized_inner_product(test_A, test_a, sum_opt_test, test_n);

    if (!verify_result(sum_naive_test, sum_opt_test)) {
        cout << "【错误】算法结果验证失败！" << endl;
        return 1;
    }
    cout << "【成功】算法结果验证通过！" << endl << endl;

    // ========== 第二步：性能测试 ==========
    const vector<int> sizes = {64, 128, 256, 512, 1024, 2048, 4096};
    const int repeats = 100;

    // C++ 标准输出表头
    cout << "矩阵规模n\t平凡算法耗时(s)\t优化算法耗时(s)\t加速比(优化/平凡)" << endl;
    cout << "------------------------------------------------------------------------------------" << endl;

    for (int n : sizes) {  // C++ 范围for循环
        vector<double> A(static_cast<size_t>(n) * n);
        vector<double> a(n);
        vector<double> sum(n);

        generate_data(A, a, n);

        // 测试平凡算法
        double start_time = get_time();
        for (int r = 0; r < repeats; r++) {
            naive_inner_product(A, a, sum, n);
        }
        double end_time = get_time();
        const double t_naive = (end_time - start_time) / repeats;

        // 测试优化算法
        start_time = get_time();
        for (int r = 0; r < repeats; r++) {
            optimized_inner_product(A, a, sum, n);
        }
        end_time = get_time();
        const double t_opt = (end_time - start_time) / repeats;

        const double speedup = t_naive / t_opt;

        // C++ 格式化输出
        cout << n << "\t\t"
             << fixed;  // 固定小数格式
        cout.precision(6);
        cout << t_naive << "\t\t"
             << t_opt << "\t\t"
             << fixed;
        cout.precision(2);
        cout << speedup << "x" << endl;
    }

    cout << endl << "测试全部完成！" << endl;
    return 0;
}
