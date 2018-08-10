#pragma once

#include <queue>
#include <mutex>

namespace GIS 
{

    template <typename T>
    class MutexQueue 
    {  
    public:

        void Enqueue( const T & data ) 
        {
            std::unique_lock<std::mutex> guard { mutex_ };
            queue_.push( data );
        }

        bool TryDequeue( T & data ) {
            std::unique_lock<std::mutex> guard { mutex_ };
            if( queue_.empty() ) 
                return false;
            data_ = queue_.front();
            queue_.pop();
        }

    private:

        std::mutex mutex_;

        std::queue<T> queue_;

    };

}