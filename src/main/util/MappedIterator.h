//
// Created by Khyber on 1/18/2019.
//

#ifndef SiliconScratch_MappedIterator_H
#define SiliconScratch_MappedIterator_H

#include <iterator>

template <typename Iterator, typename Map>
class MappedIterator {

private:
    
    Iterator i;
    const Map f;
    
    using Traits = std::iterator_traits<Iterator>;

public:
    
    using T = typename std::remove_reference<decltype(f(*i))>::type;
    using Ref = T&;
    using Ptr = T*;
    using Diff = typename Traits::difference_type;
    using Category = typename Traits::iterator_category;
    
    using value_type = T;
    using reference = Ref;
    using pointer = Ptr;
    using difference_type = Diff;
    using iterator_category = Category;

public:
    
    constexpr MappedIterator(Iterator iterator, Map map) noexcept
            : i(iterator), f(map) {}
    
    constexpr MappedIterator(MappedIterator&& other) noexcept = default;
    
    constexpr MappedIterator(const MappedIterator& other) noexcept = default;
    
    Iterator base() const noexcept {
        return i;
    }
    
    Ref operator*() const noexcept(noexcept(Map()(*Iterator()))) {
        return f(*i);
    }
    
    Ptr operator->() const noexcept(noexcept(*MappedIterator(Iterator(), Map()))) {
        return &operator*();
    }
    
    MappedIterator& operator++() noexcept(noexcept(++Iterator())) {
        ++i;
        return *this;
    }
    
    const MappedIterator operator++(int) noexcept(noexcept(++Iterator())) {
        MappedIterator tmp(*this);
        ++i;
        return tmp;
    }
    
    MappedIterator& operator--() noexcept(noexcept(--Iterator())) {
        --i;
        return *this;
    }
    
    const MappedIterator operator--(int) noexcept(noexcept(--Iterator())) {
        MappedIterator tmp(*this);
        --i;
        return tmp;
    }
    
    MappedIterator& operator+=(Diff n) noexcept(noexcept(Iterator() += 0)) {
        i += n;
    }
    
    const MappedIterator operator+(Diff n) const noexcept(noexcept(Iterator() + 0)) {
        return MappedIterator(i + n, f);
    }
    
    MappedIterator& operator-=(Diff n) noexcept(noexcept(Iterator() -= 0)) {
        i -= n;
    }
    
    const MappedIterator operator-(Diff n) const noexcept(noexcept(Iterator() - 0)) {
        return MappedIterator(i - n, f);
    }
    
    Ref operator[](Diff n) const noexcept(noexcept(Map()(Iterator()[0]))) {
        return f(i[n]);
    }
    
};

template <typename Iterator, typename Map>
bool operator==(
        const MappedIterator<Iterator, Map>& lhs,
        const MappedIterator<Iterator, Map>& rhs
) noexcept(noexcept(Iterator() == Iterator())) {
    return lhs.base() == rhs.base();
}

template <typename Iterator, typename Map>
bool operator!=(
        const MappedIterator<Iterator, Map>& lhs,
        const MappedIterator<Iterator, Map>& rhs
) noexcept(noexcept(Iterator() != Iterator())) {
    return lhs.base() != rhs.base();
}

template <typename Iterator, typename Map>
bool operator<(
        const MappedIterator<Iterator, Map>& lhs,
        const MappedIterator<Iterator, Map>& rhs
) noexcept(noexcept(Iterator() < Iterator())) {
    return lhs.base() < rhs.base();
}

template <typename Iterator, typename Map>
bool operator>(
        const MappedIterator<Iterator, Map>& lhs,
        const MappedIterator<Iterator, Map>& rhs
) noexcept(noexcept(Iterator() > Iterator())) {
    return lhs.base() > rhs.base();
}

template <typename Iterator, typename Map>
bool operator<=(
        const MappedIterator<Iterator, Map>& lhs,
        const MappedIterator<Iterator, Map>& rhs
) noexcept(noexcept(Iterator() <= Iterator())) {
    return lhs.base() <= rhs.base();
}

template <typename Iterator, typename Map>
bool operator>=(
        const MappedIterator<Iterator, Map>& lhs,
        const MappedIterator<Iterator, Map>& rhs
) noexcept(noexcept(Iterator() >= Iterator())) {
    return lhs.base() >= rhs.base();
}

template <typename Iterator, typename Map>
MappedIterator<Iterator, Map> operator+(
        typename MappedIterator<Iterator, Map>::Diff n,
        const MappedIterator<Iterator, Map>& rhs
) noexcept(noexcept(0 + Iterator())) {
    return MappedIterator<Iterator, Map>(n + rhs.base());
}

template <typename Iterator, typename Map>
MappedIterator<Iterator, Map> operator-(
        typename MappedIterator<Iterator, Map>::Diff n,
        const MappedIterator<Iterator, Map>& rhs
) noexcept(noexcept(0 - Iterator())) {
    return MappedIterator<Iterator, Map>(n - rhs.base());
}

namespace iterators {
    
    template <typename Iterator, typename Map>
    constexpr MappedIterator<Iterator, Map> map(Iterator iterator, Map map) noexcept {
        return MappedIterator<Iterator, Map>(iterator, map);
    }
    
}

template <typename Iterable, typename Map>
class MappedIterable {

private:
    
    // TODO
    
};

namespace iterables {



}

#endif // SiliconScratch_MappedIterator_H
