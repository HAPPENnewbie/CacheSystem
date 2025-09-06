/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-30 20:37:27
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-09-06 16:11:40
 * @FilePath: \CacheSystem\testCase.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iostream>
#include <chrono>  // 包含Timer
#include <string>
#include <array>
#include <vector>

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


void testHotDataAccess() {
    std::cout << "\n === 测试场景1: 热点数据访问测试 ===" << std::endl;

    const int CAPACITY = 20;         // 缓存容量
    const int OPERATIONS = 500000;   // 总操作次数
    const int HOT_KEYS = 20;         // 热点数据数量
    const int COLD_KEYS = 5000;      // 冷数据数量

    CacheSystem::LruCache<int, std::string> lru(CAPACITY);

    // 随机决定 读还是写操作 访问热点 还是 冷数据
    std::random_device rd;
    std::mt19937 gen(rd());

    // 基类指针指向派生类对象”，从而把 不同淘汰策略的缓存实例 统一塞进同一个容器里，方便后续 批量测试、统一调用
    std::array<CacheSystem::CachePolicy<int, std::string>*, 1> caches = {&lru};
    std::vector<int> hits(1, 0);  //命中计数器
    std::vector<int> get_operations(1, 0);  //总查询计数器
    std::vector<std::string> names = {"LRU"}; // 算法名称

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
            
        }
    }
}

int main() {
    
    return 0;    
}