/// @file enumerate_view.hpp
#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ns {
  // clang-format off
  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  struct enumerate_view : std::ranges::view_interface<enumerate_view<View>> {
    // clang-format on
  private:
    View base_ = View();

    class iterator;
    class sentinel;

  public:
    enumerate_view() requires std::default_initializable<View>
    = default;
    constexpr enumerate_view(View base) : base_(std::move(base)) {}

    constexpr iterator begin() { return {std::ranges::begin(base_), 0}; }

    constexpr auto end() { return sentinel(std::ranges::end(base_)); }
  };

  template <class Range>
  enumerate_view(Range&&) -> enumerate_view<std::views::all_t<Range>>;

  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  struct enumerate_view<View>::iterator {
  private:
    std::ranges::iterator_t<View> current_ = std::ranges::iterator_t<View>();
    std::size_t count_ = 0;

  public:
    using difference_type = std::ranges::range_difference_t<View>;
    using value_type = std::pair<std::size_t, std::ranges::range_value_t<View>>;
    using iterator_category = std::input_iterator_tag;
    // clang-format off
    using iterator_concept =
      std::conditional_t<std::ranges::random_access_range<View>, std::random_access_iterator_tag,
      std::conditional_t<std::ranges::bidirectional_range<View>, std::bidirectional_iterator_tag,
      std::conditional_t<std::ranges::forward_range<View>,       std::forward_iterator_tag,
      /* else */                                                 std::input_iterator_tag>>>;
    // clang-format on

    iterator() requires std::default_initializable<std::ranges::iterator_t<View>>
    = default;
    constexpr iterator(std::ranges::iterator_t<View> current, std::size_t count)
      : current_(std::move(current)), count_(std::move(count)) {}

    constexpr const std::ranges::iterator_t<View>& base() const& noexcept { return current_; }
    constexpr std::ranges::iterator_t<View> base() && { return std::move(current_); }

    constexpr std::pair<std::size_t, std::ranges::range_reference_t<View>> //
    operator*() const {
      return {count_, *current_};
    }

    constexpr iterator& operator++() {
      ++current_;
      ++count_;
      return *this;
    }
    constexpr void operator++(int) { ++*this; }
    constexpr iterator operator++(int) requires std::ranges::forward_range<View> {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--() requires std::ranges::bidirectional_range<View> {
      --current_;
      --count_;
      return *this;
    }
    constexpr iterator operator--(int) requires std::ranges::bidirectional_range<View> {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator&
    operator+=(difference_type n) requires std::ranges::random_access_range<View> {
      current_ += n;
      count_ += n;
      return *this;
    }
    constexpr iterator&
    operator-=(difference_type n) requires std::ranges::random_access_range<View> {
      current_ -= n;
      count_ -= n;
      return *this;
    }
    constexpr std::pair<std::size_t, std::ranges::range_reference_t<View>>
    operator[](difference_type n) const requires std::ranges::random_access_range<View> {
      return *(*this + n);
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y) //
      requires std::equality_comparable<std::ranges::iterator_t<View>> {
      return x.current_ == y.current_;
    }

    friend constexpr bool operator<(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<View> {
      return x.current_ < y.current_;
    }
    friend constexpr bool operator>(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<View> {
      return y < x;
    }
    friend constexpr bool operator<=(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<View> {
      return not(y < x);
    }
    friend constexpr bool operator>=(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<View> {
      return not(x < y);
    }
    friend constexpr auto operator<=>(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<View> and                   //
      std::three_way_comparable<std::ranges::iterator_t<View>> {
      return x.current_ <=> y.current_;
    }

    friend constexpr iterator operator+(iterator x, difference_type n) //
      requires std::ranges::random_access_range<View> {
      x += n;
      return x;
    }
    friend constexpr iterator operator+(difference_type n, iterator x) //
      requires std::ranges::random_access_range<View> {
      x += n;
      return x;
    }
    friend constexpr iterator operator-(iterator x, difference_type n) //
      requires std::ranges::random_access_range<View> {
      x -= n;
      return x;
    }
    friend constexpr difference_type operator-(const iterator& x, const iterator& y) //
      requires std::ranges::random_access_range<View> {
      return x.current_ - y.current_;
    }
  };

  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  struct enumerate_view<View>::sentinel {
  private:
    std::ranges::sentinel_t<View> end_ = std::ranges::sentinel_t<View>();

  public:
    sentinel() = default;
    constexpr explicit sentinel(std::ranges::sentinel_t<View> end) : end_(std::move(end)) {}

    friend constexpr bool operator==(const iterator& x, const sentinel& y) requires
      std::sentinel_for<std::ranges::sentinel_t<View>, std::ranges::iterator_t<View>> {
      return x.base() == y.end_;
    }
  };
} // namespace ns
