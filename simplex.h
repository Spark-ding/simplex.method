#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <stddef.h>

typedef enum {
    SIMPLEX_OPTIMAL = 0,
    SIMPLEX_UNBOUNDED = 1,
    SIMPLEX_INVALID_INPUT = 2,
    SIMPLEX_NUMERIC_ERROR = 3
} SimplexStatus;

typedef struct {
    size_t constraints;
    size_t variables;
    const double *objective;
    const double *coefficients;
    const double *rhs;
} LinearProgram;

/* Solve max c^T x subject to A x <= b and x >= 0.
 * The caller owns solution and objective_value. */
SimplexStatus simplex_solve(const LinearProgram *problem,
                            double *solution,
                            double *objective_value);

const char *simplex_status_string(SimplexStatus status);

#endif
