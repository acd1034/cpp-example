#include <array>
#include <cassert>
#include <concepts>
#include <format>
#include <functional>
#include <numeric>
#include <type_traits>
#include <utility>

namespace ns {
  // valid_tuple

  // clang-format off
  template <class Tuple>
  struct is_tuple : std::false_type {};
  template <class Tuple>
  struct is_tuple<const Tuple> : is_tuple<Tuple> {};
  template <class... Ts>
  struct is_tuple<std::tuple<Ts...>> : std::true_type {};
  template <class Tuple>
  inline constexpr bool is_tuple_v = is_tuple<Tuple>::value;

  template <class Tuple>
  struct holds_rvalue_reference;
  template <class Tuple>
  struct holds_rvalue_reference<const Tuple> : holds_rvalue_reference<Tuple> {};
  template <class... Ts>
  struct holds_rvalue_reference<std::tuple<Ts...>>
    : std::disjunction<std::is_rvalue_reference<Ts>...> {};
  template <class Tuple>
  inline constexpr bool holds_rvalue_reference_v = holds_rvalue_reference<Tuple>::value;
  // clang-format on

  template <class Tuple>
  concept valid_tuple = is_tuple_v<std::remove_reference_t<Tuple>> and
    not(std::is_lvalue_reference_v<Tuple> and
        holds_rvalue_reference_v<std::remove_reference_t<Tuple>>);

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
  // Overload for function objects
  template <class F, bool isClass = std::is_class<F>::value>
  struct function_traits : function_traits<decltype(&F::operator())> {};
  // Overload for class functions
  template <class Class, class R, class... Args>
  struct function_traits<R (Class::*)(Args...), false> {
    using result_type = R;
    using args_type = std::tuple<Args...>;
  };
  // Overload for const class functions
  template <class Class, class R, class... Args>
  struct function_traits<R (Class::*)(Args...) const, false>
    : function_traits<R (Class::*)(Args...)> {};
  // Overload for function pointers
  template <class R, class... Args>
  struct function_traits<R (*)(Args...), false> {
    using result_type = R;
    using args_type = std::tuple<Args...>;
  };
  // Overload for const funtion pointers
  template <class R, class... Args>
  struct function_traits<R (*const)(Args...), false>
    : function_traits<R (*)(Args...)> {};
  // Overload for function references
  template <class R, class... Args>
  struct function_traits<R (&)(Args...), false>
    : function_traits<R (*)(Args...)> {};

  template <class F>
  using function_result_type = typename function_traits<F>::result_type;
  template <class F>
  using function_args_type = typename function_traits<F>::args_type;

  // unordered_fn

  template <class F>
  struct unordered_fn_t {
    F f;

    template <class G, class... Args>
    static constexpr auto call(G&& g, Args&&... args)
      -> function_result_type<std::remove_cvref_t<G>> {
      return [&]<class... Ts>(std::in_place_type_t<std::tuple<Ts...>>) {
        auto tpl = std::make_tuple(std::forward<Args>(args)...);
        auto reordered_tpl =
          tuple_select_by_type<std::remove_cvref_t<Ts>...>(std::move(tpl));
        return std::apply(std::forward<G>(g), std::move(reordered_tpl));
      }(std::in_place_type<function_args_type<std::remove_cvref_t<G>>>);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) & noexcept(
      noexcept(call(f, std::forward<Args>(args)...)))
      -> decltype(call(f, std::forward<Args>(args)...)) {
      return call(f, std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) const& noexcept(
      noexcept(call(f, std::forward<Args>(args)...)))
      -> decltype(call(f, std::forward<Args>(args)...)) {
      return call(f, std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) && noexcept(
      noexcept(call(std::move(f), std::forward<Args>(args)...)))
      -> decltype(call(std::move(f), std::forward<Args>(args)...)) {
      return call(std::move(f), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) const&& noexcept(
      noexcept(call(std::move(f), std::forward<Args>(args)...)))
      -> decltype(call(std::move(f), std::forward<Args>(args)...)) {
      return call(std::move(f), std::forward<Args>(args)...);
    }
  };

  template <class F>
  constexpr auto unordered_fn(F&& f) -> unordered_fn_t<std::remove_cvref_t<F>> {
    return {std::forward<F>(f)};
  }

  // make_permutation

  constexpr auto xorshift64(std::uint64_t max, std::uint64_t state)
    -> std::pair<std::uint64_t, std::uint64_t> {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return {state % (max + 1), state};
  }

  template <std::size_t N>
  constexpr auto make_permutation(std::uint64_t state)
    -> std::array<std::size_t, N> {
    if constexpr (N == 0) {
      return {};
    } else {
      std::array<std::size_t, N> arr{};
      std::iota(arr.begin(), arr.end(), 0);
      for (std::size_t i = N - 1, j; i > 0; --i) {
        std::tie(j, state) = xorshift64(i, state);
        std::swap(arr[i], arr[j]);
      }
      return arr;
    }
  }

  // tuple_shuffle

  template <std::uint64_t State>
  constexpr decltype(auto) tuple_shuffle(auto&& tpl) {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      constexpr auto indices = make_permutation<sizeof...(Is)>(State);
      return tuple_select<indices[Is]...>(std::forward<decltype(tpl)>(tpl));
    }(std::make_index_sequence<
             std::tuple_size_v<std::remove_reference_t<decltype(tpl)>>>{});
  }
} // namespace ns
