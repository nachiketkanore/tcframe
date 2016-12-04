#pragma once

#include <iostream>
#include <stdexcept>

#include "tcframe/spec/io.hpp"
#include "tcframe/spec/variable.hpp"
#include "tcframe/util.hpp"

using std::endl;
using std::istream;
using std::ostream;
using std::runtime_error;

namespace tcframe {

class GridIOSegmentManipulator {
public:
    static string parse(GridIOSegment* segment, istream* in) {
        Matrix* variable = segment->variable();
        variable->clear();
        variable->parseFrom(in, *segment->rows(), *segment->columns());

        return TokenFormatter::formatMatrixElement(variable->name(),
                                                   variable->rows() - 1,
                                                   variable->columns(variable->rows() - 1) - 1);
    }

    static void print(GridIOSegment* segment, ostream* out) {
        checkMatrixSize(segment);

        Matrix* variable = segment->variable();
        variable->printTo(out);
    }

private:
    static void checkMatrixSize(GridIOSegment* segment) {
        Matrix* variable = segment->variable();

        if (variable->rows() != *segment->rows()) {
            throw runtime_error(
                    "Number of rows of matrix " + TokenFormatter::formatVariable(variable->name())
                    + " unsatisfied. Expected: " + StringUtils::toString(*segment->rows())
                    + ", actual: " + StringUtils::toString(variable->rows()));
        }
        for (int r = 0; r < variable->rows(); r++) {
            if (variable->columns(r) != *segment->columns()) {
                throw runtime_error(
                        "Number of columns of row " + StringUtils::toString(r)
                        + " of matrix " + TokenFormatter::formatVariable(variable->name())
                        + " unsatisfied. Expected: " + StringUtils::toString(*segment->columns())
                        + ", actual: " + StringUtils::toString(variable->columns(r)));
            }
        }
    }
};

}
