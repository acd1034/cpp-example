#include <cassert>
#include <forward_list>
#include <list>
#include <vector>

/// @file enumerate_view.hpp
#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ns {
  /// @tparam View 元となる view の型
  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  struct enumerate_view : std::ranges::view_interface<enumerate_view<View>> {
  private:
    //! 元となる view
    View base_ = View();

    template <bool Const>
    struct iterator;
    template <bool Const>
    struct sentinel;

  public:
    enumerate_view() requires std::default_initializable<View>
    = default;
    constexpr enumerate_view(View base) : base_(std::move(base)) {}

    constexpr iterator<false> begin() { return {std::ranges::begin(base_), 0}; }
    constexpr iterator<true>
    begin() const requires std::ranges::input_range<const View> {
      return {std::ranges::begin(base_), 0};
    }

    constexpr auto end() {
      if constexpr (std::ranges::common_range<View> and //
                    std::ranges::sized_range<View>)
        return iterator<false>(std::ranges::end(base_),
                               std::ranges::size(base_));
      else
        return sentinel<false>(std::ranges::end(base_));
    }
    constexpr auto end() const requires std::ranges::input_range<const View> {
      if constexpr (std::ranges::common_range<const View> and //
                    std::ranges::sized_range<const View>)
        return iterator<true>(std::ranges::end(base_),
                              std::ranges::size(base_));
      else
        return sentinel<true>(std::ranges::end(base_));
    }

    constexpr auto size() requires std::ranges::sized_range<View> {
      return std::ranges::size(base_);
    }
    constexpr auto size() const requires std::ranges::sized_range<const View> {
      return std::ranges::size(base_);
    }
  };

  template <class Range>
  enumerate_view(Range&&) -> enumerate_view<std::views::all_t<Range>>;

  template <class View>
  struct deduce_iterator_category {};

  template <class View>
  requires requires {
    typename std::iterator_traits<
      std::ranges::iterator_t<View>>::iterator_category;
  }
  struct deduce_iterator_category<View> {
    using iterator_category = std::input_iterator_tag;
  };

  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  template <bool Const>
  struct enumerate_view<View>::iterator : deduce_iterator_category<View> {
  private:
    using Base = std::conditional_t<Const, const View, View>;
    //! 元となるイテレータの現在位置
    std::ranges::iterator_t<Base> current_ = std::ranges::iterator_t<Base>();
    //! 現在のインデックス
    std::size_t count_ = 0;

  public:
    using difference_type = std::ranges::range_difference_t<Base>;
    using value_type = std::pair<std::size_t, std::ranges::range_value_t<Base>>;
    // clang-format off
    using iterator_concept =
      std::conditional_t<std::ranges::random_access_range<Base>, std::random_access_iterator_tag,
      std::conditional_t<std::ranges::bidirectional_range<Base>, std::bidirectional_iterator_tag,
      std::conditional_t<std::ranges::forward_range<Base>,       std::forward_iterator_tag,
      /* else */                                                 std::input_iterator_tag>>>;
    // clang-format on

    iterator() requires
      std::default_initializable<std::ranges::iterator_t<Base>>
    = default;
    constexpr iterator(std::ranges::iterator_t<Base> current, std::size_t count)
      : current_(std::move(current)), count_(std::move(count)) {}
    constexpr /* implicit */ iterator(iterator<not Const> other) requires Const
      and std::convertible_to<std::ranges::iterator_t<View>,
                              std::ranges::iterator_t<Base>>
      : current_(std::move(other.current_)), count_(std::move(other.count_)) {}

    constexpr const std::ranges::iterator_t<Base>& base() const& noexcept {
      return current_;
    }
    constexpr std::ranges::iterator_t<Base> base() && {
      return std::move(current_);
    }

    constexpr std::pair<std::size_t, std::ranges::range_reference_t<Base>>
    operator*() const {
      return {count_, *current_};
    }

    constexpr iterator& operator++() {
      ++current_;
      ++count_;
      return *this;
    }
    constexpr void operator++(int) { ++*this; }
    constexpr iterator
    operator++(int) requires std::ranges::forward_range<Base> {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator&
    operator--() requires std::ranges::bidirectional_range<Base> {
      --current_;
      --count_;
      return *this;
    }
    constexpr iterator
    operator--(int) requires std::ranges::bidirectional_range<Base> {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(difference_type n) //
      requires std::ranges::random_access_range<Base> {
      current_ += n;
      count_ += n;
      return *this;
    }
    constexpr iterator& operator-=(difference_type n) //
      requires std::ranges::random_access_range<Base> {
      return *this += -n;
    }
    constexpr std::pair<std::size_t, std::ranges::range_reference_t<Base>>
    operator[](difference_type n) const //
      requires std::ranges::random_access_range<Base> {
      return *(*this + n);
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y) //
      requires std::equality_comparable<std::ranges::iterator_t<Base>> {
      return x.current_ == y.current_;
    }

    friend constexpr bool operator<(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base> {
      return x.current_ < y.current_;
    }
    friend constexpr bool operator>(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base> {
      return y < x;
    }
    friend constexpr bool operator<=(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base> {
      return not(y < x);
    }
    friend constexpr bool operator>=(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base> {
      return not(x < y);
    }
    friend constexpr auto operator<=>(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base> and                   //
      std::three_way_comparable<std::ranges::iterator_t<Base>> {
      return x.current_ <=> y.current_;
    }

    friend constexpr iterator operator+(iterator x, difference_type n) //
      requires std::ranges::random_access_range<Base> {
      x += n;
      return x;
    }
    friend constexpr iterator operator+(difference_type n, iterator x) //
      requires std::ranges::random_access_range<Base> {
      x += n;
      return x;
    }
    friend constexpr iterator operator-(iterator x, difference_type n) //
      requires std::ranges::random_access_range<Base> {
      x -= n;
      return x;
    }
    friend constexpr difference_type //
    operator-(const iterator& x, const iterator& y) requires
      std::ranges::random_access_range<Base> {
      return x.current_ - y.current_;
    }

    friend constexpr std::pair<std::size_t,
                               std::ranges::range_rvalue_reference_t<View>>
    iter_move(const iterator& x) noexcept(
      noexcept(std::ranges::iter_move(x.current_))) {
      return {x.count_, std::ranges::iter_move(x.current_)};
    }
  };

  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  template <bool Const>
  struct enumerate_view<View>::sentinel {
  private:
    using Base = std::conditional_t<Const, const View, View>;
    //! 元となる view の番兵イテレータ
    std::ranges::sentinel_t<Base> end_ = std::ranges::sentinel_t<Base>();

  public:
    sentinel() = default;
    constexpr explicit sentinel(std::ranges::sentinel_t<Base> end)
      : end_(std::move(end)) {}
    constexpr /* implicit */ sentinel(sentinel<not Const> other) requires Const
      and std::convertible_to<std::ranges::sentinel_t<View>,
                              std::ranges::sentinel_t<Base>>
      : end_(std::move(other.end_)) {}

    friend constexpr bool //
    operator==(const iterator<Const>& x, const sentinel& y) requires
      std::sentinel_for<std::ranges::sentinel_t<Base>,
                        std::ranges::iterator_t<Base>> {
      return x.base() == y.end_;
    }

    friend constexpr std::ranges::range_difference_t<Base>
    operator-(const iterator<Const>& x, const sentinel& y) requires
      std::sized_sentinel_for<std::ranges::sentinel_t<Base>,
                              std::ranges::iterator_t<Base>> {
      return x.base() - y.end_;
    }
    friend constexpr std::ranges::range_difference_t<Base>
    operator-(const sentinel& x, const iterator<Const>& y) requires
      std::sized_sentinel_for<std::ranges::sentinel_t<Base>,
                              std::ranges::iterator_t<Base>> {
      return x.end_ - y.base();
    }
  };
} // namespace ns

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

int main() {
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it) == 'a');
    ++it;
    assert(it == std::ranges::end(ev));
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(std::get<0>(*it) == 2);
    assert(std::get<1>(*it++) == 'c');
    assert(it == std::ranges::end(ev));
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(std::get<0>(*it) == 2);
    assert(std::get<1>(*it++) == 'c');
    assert(it == std::ranges::end(ev));
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(std::get<0>(*it) == 2);
    assert(std::get<1>(*it++) == 'c');
    assert(it == std::ranges::end(ev));
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    ns::enumerate_view ev(v);
    std::random_access_iterator auto begin = std::ranges::begin(ev);
    auto end = std::ranges::end(ev);
    static_assert(std::sized_sentinel_for<decltype(end), decltype(begin)>);
    assert(std::ranges::end(ev) - std::ranges::begin(ev) == 3);
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    ns::enumerate_view ev(v);
    static_assert(std::ranges::sized_range<decltype(ev)>);
    assert(std::ranges::size(ev) == 3);
  }
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(it == std::ranges::end(ev));
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(it == std::ranges::end(ev));
  }
  {
    test_range<char> tr{'a'};
    const ns::enumerate_view ev(tr);
    static_assert(std::ranges::input_range<decltype(ev)>);
    static_assert(not std::ranges::forward_range<decltype(ev)>);
    static_assert(not std::ranges::common_range<decltype(ev)>);

    std::input_iterator auto it = std::ranges::begin(ev);
    // test_cpp20_input_iterator は _Cpp17InputIterator_ 要件を満たさない
    // static_assert(
    //   not std::derived_from<
    //     typename std::iterator_traits<decltype(it)>::iterator_category,
    //     std::input_iterator_tag>);

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it) == 'a');
    ++it;
    assert(it == std::ranges::end(ev));
  }
  {
    std::forward_list<char> fl{'a', 'b', 'c'};
    const ns::enumerate_view ev(fl);
    static_assert(std::ranges::forward_range<decltype(ev)>);
    static_assert(not std::ranges::bidirectional_range<decltype(ev)>);
    static_assert(not std::ranges::common_range<decltype(ev)>);
    static_assert(std::same_as<std::ranges::range_reference_t<decltype(ev)>,
                               std::pair<unsigned long, char&>>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(ev)>,
                               std::pair<unsigned long, char>>);

    std::forward_iterator auto it = std::ranges::begin(ev);
    static_assert(
      std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::input_iterator_tag>);
    static_assert(
      not std::derived_from<
        typename std::iterator_traits<decltype(it)>::iterator_category,
        std::forward_iterator_tag>);

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(std::get<0>(*it) == 2);
    assert(std::get<1>(*it++) == 'c');
    assert(it == std::ranges::end(ev));
  }
  {
    std::list<char> l{'a', 'b', 'c'};
    const ns::enumerate_view ev(l);
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(std::get<0>(*it) == 2);
    assert(std::get<1>(*it++) == 'c');
    assert(it == std::ranges::end(ev));
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    const ns::enumerate_view ev(v);
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(std::get<0>(*it) == 2);
    assert(std::get<1>(*it++) == 'c');
    assert(it == std::ranges::end(ev));
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    const ns::enumerate_view ev(v);
    std::random_access_iterator auto begin = std::ranges::begin(ev);
    auto end = std::ranges::end(ev);
    static_assert(std::sized_sentinel_for<decltype(end), decltype(begin)>);
    assert(std::ranges::end(ev) - std::ranges::begin(ev) == 3);
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    const ns::enumerate_view ev(v);
    static_assert(std::ranges::sized_range<decltype(ev)>);
    assert(std::ranges::size(ev) == 3);
  }
  {
    std::list<char> l{'a', 'b', 'c'};
    std::ranges::subrange sr(std::counted_iterator(std::ranges::begin(l), 2),
                             std::default_sentinel);
    const ns::enumerate_view ev(sr);
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(it == std::ranges::end(ev));
  }
  {
    std::vector<char> v{'a', 'b', 'c'};
    std::ranges::subrange sr(std::counted_iterator(std::ranges::begin(v), 2),
                             std::default_sentinel);
    const ns::enumerate_view ev(sr);
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

    assert(std::get<0>(*it) == 0);
    assert(std::get<1>(*it++) == 'a');
    assert(std::get<0>(*it) == 1);
    assert(std::get<1>(*it++) == 'b');
    assert(it == std::ranges::end(ev));
  }
}
