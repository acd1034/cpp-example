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
    constexpr iterator(std::ranges::iterator_t<View> current, std::size_t count)
      : current_(std::move(current)), count_(std::move(count)) {}
  };

  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  struct enumerate_view<View>::sentinel {
  private:
    //! 元となる view の番兵イテレータ
    std::ranges::sentinel_t<View> end_ = std::ranges::sentinel_t<View>();

  public:
    constexpr explicit sentinel(std::ranges::sentinel_t<View> end) : end_(std::move(end)) {}
  };
} // namespace ns
