#pragma once

#include <unordered_set>
#include <mutex>

namespace GIS {

    template <typename T, typename Hash = std::hash<T>>
    class MutexSet {
    public:

        using KeyType = T;

        bool Insert( const KeyType & key ) 
        {
            std::lock_guard<std::mutex> guard { mutex_ };
            return set_.insert( key ).second;
        }

        bool Exists( const KeyType & key ) 
        {
            std::lock_guard<std::mutex> guard { mutex_ };
            return set_.count( key );
        }

        bool Remove( const KeyType & key ) 
        {
            std::lock_guard<std::mutex> guard { mutex_ };
            auto iter = set_.find( key );
            if( iter != set_.end() )
            {
                set_.erase( iter );
                return true;
            }
            return false;
        }

    private:

        std::mutex mutex_;

        std::unordered_set<T, Hash> set_;

    };

}