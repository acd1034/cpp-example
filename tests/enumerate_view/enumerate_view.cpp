#include <forward_list>
#include <list>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <ns/enumerate_view.hpp>

// begin テスト用の view

template <class T>
struct test_cpp20_input_iterator {
private:
  T* ptr_ = nullptr;

public:
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_cv_t<T>;
  using iterator_concept = std::input_iterator_tag;
  test_cpp20_input_iterator() = default;
  constexpr explicit test_cpp20_input_iterator(T* ptr) : ptr_(std::move(ptr)) {}
  constexpr decltype(auto) operator*() const { return *ptr_; }
  constexpr test_cpp20_input_iterator& operator++() {
    ++ptr_;
    return *this;
  }
  constexpr void operator++(int) { ++ptr_; }
  constexpr bool operator==(const test_cpp20_input_iterator& other) const {
    return ptr_ == other.ptr_;
  }
};

template <class T>
struct test_range {
private:
  T value{};

public:
  test_range() = default;
  constexpr explicit test_range(T v) : value(std::move(v)) {}
  auto begin() { return test_cpp20_input_iterator<T>(std::addressof(value)); }
  auto begin() const {
    return test_cpp20_input_iterator<const T>(std::addressof(value));
  }
  auto end() { return test_cpp20_input_iterator<T>(std::addressof(value) + 1); }
  auto end() const {
    return test_cpp20_input_iterator<const T>(std::addressof(value) + 1);
  }
};

static_assert(std::ranges::input_range<test_range<int>>);
static_assert(not std::ranges::forward_range<test_range<int>>);

// end テスト用の view

using testing_view = ns::enumerate_view<std::views::all_t<test_range<int>>>;
static_assert(
  std::input_or_output_iterator<std::ranges::iterator_t<testing_view>>);
static_assert(std::sentinel_for<std::ranges::sentinel_t<testing_view>,
                                std::ranges::iterator_t<testing_view>>);
static_assert(std::ranges::view<testing_view>);

TEST_CASE("enumerate_view", "[enumerate_view]") {
  {
    test_range<char> tr{'a'};
    ns::enumerate_view ev(tr);
    static_assert(std::ranges::input_range<decltype(ev)>);
    static_assert(not std::ranges::forward_range<decltype(ev)>);
    static_assert(not std::ranges::common_range<decltype(ev)>);

    std::input_iterator auto it = std::ranges::begin(ev);
    // test_cpp20_input_iterator は _Cpp17InputIterator_ 要件を満たさない
    // static_assert(
    //   not std::derived_from<
    //     typename std::iterator_traits<decltype(it)>::iterator_category,
    //     std::input_iterator_tag>);

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
    static_assert(not std::ranges::common_range<decltype(ev)>);

    std::forward_iterator auto it = std::ranges::begin(ev);
    static_assert(
      std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::input_iterator_tag>);
    static_assert(
      not std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::forward_iterator_tag>);

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
    static_assert(std::ranges::common_range<decltype(ev)>);

    std::bidirectional_iterator auto it = std::ranges::begin(ev);
    static_assert(
      std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::input_iterator_tag>);
    static_assert(
      not std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::forward_iterator_tag>);

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
    static_assert(std::ranges::common_range<decltype(ev)>);

    std::random_access_iterator auto it = std::ranges::begin(ev);
    static_assert(
      std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::input_iterator_tag>);
    static_assert(
      not std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::forward_iterator_tag>);

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

TEST_CASE("enumerate_view", "[enumerate_view][common]") {
  {
    std::list<char> l{'a', 'b', 'c'};
    std::ranges::subrange sr(std::counted_iterator(std::ranges::begin(l), 2),
                             std::default_sentinel);
    ns::enumerate_view ev(sr);
    static_assert(std::ranges::bidirectional_range<decltype(ev)>);
    static_assert(not std::ranges::random_access_range<decltype(ev)>);
    static_assert(not std::ranges::common_range<decltype(ev)>);

    std::bidirectional_iterator auto it = std::ranges::begin(ev);
    static_assert(
      std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::input_iterator_tag>);
    static_assert(
      not std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::forward_iterator_tag>);

    CHECK(std::get<0>(*it) == 0);
    CHECK(std::get<1>(*it++) == 'a');
    CHECK(std::get<0>(*it) == 1);
    CHECK(std::get<1>(*it++) == 'b');
    CHECK(it == std::ranges::end(ev));
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    std::ranges::subrange sr(std::counted_iterator(std::ranges::begin(v), 2),
                             std::default_sentinel);
    ns::enumerate_view ev(sr);
    static_assert(std::ranges::random_access_range<decltype(ev)>);
    static_assert(not std::ranges::common_range<decltype(ev)>);

    std::random_access_iterator auto it = std::ranges::begin(ev);
    static_assert(
      std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::input_iterator_tag>);
    static_assert(
      not std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::forward_iterator_tag>);

    CHECK(std::get<0>(*it) == 0);
    CHECK(std::get<1>(*it++) == 'a');
    CHECK(std::get<0>(*it) == 1);
    CHECK(std::get<1>(*it++) == 'b');
    CHECK(it == std::ranges::end(ev));
  }
}
