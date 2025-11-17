/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-30 20:37:27
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-10-23 15:57:02
 * @FilePath: \CacheSystem\testCase.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <iomanip>
#include <random>
#include <algorithm>
#include <array>
#include "CachePolicy.h"
#include "LruCache.h"

// 计时器工具类，测量某段代码执行时间
class Timer {
public:
    Timer()   
        : start_(std::chrono::high_resolution_clock::now()) {}
    
    double elapsed() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};


// 辅助函数：打印结果
void printResults(const std::string& testName, int capacity, 
                const std::vector<int> get_operations, 
                const std::vector<int>& hits) {
    std::cout << "=== " << testName << " 结果汇总 ===" << std::endl;
    std::cout << "缓存大小: " << capacity << std::endl;

    std::vector<std::string> names;
    if (hits.size() == 1) {
        names = {"LRU"};
    } else if (hits.size() == 2) {
        names = {"LRU", "LRU_K"};
    }
    
    for (size_t i = 0; i < hits.size(); ++i) {
        double hitRate = 100.0 * hits[i] / get_operations[i];
        std::cout << (i < names.size() ? names[i] : "Algorithm " + std::to_string(i+1)) 
                  << " - 命中率: " << std::fixed << std::setprecision(2) 
                  << hitRate << "% ";
        // 添加具体命中次数和总操作次数
        std::cout << "(" << hits[i] << "/" << get_operations[i] << ")" << std::endl;
    }
    std::cout << std::endl;  // 添加空行，使输出更清晰
}



void testHotDataAccess() {
    std::cout << "\n === Test Case 1: Hot-Keys data access test ===" << std::endl;

    const int CAPACITY = 20;         // 缓存容量
    const int OPERATIONS = 500000;   // 总操作次数
    const int HOT_KEYS = 20;         // 热点数据数量
    const int COLD_KEYS = 5000;      // 冷数据数量

    CacheSystem::LruCache<int, std::string> lru(CAPACITY);
    // 为LRU-K设置合适的参数：
    // - 主缓存容量与其他算法相同
    // - 历史记录容量设为可能访问的所有键数量
    // - k=2表示数据被访问2次后才会进入缓存，适合区分热点和冷数据
    CacheSystem::LruKCache<int, std::string> lruk(CAPACITY, HOT_KEYS + COLD_KEYS, 2);

    // 随机决定 读还是写操作 访问热点 还是 冷数据
    std::random_device rd;
    std::mt19937 gen(rd());

    // 基类指针指向派生类对象”，从而把 不同淘汰策略的缓存实例 统一塞进同一个容器里，方便后续 批量测试、统一调用
    std::array<CacheSystem::CachePolicy<int, std::string>*, 2> caches = {&lru, &lruk};
    std::vector<int> hits(2, 0);  //命中计数器
    std::vector<int> get_operations(2, 0);  //总查询计数器
    std::vector<std::string> names = {"LRU", "LRU-K"}; // 算法名称

    // 为所有的缓存对象进行相同的操作序列测试
    for (int i = 0; i < caches.size(); ++i) {
        // 先预热缓存，插入一些数据
        for (int key = 0; key < HOT_KEYS; ++key) {
            std::string value = "value" + std::to_string(key);
            caches[i] -> put(key, value);
        }

        // 交替进行put和get操作, 模拟真实场景
        for (int op = 0; op < OPERATIONS; ++op) {
            // 大多数缓存系统中读操作比写操作频繁
            // 所以设置30%概率进行写操作
            bool isPut = (gen() % 100 < 30);
            int key;

            // 70%概率访问热点数据，30%概率访问冷数据
            if (gen() % 100 < 70) {
                key = gen() % HOT_KEYS;  // 热点数据
            } else {
                key = HOT_KEYS + (gen() % COLD_KEYS);   // 冷数据
            }

            if (isPut) {
                // 执行put操作, 测试里加入写操作，是为了模拟真实场景，看看缓存策略在读写混合负载下的表现
                std::string value = "value" + std::to_string(key) + "-v" + std::to_string(op % 100);
                caches[i] -> put(key, value);
            } else {
                // 执行get 操作并记录命中情况
                std::string result;  // 没有赋值，是来接受值的，get中会把value给他
                get_operations[i]++;   // 记录读操作的总次数，用来算命中率
                if (caches[i] -> get(key, result)) {
                    hits[i]++;
                }
            }
        }
    }

    // 打印测试结果
    printResults("热点数据访问测试", CAPACITY, get_operations, hits);
}



int main() {
    testHotDataAccess();
    return 0;    
}