.PHONY: fmt
fmt:
	find include -name "*.hpp" | xargs clang-format -i
	find tests -name "*.hpp" | xargs clang-format -i
	find tests -name "*.cpp" | xargs clang-format -i
