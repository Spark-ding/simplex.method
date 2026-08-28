#include "simplex.h"

#include <stdio.h>
#include <stdlib.h>

static int read_problem(const char *path, LinearProgram *problem,
                        double **objective, double **coefficients, double **rhs) {
    FILE *file = fopen(path, "r");
    unsigned long constraints;
    unsigned long variables;
    if (file == NULL || fscanf(file, "%lu %lu", &constraints, &variables) != 2 ||
        constraints == 0 || variables == 0) {
        if (file != NULL) {
            fclose(file);
        }
        return 0;
    }
    problem->constraints = (size_t)constraints;
    problem->variables = (size_t)variables;

    *objective = malloc(problem->variables * sizeof(**objective));
    *coefficients = malloc(problem->constraints * problem->variables *
                           sizeof(**coefficients));
    *rhs = malloc(problem->constraints * sizeof(**rhs));
    if (*objective == NULL || *coefficients == NULL || *rhs == NULL) {
        fclose(file);
        free(*objective);
        free(*coefficients);
        free(*rhs);
        return 0;
    }

    for (size_t i = 0; i < problem->variables; ++i) {
        if (fscanf(file, "%lf", &(*objective)[i]) != 1) {
            fclose(file);
            return 0;
        }
    }
    for (size_t row = 0; row < problem->constraints; ++row) {
        for (size_t column = 0; column < problem->variables; ++column) {
            if (fscanf(file, "%lf", &(*coefficients)[row * problem->variables + column]) != 1) {
                fclose(file);
                return 0;
            }
        }
        if (fscanf(file, "%lf", &(*rhs)[row]) != 1) {
            fclose(file);
            return 0;
        }
    }
    fclose(file);
    problem->objective = *objective;
    problem->coefficients = *coefficients;
    problem->rhs = *rhs;
    return 1;
}

int main(int argc, char **argv) {
    double default_objective[] = {3.0, 2.0};
    double default_coefficients[] = {1.0, 1.0, 4.0, 1.0};
    double default_rhs[] = {4.0, 8.0};
    LinearProgram problem = {2, 2, default_objective, default_coefficients,
                             default_rhs};
    double *owned_objective = NULL;
    double *owned_coefficients = NULL;
    double *owned_rhs = NULL;

    if (argc > 2) {
        fprintf(stderr, "用法: %s [problem.txt]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2 && !read_problem(argv[1], &problem, &owned_objective,
                                   &owned_coefficients, &owned_rhs)) {
        fprintf(stderr, "无法读取问题文件: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    double *solution = calloc(problem.variables, sizeof(*solution));
    double objective_value = 0.0;
    SimplexStatus status = simplex_solve(&problem, solution, &objective_value);
    printf("status: %s\n", simplex_status_string(status));
    if (status == SIMPLEX_OPTIMAL) {
        printf("objective: %.6f\n", objective_value);
        for (size_t i = 0; i < problem.variables; ++i) {
            printf("x%zu = %.6f\n", i + 1, solution[i]);
        }
    }

    free(solution);
    free(owned_objective);
    free(owned_coefficients);
    free(owned_rhs);
    return status == SIMPLEX_OPTIMAL ? EXIT_SUCCESS : EXIT_FAILURE;
}
