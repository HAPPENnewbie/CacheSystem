/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-31 13:50:46
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-09-06 16:02:07
 * @FilePath: \CacheSystem\LruCache.h
 * @Description: ����Ĭ������,������`customMade`, ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <mutex>
#include <unordered_map>  // ʹ�ù�ϣ��
#include <list>   // ʹ��˫������

#include "CachePolicy.h"

namespace CacheSystem
{

    // ǰ��˵��
    template <typename Key, typename Value>
    calss LruCache;

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
        size_t accessCount;                       // ���ʴ���
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
        using LruNodeType = LruNode<Key, Value>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeMap = std::unorered_map<Key, NodePtr>;

        // ���캯���� ���뻺������
        LruCache(int capacity)
            : capacity_(capacity)
        {
            initializer_list(); // ��ʼ���ڲ�˫������
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
            Value value{};   // 1. ����һ��Ĭ�Ϲ���� Value ���󣨱��� int ���� 0��
            get(key, value);  // 2. ���� bool get(Key, Value&) �汾�����Զ�ȡ����
            return value;   // 3. ���� value��������û���ҵ�������Ҳ������Ż�һ��
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
        void initialize() 
        {
            // ��һ����β����ڵ㣨���㴦��߽磩�����治����ʵ����
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyHead_ -> next_ = dummyTail_;
            dummyTail_ -> prev_ = dummyHead_;
        }

        void updateExistingNode(NodePtr node, const Value& value) {
            // node : �Ѵ����������е��Ǹ��ڵ�, ��������Ϊ���޸���������¼���ʹ��
            // value : Ҫд�����ֵ
            node -> setValue(value);
            moveToMostRecent(node);
        }
        
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

        void removeNode(NodePtr node) 
        {
            //������
            cout << "������" << endl;
        }

        // ��β������ָ��
        void insertNode(NodePtr node) {
            node -> next_ = dumyTail_;
            node -> prev_ = dummyTail_ -> prev_;
            dummyTail_ -> prev_.lock() -> next_ = node; // ʹ��lock()��ȡshared_ptr
            dummyTail_->prev_ = node;
        }

        // ����������ٷ���
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
    }

}