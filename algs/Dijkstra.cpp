#include "Dijkstra.hpp"

#include <algorithm>


/**
 * Returns the shortest path between src and dst in symmetric graphs.
 * */
std::vector<edge_t> cav::Dijkstra::solve(std::vector<len_t>& ecosts, node_t src, node_t dst) {

    node_t nnodes = inst.get_nodes_num();
    Q.reset();
    nodes.assign(nnodes, DijkNode(UNINIT_EDGE, 0));

    add_or_update_adj_nodes(ecosts, dst);

    while (!Q.empty()) {
        node_t u = Q.get()->get_node_idx(nodes);
        if (u == src) { return make_path(src, dst); }

        add_or_update_adj_nodes(ecosts, u);
    }

    return std::vector<edge_t>();
}

void cav::Dijkstra::add_or_update_adj_nodes(std::vector<len_t>& ecosts, node_t u) {

    for (auto [n, _] : inst.get_adjacent(u)) {

        const std::vector<edge_t>& p_eids = inst.get_parallel_edges(u, n);
        edge_t best_eid = *std::min_element(p_eids.begin(), p_eids.end(), [&ecosts](edge_t e1, edge_t e2) { return ecosts[e1] < ecosts[e2]; });

        if (ecosts[best_eid] >= FORBIDDEN_LEN) continue;

        len_t curr_dist = nodes[u].dist + ecosts[best_eid];
        if (nodes[n].edge == UNINIT_EDGE) {
            nodes[n] = DijkNode(best_eid, curr_dist);
            Q.insert(std::addressof(nodes[n]));

        } else if (nodes[n].dist > curr_dist) {
            nodes[n].edge = best_eid;
            Q.update(nodes[n].hidx, curr_dist);
        }
    }
}

std::vector<edge_t> cav::Dijkstra::make_path(node_t src, node_t dst) {

    std::vector<edge_t> path;
    edge_t n = src;
    do {
        path.emplace_back(nodes[n].edge);
        auto [a, b] = inst.get_nodes_of_edge(nodes[n].edge);
        n = (n != a ? a : b);
    } while (n != dst);

    return path;
}