#include <tuple>
#include <type_traits>

// entry point, take in a tuple with some types
// e.g. Reverse<std::tuple<t1, t2, ..., tn>>
template <typename... Types> struct Reverse<std::tuple<Types...>> {
  // forward to intermediate step, with empty right result tuple
  using Type = Reverse<std::tuple<Types...>, std::tuple<>>::Type;
};

// intermediate step, move types from left into right tuple
// e.g. Reverse<std::tuple<t3, ..., tn>, std::tuple<t2, t1>>
template <typename F, typename... Types, typename... Reversed>
struct Reverse<std::tuple<F, Types...>, std::tuple<Reversed...>> {
  // continue moving F from left tuple into right result tuple
  using Type = Reverse<std::tuple<Types...>, std::tuple<F, Reversed...>>::Type;
};

// final step, left tuple is empty, so we know we have everything reversed
// e.g. Reverse<std::tuple<>, std::tuple<tn, ..., t2, t1>>
template <typename... Reversed>
struct Reverse<std::tuple<>, std::tuple<Reversed...>> {
  // call base case, which simply makes it a tuple type
  using Type = Reverse<Reversed...>::Type;
};

// types would simply be <tn, ..., t2, t1>
template <typename... Types> struct Reverse {
  // tada! we have our reversed tuple, std::tuple<tn, ..., t2, t1>
  using Type = std::tuple<Types...>;
};

int main() {
  static_assert(
      std::is_same_v<typename Reverse<std::tuple<int, bool, double>>::Type,
                     std::tuple<double, bool, int>>);
}