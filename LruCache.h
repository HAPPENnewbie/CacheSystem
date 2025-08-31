#pragma once

namespace CacheSystemPolicy {

// 前向说明
template <typename Key, typename Value> calss LruCache;

template <typename Key, typename Value> 
class LruNode {
public:
    LruNode(Key key, Value value)
        : key_(key)
        , value_(value)
        , accessCount_(1)
        {}
    
    // 访问器
    Key getKey() const {return key_;}
    Value getValue() const {return value_;}
    void setValue(const Value& value) {value_ = value;}
    size_t getAccessCount() const {return accessCount_;}
    void increamentAccessCount() {++accessCount_;}


private:
    Key key_;
    Value value_;
    size_t accessCount; // 访问次数
    std::weak_ptr<LruNode<Key, Value>> prev_; // 改为weak_ptr打破循环引用
    std::shared_ptr<LruNode<Key, Value>> next_;

    friend class LruCache<Key, Value>;

};

template<typename Key, typename Value>
class LruCache{
public:
    // 别名声明 
    using LruNodeType = LruNode<Key, Value>;
    using NodePtr = std::shared_ptr<LruNodeType>;
    using NodeMap = std::unorered_map<Key, NodePtr>;

    LruCache(int capacity) 
        : capacity_(capacity)
    {
        initializer_list();
    }

    ~LruCache() override = default;
}

    
}