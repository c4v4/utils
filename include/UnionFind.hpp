#ifndef UNION_FIND_HPP
#define UNION_FIND_HPP
#include <stddef.h>

#include <vector>

namespace cav {
    /**
     * @brief Vectorized union find with size and path compression heuristics.
     * The type for the indices can be chosen to further enhace cache locality.
     *
     * @tparam Int integral type that can represent any index of the vector nodes.
     */
    template <typename Int = size_t>
    class UnionFind {
    public:
        struct Node {
            Int size, parent;
        };

    public:
        UnionFind(Int size) {
            nodes.resize(size);
            for (Int i = 0; i < size; ++i) { nodes[i] = {1, i}; }
        }

        inline Int make_set() {
            Int old_size = nodes.size();
            nodes.emplace_back(1, old_size);
            return old_size;
        }

        Int find(Int n) {
            Int &p = nodes[n].parent;
            if (n != p) p = find(p);
            return p;
        }

        inline bool link_nodes(Int r1, Int r2) {
            if (r1 != r2) {
                if (r1->size >= r2->size) {
                    r2->parent = r1;
                    r1->size += r2->size;
                } else {
                    r1->parent = r2;
                    r2->size += r1->size;
                }
                return false;
            }
            return true;
        }

        inline bool union_nodes(Int n1, Int n2) { return link_nodes(find(n1), find(n2)); }

        inline Int get_comp_size(Int n) const { return nodes[n].size; }

    private:
        std::vector<Node> nodes;
    };
}  // namespace cav
#endif