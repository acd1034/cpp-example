#include <cassert>
#include <concepts>
#include <functional>
#include <iostream>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ns {
  // tuple_select
  template <std::size_t... Is>
  constexpr auto tuple_select(auto&& tpl) -> std::tuple<
    std::tuple_element_t<Is, std::remove_reference_t<decltype(tpl)>>...> {
    return {std::get<Is>(std::forward<decltype(tpl)>(tpl))...};
  }

  // tuple_element_index_v
  // clang-format off
  template <std::size_t I, class T>
  struct tuple_leaf {};
  template <class Seq, class... Ts>
  struct tuple_plant;
  template <std::size_t... Is, class... Ts>
  struct tuple_plant<std::index_sequence<Is...>, Ts...> : tuple_leaf<Is, Ts>... {};
  template <class T, std::size_t I>
  std::integral_constant<std::size_t, I> get_index(tuple_leaf<I, T>); // undefined

  template <class T, class Tuple>
  struct tuple_element_index;
  template <class T, class Tuple>
  struct tuple_element_index<T, const Tuple> : tuple_element_index<T, Tuple> {};
  template <class T, class... Ts>
  struct tuple_element_index<T, std::tuple<Ts...>>
    : decltype(get_index<T>(tuple_plant<std::index_sequence_for<Ts...>, Ts...>{})) {};
  template <class T, class Tuple>
  inline constexpr std::size_t tuple_element_index_v = tuple_element_index<T, Tuple>::value;
  // clang-format on

  // tuple_select_by_type
  template <typename... Ts>
  constexpr decltype(auto) tuple_select_by_type(auto&& tpl) {
    return tuple_select<
      tuple_element_index_v<Ts, std::remove_reference_t<decltype(tpl)>>...>(
      std::forward<decltype(tpl)>(tpl));
  }
} // namespace ns
