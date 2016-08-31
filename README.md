LRUCache11
==========

A header only C++11 LRU Cache template class that allows you to define key, value and optionally the Map type. uses a double linked list and a ```std::unordered_map``` style container to provide fast insert, delete and update

No dependencies other than the C++ standard library. This is a C++11 remake of my earlier LRUCache project (https://github.com/mohaps/lrucache)

The goal was to create a fast LRUCache header only library and to avoid any dependencies like boost.

Enjoy and drop me a line.


Usage Example
---------------
```cpp
#include "LRUCache11.hpp"
namespace lru
{
	void test()
	{
		lru11::Cache<std::string, std::string> cache(3,0);
		cache.insert("hello", "world");
		cache.insert("foo", "bar");
		
		std::cout<<"checking refresh : "<<cache.get("hello")<<std::endl;
		cache.insert("hello1", "world1");
		cache.insert("foo1", "bar1");
	}
}

int main(int argc, char **argv)
{
	lru::test();
	return 0;
}
```

Build with ```g++ -o sample_main -std=c++11 SampleMain.cpp```

License
-------

BSD License

```
/*
 * LRUCache11 - a templated C++11 based LRU cache class that allows specification of
 * key, value and optionally the map container type (defaults to std::unordered_map)
 * By using the std::map and a linked list of keys it allows O(1) insert, delete and
 * refresh operations.
 *
 * This is a header-only library and all you need is the LRUCache11.hpp file
 *
 * Github: https://github.com/mohaps/lrucache11
 *
 * This is a follow-up to the LRUCache project - https://github.com/mohaps/lrucache
 *
 * Copyright (c) 2012-22 SAURAV MOHAPATRA <mohaps@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
```


Comments/Crits
---------------

Please contact author at mohaps@gmail.com

Links
--------
* Wikipedia Entry on LRU Caching : http://en.wikipedia.org/wiki/Cache_algorithms#Least_Recently_Used
* Earlier version of the library : https://github.com/mohaps/lrucache
