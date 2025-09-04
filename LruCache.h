/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-31 13:50:46
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-09-04 15:10:29
 * @FilePath: \CacheSystem\LruCache.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <mutex>
#include <unordered_map>  // 使用哈希表
#include <list>   // 使用双向链表

namespace CacheSystemPolicy
{

    // 前向说明
    template <typename Key, typename Value>
    calss LruCache;

    template <typename Key, typename Value>
    class LruNode
    {
    public:
        LruNode(Key key, Value value)
            : key_(key), value_(value), accessCount_(1)
        {
        }

        // 访问器
        Key getKey() const { return key_; }
        Value getValue() const { return value_; }
        void setValue(const Value &value) { value_ = value; }
        size_t getAccessCount() const { return accessCount_; }
        void increamentAccessCount() { ++accessCount_; }

    private:
        Key key_;
        Value value_;
        size_t accessCount;                       // 访问次数
        std::weak_ptr<LruNode<Key, Value>> prev_; // 改为weak_ptr打破循环引用
        std::shared_ptr<LruNode<Key, Value>> next_;

        // 声明外部的LruCache 是友元类，LruCache可以访问友元内里的所有
        friend class LruCache<Key, Value>;
    };

    template <typename Key, typename Value>
    class LruCache
    {
    public:
        // 别名声明
        using LruNodeType = LruNode<Key, Value>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeMap = std::unorered_map<Key, NodePtr>;

        LruCache(int capacity)
            : capacity_(capacity)
        {
            initializer_list(); // 初始化内部双向链表
        }

        ~LruCache() override = default;

        // 添加缓存
        void put(Key key, Value value) override
        {
            if (capacity_ <= 0) // 缓存已满无法添加
                return;
            // 整个函数体加锁，保证多线程同时 put 不会破坏内部链表和哈希表
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                // 如果在当前容器中,则更新value,并调用get方法，代表该数据刚被访问
                updateExistingNode(it -> second, value);
                return ;
            }

            addNewNode(key, value);
        }

    private:
        void initialize() {
            // 造一个假头节点（方便处理边界），里面不存真实数据
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
        }

    private:
        int capacity_;    // 缓存容量
        NodeMap nodeMap_; // key -> Node
        std::mutex mutex_;
        NodePtr dummyHead_;
        NodePtr dummyTail_;
    }

}