#pragma once

#include <map>
#include <mutex>
#include <vector>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);


    struct Access {
    private:
        std::lock_guard<std::mutex> guard_;
    public:
        Value &ref_to_value;
        Access (std::pair<std::map<Key, Value>, std::mutex> &map, const Key &key) :
                guard_(map.second), ref_to_value(map.first[key]) {}
    };

    explicit ConcurrentMap(size_t bucket_count) : map_pool_(bucket_count), bucket_count_(bucket_count) {};

    Access operator[](const Key& key);

    std::map<Key, Value> BuildOrdinaryMap();

private:
    std::vector<std::pair<std::map<Key, Value>, std::mutex>> map_pool_;
    const size_t bucket_count_;
};

template<typename Key, typename Value>
typename ConcurrentMap<Key, Value>::Access
ConcurrentMap<Key, Value>::operator[](const Key &key) {
    size_t u_key = static_cast<size_t>(key);
    size_t bucket = u_key % bucket_count_;
    return Access(map_pool_[bucket], key);
}

template<typename Key, typename Value>
std::map<Key, Value> ConcurrentMap<Key, Value>::BuildOrdinaryMap() {
    std::map<Key, Value> result;
    for(size_t bucket = 0; bucket < bucket_count_; ++bucket) {
        map_pool_[bucket].second.lock();
    }
    for(size_t bucket = 0; bucket < bucket_count_; ++bucket) {
        result.insert(map_pool_[bucket].first.begin(), map_pool_[bucket].first.end());
    }
    for(size_t bucket = 0; bucket < bucket_count_; ++bucket) {
        map_pool_[bucket].second.unlock();
    }
    return result;
}
