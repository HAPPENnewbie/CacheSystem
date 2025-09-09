/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-30 20:37:27
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-09-09 21:32:19
 * @FilePath: \CacheSystem\testCase.cpp
 * @Description: ����Ĭ������,������`customMade`, ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
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

// ��ʱ�������࣬����ĳ�δ���ִ��ʱ��
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


// ������������ӡ���
void printResults(const std::string& testName, int capacity, 
                const std::vector<int> get_operations, 
                const std::vector<int>& hits) {
    std::cout << "=== " << testName << " ������� ===" << std::endl;
    std::cout << "�����С: " << capacity << std::endl;

    std::vector<std::string> names;
    if (hits.size() == 1) {
        names = {"LRU"};
    }
    
    for (size_t i = 0; i < hits.size(); ++i) {
        double hitRate = 100.0 * hits[i] / get_operations[i];
        std::cout << (i < names.size() ? names[i] : "Algorithm " + std::to_string(i+1)) 
                  << " - ������: " << std::fixed << std::setprecision(2) 
                  << hitRate << "% ";
        // ��Ӿ������д������ܲ�������
        std::cout << "(" << hits[i] << "/" << get_operations[i] << ")" << std::endl;
    }
    std::cout << std::endl;  // ��ӿ��У�ʹ���������
}



void testHotDataAccess() {
    std::cout << "\n === ���Գ���1: �ȵ����ݷ��ʲ��� ===" << std::endl;

    const int CAPACITY = 20;         // ��������
    const int OPERATIONS = 500000;   // �ܲ�������
    const int HOT_KEYS = 20;         // �ȵ���������
    const int COLD_KEYS = 5000;      // ����������

    CacheSystem::LruCache<int, std::string> lru(CAPACITY);

    // ������� ������д���� �����ȵ� ���� ������
    std::random_device rd;
    std::mt19937 gen(rd());

    // ����ָ��ָ����������󡱣��Ӷ��� ��ͬ��̭���ԵĻ���ʵ�� ͳһ����ͬһ�������������� �������ԡ�ͳһ����
    std::array<CacheSystem::CachePolicy<int, std::string>*, 1> caches = {&lru};
    std::vector<int> hits(1, 0);  //���м�����
    std::vector<int> get_operations(1, 0);  //�ܲ�ѯ������
    std::vector<std::string> names = {"LRU"}; // �㷨����

    // Ϊ���еĻ�����������ͬ�Ĳ������в���
    for (int i = 0; i < caches.size(); ++i) {
        // ��Ԥ�Ȼ��棬����һЩ����
        for (int key = 0; key < HOT_KEYS; ++key) {
            std::string value = "value" + std::to_string(key);
            caches[i] -> put(key, value);
        }

        // �������put��get����, ģ����ʵ����
        for (int op = 0; op < OPERATIONS; ++op) {
            // ���������ϵͳ�ж�������д����Ƶ��
            // ��������30%���ʽ���д����
            bool isPut = (gen() % 100 < 30);
            int key;

            // 70%���ʷ����ȵ����ݣ�30%���ʷ���������
            if (gen() % 100 < 70) {
                key = gen() % HOT_KEYS;  // �ȵ�����
            } else {
                key = HOT_KEYS + (gen() % COLD_KEYS);   // ������
            }

            if (isPut) {
                // ִ��put����
                std::string value = "value" + std::to_string(key) + "-v" + std::to_string(op % 100);
                caches[i] -> put(key, value);
            } else {
                // ִ��get ��������¼�������
                std::string result;  // û�и�ֵ����������ֵ�ģ�get�л��value����
                get_operations[i]++;
                if (caches[i] -> get(key, result)) {
                    hits[i]++;
                }
            }
        }
    }

    // ��ӡ���Խ��
    printResults("�ȵ����ݷ��ʲ���", CAPACITY, get_operations, hits);
}

int main() {
    testHotDataAccess();
    return 0;    
}