/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-31 13:50:46
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-09-09 21:32:36
 * @FilePath: \CacheSystem\LruCache.h
 * @Description: ����Ĭ������,������`customMade`, ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <cstring>
#include <mutex>
#include <unordered_map>  // ʹ�ù�ϣ��
#include <list>   // ʹ��˫������
#include <memory>
#include "CachePolicy.h"

namespace CacheSystem
{

    // ǰ��˵��
    template <typename Key, typename Value>
    class LruCache;

    // ����ڵ��࣬���Կ�����˫�򻺴�����Ľڵ�
    template <typename Key, typename Value>
    class LruNode
    {
    public:
        LruNode(Key key, Value value)
            : key_(key), value_(value), accessCount_(1)
        {}

        // ������
        Key getKey() const { return key_; }
        Value getValue() const { return value_; }
        void setValue(const Value &value) { value_ = value; }
        size_t getAccessCount() const { return accessCount_; }
        void increamentAccessCount() { ++accessCount_; }

    private:
        Key key_;
        Value value_;
        size_t accessCount_;                       // ���ʴ���
        std::weak_ptr<LruNode<Key, Value>> prev_; // ��Ϊweak_ptr����ѭ������
        std::shared_ptr<LruNode<Key, Value>> next_;

        // �����ⲿ��LruCache ����Ԫ�࣬LruCache���Է�����Ԫ���������
        friend class LruCache<Key, Value>;
    };


    template <typename Key, typename Value>
    class LruCache: public CachePolicy<Key, Value>
    {
    public:
        // ��������
        using LruNodeType = LruNode<Key, Value>;   // ����ڵ�
        using NodePtr = std::shared_ptr<LruNodeType>;    // �ڵ������ָ�루�����������ڣ�
        using NodeMap = std::unordered_map<Key, NodePtr>; // ��ϣ���������ٲ���key��û���ڵ�ǰ������

        // ���캯���� ���뻺�������� ��ʼ��������������
        LruCache(int capacity)
            : capacity_(capacity)
        {
            initializeList(); // ��ʼ���ڲ�˫������
        }

        ~LruCache() override = default;

        // ��ӻ���(��д����)
        void put(Key key, Value value) override
        {
            if (capacity_ <= 0) // ���������޷����
                return;
            // �����������������֤���߳�ͬʱ put �����ƻ��ڲ�����͹�ϣ��
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                // key�ڻ�����ʱ����������Ӧ��ֵ�����������Ϊ�ոձ����ʹ�
                updateExistingNode(it -> second, value);
                return;
            }
            // ���ڻ�����������½ڵ�
            addNewNode(key, value);
        }

        //  ������ڽڵ㣬���½ڵ��value������true�� ��������ڽڵ㣬�򷵻�false
        bool get(Key key, Value& value) override
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                moveToMostRecent(it -> second);
                value = it -> second -> getValue();  // ������ھ͸���value��ֵ
                return true;
            }
            return false;
        }

        Value get(Key key) override
        {
            Value value{};   // 1. ����һ��Ĭ�Ϲ���� Value ���󣨱��� int ���� 0��
            get(key, value);  // 2. ���� bool get(Key, Value&) �汾�����Զ�ȡ����
            return value;   // 3. ���� value�� �ҵ��˷���value, û�ҵ�����ԭ����ʼ����0
        } 

        // ɾ��ָ��Ԫ��
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
            // ��������ͷβ�ڵ㣬��ʼʱ�໥���ӣ�����Ϊ�գ�
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());  // ������LruNodeType�Ķ��󲢷���������ָ�룬��ֵ����Ա����dummyHead_
            dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyHead_ -> next_ = dummyTail_;      
            dummyTail_ -> prev_ = dummyHead_;
        }

        // ���ڵ��ƶ������������β����������value
        void updateExistingNode(NodePtr node, const Value& value) {
            // node : �Ѵ����������е��Ǹ��ڵ�, ��������Ϊ���޸���������¼���ʹ��
            // value : Ҫд�����ֵ
            node -> setValue(value);
            moveToMostRecent(node);
        }
        
        // ����½ڵ㵽���������У������¹�ϣ��
        void addNewNode(const Key& key, const Value& value) {
            //  û�л���ռ��ˣ�����һ��
            if (nodeMap_.size() >= capacity_) {
                evictLeastRecent();
            }
            NodePtr newNode = std::make_shared<LruNodeType>(key, value);
            insertNode(newNode);
            nodeMap_[key] = newNode;  
        }


        // ���ýڵ��ƶ������µ�λ��
        void moveToMostRecent(NodePtr node) {
            removeNode(node);
            insertNode(node);
        }

        // ��˫���������Ƴ�����ڵ�
        void removeNode(NodePtr node) 
        {
            if (!node -> prev_.expired() && node -> next_) {
                auto prev = node -> prev_.lock();
                prev -> next_ = node -> next_;
                node -> next_ -> prev_ = prev;
                node -> next_ = nullptr;
            }
        }

        // ��β������ָ��
        void insertNode(NodePtr node) {
            node -> next_ = dummyTail_;
            node -> prev_ = dummyTail_ -> prev_;
            dummyTail_ -> prev_.lock() -> next_ = node; // prev��weakָ�룬����ֱ�ӷ��ʽڵ�ĳ�Աnetx_, ����Ҫ��ʱת��Ϊsharedָ��
            dummyTail_->prev_ = node;
        }

        // ����������ٷ��ʣ������������ǰ��Ľڵ�
        void evictLeastRecent() {
            NodePtr leastRecent  = dummyHead_ -> next_;
            removeNode(leastRecent);
            nodeMap_.erase(leastRecent -> getKey());
        }

    private:
        int capacity_;    // ��������
        NodeMap nodeMap_; // key -> Node
        std::mutex mutex_;
        NodePtr dummyHead_;
        NodePtr dummyTail_;
    };



}