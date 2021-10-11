#ifndef CAV_RANDOMITERATOR_HPP
#define CAV_RANDOMITERATOR_HPP

#include <iterator>
#include <numeric>

namespace cav {

    template <typename T>
    struct plus {
        constexpr T operator()(T t, long int i) const { return t + i; }
    };

    template <typename T>
    struct minus {
        constexpr long int operator()(T t1, T t2) const { return t1 - t2; }
    };

    template <typename T>
    struct defer {
        constexpr auto& operator()(T t) const { return *t; }
    };


    template <typename Base, template <typename T> class OperPlus = plus, template <typename T> class OperLess = std::less,
              template <typename T> class OperMinus = minus, template <typename T> class OperDefer = defer>
    struct RandomIterator {
        typedef typename std::random_access_iterator_tag iterator_category;
        typedef typename std::result_of_t<OperMinus<Base>(Base, Base)> difference_type;
        typedef typename std::remove_reference_t<typename std::result_of_t<OperDefer<Base>(Base&)>> value_type;
        typedef value_type* pointer;
        typedef value_type& reference;

    public:
        RandomIterator(Base base_) : base(base_){};

        inline reference operator*() { return OperDefer<Base>()(base); }
        inline reference operator[](difference_type i) const { return OperDefer<Base>()(OperPlus<Base>()(base, i)); }

        inline RandomIterator operator+(difference_type i) noexcept { return RandomIterator(OperPlus<Base>()(base, i)); }
        inline RandomIterator operator++() noexcept { return preOperator(1); }
        inline RandomIterator operator+=(difference_type i) noexcept { return preOperator(i); }
        inline RandomIterator operator++(int) noexcept { return postOperator(1); }

        inline RandomIterator operator--() noexcept { return preOperator(-1); }
        inline RandomIterator operator--(int) noexcept { return postOperator(-1); }
        inline RandomIterator operator-(difference_type i) const noexcept { return preOperator(-i); }
        inline RandomIterator operator-=(difference_type i) noexcept { return preOperator(-i); }
        inline difference_type operator-(RandomIterator x) const noexcept { return OperMinus<Base>()(base, x.base); }

        inline bool operator<(const RandomIterator& other) const noexcept { return OperLess<Base>()(base, other.base); }
        inline bool operator==(const RandomIterator& other) const noexcept { return !(*this < other) && !(other < *this); }
        inline bool operator!=(const RandomIterator& other) const noexcept { return !(*this == other); }
        inline bool operator>(const RandomIterator& other) const noexcept { return other < *this; }
        inline bool operator>=(const RandomIterator& other) const noexcept { return !(*this < other); }
        inline bool operator<=(const RandomIterator& other) const noexcept { return !(other < *this); }

    private:
        inline RandomIterator postOperator(difference_type i) {
            RandomIterator curr = base;
            base = OperPlus<Base>()(base, i);
            return curr;
        }

        inline RandomIterator preOperator(difference_type i) {
            base = OperPlus<Base>()(base, i);
            return *this;
        }

        Base base;
    };
}  // namespace cav

#endif