#include <ilcplex/cplex.h>

extern "C" {
#include "../concorde/concorde.h"
}

#include "TSP.hpp"
#include "parsing.hpp"

#define PURGEABLE CPX_USECUT_FILTER
#define EPSILON 1E-6
#define BIG_INTEGER_SOL 1000000000


// x variables definition and "edges for node = 2" constraints
void build_sym_x(TSPInstance &inst, CPXENVptr env, CPXLPptr lp) {
    double zero = 0.0;

    char binary = 'B';
    double ub = 1.0;
    for (int i = 0; i < inst.dimension; ++i) {
        for (int j = i + 1; j < inst.dimension; ++j) {
            double obj = inst.dist(i, j);
            if (inst.dist(i, j) != inst.elength[inst.xpos_sym(i, j)]) fmt::print(stderr, " wrong edge length {} {}\n", i, j);

            if (CPXnewcols(env, lp, 1, &obj, &zero, &ub, &binary, NULL)) fmt::print(stderr, " wrong CPXnewcols on x var.s\n");
            if (CPXgetnumcols(env, lp) - 1 != inst.xpos_sym(i, j)) fmt::print(stderr, " wrong position for x var.s\n");
        }
    }

    double rhs = 2.0;
    char sense = 'E';
    for (int h = 0; h < inst.dimension; ++h)  // out-degree
    {
        int lastrow = CPXgetnumrows(env, lp);
        if (CPXnewrows(env, lp, 1, &rhs, &sense, NULL, NULL)) fmt::print(stderr, " Wrong CPXnewrows [deg]\n");

        for (int i = 0; i < inst.dimension; ++i) {
            if (i != h)
                if (CPXchgcoef(env, lp, lastrow, inst.xpos_sym(i, h), 1.0)) fmt::print(stderr, " Wrong CPXchgcoef [x1]\n");
        }
    }

    CPXwriteprob(env, lp, "model.lp", NULL);
}

int main(int argc, char **argv) {
    if (argc < 2) { return 1; }

    auto inst = TSPInstance(std::string(argv[1]));

    CPXENVptr env;
    CPXLPptr lp;

    int error;
    env = CPXopenCPLEX(&error);
    lp = CPXcreateprob(env, &error, "TSP");
    error = CPXsetintparam(env, CPXPARAM_Read_DataCheck, CPX_DATACHECK_ASSIST);
    CPXsetintparam(env, CPXPARAM_RandomSeed, 0);

    build_sym_x(inst, env, lp);
    generic_usrcallback_solve(inst, env, lp);

    if (CPXfreeprob(env, &lp)) fmt::print(stderr, "Error during deallocating the problem.\n");
    if (CPXcloseCPLEX(&env)) fmt::print(stderr, "Error closing CPLEX.\n");
}