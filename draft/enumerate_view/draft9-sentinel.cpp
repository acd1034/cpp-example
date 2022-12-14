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
  struct enumerate_view {
  private:
    //! 元となる view
    View base_ = View();

    class iterator;
    class sentinel;

  public:
    constexpr enumerate_view(View base) : base_(std::move(base)) {}
  };

  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  struct enumerate_view<View>::iterator {
  private:
    //! 元となるイテレータの現在位置
    std::ranges::iterator_t<View> current_ = std::ranges::iterator_t<View>();
    //! 現在のインデックス
    std::size_t count_ = 0;

  public:
    using difference_type = std::ranges::range_difference_t<View>;

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
  };

  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  struct enumerate_view<View>::sentinel {
  private:
    //! 元となる view の終端
    std::ranges::sentinel_t<View> end_ = std::ranges::sentinel_t<View>();

  public:
    sentinel() requires std::default_initializable<std::ranges::sentinel_t<View>>
    = default;
    constexpr explicit sentinel(std::ranges::sentinel_t<View> end) : end_(std::move(end)) {}

    friend constexpr bool operator==(const iterator& x, const sentinel& y) requires
      std::sentinel_for<std::ranges::sentinel_t<View>, std::ranges::iterator_t<View>> {
      return x.base() == y.end_;
    }
  };
} // namespace ns
