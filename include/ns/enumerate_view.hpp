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
    enumerate_view()
      requires std::default_initializable<View>
    = default;
    constexpr enumerate_view(View base) : base_(std::move(base)) {}

    constexpr iterator<false> begin() {
      return {std::ranges::begin(base_), 0};
    }
    constexpr iterator<true> begin() const
      requires std::ranges::input_range<const View>
    {
      return {std::ranges::begin(base_), 0};
    }

    constexpr auto end() {
      if constexpr (
        std::ranges::common_range<View> and //
        std::ranges::sized_range<View>)
        return iterator<false>(
          std::ranges::end(base_), std::ranges::size(base_));
      else
        return sentinel<false>(std::ranges::end(base_));
    }
    constexpr auto end() const
      requires std::ranges::input_range<const View>
    {
      if constexpr (
        std::ranges::common_range<const View> and //
        std::ranges::sized_range<const View>)
        return iterator<true>(
          std::ranges::end(base_), std::ranges::size(base_));
      else
        return sentinel<true>(std::ranges::end(base_));
    }

    constexpr auto size()
      requires std::ranges::sized_range<View>
    {
      return std::ranges::size(base_);
    }
    constexpr auto size() const
      requires std::ranges::sized_range<const View>
    {
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

    iterator()
      requires std::default_initializable<std::ranges::iterator_t<Base>>
    = default;
    constexpr iterator(std::ranges::iterator_t<Base> current, std::size_t count)
      : current_(std::move(current)), count_(std::move(count)) {}
    constexpr /* implicit */ iterator(iterator<not Const> other)
      requires Const and
                 std::convertible_to<
                   std::ranges::iterator_t<View>,
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
    constexpr void operator++(int) {
      ++*this;
    }
    constexpr iterator operator++(int)
      requires std::ranges::forward_range<Base>
    {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
      requires std::ranges::bidirectional_range<Base>
    {
      --current_;
      --count_;
      return *this;
    }
    constexpr iterator operator--(int)
      requires std::ranges::bidirectional_range<Base>
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(difference_type n) //
      requires std::ranges::random_access_range<Base>
    {
      current_ += n;
      count_ += n;
      return *this;
    }
    constexpr iterator& operator-=(difference_type n) //
      requires std::ranges::random_access_range<Base>
    {
      return *this += -n;
    }
    constexpr std::pair<std::size_t, std::ranges::range_reference_t<Base>>
    operator[](difference_type n) const //
      requires std::ranges::random_access_range<Base>
    {
      return *(*this + n);
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y) //
      requires std::equality_comparable<std::ranges::iterator_t<Base>>
    {
      return x.current_ == y.current_;
    }

    friend constexpr bool operator<(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base>
    {
      return x.current_ < y.current_;
    }
    friend constexpr bool operator>(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base>
    {
      return y < x;
    }
    friend constexpr bool operator<=(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base>
    {
      return not(y < x);
    }
    friend constexpr bool operator>=(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base>
    {
      return not(x < y);
    }
    friend constexpr auto operator<=>(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<Base> and //
      std::three_way_comparable<std::ranges::iterator_t<Base>>
    {
      return x.current_ <=> y.current_;
    }

    friend constexpr iterator operator+(iterator x, difference_type n) //
      requires std::ranges::random_access_range<Base>
    {
      x += n;
      return x;
    }
    friend constexpr iterator operator+(difference_type n, iterator x) //
      requires std::ranges::random_access_range<Base>
    {
      x += n;
      return x;
    }
    friend constexpr iterator operator-(iterator x, difference_type n) //
      requires std::ranges::random_access_range<Base>
    {
      x -= n;
      return x;
    }
    friend constexpr difference_type //
    operator-(const iterator & x, const iterator & y)
      requires std::ranges::random_access_range<Base>
    {
      return x.current_ - y.current_;
    }

    friend constexpr std::
      pair<std::size_t, std::ranges::range_rvalue_reference_t<View>>
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
    constexpr /* implicit */ sentinel(sentinel<not Const> other)
      requires Const and
      std::convertible_to<
                 std::ranges::sentinel_t<View>,
                 std::ranges::sentinel_t<Base>>
      : end_(std::move(other.end_)) {}

    friend constexpr bool //
    operator==(const iterator<Const>& x, const sentinel & y)
      requires std::sentinel_for<
        std::ranges::sentinel_t<Base>,
        std::ranges::iterator_t<Base>>
    {
      return x.base() == y.end_;
    }

    friend constexpr std::ranges::range_difference_t<Base> operator-(
      const iterator<Const>& x,
      const sentinel& y)
      requires std::sized_sentinel_for<
        std::ranges::sentinel_t<Base>,
        std::ranges::iterator_t<Base>>
    {
      return x.base() - y.end_;
    }
    friend constexpr std::ranges::range_difference_t<Base> operator-(
      const sentinel& x,
      const iterator<Const>& y)
      requires std::sized_sentinel_for<
        std::ranges::sentinel_t<Base>,
        std::ranges::iterator_t<Base>>
    {
      return x.end_ - y.base();
    }
  };
} // namespace ns
