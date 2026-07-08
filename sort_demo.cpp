/*
 * 排序算法演示 - Sorting Algorithm Demo
 * 支持: 快速排序 (Quick Sort) / 归并排序 (Merge Sort)
 *
 * 编译: g++ -std=c++17 -O2 sort_demo.cpp -o sort_demo
 * 运行: ./sort_demo
 */

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

// ============================================================
//  终端颜色
// ============================================================
namespace Color {
const char* RESET   = "\033[0m";
const char* RED     = "\033[31m";
const char* GREEN   = "\033[32m";
const char* YELLOW  = "\033[33m";
const char* BLUE    = "\033[34m";
const char* MAGENTA = "\033[35m";
const char* CYAN    = "\033[36m";
const char* WHITE   = "\033[37m";
const char* BOLD    = "\033[1m";
const char* DIM     = "\033[2m";
} // namespace Color

// ============================================================
//  用柱状图可视化当前数组
// ============================================================
void visualize(const std::vector<int>& arr,
               int  pivot_idx   = -1,
               int  left_bound  = -1,
               int  right_bound = -1,
               const std::string& label = "") {
    // 找出最大值用于缩放
    int max_val = *std::max_element(arr.begin(), arr.end());
    if (max_val == 0) max_val = 1;

    const int MAX_BAR = 30; // 最长柱子高度(字符数)

    if (!label.empty()) {
        std::cout << Color::BOLD << Color::CYAN
                  << "\n  " << label << Color::RESET << "\n";
    }

    for (size_t i = 0; i < arr.size(); ++i) {
        int bar_len = static_cast<int>((static_cast<double>(arr[i]) / max_val) * MAX_BAR);
        if (bar_len < 1 && arr[i] > 0) bar_len = 1;

        // 选择颜色
        const char* color = Color::WHITE;
        if (static_cast<int>(i) == pivot_idx) {
            color = Color::RED; // 枢轴
        } else if (left_bound >= 0 && right_bound >= 0 &&
                   static_cast<int>(i) >= left_bound &&
                   static_cast<int>(i) <= right_bound) {
            color = Color::YELLOW; // 当前处理区间
        }

        std::cout << "  " << std::setw(3) << arr[i] << " " << color;
        for (int j = 0; j < bar_len; ++j) std::cout << "█";
        std::cout << Color::RESET << "\n";
    }
    std::cout << std::endl;
}

// ============================================================
//  辅助: 生成随机数组
// ============================================================
std::vector<int> generate_array(int size, int min_val = 1, int max_val = 99) {
    std::vector<int> arr(size);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(min_val, max_val);
    for (int& v : arr) v = dist(rng);
    return arr;
}

