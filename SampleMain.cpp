// this is a simple example, seen in the Readme

#include "LRUCache11.hpp"
#include <iostream>

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

	lru11::Cache<int, int> ic(2);
	ic.insert(1, 10);
	ic.insert(2, 20);
	const auto& iref = ic.get(1);
	std::cout << "value : "<< iref << std::endl;
	ic.insert(3, 30);
	std::cout << "value (old) : "<< iref << std::endl; 
    }
}

int main(int argc, char **argv)
{
    lru::test();
    return 0;
}
