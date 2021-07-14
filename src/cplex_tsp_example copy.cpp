#include <ilcplex/cplex.h>

extern "C" {
#include "../concorde/concorde.h"
}

#include "TSP.hpp"

#define PURGEABLE CPX_USECUT_FILTER
#define EPSILON 1E-6
#define BIG_INTEGER_SOL 1000000000

struct generic_input {
    TSPInstance *inst;
    double *ones_for_cplex;
    struct t_local {
        TSPInstance *inst;
        double *ones_for_cplex;
        CPXCALLBACKCONTEXTptr context;
        int ncuts;
        double *xstar;
        int *rmatind;
    } * l;
};

int generic_doit_fn_concorde([[maybe_unused]] double cutval, int nnodescut, int *cut, void *in) {
    generic_input::t_local *datal = (generic_input::t_local *)in;

    if (cutval > 2.0) {
        fmt::print(stderr, "Warning: Cut of value %f in add_exact\n", cutval);
        return 0;
    }

    TSPInstance *inst = datal->inst;
    int *rmatind = datal->rmatind;

    int nnz = 0, rmatbeg = 0;
    char sense = 'L';
    for (int i = 0; i < nnodescut; i++) {
        for (int j = i + 1; j < nnodescut; j++) { rmatind[nnz++] = inst->xpos_sym(cut[i], cut[j]); }
    }

    double rhs = nnz - 1;
    int purgeable = PURGEABLE, local = 0;
    if (CPXcallbackaddusercuts(datal->context, 1, nnz, &rhs, &sense, &rmatbeg, rmatind, datal->ones_for_cplex, &purgeable, &local))
        fmt::print(stderr, "Error CPXcallbackaddusercuts");

    ++datal->ncuts;

    return 0;
}

int generic_addSec_concorde_frac(generic_input *data, double *xstar, CPXCALLBACKCONTEXTptr context, int t) {
    TSPInstance &inst = *data->inst;
    int *rmatind = data->l[t].rmatind;
    int ncomp = 0, nsec = 0, *compscount = NULL, *comps = NULL;

    // find connected component with concorde
    if (CCcut_connect_components(inst.dimension, inst.ecount, inst.elist, xstar, &ncomp, &compscount, &comps))
        fmt::print(stderr, "Error in CCcut_connect_components");

    if (ncomp == 1) {
        data->l[t].context = context;
        data->l[t].ncuts = 0;
        if (CCcut_violated_cuts(inst.dimension, inst.ecount, inst.elist, xstar, 2.0 - EPSILON, generic_doit_fn_concorde, (void *)data->l))
            fmt::print(stderr, "Error in CCcut_violated_cuts");
        nsec = data->l[t].ncuts;

    } else if (ncomp > 1) {
        int nnz = 0, num_comp_pred = 0, rmatbeg = 0, purgeable = 1, local = 0;
        char sense = 'L';
        for (int i = 0; i < ncomp; i++) {
            int num_comp_current = compscount[i];
            nnz = 0;
            for (int j = 0; j < num_comp_current; j++) {
                for (int k = j + 1; k < num_comp_current; k++) { rmatind[nnz++] = inst.xpos_sym(comps[num_comp_pred + j], comps[num_comp_pred + k]); }
            }

            ++nsec;
            double rhs = num_comp_current - 1;
            if (CPXcallbackaddusercuts(context, 1, nnz, &rhs, &sense, &rmatbeg, rmatind, data->ones_for_cplex, &purgeable, &local))
                fmt::print(stderr, "Error CPXcallbackaddusercuts");
            num_comp_pred += num_comp_current;
        }
    }

    return nsec;
}


int generic_addSec_concorde(generic_input *data, double *xstar, CPXCALLBACKCONTEXTptr context, int t) {
    TSPInstance &inst = *data->inst;
    int *rmatind = data->l[t].rmatind;
    int ncomp = 0, *compscount = NULL, *comps = NULL;

    // find connected component with concorde
    CCcut_connect_components(inst.dimension, inst.ecount, inst.elist, xstar, &ncomp, &compscount, &comps);

    int num_comp_pred = 0, rmatbeg = 0, nnz = 0, nsec = 0;
    char sense = 'L';
    if (ncomp > 1) {
        for (int i = 0; i < ncomp; i++) {
            int num_comp_current = compscount[i];
            nnz = 0;
            for (int j = 0; j < num_comp_current; j++) {
                for (int k = j + 1; k < num_comp_current; k++) { rmatind[nnz++] = inst.xpos_sym(comps[num_comp_pred + j], comps[num_comp_pred + k]); }
            }

            nsec++;
            double rhs = num_comp_current - 1;
            if (CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &rmatbeg, rmatind, data->ones_for_cplex))
                fmt::print(stderr, "Error CPXcallbackrejectcandidate");
            num_comp_pred += num_comp_current;
        }
    }

    return nsec;
}

