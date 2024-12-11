#include <cassert>
#include <concepts>
#include <format>
#include <functional>
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

  // tuple_format
  template <class Tuple, std::size_t... Is>
  constexpr auto tuple_format(const Tuple& tpl, std::index_sequence<Is...>)
    -> std::string {
    std::string str{};
    const char* dlm = "";
    using swallow = std::initializer_list<int>;
    (void)swallow{
      (void(
         str = std::format(
           "{}{}{}", str, std::exchange(dlm, ", "), std::get<Is>(tpl))),
       0)...};
    return std::format("({})", str);
  }

  template <class Tuple>
  constexpr auto tuple_format(const Tuple& tpl) -> std::string {
    return tuple_format(
      tpl, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
  }

  // function_result_type, function_args_type
  // https://github.com/llvm/llvm-project/blob/2f18b5ef030e37f3b229e767081a804b7c038a07/llvm/include/llvm/ADT/STLExtras.h#L86
  template <class F>
  struct function_traits;
  template <class R, class... Args>
  struct function_traits<R (*)(Args...)> {
    using result_type = R;
    using args_type = std::tuple<Args...>;
  };
  // Overload for const pointers
  template <class R, class... Args>
  struct function_traits<R (*const)(Args...)>
    : function_traits<R (*)(Args...)> {};
  // Overload for references
  template <class R, class... Args>
  struct function_traits<R (&)(Args...)> : function_traits<R (*)(Args...)> {};

  template <class F>
  using function_result_type = typename function_traits<F>::result_type;
  template <class F>
  using function_args_type = typename function_traits<F>::args_type;
} // namespace ns
