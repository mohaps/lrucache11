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

// compile with g++ -std=c++11 -o sample_main LRUCache11.cpp -lpthread
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>

#include "LRUCache11.hpp"

using namespace lru11;
typedef kv::Node<std::string, int32_t> KVNode;
typedef kv::List<std::string, int32_t> KVList;
typedef Cache<std::string, int32_t> KVCache;

// test the vanilla version of the cache
void testNoLock() {

	// with no lock
	auto cachePrint =
			[&] (const KVCache& c) {
				std::cout << "Cache (size: "<<c.size()<<") (max="<<c.getMaxSize()<<") (e="<<c.getElasticity()<<") (allowed:" << c.getMaxAllowedSize()<<")"<< std::endl;
				size_t index = 0;
				auto nodePrint = [&] (const KVNode& n) {
					std::cout << " ... [" << ++index <<"] " << n.key << " => " << n.value << std::endl;
				};
				c.cwalk(nodePrint);
			};
	KVCache c(5, 2);
	c.insert("hello", 1);
	c.insert("world", 2);
	c.insert("foo", 3);
	c.insert("bar", 4);
	c.insert("blanga", 5);
	cachePrint(c);
	c.insert("toodloo", 6);
	cachePrint(c);
	c.insert("wagamama", 7);
	cachePrint(c);
	c.get("blanga");
	std::cout << "... blanga should move to the bottom ..." << std::endl;
	cachePrint(c);
	c.get("foo");
	std::cout << "... foo should move to the bottom ..." << std::endl;
	cachePrint(c);
}

// Test a thread-safe version of the cache with a std::mutex
void testWithLock() {

	using LCache = Cache<std::string, std::string, std::mutex>;
	auto cachePrint2 =
				[&] (const LCache& c) {
				std::cout << "Cache (size: "<<c.size()<<") (max="<<c.getMaxSize()<<") (e="<<c.getElasticity()<<") (allowed:" << c.getMaxAllowedSize()<<")"<< std::endl;
				size_t index = 0;
				auto nodePrint = [&] (const LCache::node_type& n) {
					std::cout << " ... [" << ++index <<"] " << n.key << " => " << n.value << std::endl;
				};
				c.cwalk(nodePrint);
	};
	// with a lock
	LCache lc(25,2);
	auto worker = [&] () {
		std::ostringstream os;
		os << std::this_thread::get_id();
		std::string id = os.str();

		for (int i = 0; i < 10; i++) {
			std::ostringstream os2;
			os2 << "id:"<<id<<":"<<i;
			lc.insert(os2.str(), id);

		}
	};
	std::vector<std::unique_ptr<std::thread>> workers;
	workers.reserve(100);
	for (int i = 0; i < 100; i++) {
		workers.push_back(std::unique_ptr<std::thread>(
				new std::thread(worker)));
	}

	for (const auto& w : workers) {
		w->join();
	}
	std::cout << "... workers finished!" << std::endl;
	cachePrint2(lc);
}

int main(int argc, char** argv) {

	testNoLock();
	testWithLock();
	return 0;
}
