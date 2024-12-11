#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ns/tuple_arrange.hpp>
#include <chrono>
#include <format>
#include <string>

std::string test_fn(int, const double&, std::string&&);

TEST_CASE("tuple_arrange", "[tuple_arrange]") {
  { // tuple_select
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
  { // tuple_element_index_v
    using Tuple = std::tuple<int, double, std::string>;
    STATIC_CHECK(ns::tuple_element_index_v<int, Tuple> == 0);
    STATIC_CHECK(ns::tuple_element_index_v<double, Tuple> == 1);
    STATIC_CHECK(ns::tuple_element_index_v<std::string, Tuple> == 2);
  }
  { // tuple_select_by_type
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
  { // tuple_format
    std::tuple tpl{0, 3.14, std::string("Hello")};
    CHECK(ns::tuple_format(tpl) == "(0, 3.14, Hello)");
  }
  { // function_result_type, function_args_type
    // clang-format off
    STATIC_CHECK(std::is_same_v<
                 ns::function_result_type<decltype(&test_fn)>,
                 std::string>);
    // clang-format on
    STATIC_CHECK(std::is_same_v<
                 ns::function_args_type<decltype(&test_fn)>,
                 std::tuple<int, const double&, std::string&&>>);
  }
}

std::string to_str(const int& i, const double& d, const std::string& s) {
  return std::format("({}, {}, {})", i, d, s);
}

TEST_CASE("unordered_fn", "[tupple_arrange][unordered_fn]") {
  { // unordered_fn
    constexpr auto unordered_to_str = ns::unordered_fn(&to_str);
    CHECK(
      unordered_to_str(3.14, 0, std::string("Hello")) == "(0, 3.14, Hello)");
  }
}
