#include <iostream>
#include <vector>
#include <cmath>
#include <windows.h>
#include <iomanip>  

using namespace std;

//  高精度计时
double get_time()
{
    LARGE_INTEGER freq, cnt;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&cnt);
    return static_cast<double>(cnt.QuadPart) / freq.QuadPart;
}

// 生成测试数据
void generate_data(double* a, int n)
{
    for (int i = 0; i < n; ++i)
        a[i] = 1.0;
}

// 1. 平凡链式累加
double naive_sum(const double* a, int n)
{
    double sum = 0.0;
    for (int i = 0; i < n; ++i)
        sum += a[i];
    return sum;
}

// 2. 两路指令级并行累加
double two_way_sum(const double* a, int n)
{
    double sum1 = 0.0, sum2 = 0.0;
    for (int i = 0; i < n; i += 2)
    {
        sum1 += a[i];
        sum2 += a[i + 1];
    }
    return sum1 + sum2;
}

// 3. 两两递归求和
double recursive_pair_sum(const double* a, int n)
{
    double* temp = new double[n];
    for (int i = 0; i < n; ++i)
        temp[i] = a[i];

    for (int m = n; m > 1; m /= 2)
    {
        for (int i = 0; i < m / 2; ++i)
            temp[i] = temp[2*i] + temp[2*i + 1];
    }

    double res = temp[0];
    delete[] temp;
    return res;
}

// 4. 四路循环展开 + 指令级并行
double unrolled_four_way_sum(const double* a, int n)
{
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
    int i = 0;
    for (; i + 3 < n; i += 4)
    {
        sum1 += a[i];
        sum2 += a[i+1];
        sum3 += a[i+2];
        sum4 += a[i+3];
    }
    for (; i < n; ++i)
        sum1 += a[i];

    return sum1 + sum2 + sum3 + sum4;
}

// 结果校验
bool verify_result(double result, double expected)
{
    return fabs(result - expected) < 1e-10;
}

int main()
{
    // 全局设置：固定小数格式，禁用科学计数法
    cout << fixed;

    // 小规模正确性测试
    const int test_n = 8;
    double* test_a = new double[test_n];
    generate_data(test_a, test_n);
    double expected = static_cast<double>(test_n);

    bool all_ok = true;
    if (!verify_result(naive_sum(test_a, test_n), expected))
    {
        cout << "【错误】平凡算法验证失败" << endl;
        all_ok = false;
    }
    if (!verify_result(two_way_sum(test_a, test_n), expected))
    {
        cout << "【错误】两路并行算法验证失败" << endl;
        all_ok = false;
    }
    if (!verify_result(recursive_pair_sum(test_a, test_n), expected))
    {
        cout << "【错误】两两递归算法验证失败" << endl;
        all_ok = false;
    }
    if (!verify_result(unrolled_four_way_sum(test_a, test_n), expected))
    {
        cout << "【错误】四路展开算法验证失败" << endl;
        all_ok = false;
    }

    if (!all_ok)
    {
        delete[] test_a;
        return 1;
    }
    cout << "【成功】所有算法结果验证通过\n" << endl;
    delete[] test_a;

    // 性能测试配置
    vector<int> sizes = {64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};
    vector<int> repeats = {100000,100000,100000,100000,10000,10000,10000,1000,1000,1000,1000};

    cout << "规模n\t平凡(s)\t\t两路(s)\t\t递归(s)\t\t四路展开(s)\t两路加速\t递归加速\t展开加速" << endl;
    cout << "-----------------------------------------------------------------------------------------" << endl;

    for (size_t idx = 0; idx < sizes.size(); ++idx)
    {
        int n = sizes[idx];
        int rep = repeats[idx];

        double* a = new double[n];
        generate_data(a, n);

        // 计时：平凡
        double t1 = get_time();
        for (int r = 0; r < rep; ++r) naive_sum(a, n);
        double t_naive = (get_time() - t1) / rep;

        // 计时：两路
        t1 = get_time();
        for (int r = 0; r < rep; ++r) two_way_sum(a, n);
        double t_two = (get_time() - t1) / rep;

        // 计时：递归两两
        t1 = get_time();
        for (int r = 0; r < rep; ++r) recursive_pair_sum(a, n);
        double t_rec = (get_time() - t1) / rep;

        // 计时：四路展开
        t1 = get_time();
        for (int r = 0; r < rep; ++r) unrolled_four_way_sum(a, n);
        double t_unroll = (get_time() - t1) / rep;

        // 加速比
        double sp_two  = t_naive / t_two;
        double sp_rec  = t_naive / t_rec;
        double sp_unroll = t_naive / t_unroll;

        // 格式化输出：6位小数（时间），2位小数（加速比）
        cout << n << "\t";
        cout << setprecision(8) << t_naive << "\t";
        cout << setprecision(8) << t_two << "\t";
        cout << setprecision(8) << t_rec << "\t";
        cout << setprecision(8) << t_unroll << "\t";
        cout << setprecision(2) << sp_two << "x\t\t";
        cout << setprecision(2) << sp_rec << "x\t\t";
        cout << setprecision(2) << sp_unroll << "x" << endl;

        delete[] a;
    }

    cout << "\n测试全部完成！" << endl;
    return 0;
}
