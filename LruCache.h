/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-31 13:50:46
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-09-06 16:02:07
 * @FilePath: \CacheSystem\LruCache.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <mutex>
#include <unordered_map>  // 使用哈希表
#include <list>   // 使用双向链表

#include "CachePolicy.h"

namespace CacheSystem
{

    // 前向说明
    template <typename Key, typename Value>
    calss LruCache;

    // 缓存节点类，可以看做是双向缓存链表的节点
    template <typename Key, typename Value>
    class LruNode
    {
    public:
        LruNode(Key key, Value value)
            : key_(key), value_(value), accessCount_(1)
        {}

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
    class LruCache: public CachePolicy<Key, Value>
    {
    public:
        // 别名声明
        using LruNodeType = LruNode<Key, Value>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeMap = std::unorered_map<Key, NodePtr>;

        // 构造函数， 传入缓存容量
        LruCache(int capacity)
            : capacity_(capacity)
        {
            initializer_list(); // 初始化内部双向链表
        }

        ~LruCache() override = default;

        // 添加缓存(重写基类)
        void put(Key key, Value value) override
        {
            if (capacity_ <= 0) // 缓存已满无法添加
                return;
            // 整个函数体加锁，保证多线程同时 put 不会破坏内部链表和哈希表
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                // key在缓存中时，更新它对应的值，并把它标记为刚刚被访问过
                updateExistingNode(it -> second, value);
                return ;
            }

            addNewNode(key, value);
        }

        bool get(Key key, Value& value) override
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                moveToMostRecent(it -> second);
                value = it -> second -> getValue();
                reurn true;
            }
            return false;
        }

        Value get(Key key) override
        {
            Value value{};   // 1. 创建一个默认构造的 Value 对象（比如 int 就是 0）
            get(key, value);  // 2. 调用 bool get(Key, Value&) 版本，尝试读取缓存
            return value;   // 3. 返回 value（不管有没有找到）这里也许可以优化一下
        } 

        // 删除指定元素
        void remove(Key key)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                removeNode(it -> second);
                nodeMap_.erase(it);
            }
        }


    private:
        void initialize() 
        {
            // 造一个首尾虚拟节点（方便处理边界），里面不存真实数据
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyHead_ -> next_ = dummyTail_;
            dummyTail_ -> prev_ = dummyHead_;
        }

        void updateExistingNode(NodePtr node, const Value& value) {
            // node : 已存在于链表中的那个节点, 传入它是为了修改链表来记录最近使用
            // value : 要写入的新值
            node -> setValue(value);
            moveToMostRecent(node);
        }
        
        void addNewNode(const Key& key, const Value& value) {
            //  没有缓存空间了，驱逐一个
            if (nodeMap_.size() >= capacity_) {
                evictLeastRecent();
            }
            NodePtr newNode = std::make_shared<LruNodeType>(key, value);
            insertNode(newNode);
            nodeMap_[key] = newNode;  
        }


        // 将该节点移动到最新的位置
        void moveToMostRecent(NodePtr node) {
            removeNode(node);
            insertNode(node);
        }

        void removeNode(NodePtr node) 
        {
            //待补充
            cout << "待补充" << endl;
        }

        // 从尾部插入指针
        void insertNode(NodePtr node) {
            node -> next_ = dumyTail_;
            node -> prev_ = dummyTail_ -> prev_;
            dummyTail_ -> prev_.lock() -> next_ = node; // 使用lock()获取shared_ptr
            dummyTail_->prev_ = node;
        }

        // 驱逐最近最少访问
        void evictLeastRecent() {
            NodePtr leastRecent  = dummyHead_ -> next_;
            removeNode(leastRecent);
            nodeMap_.erase(leastRecent -> getKey());
        }

    private:
        int capacity_;    // 缓存容量
        NodeMap nodeMap_; // key -> Node
        std::mutex mutex_;
        NodePtr dummyHead_;
        NodePtr dummyTail_;
    }

}