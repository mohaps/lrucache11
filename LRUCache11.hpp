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
#pragma once
#include <cstdint>
#include <stdexcept>
#include <thread>
#include <unordered_map>

namespace lru11 {
/**
 * base class to prevent copy
 * use as ClassName : private NoCopy {}
 * to prevent copy constructor of ClassName and assignment by copy
 */
class NoCopy {
public:
	virtual ~NoCopy() = default;
protected:
	NoCopy() = default;
private:
	NoCopy(const NoCopy&) = delete;
	const NoCopy& operator =(const NoCopy&) = delete;
};
/*
 * a noop lockable concept that can be used in place of std::mutex
 */
class NullLock {
public:
	void lock() {
	}
	void unlock() {
	}
	bool try_lock() {
		return true;
	}
};

/**
 * The internal KV namespace that implements a doubly linked list of KV Nodes
 * These impls are intended for use by the LRU Cache only and shouldn't be used
 * as generic impls. To achieve O(1) operations, the doubly linked list class makes
 * some assumptions about the node pointers passed to remove() i.e. that the node already
 * exists in the list and assumes that the node key is not present for push()
 *
 * This allows us to update the size without scanning the whole list
 *
 * when used from the Cache call sites, the map of keys to KVNode* in it
 * makes sure these conditions are enforced.
 */
namespace kv {

/**
 * a node in a doubly-linked list that has a key and a value
 */
template<class Key, class Value>
struct Node: private NoCopy {
	typedef Key key_type;
	typedef Value value_type;
	Node* prev;
	Node* next;
	key_type key;
	value_type value;
	Node(const key_type& k, const value_type& v) :
			prev(nullptr), next(nullptr), key(k), value(v) {
	}
	virtual ~Node() {
		cleanup();
	}
	void cleanup() {
		delete next;
		next = nullptr;
		prev = nullptr;
	}
	void unlink() {
		if (next) {
			next->prev = prev;
		}
		if (prev) {
			prev->next = next;
		}
		next = nullptr;
		prev = nullptr;
	}
	template<typename F>
	void walk(F& f) {
		f(*this);
		if (this->next) {
			this->next->walk(f);
		}
	}
	template<typename F>
	void rwalk(F& f) {
		f(*this);
		if (this->prev) {
			this->prev->rwalk(f);
		}
	}
	template<typename F>
	void cwalk(F& f) const {
		f(*this);
		if (this->next) {
			this->next->cwalk(f);
		}
	}
	template<typename F>
	void crwalk(F& f) const {
		f(*this);
		if (this->prev) {
			this->prev->crwalk(f);
		}
	}
};

/**
 * the doubly-linked list impl
 */
template<typename K, typename V>
struct List {
	typedef Node<K, V> node_type;
	node_type* head;
	node_type* tail;
	size_t len;
	List() :
			head(nullptr), tail(nullptr), len(0) {
	}
	virtual ~List() {
		clear();
	}
	void clear() {
		delete head;
		head = nullptr;
		tail = nullptr;
		len = 0;
	}

	bool empty() const {
		return head == nullptr;
	}

	size_t size() const {
		return len;
	}

