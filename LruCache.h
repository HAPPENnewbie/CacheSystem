/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-31 13:50:46
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-09-04 15:10:29
 * @FilePath: \CacheSystem\LruCache.h
 * @Description: ����Ĭ������,������`customMade`, ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <mutex>
#include <unordered_map>  // ʹ�ù�ϣ��
#include <list>   // ʹ��˫������

namespace CacheSystemPolicy
{

    // ǰ��˵��
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
    class LruCache
    {
    public:
        // ��������
        using LruNodeType = LruNode<Key, Value>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeMap = std::unorered_map<Key, NodePtr>;

        LruCache(int capacity)
            : capacity_(capacity)
        {
            initializer_list(); // ��ʼ���ڲ�˫������
        }

        ~LruCache() override = default;

        // ��ӻ���
        void put(Key key, Value value) override
        {
            if (capacity_ <= 0) // ���������޷����
                return;
            // �����������������֤���߳�ͬʱ put �����ƻ��ڲ�����͹�ϣ��
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                // ����ڵ�ǰ������,�����value,������get��������������ݸձ�����
                updateExistingNode(it -> second, value);
                return ;
            }

            addNewNode(key, value);
        }

    private:
        void initialize() {
            // ��һ����ͷ�ڵ㣨���㴦��߽磩�����治����ʵ����
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
        }

    private:
        int capacity_;    // ��������
        NodeMap nodeMap_; // key -> Node
        std::mutex mutex_;
        NodePtr dummyHead_;
        NodePtr dummyTail_;
    }

}