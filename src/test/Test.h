//
// Created by Khyber on 1/8/2019.
//

#ifndef ScratchWasmRenderer_Test_H
#define ScratchWasmRenderer_Test_H

#include <functional>
#include <string_view>

using TestFunc = bool();

class Test {

private:
    
    TestFunc* test;
    std::string_view name;

public:
    
    constexpr Test(TestFunc* test, std::string_view name) noexcept
            : test(test), name(name) {}
    
    bool run(size_t testNum, std::ostream& out, std::ostream& err) const;

private:
    
    void printNameAndNumber(size_t testNum, std::ostream& out) const;
    
    void printSuccess(bool succeeded, size_t testNum, std::ostream& out) const;
    
};

#endif // ScratchWasmRenderer_Test_H
