#include <forward_list>
#include <list>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <ns/enumerate_view.hpp>

// begin test 用の view

template <class T>
struct test_input_iterator {
private:
  T* ptr_ = nullptr;

public:
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_cv_t<T>;
  using iterator_concept = std::input_iterator_tag;
  test_input_iterator() = default;
  constexpr explicit test_input_iterator(T* ptr) : ptr_(std::move(ptr)) {}
  constexpr decltype(auto) operator*() const { return *ptr_; }
  constexpr test_input_iterator& operator++() {
    ++ptr_;
    return *this;
  }
  constexpr void operator++(int) { ++ptr_; }
  constexpr bool operator==(const test_input_iterator& other) const {
    return ptr_ == other.ptr_;
  }
};

template <class T>
struct test_view : std::ranges::view_base {
private:
  T value{};

public:
  test_view() = default;
  constexpr explicit test_view(T v) : value(std::move(v)) {}
  auto begin() { return test_input_iterator<T>(std::addressof(value)); }
  auto begin() const {
    return test_input_iterator<const T>(std::addressof(value));
  }
  auto end() { return test_input_iterator<T>(std::addressof(value) + 1); }
  auto end() const {
    return test_input_iterator<const T>(std::addressof(value) + 1);
  }
};

static_assert(std::ranges::view<test_view<int>>);
static_assert(std::ranges::input_range<test_view<int>>);
static_assert(not std::ranges::forward_range<test_view<int>>);

// end test 用の view

using testing_view = ns::enumerate_view<test_view<int>>;
static_assert(
  std::input_or_output_iterator<std::ranges::iterator_t<testing_view>>);
static_assert(std::sentinel_for<std::ranges::sentinel_t<testing_view>,
                                std::ranges::iterator_t<testing_view>>);
static_assert(std::ranges::view<testing_view>);

TEST_CASE("enumerate_view", "[enumerate_view]") {
  {
    test_view<char> tv{'a'};
    ns::enumerate_view ev(tv);
    static_assert(std::ranges::input_range<decltype(ev)>);
    static_assert(not std::ranges::forward_range<decltype(ev)>);

    std::input_iterator auto it = std::ranges::begin(ev);
    CHECK(std::get<0>(*it) == 0);
    CHECK(std::get<1>(*it) == 'a');
    ++it;
    CHECK(it == std::ranges::end(ev));
  }
  {
    std::forward_list<char> fl{'a', 'b', 'c'};
    ns::enumerate_view ev(fl);
    static_assert(std::ranges::forward_range<decltype(ev)>);
    static_assert(not std::ranges::bidirectional_range<decltype(ev)>);

    std::forward_iterator auto it = std::ranges::begin(ev);
    CHECK(std::get<0>(*it) == 0);
    CHECK(std::get<1>(*it++) == 'a');
    CHECK(std::get<0>(*it) == 1);
    CHECK(std::get<1>(*it++) == 'b');
    CHECK(std::get<0>(*it) == 2);
    CHECK(std::get<1>(*it++) == 'c');
    CHECK(it == std::ranges::end(ev));
  }
  {
    std::list<char> l{'a', 'b', 'c'};
    ns::enumerate_view ev(l);
    static_assert(std::ranges::bidirectional_range<decltype(ev)>);
    static_assert(not std::ranges::random_access_range<decltype(ev)>);

    std::bidirectional_iterator auto it = std::ranges::begin(ev);
    CHECK(std::get<0>(*it) == 0);
    CHECK(std::get<1>(*it++) == 'a');
    CHECK(std::get<0>(*it) == 1);
    CHECK(std::get<1>(*it++) == 'b');
    CHECK(std::get<0>(*it) == 2);
    CHECK(std::get<1>(*it++) == 'c');
    CHECK(it == std::ranges::end(ev));
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    ns::enumerate_view ev(v);
    static_assert(std::ranges::random_access_range<decltype(ev)>);

    std::random_access_iterator auto it = std::ranges::begin(ev);
    CHECK(std::get<0>(*it) == 0);
    CHECK(std::get<1>(*it++) == 'a');
    CHECK(std::get<0>(*it) == 1);
    CHECK(std::get<1>(*it++) == 'b');
    CHECK(std::get<0>(*it) == 2);
    CHECK(std::get<1>(*it++) == 'c');
    CHECK(it == std::ranges::end(ev));
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    ns::enumerate_view ev(v);
    std::random_access_iterator auto begin = std::ranges::begin(ev);
    auto end = std::ranges::end(ev);
    static_assert(std::sized_sentinel_for<decltype(end), decltype(begin)>);
    CHECK(std::ranges::end(ev) - std::ranges::begin(ev) == 3);
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    ns::enumerate_view ev(v);
    static_assert(std::ranges::sized_range<decltype(ev)>);
    CHECK(std::ranges::size(ev) == 3);
  }
}
