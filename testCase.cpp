/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-30 20:37:27
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-08-31 16:42:50
 * @FilePath: \CacheSystem\testCase.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iostream>
#include <chrono>  // 包含Timer
#include <string>
#include <array>

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

    CacheSystemPolicy::LruCache<int, std::string> lru(CAPACITY);

    // 随机决定 读还是写操作 访问热点 还是 冷数据
    std::random_device rd;
    std::mt19937 gen(rd());

    // 把不同的缓存实现“打包”成统一接口
    std::array<CacheSystemPolicy::>
}

int main() {
    return 0;    
}