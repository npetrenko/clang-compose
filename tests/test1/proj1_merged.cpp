#include <iterator>
#include <type_traits>
#include <vector>

class FromInclude {
};

int second_include();

struct Hey {
    struct ImInHey {
    };
};

template <class>
void TemplateDecl();

struct ThisIsPacked {
};

namespace detail {

template <class Iter, class = typename std::iterator_traits<Iter>::iterator_category>
class IRangeImplBase {
public:
    IRangeImplBase(Iter begin, Iter end) noexcept(std::is_nothrow_move_constructible_v<Iter>)
        : begin_(std::move(begin)), end_(std::move(end)) {
    }

    template <class Container>
    IRangeImplBase(Container&& container) noexcept(
        noexcept(container.begin()) &&
        std::is_nothrow_move_constructible_v<decltype(container.begin())>)
        : begin_(container.begin()), end_(container.end()) {
        static_assert(std::is_reference_v<Container>,
                      "IRange cannot be invoked on temporary objects");
        static_assert(std::is_same_v<decltype(container.begin()), decltype(container.end())>,
                      "IRange doesn't support containers with different begin-end iterator types");
    }

    Iter begin() const {  // NOLINT
        return begin_;
    }

    Iter end() const {  // NOLINT
        return end_;
    }

private:
    Iter begin_, end_;
};

template <class Iter>
class RandomIRange : public IRangeImplBase<Iter> {
    using Base = IRangeImplBase<Iter>;

public:
    using Base::Base;
    using SizeT = typename std::iterator_traits<Iter>::difference_type;

    SizeT size() const {
        return std::distance(this->begin(), this->end());
    }
};

template <class Iter>
using OverloadedBase =
    std::conditional_t<std::is_same_v<typename std::iterator_traits<Iter>::iterator_category,
                                      std::random_access_iterator_tag>,
                       RandomIRange<Iter>, IRangeImplBase<Iter>>;

}

template <class Iter>
class IRange : public detail::OverloadedBase<Iter> {
    using Base = detail::OverloadedBase<Iter>;

public:
    using Base::Base;
};

template <class Container>
IRange(Container&& cont)->IRange<decltype(cont.begin())>;

class Class {
public:
    int Method() {
        return 0;
    }

private:
    int so_private_;
};

class OutOfLine {
public:
    int OtherMethod();
};

int OutOfLine::OtherMethod() {
    return -1;
}

class NoInlineclass {
};

int main() {
    int x = 1 + 1;
    int a;
    a = x + 2;
    if (a < 2) {
        a = 51;
    }
    return x - 2;
}

