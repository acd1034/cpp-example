#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

//

for (auto x : std::views::iota(0)
                | std::views::filter([](auto x) { return x % 2 == 0; })
                | std::views::transform([](auto x) { return x * x; })
                | std::views::take(4))
  std::cout << x << ','; // output: 0,4,16,36,

//

std::forward_list<int> fl{};
// fl は common_range
static_assert(std::ranges::common_range<decltype(fl)>);
std::ranges::take_view taken(fl, 0);
// taken は common_range ではない
static_assert(not std::ranges::common_range<decltype(taken)>);

const std::vector<int> v{};
// v は const-iterable
static_assert(std::ranges::range<decltype(v)>);
const std::ranges::filter_view filtered(v, std::identity{});
// filtered は const-iterable ではない
static_assert(not std::ranges::range<decltype(filtered)>);

// clang-format off
template <class T>
concept simple_view = // exposition only
  std::ranges::view<T> and std::ranges::range<const T> and
  std::same_as<std::ranges::iterator_t<T>, std::ranges::iterator_t<const T>> and
  std::same_as<std::ranges::sentinel_t<T>, std::ranges::sentinel_t<const T>>;
// clang-format on

// view の作り方の例

std::vector<int> v{0, 1, 2};
auto pred = [](auto x) { return x % 2 == 0; };
std::ranges::filter_view filtered(v, pred);
auto filtered2 = std::views::filter(v, pred);

auto filtered3 = v | std::views::filter(pred);

// enumerate (range adaptor closure)

struct enumerate_fn : std::ranges::range_adaptor_closure<enumerate_fn> {
  template <std::ranges::viewable_range Range>
  constexpr auto operator()(Range&& range) const
    noexcept(noexcept(enumerate_view(std::forward<Range>(range)))) {
    return enumerate_view(std::forward<Range>(range));
  }
};

inline namespace cpo {
  inline constexpr auto enumerate = enumerate_fn();
} // namespace cpo

// filter (range adaptor)
// clang-format off

struct filter_fn {
  template <std::ranges::viewable_range Range, class Pred>
  constexpr auto operator()(Range&& range, Pred&& pred) const
    noexcept(noexcept(
      std::ranges::filter_view(std::forward<Range>(range), std::forward<Pred>(pred)))) {
    return
      std::ranges::filter_view(std::forward<Range>(range), std::forward<Pred>(pred));
  }
  template <class Pred>
  requires std::constructible_from<std::decay_t<Pred>, Pred>
  constexpr auto operator()(Pred&& pred) const
    noexcept(noexcept(std::is_nothrow_constructible_v<std::decay_t<Pred>, Pred>)) {
    return
      std::ranges::range_adaptor_closure(std::bind_back(*this, std::forward<Pred>(pred)));
  }
};

inline namespace cpo {
  inline constexpr auto filter = filter_fn();
} // namespace cpo
