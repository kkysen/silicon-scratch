//
// Created by Khyber on 1/19/2019.
//

#include <iostream>
#include <memory>
#include <vector>
#include <list>
//#include <src/main/util/MappedIterator.h>
#include "../main/util/MappedIterator.h"

class C {

public:
    
    const int x;
    
    C(int x) : x(x) {
        std::cout << "construct" << std::endl;
    }

//    C(const C& c) : x(c.x) {
//        std::cout << "copy" << std::endl;
//    }
    
    C(const C& c) = delete;

//    C(C&& c) noexcept : x(c.x) {
//        std::cout << "move" << std::endl;
//    }
    
    C(C&& c) = delete;
    
    ~C() {
        std::cout << "destruct" << std::endl;
    }
    
};

const C& f(const std::unique_ptr<C>& p) {
    return *p;
}

class V {

public:
    
    std::vector<std::unique_ptr<C>> v;
    
    explicit V(std::vector<int> ints) {
        for (const auto i : ints) {
            v.push_back(std::make_unique<C>(i));
        }
    }
    
    decltype(auto) begin() {
        return iterators::map(v.begin(), f);
    }
    
    decltype(auto) end() {
        return iterators::map(v.end(), f);
    }
    
};

void misc() {

//    {
//        const auto p = std::make_unique<C>(1);
//        const auto& c = f(p);
//        std::cout << c.x << std::endl;
//    }
//
//    {
//        std::vector<C> a {4, 5, 6};
//        auto&& it = a.begin();
//        std::cout << it->x << std::endl;
//    }
//
//    {
//        std::list<C> a {1, 2, 3};
//        auto&& it = a.begin();
//        std::cout << it->x << std::endl;
//    }
    
    {
        std::vector<int> a = {7, 8, 9};
        V v(a);
        
        for (auto&& x : v) {
            std::cout << x.x << std::endl;
        }
    }
}

//int main() {
//    misc();
//}