#pragma once

#include <unordered_map>
#include <mutex>

namespace GIS {

    template <typename Key, typename Value>
    class MutexMap {
    public:

        using KeyType = Key;

        using ValueType = Value;

        bool Exists( const KeyType & key ) const noexcept 
        {
            std::lock_guard<std::mutex> guard { mutex_ };
            return map_.find( )
        }

        bool Remove( const KeyType & key ) 
        {
            std::lock_guard<std::mutex> guard { mutex_ };
            auto iter = map_.find( key );
            if( iter != map_.end() )
            {
                map_.erase( iter );
                return true;
            }
            return false;
        }

        void SetValue( const KeyType & key, const ValueType & value ) 
        {
            std::lock_guard<std::mutex> guard { mutex_ };
            map_[key] = value;
        }

        bool GetValue( const KeyType & key, ValueType & value ) 
        {
            std::lock_guard<std::mutex> guard { mutex_ };
            auto iter = map_.find( key );
            if( map_.end() != iter ) 
            {
                value = iter->second;
                return true;
            }
            return false;
        }

        bool GetValue( const KeyType & key, ValueType & value ) const 
        {
            std::lock_guard<std::mutex> guard { mutex_ };
            auto iter = map_.find( key );
            if( map_.end() != iter ) 
            {
                value = iter->second;
                return true;
            }
            return false;
        }

    private:

        mutable std::mutex mutex_;

        std::unordered_map<KeyType, ValueType> map_;

    };

}