static int CPXPUBLIC my_generic_lazycallback(CPXCALLBACKCONTEXTptr context, [[maybe_unused]] CPXLONG contextid, void *cbhandle) {
    auto data = (generic_input *)cbhandle;
    TSPInstance &inst = *data->inst;

    int mythread = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);

    // get solution
    double objval = CPX_INFBOUND;
    double *xstar = data->l[mythread].xstar;
    if (CPXcallbackgetcandidatepoint(context, xstar, 0, inst.ecount - 1, &objval)) fmt::print(stderr, "Error get node in callback");

    // apply cut separator and possibly add violated cuts
    int ncuts = generic_addSec_concorde(data, xstar, context, mythread);

    double zbest = -1;
    CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &zbest);
    if (zbest > BIG_INTEGER_SOL) zbest = -1;

    if (ncuts) {
        fmt::print("\n---------------------GENERIC-LAZY-CALLBACK------------------\n");
        fmt::print("	Thread number:      {}\n", mythread);
        fmt::print("	Number Sec added:   {}\n", ncuts);
        fmt::print("	Objfunc Value:      {}\n", objval);
        fmt::print("	Best int solution:  {}\n", zbest);
        fmt::print("--------------------------------------------------------------\n");
    } else {
        int tourcost = 0;
        for (int i = 0; i < inst.ecount; ++i) {
            if (xstar[i] > 0.5) tourcost += inst.elength[i];
        }
        fmt::print("\n---------------------GENERIC-LAZY-CALLBACK------------------\n");
        fmt::print("	Objfunc Value:                  {}\n", objval);
        fmt::print("	*Best int solution by cplex:    {}\n", zbest);
        fmt::print("	*Best int solution by hand:     {}\n", tourcost);
        fmt::print("--------------------------------------------------------------\n");
    }

    return 0;
}

static int CPXPUBLIC my_generic_usrcallback(CPXCALLBACKCONTEXTptr context, [[maybe_unused]] CPXLONG contextid, void *cbhandle) {
    int nodecount = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_NODECOUNT, &nodecount);
    /* if (nodecount > 10) {
        if (nodecount & 127) return 0;
        else if (nodecount > 16384)
            return 0;
    } */

    auto data = (generic_input *)cbhandle;
    TSPInstance &inst = *data->inst;

    int mythread = -1;
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &mythread);

    double zbest = -1;
    CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &zbest);
    if (zbest > BIG_INTEGER_SOL) zbest = -1;

    // get solution
    double objval = CPX_INFBOUND;
    double *xstar = data->l[mythread].xstar;
    if (CPXcallbackgetrelaxationpoint(context, xstar, 0, inst.ecount - 1, &objval)) fmt::print(stderr, "Error get node in callback");

    // apply cut separator and possibly add violated cuts
    int ncuts = 0;
    if (objval < zbest) { ncuts = generic_addSec_concorde_frac(data, xstar, context, mythread); }

    if (ncuts) {
        fmt::print("\n--------------------GENERIC-USER-CALLBACK---------------------\n");
        fmt::print("	Thread number:          {}\n", mythread);
        fmt::print("	Number Sec added:       {}\n", ncuts);
        fmt::print("	Objfunc Value:          {}\n", objval);
        fmt::print("	Best integer solution:  {}\n", zbest);
        fmt::print("	Number of nodes solved: {}\n", nodecount);
        fmt::print("--------------------------------------------------------------\n");
    }

    return 0;
}

static int CPXPUBLIC my_generic_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *cbhandle) {
    switch (contextid) {
    case CPX_CALLBACKCONTEXT_CANDIDATE:
        return my_generic_lazycallback(context, contextid, cbhandle);
        break;
    case CPX_CALLBACKCONTEXT_RELAXATION:
        return my_generic_usrcallback(context, contextid, cbhandle);
        break;
    }
    return 0;
}

void generic_callback_solve(TSPInstance &inst, CPXENVptr env, CPXLPptr lp) { }

void generic_usrcallback_solve(TSPInstance &inst, CPXENVptr env, CPXLPptr lp) {
    int ncores = 1;
    CPXgetnumcores(env, &ncores);

    generic_input data;
    data.inst = &inst;
    data.ones_for_cplex = new double[inst.ecount];
    std::fill(data.ones_for_cplex, data.ones_for_cplex + inst.ecount, 1.0);
    data.l = new generic_input::t_local[ncores];
    for (int i = 0; i < ncores; ++i) {
        data.l[i].inst = data.inst;
        data.l[i].ones_for_cplex = data.ones_for_cplex;
        data.l[i].rmatind = new int[inst.ecount];
        data.l[i].xstar = new double[inst.ecount];
    }

    CPXcallbacksetfunc(env, lp, CPX_CALLBACKCONTEXT_RELAXATION | CPX_CALLBACKCONTEXT_CANDIDATE, my_generic_callback, &data);

    // CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
    CPXsetintparam(env, CPX_PARAM_MIPDISPLAY, 2);
    CPXsetintparam(env, CPX_PARAM_THREADS, ncores);
    assert(CPXgetnumcols(env, lp) == inst.ecount);

    double gaplimit = 0.000001;
    CPXsetdblparam(env, CPX_PARAM_EPGAP, gaplimit);

    if (CPXmipopt(env, lp)) fmt::print(stderr, "Error resolving the model.\n");

    CPXcallbacksetfunc(env, lp, CPX_CALLBACKCONTEXT_RELAXATION | CPX_CALLBACKCONTEXT_CANDIDATE, NULL, &data);

    for (int i = 0; i < ncores; ++i) {
        delete[] data.l[i].xstar;
        delete[] data.l[i].rmatind;
    }
    delete[] data.l;
    delete[] data.ones_for_cplex;
}


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