#ifndef CAV_BINARYHEAP_HPP
#define CAV_BINARYHEAP_HPP

#include <cassert>
#include <vector>

#include "functors.hpp"

#define LEFT(X) (2 * (X) + 1)
#define RIGHT(X) (2 * (X) + 2)
#define PARENT(X) (((X)-1) / 2)

namespace cav {
    /**
     * @brief Binary Heap sorting the minimum first.
     *
     * @tparam T        Type of the elements stored in the heap
     * @tparam Cmp      Binary operator, if scalar: return first - second;
     * @tparam Updt     Update the value of an element and return a value equal to Cmp()(old_element, new_element)
     * @tparam GetIdx   Given an element return the index in the heap
     * @tparam SetIdx   Set the index of an element
     * @tparam unheaped Constant value used to identify element not in the heap
     */
    template <typename T, class Cmp, class GetIdx, class SetIdx, class Updt, int unheaped = -1>
    class BinaryHeap {

        std::vector<T> heap;

        int inline min_lr(T& parent, int lindex, int rindex) {
            const int hsize = heap.size();
            int smallest = lindex;
            if (rindex < hsize && Cmp()(heap[rindex], heap[lindex]) < 0) { smallest = rindex; }  // !! rindex < lindex always !!
            if (smallest < hsize && Cmp()(heap[smallest], parent) < 0) { return smallest; }

            return unheaped;
        }

        void heapify(int hindex) {

            int smallest = min_lr(heap[hindex], LEFT(hindex), RIGHT(hindex));
            if (smallest == unheaped) { return; }

            auto elem = std::move(heap[hindex]);
            while (smallest != unheaped) {

                SetIdx()(heap[smallest], hindex);
                heap[hindex] = std::move(heap[smallest]);
                hindex = smallest;
                smallest = min_lr(elem, LEFT(hindex), RIGHT(hindex));
            }

            SetIdx()(elem, hindex);
            heap[hindex] = std::move(elem);
        }

        void upsift(int hindex) {

            if (hindex == 0) { return; }

            int pindex = hindex;
            auto elem = std::move(heap[hindex]);
            while (hindex && Cmp()(elem, heap[pindex = PARENT(pindex)]) < 0) {

                SetIdx()(heap[pindex], hindex);
                heap[hindex] = std::move(heap[pindex]);
                hindex = pindex;
            }

            SetIdx()(elem, hindex);
            heap[hindex] = std::move(elem);
        }

        bool is_heap() {

            const int hsize = heap.size();
            for (int n = 0; n < hsize; ++n) {
                const auto& t = heap[n];
                if (int idx = GetIdx()(t); idx != n) { return false; }
            }

            for (int n = 0; n < hsize; ++n) {
                int lindex = LEFT(n);
                if (lindex < hsize && Cmp()(heap[lindex], heap[n]) < 0) { return false; }

                int rindex = RIGHT(n);
                if (rindex < hsize && Cmp()(heap[rindex], heap[n]) < 0) { return false; }
            }

            return true;
        }

    public:
        BinaryHeap() { }
        BinaryHeap(const BinaryHeap& bh) : heap(bh.heap) { }
        BinaryHeap(BinaryHeap&& bh) : heap(std::move(bh.heap)) { }

        void reset() {
            for (auto& t : heap) { SetIdx()(t, unheaped); }
            heap.clear();
        }

        bool empty() const { return heap.empty(); }

        void insert(T elem) {

            const int hindex = heap.size();
            SetIdx()(elem, hindex);
            heap.emplace_back(elem);
            upsift(hindex);

            assert(is_heap());
        }

        T get() {
            assert(!heap.empty());

            SetIdx()(heap[0], unheaped);
            auto elem = std::move(heap[0]);

            SetIdx()(heap.back(), 0);
            heap[0] = std::move(heap.back());
            heap.pop_back();
            heapify(0);

            assert(is_heap());
            return elem;
        }

        void remove(int hindex) {

            if (hindex < static_cast<int>(heap.size()) - 1) {
                auto last = std::move(heap.back());
                heap.pop_back();
                replace(hindex, std::move(last));
            } else {
                SetIdx()(heap[hindex], unheaped);
                heap.pop_back();
            }

            assert(is_heap());
        }

        void replace(int hindex, T elem) {
            assert(hindex >= 0 && hindex < static_cast<int>(heap.size()));

            const auto case3 = Cmp()(heap[hindex], elem);

            SetIdx()(heap[hindex], unheaped);
            SetIdx()(elem, hindex);
            heap[hindex] = elem;

            if (case3 > 0) {
                upsift(hindex);
            } else if (case3 < 0) {
                heapify(hindex);
            }

            assert(is_heap());
        }

        auto size() const { return heap.size(); }

        T& spy(int hindex) { return heap[hindex]; }

        template <typename... Args>
        void update(int hindex, Args&&... args) {
            assert(hindex >= 0 && hindex < static_cast<int>(heap.size()));

            const auto case3 = Updt()(heap[hindex], std::forward<Args>(args)...);

            if (case3 > 0) {
                upsift(hindex);
            } else if (case3 < 0) {
                heapify(hindex);
            }

            assert(is_heap());
        }
    };


    ///////// SPECIALIZATION FOR STRUCTS /////////
    template <typename N, auto field>
    struct CmpFieldStruct {
        auto operator()(const N& r1, const N& r2) { return get_field_ref<N, field>()(r1) - get_field_ref<N, field>()(r2); }
    };

    template <typename N, auto field>
    struct GetIdxFieldStruct {
        auto operator()(const N& r1) { return get_field_ref<N, field>()(r1); }
    };

    template <typename N, auto field>
    struct SetIdxFieldStruct {
        void operator()(N& r1, int idx) { get_field_ref<N, field>()(r1) = idx; }
    };

    template <typename N, auto field>
    struct UpdtFieldStruct {
        auto operator()(N& r1, typename std::decay<decltype(std::declval<N>().*field)>::type val) {
            const auto res = get_field_ref<N, field>()(r1) - val;
            get_field_ref<N, field>()(r1) = val;
            return res;
        }
    };

    template <typename N, auto fidx, auto fval>
    class BinaryHeapStruct : public BinaryHeap<N, CmpFieldStruct<N, fval>, GetIdxFieldStruct<N, fidx>, SetIdxFieldStruct<N, fidx>, UpdtFieldStruct<N, fval>> {
    };


    ///////// SPECIALIZATION FOR POINTERS TO STRUCT /////////

    template <typename N, auto field>
    struct CmpFieldPtr {
        auto operator()(N* r1, N* r2) { return get_field_ref<N, field>()(*r1) - get_field_ref<N, field>()(*r2); }
    };

    template <typename N, auto field>
    struct GetIdxFieldPtr {
        auto operator()(N* r1) { return get_field_ref<N, field>()(*r1); }
    };

    template <typename N, auto field>
    struct SetIdxFieldPtr {
        void operator()(N* r1, int idx) { get_field_ref<N, field>()(*r1) = idx; }
    };

    template <typename N, auto field>
    struct UpdtFieldPtr {
        auto operator()(N* r1, typename std::decay<decltype(std::declval<N>().*field)>::type val) {
            const auto res = get_field_ref<N, field>()(*r1) - val;
            get_field_ref<N, field>()(*r1) = val;
            return res;
        }
    };

    template <typename N, auto fidx, auto fval>
    class BinaryHeapPtr : public BinaryHeap<N*, CmpFieldPtr<N, fval>, GetIdxFieldPtr<N, fidx>, SetIdxFieldPtr<N, fidx>, UpdtFieldPtr<N, fval>> { };

}  // namespace cav
#endif