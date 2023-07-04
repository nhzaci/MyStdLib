#include <iostream>
#include <variant>
#include <vector>

namespace my_std {

// references:
// - Fernando Garcia:
// https://medium.com/@mortificador/implementing-std-tuple-in-c-17-3cc5c6da7277

// stores a single element
template <size_t index, typename T> class _tuple_data {
private:
  T data;

public:
  _tuple_data() : data{} {}
  _tuple_data(T const &v) { data = v; }
  _tuple_data(T &&v) { data = std::move(v); }

  T &get() { return data; }
};

// template base case
template <size_t _index, typename... types> class _tuple_recur_base {
public:
  _tuple_recur_base() {}

  template <typename... CArgs> _tuple_recur_base(CArgs &&...args) {}
};

// template partial specialization
template <size_t _index, typename L, typename... types>
class _tuple_recur_base<_index, L, types...>
    : public _tuple_data<_index, typename std::remove_reference<L>::type>,
      public _tuple_recur_base<_index + 1, types...> {
public:
  template <typename... CArgs>
  _tuple_recur_base(CArgs &&...args)
      : _tuple_data<_index, typename std::remove_reference<L>::type>(),
        _tuple_recur_base<_index + 1, types...>(std::forward<CArgs>(args)...) {}

  template <typename CL, typename... CArgs>
  _tuple_recur_base(CL &&arg, CArgs &&...args)
      : _tuple_data<_index, typename std::remove_reference<L>::type>(
            std::forward<CL>(arg)),
        _tuple_recur_base<_index + 1, types...>(std::forward<CArgs>(args)...) {}
};

// expose only the tuple api
template <typename L, typename... types>
class tuple : public _tuple_recur_base<0, L, types...> {
public:
  template <typename... CArgs>
  tuple(CArgs &&...args)
      : _tuple_recur_base<0, L, types...>(std::forward<CArgs>(args)...) {}
};

template <size_t index, typename L, typename... Args> struct extract_type_at {
  using type = typename extract_type_at<index - 1, Args...>::type;
};

template <typename L, typename... Args> struct extract_type_at<0, L, Args...> {
  using type = L;
};

template <size_t index, typename... Args> auto &get(tuple<Args...> &t) {
  return static_cast<_tuple_data<
      index, typename extract_type_at<index, Args...>::type> &>(t)
      .get();
}

}; // namespace my_std
