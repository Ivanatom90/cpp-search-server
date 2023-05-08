#pragma once
#include <mutex>
#include <map>
#include <vector>

template <typename Key, typename Value>
class ConcurrentMap {
public:
    struct Busket{
        std::mutex mut;
        std::map<Key, Value> busket_map;
    };
    struct Access {
        Access(Busket& busket, const Key key): guard(busket.mut), ref_to_value(busket.busket_map[key])  {}
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };



    explicit ConcurrentMap(size_t bucket_count):
        bucket_count_(bucket_count),
        buskets_(bucket_count){}


    Access operator[](const Key& key){
        uint64_t index = (static_cast<uint64_t>(key)) % bucket_count_;
        return { Access(buskets_[index], key)};
    }

    std::map<Key, Value> BuildOrdinaryMap(){
        std::map<Key, Value> res;
        for(auto& map : buskets_){
            std::lock_guard guard(map.mut);
            res.insert(map.busket_map.begin(), map.busket_map.end());
        }
        return res;
    }

    void Erase(const Key& key){
        uint64_t index = (static_cast<uint64_t>(key)) % bucket_count_;
        std::lock_guard(buskets_[index].mut);
        buskets_[index].busket_map.erase(key);
    }

private:
    const size_t bucket_count_;
    std::vector<Busket> buskets_;
};
