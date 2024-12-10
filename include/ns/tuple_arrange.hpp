#include <cassert>
#include <concepts>
#include <functional>
#include <iostream>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ns {
  template <std::size_t... Is>
  constexpr auto tuple_select(auto&& tpl) -> std::tuple<
    std::tuple_element_t<Is, std::remove_reference_t<decltype(tpl)>>...> {
    return {std::get<Is>(std::forward<decltype(tpl)>(tpl))...};
  }
} // namespace ns
