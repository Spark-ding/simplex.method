#include "simplex.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#define EPSILON 1e-9

static double *cell(double *tableau, size_t width, size_t row, size_t column) {
    return &tableau[row * width + column];
}

static void pivot(double *tableau, size_t width, size_t rows,
                  size_t pivot_row, size_t pivot_column) {
    double pivot_value = *cell(tableau, width, pivot_row, pivot_column);

    for (size_t column = 0; column < width; ++column) {
        *cell(tableau, width, pivot_row, column) /= pivot_value;
    }

    for (size_t row = 0; row < rows; ++row) {
        if (row == pivot_row) {
            continue;
        }

        double factor = *cell(tableau, width, row, pivot_column);
        if (fabs(factor) < EPSILON) {
            continue;
        }

        for (size_t column = 0; column < width; ++column) {
            *cell(tableau, width, row, column) -=
                factor * *cell(tableau, width, pivot_row, column);
        }
    }
}

SimplexStatus simplex_solve(const LinearProgram *problem,
                            double *solution,
                            double *objective_value) {
    if (problem == NULL || solution == NULL || objective_value == NULL ||
        problem->constraints == 0 || problem->variables == 0 ||
        problem->objective == NULL || problem->coefficients == NULL ||
        problem->rhs == NULL) {
        return SIMPLEX_INVALID_INPUT;
    }

    size_t m = problem->constraints;
    size_t n = problem->variables;
    size_t width = n + m + 1;
    size_t rows = m + 1;
    double *tableau = calloc(rows * width, sizeof(*tableau));
    size_t *basis = malloc(m * sizeof(*basis));
    if (tableau == NULL || basis == NULL) {
        free(tableau);
        free(basis);
        return SIMPLEX_NUMERIC_ERROR;
    }

    for (size_t row = 0; row < m; ++row) {
        if (problem->rhs[row] < -EPSILON) {
            free(tableau);
            free(basis);
            return SIMPLEX_INVALID_INPUT;
        }
        basis[row] = n + row;
        for (size_t column = 0; column < n; ++column) {
            *cell(tableau, width, row, column) =
                problem->coefficients[row * n + column];
        }
        *cell(tableau, width, row, n + row) = 1.0;
        *cell(tableau, width, row, width - 1) = problem->rhs[row];
    }

    for (size_t column = 0; column < n; ++column) {
        *cell(tableau, width, m, column) = -problem->objective[column];
    }

    while (1) {
        size_t entering = n + m;
        for (size_t column = 0; column < n + m; ++column) {
            if (*cell(tableau, width, m, column) < -EPSILON) {
                entering = column;
                break; /* Bland's rule: choose the smallest index. */
            }
        }
        if (entering == n + m) {
            break;
        }

        size_t leaving = m;
        double best_ratio = DBL_MAX;
        for (size_t row = 0; row < m; ++row) {
            double coefficient = *cell(tableau, width, row, entering);
            if (coefficient > EPSILON) {
                double ratio = *cell(tableau, width, row, width - 1) / coefficient;
                if (ratio < best_ratio - EPSILON ||
                    (fabs(ratio - best_ratio) <= EPSILON &&
                     basis[row] < (leaving < m ? basis[leaving] : SIZE_MAX))) {
                    best_ratio = ratio;
                    leaving = row;
                }
            }
        }

        if (leaving == m) {
            free(tableau);
            free(basis);
            return SIMPLEX_UNBOUNDED;
        }

        pivot(tableau, width, rows, leaving, entering);
        basis[leaving] = entering;
    }

    for (size_t variable = 0; variable < n; ++variable) {
        solution[variable] = 0.0;
    }
    for (size_t row = 0; row < m; ++row) {
        if (basis[row] < n) {
            solution[basis[row]] = *cell(tableau, width, row, width - 1);
        }
    }
    *objective_value = *cell(tableau, width, m, width - 1);

    free(tableau);
    free(basis);
    return SIMPLEX_OPTIMAL;
}

const char *simplex_status_string(SimplexStatus status) {
    switch (status) {
        case SIMPLEX_OPTIMAL:
            return "optimal";
        case SIMPLEX_UNBOUNDED:
            return "unbounded";
        case SIMPLEX_INVALID_INPUT:
            return "invalid input";
        case SIMPLEX_NUMERIC_ERROR:
            return "numeric error";
        default:
            return "unknown status";
    }
}
