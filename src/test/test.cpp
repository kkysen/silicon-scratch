//
// Created by Khyber on 12/30/2018.
//

#include "test.h"

#include <iostream>

#include "Test.h"
#include "Tests.h"

bool alwaysTrue() {
    return true;
}

#define test(func) Test(func, std::string_view(""#func))

static Test tests[] = {
        test(alwaysTrue),
};

#undef test

const bool stopAfterFirstFailure = true;

bool testAll() {
    const Tests _tests(tests);
    _tests.stopAfterFirstFailure = stopAfterFirstFailure;
    return _tests.run(std::cout, std::cerr);
}

int main() {
    return testAll() ? EXIT_SUCCESS : EXIT_FAILURE;
}
