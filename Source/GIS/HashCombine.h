#pragma once

#include <functional>

namespace GIS {

	/**
	 * boost的hash combine函数，用于合并多个分量的hash值，碰撞率比较高，下面有测试代码。
	 */
    template <class T>
	inline void HashCombine(std::size_t & seed, const T& v)
	{
	    std::hash<T> hasher;
	    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
	}

	/*
	#include <iostream>
	#include <map>
	#include <boost/functional/hash.hpp>

	size_t hashme(int i, int j, int k)
	{
		std::size_t hash = 0;
		boost::hash_combine(hash, i);
		boost::hash_combine(hash, j);
		boost::hash_combine(hash, k);
		return hash;
	}

	int main(void)
	{
		int collisions;
		std::map<std::size_t, int> myMap;

		collisions = 0;
		myMap.clear();
		for (int i = 0; i < 100; ++i)
			for (int j = 0; j < 100; ++j)
				for (int k = 0; k < 100; ++k)
				{
					int &ref = myMap[hashme(i, j, k)];
					if (ref != 0)
						++collisions;
					++ref;
				}
		std::cout << collisions << " collisions" << std::endl;
	}
	*/

}