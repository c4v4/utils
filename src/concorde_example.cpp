
#include <string>

#include "TSP.hpp"

extern "C" {
#include "../concorde/concorde.h"
}

int main(int argc, char** argv) {
    if (argc < 2) { return 1; }

    auto inst = TSPInstance(std::string(argv[1]));

    int rval;
    double optval, timebound = 10000000;
    int success, foundtour, hit_timebound = 0;
    CCrandstate rstate;
    static int silent = 1;
    CCutil_sprand(0, &rstate);

    std::string rname("*" + inst.name + "*");
    remove(rname.c_str());

    char* name = CCtsp_problabel(inst.name.c_str());
    fmt::print("{}\n", name);

    CCdatagroup dat;
    CCutil_init_datagroup(&dat);

    int* tour = new int[inst.dimension];

    rval = CCutil_graph2dat_matrix(inst.dimension, inst.ecount, inst.elist, inst.elength, 100000000, &dat);
    rval = CCtsp_solve_dat(inst.dimension, &dat, NULL, tour, NULL, &optval, &success, &foundtour, name, &timebound, &hit_timebound, silent, &rstate);

    if (hit_timebound) { fmt::print(stderr, "#TIMELIMIT {} \n", timebound); }

    CCutil_freedatagroup(&dat);
    CC_IFFREE(name, char);
    delete[] tour;
    return rval;
}