//
// Created by Khyber on 1/8/2019.
//

#include "Tests.h"

#include <ostream>

bool Tests::run(std::ostream& out, std::ostream& err) const {
    bool anyFailed = false;
    out << "running " << tests.size() << " tests:" << std::endl;
    for (size_t i = 0; i < tests.size(); i++) {
        const bool succeeded = tests[i].run(i + 1, out, err);
        if (!succeeded) {
            anyFailed = true;
            if (stopAfterFirstFailure) {
                return false;
            }
        }
    }
    return anyFailed;
}
