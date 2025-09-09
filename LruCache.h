/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-31 13:50:46
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-09-09 21:32:36
 * @FilePath: \CacheSystem\LruCache.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <cstring>
#include <mutex>
#include <unordered_map>  // 使用哈希表
#include <list>   // 使用双向链表
#include <memory>
#include "CachePolicy.h"

namespace CacheSystem
{

    // 前向说明
    template <typename Key, typename Value>
    class LruCache;

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
        size_t accessCount_;                       // 访问次数
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
        using LruNodeType = LruNode<Key, Value>;   // 缓存节点
        using NodePtr = std::shared_ptr<LruNodeType>;    // 节点的智能指针（管理生命周期）
        using NodeMap = std::unordered_map<Key, NodePtr>; // 哈希表：用来快速查找key有没有在当前缓存中

        // 构造函数： 传入缓存容量， 初始化缓存上限链表
        LruCache(int capacity)
            : capacity_(capacity)
        {
            initializeList(); // 初始化内部双向链表
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
                return;
            }
            // 不在缓存中则添加新节点
            addNewNode(key, value);
        }

        //  如果存在节点，更新节点的value并返回true； 如果不存在节点，则返回false
        bool get(Key key, Value& value) override
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                moveToMostRecent(it -> second);
                value = it -> second -> getValue();  // 如果存在就更新value的值
                return true;
            }
            return false;
        }

        Value get(Key key) override
        {
            Value value{};   // 1. 创建一个默认构造的 Value 对象（比如 int 就是 0）
            get(key, value);  // 2. 调用 bool get(Key, Value&) 版本，尝试读取缓存
            return value;   // 3. 返回 value， 找到了返回value, 没找到还是原本初始化的0
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
        void initializeList() 
        {
            // 创建虚拟头尾节点，初始时相互连接（链表为空）
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());  // 创建了LruNodeType的对象并返回了智能指针，赋值给成员变量dummyHead_
            dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyHead_ -> next_ = dummyTail_;      
            dummyTail_ -> prev_ = dummyHead_;
        }

        // 将节点移动到缓存链表的尾部，并更新value
        void updateExistingNode(NodePtr node, const Value& value) {
            // node : 已存在于链表中的那个节点, 传入它是为了修改链表来记录最近使用
            // value : 要写入的新值
            node -> setValue(value);
            moveToMostRecent(node);
        }
        
        // 添加新节点到缓存链表中，并更新哈希表
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

        // 从双向链表中移除缓存节点
        void removeNode(NodePtr node) 
        {
            if (!node -> prev_.expired() && node -> next_) {
                auto prev = node -> prev_.lock();
                prev -> next_ = node -> next_;
                node -> next_ -> prev_ = prev;
                node -> next_ = nullptr;
            }
        }

        // 从尾部插入指针
        void insertNode(NodePtr node) {
            node -> next_ = dummyTail_;
            node -> prev_ = dummyTail_ -> prev_;
            dummyTail_ -> prev_.lock() -> next_ = node; // prev是weak指针，不能直接访问节点的成员netx_, 所以要临时转换为shared指针
            dummyTail_->prev_ = node;
        }

        // 驱逐最近最少访问：即驱逐队列最前面的节点
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
    };



}