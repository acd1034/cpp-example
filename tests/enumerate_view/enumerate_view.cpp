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
  constexpr bool operator==(const test_input_iterator& other) const { return ptr_ == other.ptr_; }
};

template <class T>
struct test_view : std::ranges::view_base {
private:
  T value{};

public:
  test_view() = default;
  constexpr explicit test_view(T v) : value(std::move(v)) {}
  auto begin() { return test_input_iterator<T>(std::addressof(value)); }
  auto begin() const { return test_input_iterator<const T>(std::addressof(value)); }
  auto end() { return test_input_iterator<T>(std::addressof(value) + 1); }
  auto end() const { return test_input_iterator<const T>(std::addressof(value) + 1); }
};

static_assert(std::ranges::view<test_view<int>>);
static_assert(std::ranges::input_range<test_view<int>>);
static_assert(not std::ranges::forward_range<test_view<int>>);

// end test 用の view

using testing_view = ns::enumerate_view<test_view<int>>;
// まだ動かない
// static_assert(std::input_or_output_iterator<std::ranges::iterator_t<testing_view>>);
// static_assert(std::sentinel_for<std::ranges::sentinel_t<testing_view>, std::ranges::iterator_t<testing_view>>);

// TEST_CASE("enumerate_view", "[enumerate_view]") {}
