#ifndef CAV_DIJKSTRA_HPP
#define CAV_DIJKSTRA_HPP
#include <limits>
#include <vector>

#include "BinaryHeap.hpp"
#include "Instance.hpp"
#include "types.hpp"

namespace cav {

    class Dijkstra {
        //////////// MISH ////////////
    public:
        static constexpr len_t FORBIDDEN_LEN = 10e10;

    private:
        struct DijkNode {
            DijkNode(edge_t edge_, len_t dist_, node_t hidx_ = 0) : edge(edge_), hidx(hidx_), dist(dist_) { }
            inline node_t get_node_idx(const std::vector<DijkNode>& nodes) const { return this - nodes.data(); }
            edge_t edge, hidx;
            len_t dist;
        };

        using NodePQueue = BinaryHeapPtr<DijkNode, &DijkNode::hidx, &DijkNode::dist>;
        static constexpr edge_t UNINIT_EDGE = std::numeric_limits<edge_t>::max();


        //////////// METHODS ////////////
    public:
        Dijkstra(const Instance& inst_) : inst(inst_) { }
        std::vector<edge_t> solve(std::vector<len_t>& ecosts, node_t src, node_t dst);
        inline std::vector<edge_t> operator()(std::vector<len_t>& ecosts, node_t src, node_t dst) { return solve(ecosts, src, dst); }

    private:
        std::vector<edge_t> make_path(node_t src, node_t dst);
        void add_or_update_adj_nodes(std::vector<len_t>& ecosts, node_t u);


        //////////// FIELDS ////////////
    private:
        const Instance& inst;
        std::vector<DijkNode> nodes;
        NodePQueue Q;
    };
}  // namespace cav

#endif