#include <utility>
#include <iostream>

//
// Created by Khyber on 1/8/2019.
//

#include "Test.h"
#include "Test.h"


bool Test::run(const size_t testNum, std::ostream& out, std::ostream& err) const {
    out << "running ";
    printNameAndNumber(testNum, out);
    out << ":" << std::endl;
    const bool succeeded = test();
    printSuccess(succeeded, testNum, out);
    if (!succeeded) {
        printSuccess(succeeded, testNum, err);
    }
    return succeeded;
}

void Test::printNameAndNumber(size_t testNum, std::ostream& out) const {
    ou