	node_type* pop() {
		if (empty()) {
			return nullptr;
		}
		node_type* newHead = head->next;
		head->unlink();
		node_type* oldHead = head;
		head = newHead;
		--len;
		if (len == 0) {
			tail = nullptr;
		}
		return oldHead;
	}
	// assumes n exists in list
	node_type* remove(node_type* n) {
		if (!n) {
			return nullptr;
		}
		if (n == head) {
			head = n->next;
		}
		if (n == tail) {
			tail = n->prev;
		}
		n->unlink();
		--len;
		return n;
	}
	// assumes n doesn't already exist in list
	void push(node_type* n) {
		if (!n) {
			return;
		}
		n->unlink();
		if (!head) {
			head = n;
		} else if (head == tail) {
			head->next = n;
		} else {
			tail->next = n;
			n->prev = tail;
		}
		tail = n;
		++len;
	}
};
}

/**
 * error raised when a key not in cache is passed to get()
 */
class KeyNotFound: public std::invalid_argument {
public:
	KeyNotFound() :
			std::invalid_argument("key_not_found") {
	}
};

/**
 *	The LRU Cache class templated by
 *		Key - key type
 *		Value - value type
 *		MapType - an associative container like std::unordered_map
 *		LockType - a lock type derived from the Lock class (default: NullLock = no synchronization)
 *
 *	The default NullLock based template is not thread-safe, however passing Lock=std::mutex will make it
 *	thread-safe
 */
template<class Key, class Value, class Lock = NullLock,
		class Map = std::unordered_map<Key, kv::Node<Key, Value>*>>
class Cache: private NoCopy {
public:
	typedef kv::Node<Key, Value> node_type;
	typedef kv::List<Key, Value> list_type;
	typedef Map map_type;
	typedef Lock lock_type;
	using Guard = std::lock_guard<lock_type>;
	/**
	 * the max size is the hard limit of keys and (maxSize + elasticity) is the soft limit
	 * the cache is allowed to grow till maxSize + elasticity and is pruned back to maxSize keys
	 * set maxSize = 0 for an unbounded cache (but in that case, you're better off using a std::unordered_map
	 * directly anyway! :)
	 */
	explicit Cache(size_t maxSize = 64, size_t elasticity = 10) :
			maxSize_(maxSize), elasticity_(elasticity) {
	}
	virtual ~Cache() = default;
	size_t size() const {
		Guard g(lock_);
		return cache_.size();
	}
	bool empty() const {
		Guard g(lock_);
		return cache_.empty();
	}
	void clear() {
		Guard g(lock_);
		cache_.clear();
		keys_.clear();
	}
	void insert(const Key& k, const Value& v) {
		Guard g(lock_);
		auto iter = cache_.find(k);
		if (iter != cache_.end()) {
			iter->second->value = v;
			keys_.remove(iter->second);
			keys_.push(iter->second);
			return;
		}
		node_type* n = new node_type(k, v);
		cache_[k] = n;
		keys_.push(n);
		prune();
	}
	bool tryGet(const Key& kIn, Value& vOut) {
		Guard g(lock_);
		auto iter = cache_.find(kIn);
		if (iter == cache_.end()) {
			return false;
		}
		keys_.remove(iter->second);
		keys_.push(iter->second);
		vOut = iter->second->value;
		return true;
	}
	const Value& get(const Key& k) {
		Guard g(lock_);
		auto iter = cache_.find(k);
		if (iter == cache_.end()) {
			throw KeyNotFound();
		}
		keys_.remove(iter->second);
		keys_.push(iter->second);
		return iter->second->value;
	}
	bool remove(const Key& k) {
		Guard g(lock_);
		auto iter = cache_.find(k);
		if (iter == cache_.end()) {
			return false;
		}
		keys_.remove(iter->second);
		delete iter->second;
		cache_.erase(iter);
		return true;
	}
	bool contains(const Key& k) {
		Guard g(lock_);
		return cache_.find(k) != cache_.end();
	}

	size_t getMaxSize() const {
		return maxSize_;
	}
	size_t getElasticity() const {
		return elasticity_;
	}
	size_t getMaxAllowedSize() const {
		return maxSize_ + elasticity_;
	}
	template<typename F>
	void cwalk(F& f) const {
		Guard g(lock_);
		if (keys_.empty()) {
			return;
		}
		keys_.head->cwalk(f);
	}
protected:
	size_t prune() {
		size_t maxAllowed = maxSize_ + elasticity_;
		if (maxSize_ == 0 || cache_.size() < maxAllowed) {
			return 0;
		}
		size_t count = 0;
		while (cache_.size() > maxSize_) {
			node_type* n = keys_.pop();
			std::unique_ptr<node_type> upn(n);
			cache_.erase(n->key);
			++count;
		}
		return count;
	}
private:
	mutable Lock lock_;
	Map cache_;
	list_type keys_;
	size_t maxSize_;
	size_t elasticity_;
};
}
