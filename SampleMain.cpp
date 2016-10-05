// this is a simple example, seen in the Readme

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
