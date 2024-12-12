#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ns/tuple_arrange.hpp>
#include <algorithm>
#include <chrono>
#include <format>
#include <string>

TEST_CASE("valid_tuple", "[tuple_arrange][valid_tuple]") {
  STATIC_CHECK(ns::valid_tuple<std::tuple<int, double, std::string>>);
  STATIC_CHECK(ns::valid_tuple<std::tuple<int&, double&&, std::string&>>);
  STATIC_CHECK(ns::valid_tuple<std::tuple<int, double, std::string>&>);
  STATIC_CHECK_FALSE(
    ns::valid_tuple<std::tuple<int&, double&&, std::string&>&>);
  STATIC_CHECK(ns::valid_tuple<std::tuple<int, double, std::string>&&>);
  STATIC_CHECK(ns::valid_tuple<std::tuple<int&, double&&, std::string&>&&>);
}

TEST_CASE("tuple_select", "[tuple_arrange][tuple_select]") {
  {
    std::tuple tpl{0, 3.14, std::string("Hello")};
    {
      auto [x] = ns::tuple_select<1>(tpl);
      CHECK(x == Catch::Approx(3.14));
    }
    {
      auto [x, y] = ns::tuple_select<1, 2>(tpl);
      CHECK(x == Catch::Approx(3.14));
      CHECK(y == "Hello");
    }
    {
      auto [x, y, z] = ns::tuple_select<1, 0, 2>(tpl);
      CHECK(x == Catch::Approx(3.14));
      CHECK(y == 0);
      CHECK(z == "Hello");
    }
  }
  {
    // Check with forward_as_tuple
    double d = 3.14;
    {
      auto&& [x] =
        ns::tuple_select<1>(std::forward_as_tuple(0, d, std::string("Hello")));
      CHECK(x == Catch::Approx(3.14));
    }
    {
      auto&& [x, y] = ns::tuple_select<1, 2>(
        std::forward_as_tuple(0, d, std::string("Hello")));
      CHECK(x == Catch::Approx(3.14));
      CHECK(y == "Hello");
    }
    {
      auto&& [x, y, z] = ns::tuple_select<1, 0, 2>(
        std::forward_as_tuple(0, d, std::string("Hello")));
      CHECK(x == Catch::Approx(3.14));
      CHECK(y == 0);
      CHECK(z == "Hello");
    }
  }
}

TEST_CASE("tuple_element_index_v", "[tuple_arrange][tuple_element_index_v]") {
  using Tuple = std::tuple<int, double, std::string>;
  STATIC_CHECK(ns::tuple_element_index_v<int, Tuple> == 0);
  STATIC_CHECK(ns::tuple_element_index_v<double, Tuple> == 1);
  STATIC_CHECK(ns::tuple_element_index_v<std::string, Tuple> == 2);
}

TEST_CASE("tuple_select_by_type", "[tuple_arrange][tuple_select_by_type]") {
  {
    std::tuple tpl{0, 3.14, std::string("Hello")};
    {
      auto [x] = ns::tuple_select_by_type<double>(tpl);
      CHECK(x == Catch::Approx(3.14));
    }
    {
      auto [x, y] = ns::tuple_select_by_type<double, std::string>(tpl);
      CHECK(x == Catch::Approx(3.14));
      CHECK(y == "Hello");
    }
    {
      auto [x, y, z] = ns::tuple_select_by_type<double, int, std::string>(tpl);
      CHECK(x == Catch::Approx(3.14));
      CHECK(y == 0);
      CHECK(z == "Hello");
    }
  }
  {
    // Check with forward_as_tuple
    double d = 3.14;
    {
      auto&& [x] = ns::tuple_select_by_type<double&>(
        std::forward_as_tuple(0, d, std::string("Hello")));
      CHECK(x == Catch::Approx(3.14));
    }
    {
      auto&& [x, y] = ns::tuple_select_by_type<double&, std::string&&>(
        std::forward_as_tuple(0, d, std::string("Hello")));
      CHECK(x == Catch::Approx(3.14));
      CHECK(y == "Hello");
    }
    {
      auto&& [x, y, z] =
        ns::tuple_select_by_type<double&, int&&, std::string&&>(
          std::forward_as_tuple(0, d, std::string("Hello")));
      CHECK(x == Catch::Approx(3.14));
      CHECK(y == 0);
      CHECK(z == "Hello");
    }
  }
}

TEST_CASE("tuple_format", "[tuple_arrange][tuple_format]") {
  std::tuple tpl{0, 3.14, std::string("Hello")};
  CHECK(ns::tuple_format(tpl) == "(0, 3.14, Hello)");
}

std::string test_fn(int, const double&, std::string&&);