// ============================================================
//  快速排序 (Quick Sort)
//  策略: 取区间中间元素为 pivot，Hoare 分区
// ============================================================
namespace QuickSort {
int step = 0;

int partition(std::vector<int>& arr, int lo, int hi, bool show_steps) {
    int mid   = lo + (hi - lo) / 2;
    int pivot = arr[mid];
    int i     = lo - 1;
    int j     = hi + 1;

    if (show_steps) {
        std::cout << Color::MAGENTA
                  << "  ── 分区: [" << lo << ".." << hi
                  << "], pivot = arr[" << mid << "] = " << pivot
                  << Color::RESET << "\n";
    }

    while (true) {
        do { ++i; } while (arr[i] < pivot);
        do { --j; } while (arr[j] > pivot);
        if (i >= j) return j;
        std::swap(arr[i], arr[j]);

        if (show_steps) {
            ++step;
            visualize(arr, -1, lo, hi,
                      "QuickSort 步骤 " + std::to_string(step) +
                      "  (交换 " + std::to_string(arr[j]) +
                      " ↔ " + std::to_string(arr[i]) + ")");
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    }
}

void quick_sort(std::vector<int>& arr, int lo, int hi, bool show_steps) {
    if (lo >= hi) return;
    int p = partition(arr, lo, hi, show_steps);
    quick_sort(arr, lo, p, show_steps);
    quick_sort(arr, p + 1, hi, show_steps);
}

void sort(std::vector<int>& arr, bool show_steps = true) {
    step = 0;
    std::cout << Color::BOLD << Color::GREEN
              << "\n══════════ 快速排序 (Quick Sort) ══════════"
              << Color::RESET << "\n";
    std::cout << "  策略: 分治 — 选 pivot，小的放左边，大的放右边\n\n";

    if (show_steps) {
        visualize(arr, -1, -1, -1, "初始数组");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    auto start = std::chrono::steady_clock::now();
    quick_sort(arr, 0, static_cast<int>(arr.size()) - 1, show_steps);
    auto end = std::chrono::steady_clock::now();

    if (show_steps) {
        visualize(arr, -1, -1, -1, "排序完成 ✓");
    }

    auto elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << Color::DIM << "  耗时: " << std::fixed
              << std::setprecision(3) << elapsed << " ms"
              << Color::RESET << "\n";
}
} // namespace QuickSort

// ============================================================
//  归并排序 (Merge Sort)
// ============================================================
namespace MergeSort {
int step = 0;

// 合并两个有序子数组 arr[lo..mid] 和 arr[mid+1..hi]
void merge(std::vector<int>& arr, int lo, int mid, int hi, bool show_steps) {
    int n1 = mid - lo + 1;
    int n2 = hi - mid;

    std::vector<int> L(arr.begin() + lo, arr.begin() + mid + 1);
    std::vector<int> R(arr.begin() + mid + 1, arr.begin() + hi + 1);

    if (show_steps) {
        std::cout << Color::MAGENTA
                  << "  ── 合并: 左[" << lo << ".." << mid
                  << "] + 右[" << (mid + 1) << ".." << hi << "]"
                  << Color::RESET << "\n";
    }

    int i = 0, j = 0, k = lo;
    while (i < n1 && j < n2) {
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    if (show_steps) {
        ++step;
        visualize(arr, -1, lo, hi,
                  "MergeSort 步骤 " + std::to_string(step) +
                  "  (合并区间 [" + std::to_string(lo) +
                  ".." + std::to_string(hi) + "])");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void merge_sort(std::vector<int>& arr, int lo, int hi, bool show_steps) {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort(arr, lo, mid, show_steps);
    merge_sort(arr, mid + 1, hi, show_steps);
    merge(arr, lo, mid, hi, show_steps);
}

void sort(std::vector<int>& arr, bool show_steps = true) {
    step = 0;
    std::cout << Color::BOLD << Color::BLUE
              << "\n══════════ 归并排序 (Merge Sort) ══════════"
              << Color::RESET << "\n";
    std::cout << "  策略: 分治 — 递归拆分至单个元素，再两两合并\n\n";

    if (show_steps) {
        visualize(arr, -1, -1, -1, "初始数组");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    auto start = std::chrono::steady_clock::now();
    merge_sort(arr, 0, static_cast<int>(arr.size()) - 1, show_steps);
    auto end = std::chrono::steady_clock::now();

    if (show_steps) {
        visualize(arr, -1, -1, -1, "排序完成 ✓");
    }

    auto elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << Color::DIM << "  耗时: " << std::fixed
              << std::setprecision(3) << elapsed << " ms"
              << Color::RESET << "\n";
}
} // namespace MergeSort

// ============================================================
//  性能对比模式 (不逐步展示，只测时间)
// ============================================================
void benchmark() {
    std::cout << Color::BOLD << Color::YELLOW
              << "\n\n══════════ 性能对比 (Benchmark) ══════════"
              << Color::RESET << "\n\n";

    const int sizes[] = {1'000, 10'000, 100'000};

    std::cout << std::setw(10) << "规模"
              << std::setw(18) << "QuickSort"
              << std::setw(18) << "MergeSort"
              << std::setw(18) << "std::sort" << "\n";
    std::cout << std::string(64, '-') << "\n";

    for (int n : sizes) {
        auto original = generate_array(n, 1, 999'999);

        // QuickSort
        auto arr1 = original;
        auto t1   = std::chrono::steady_clock::now();
        QuickSort::quick_sort(arr1, 0, n - 1, false);
        auto t2   = std::chrono::steady_clock::now();
        double qs = std::chrono::duration<double, std::milli>(t2 - t1).count();

        // MergeSort
        auto arr2 = original;
        auto t3   = std::chrono::steady_clock::now();
        MergeSort::merge_sort(arr2, 0, n - 1, false);
        auto t4   = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t4 - t3).count();

        // std::sort
        auto arr3 = original;
        auto t5   = std::chrono::steady_clock::now();
        std::sort(arr3.begin(), arr3.end());
        auto t6   = std::chrono::steady_clock::now();
        double ss = std::chrono::duration<double, std::milli>(t6 - t5).count();

        std::cout << std::setw(10) << n
                  << std::setw(15) << std::fixed << std::setprecision(2)
                  << qs << " ms"
                  << std::setw(15) << ms << " ms"
                  << std::setw(15) << ss << " ms" << "\n";
    }
}

// ============================================================
//  验证排序正确性
// ============================================================
bool verify(const std::vector<int>& arr) {
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i - 1] > arr[i]) return false;
    }
    return true;
}

// ============================================================
//  打印菜单
// ============================================================
void print_menu() {
    std::cout << Color::BOLD << "\n\n  ╔══════════════════════════════╗\n";
    std::cout << "  ║    排序算法演示  🧩        ║\n";
    std::cout << "  ╠══════════════════════════════╣\n";
    std::cout << "  ║  " << Color::GREEN << "1" << Color::RESET
              << "  快速排序 (Quick Sort)   ║\n";
    std::cout << "  ║  " << Color::BLUE << "2" << Color::RESET
              << "  归并排序 (Merge Sort)   ║\n";
    std::cout << "  ║  " << Color::YELLOW << "3" << Color::RESET
              << "  性能对比 (Benchmark)    ║\n";
    std::cout << "  ║  " << Color::RED << "0" << Color::RESET
              << "  退出                    ║\n";
    std::cout << "  ╚══════════════════════════════╝\n";
    std::cout << Color::DIM << "\n  请输入选项 [0-3]: " << Color::RESET;
}

// ============================================================
//  入口
// ============================================================
int main() {
    const int DEMO_SIZE = 15; // 演示用数组大小 (足够看清过程)

    while (true) {
        print_menu();

        int choice;
        std::cin >> choice;

        if (choice == 0) {
            std::cout << Color::DIM << "  再见!\n" << Color::RESET;
            break;
        }

        if (choice < 1 || choice > 3) {
            std::cout << Color::RED << "  无效选项，请重试\n"
                      << Color::RESET;
            continue;
        }

        if (choice == 3) {
            benchmark();
            continue;
        }

        // 生成演示数组
        auto original = generate_array(DEMO_SIZE, 1, 99);

        if (choice == 1) {
            auto arr = original;
            QuickSort::sort(arr, true);
            std::cout << (verify(arr) ? std::string(Color::GREEN) + "  ✓ 排序正确"
                                      : std::string(Color::RED) + "  ✗ 排序错误")
                      << Color::RESET << "\n";
        } else if (choice == 2) {
            auto arr = original;
            MergeSort::sort(arr, true);
            std::cout << (verify(arr) ? std::string(Color::GREEN) + "  ✓ 排序正确"
                                      : std::string(Color::RED) + "  ✗ 排序错误")
                      << Color::RESET << "\n";
        }
    }

    return 0;
}
