//
// Created by Khyber on 1/8/2019.
//

#ifndef ScratchWasmRenderer_Tests_H
#define ScratchWasmRenderer_Tests_H

#include "Test.h"

class Tests {

private:
    
    const std::vector<Test> tests;

public:
    
    mutable bool stopAfterFirstFailure = true;
    
    template <size_t N>
    explicit Tests(Test (& tests)[N])
            : tests(tests, tests + N) {}
    
    bool run(std::ostream& out, std::ostream& err) const;
    
};

#endif //ScratchWasmRenderer_Tests_H
