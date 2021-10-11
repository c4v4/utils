#ifndef CAV_VECTORSET_HPP
#define CAV_VECTORSET_HPP

#include <type_traits>
#include <unordered_set>
#include <vector>

namespace cav {
    
    template <typename T, typename Hash = std::hash<T>>
    class VectorSet {
    private:
        using viter = typename std::vector<T>::iterator;
        using const_viter = typename std::vector<T>::const_iterator;

        /**
         * @brief Since the set does not contain T, but *T,
         * this hacky struct is used to make the *T behave like
         * a T would do (for what the set is concerned).
         *
         * Note: it's still a ptr, so it has all the problems of
         * a ptr.
         * NEVER return it if it is initialized to with a local var.
         */
        struct TPtrWrap {
            TPtrWrap(T*& base_addr_, size_t idx_) : base_addr(base_addr_), idx(idx_) { }
            operator T&() const { return base_addr[idx]; }
            T* data() const { return base_addr + idx; }
            bool operator==(const TPtrWrap& other) const { return other.base_addr[other.idx] == base_addr[idx]; }

        private:
            T*& base_addr;
            size_t idx;
        };

        using Set = std::unordered_set<TPtrWrap, Hash>;
        using siter = typename Set::iterator;
        using const_siter = typename Set::const_iterator;

    public:
        template <typename... _Args>
        bool emplace_back(_Args&&... args) {
            T elem(std::forward<_Args>(args)...);
            T* elem_ptr = std::addressof(elem);
            auto wrapped_elem = TPtrWrap(elem_ptr, 0);

            if (set.count(wrapped_elem) == 0) {
                vec.emplace_back(std::move(elem));
                vec_data = vec.data();
                set.emplace(vec_data, vec.size() - 1);
                return true;
            }
            return false;
        }

        template <typename Tt>
        bool push_back(Tt&& elem) {
            return emplace_back(std::forward<Tt>(elem));
        }

        // Vector operations
        T& back() { return vec.back(); }
        T& front() { return vec.front(); }
        viter begin() { return vec.begin(); }
        viter end() { return vec.end(); }
        T& operator[](size_t i) { return vec[i]; }
        void clear() { vec.clear(), set.clear(); }

        const T& back() const { return vec.back(); }
        const T& front() const { return vec.front(); }
        const_viter begin() const { return vec.begin(); }
        const_viter end() const { return vec.end(); }
        const T& operator[](size_t i) const { return vec[i]; }
        bool empty() const { return vec.empty(); }
        size_t size() const { return vec.size(); }

        // Set Operations
        const_viter find(T elem) const {
            T* elem_ptr = std::addressof(elem);
            auto wrapped_elem = TPtrWrap(elem_ptr, 0);

            const_siter found = set.find(wrapped_elem);
            if (found != set.cend()) return const_viter(found->data());
            return vec.cend();
        }

        viter find(T elem) {
            T* elem_ptr = std::addressof(elem);
            TPtrWrap wrapped_elem(elem_ptr, 0);

            siter found = set.find(wrapped_elem);
            if (found != set.end()) return viter(found->data());
            return vec.end();
        }

        size_t count(T& elem) const { return find(elem) != vec.cend(); }
        size_t count(T& elem) { return find(elem) != vec.end(); }


        std::vector<T> get_vector() { return vec; }

    private:
        std::vector<T> vec;
        // we can't get vec.data() ptr ref, so this is a reproduction that needs to be kept updated
        T* vec_data;

        Set set;
    };
}  // namespace cav
#endif