TEST_CASE(
  "function_result_type and function_args_type",
  "[tuple_arrange][function_result_type][function_args_type]") {
  {
    // Check function pointers
    // clang-format off
    STATIC_CHECK(std::is_same_v<
                 ns::function_result_type<decltype(&test_fn)>,
                 std::string>);
    // clang-format on
    STATIC_CHECK(std::is_same_v<
                 ns::function_args_type<decltype(&test_fn)>,
                 std::tuple<int, const double&, std::string&&>>);
  }
  {
    // Check function objects
    constexpr auto test_fn_obj = [](int, const double&, std::string&&) {
      return std::string{};
    };
    // clang-format off
    STATIC_CHECK(std::is_same_v<
                 ns::function_result_type<decltype(test_fn_obj)>,
                 std::string>);
    // clang-format on
    STATIC_CHECK(std::is_same_v<
                 ns::function_args_type<decltype(test_fn_obj)>,
                 std::tuple<int, const double&, std::string&&>>);
  }
}

std::string to_str(const int& i, const double& d, const std::string& s) {
  return std::format("({}, {}, {})", i, d, s);
}

TEST_CASE("unordered_fn", "[tuple_arrange][unordered_fn]") {
  {
    // Check function pointers
    constexpr auto unordered_to_str = ns::unordered_fn(&to_str);
    CHECK(
      unordered_to_str(3.14, 0, std::string("Hello")) == "(0, 3.14, Hello)");
  }
  {
    // Check function objects
    constexpr auto to_str_fn_obj =
      [](const int& i, const double& d, const std::string& s) {
        return std::format("({}, {}, {})", i, d, s);
      };
    constexpr auto unordered_to_str = ns::unordered_fn(to_str_fn_obj);
    CHECK(
      unordered_to_str(3.14, 0, std::string("Hello")) == "(0, 3.14, Hello)");
  }
}

TEST_CASE("unordered_to_digit", "[tuple_arrange][unordered_to_digit]") {
  namespace chrono = std::chrono;
  constexpr auto to_digit =
    [](const chrono::year& y, const chrono::month& m, const chrono::day& d) {
      return 10000 * int(y) + 100 * int(unsigned(m)) + int(unsigned(d));
    };
  constexpr auto unordered_to_digit = ns::unordered_fn(to_digit);
  CHECK(
    unordered_to_digit(chrono::day{16}, chrono::December, chrono::year{2022}) ==
    20221216);
}

inline constexpr std::uint64_t Seed = 0x01234567DEADC0DE;

TEST_CASE("make_permutation", "[tuple_arrange][make_permutation]") {
  constexpr std::size_t N = 10;
  auto indices = ns::make_permutation<N>(Seed);
  std::sort(indices.begin(), indices.end());
  for (std::size_t i = 1; i < N; ++i) {
    CHECK(indices[i] != indices[i - 1]);
  }
  for (const auto& index : indices) {
    CHECK(index >= 0);
    CHECK(index < N);
  }
}

TEST_CASE("tuple_shuffle", "[tuple_arrange][tuple_shuffle]") {
  {
    std::tuple tpl{0, 3.14, std::string("Hello")};
    auto tpl2 = ns::tuple_shuffle<Seed>(tpl);
    {
      STATIC_CHECK(ns::tuple_element_index_v<int, decltype(tpl2)> < 3);
      STATIC_CHECK(ns::tuple_element_index_v<double, decltype(tpl2)> < 3);
      STATIC_CHECK(ns::tuple_element_index_v<std::string, decltype(tpl2)> < 3);
    }
    {
      auto tpl3 = ns::tuple_shuffle<Seed + 1>(tpl);
      constexpr bool Happens1In36Times =
        std::is_same_v<decltype(tpl2), decltype(tpl)> and
        std::is_same_v<decltype(tpl3), decltype(tpl)>;
      STATIC_CHECK_FALSE(Happens1In36Times);
    }
  }
  {
    // Check with forward_as_tuple
    double d = 3.14;
    auto tpl2 = ns::tuple_shuffle<Seed>(
      std::forward_as_tuple(0, d, std::string("Hello")));
    {
      STATIC_CHECK(ns::tuple_element_index_v<int&&, decltype(tpl2)> < 3);
      STATIC_CHECK(ns::tuple_element_index_v<double&, decltype(tpl2)> < 3);
      STATIC_CHECK(
        ns::tuple_element_index_v<std::string&&, decltype(tpl2)> < 3);
    }
    {
      auto tpl3 = ns::tuple_shuffle<Seed + 1>(
        std::forward_as_tuple(0, d, std::string("Hello")));
      // clang-format off
      constexpr bool Happens1In36Times =
        std::is_same_v<decltype(tpl2), std::tuple<int&&,double&,std::string&&>> and
        std::is_same_v<decltype(tpl3), std::tuple<int&&,double&,std::string&&>>;
      // clang-format on
      STATIC_CHECK_FALSE(Happens1In36Times);
    }
  }
}
