// pragma once

#include <concepts>
#include <exception>
#include <iostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace my_std {

class any {
private:
  struct container_base {
    virtual ~container_base() {}
    virtual void *operator*() { return nullptr; }
    virtual const std::type_info &type() const noexcept { return typeid(void); }
    // TODO: Is there a better way of overloading a copy?
    virtual container_base *deep_copy() { return this; }
  };

  template <typename T> struct container : public container_base {
    // == special member functions
    container() = delete;
    ~container() = default;

    container(container &&other) : t(other.t) {}
    container &operator=(container &&other) {
      t(std::move(other.t));
      return *this;
    }

    container(const container &other) : t(other.t) {}
    container &operator=(const container &other) {
      t(other.t);
      return *this;
    }
    // == end of special member functions

    // container(T t_) : t(t_) {}
    container(const T &t_) : t(t_) {}
    container(T &&t) : t(std::move(t)) {}

    void *operator*() { return &t; }
    const std::type_info &type() const noexcept { return typeid(T); }

    container_base *deep_copy() { return new container(t); }

    T t;
  };

  container_base *_t;

public:
  any() : _t(nullptr) {}

  // template <typename T> any(T t) : _t(new container(t)) {}
  template <typename T> any(const T &t) : _t(new container(t)) {}
  template <typename T> any(T &&t) : _t(new container(std::move(t))) {}

  any(const any &other) { _t = other._t->deep_copy(); }
  any &operator=(const any &other) {
    _t = other._t->deep_copy();
    return *this;
  }

  any(any &&other) {
    _t = other._t;
    other._t = nullptr;
  }
  any &operator=(any &&other) {
    _t = other._t;
    other._t = nullptr;
    return *this;
  }

  ~any() noexcept {
    if (_t)
      delete _t;
  }

  const std::type_info &type() const noexcept {
    if (_t)
      return _t->type();

    return typeid(void);
  }

  void reset() {
    if (!has_value())
      return;

    delete _t;
    _t = nullptr;
  }

  bool has_value() const { return _t != nullptr; }

  void swap(any &other) {
    auto temp = _t;
    _t = other._t;
    other._t = temp;
  }

  void *operator*() { return *(*_t); }

  const void *operator*() const { return *(*_t); }

  template <typename T> friend std::remove_cvref_t<T> any_cast(any &&);
};

class bad_any_cast : public std::exception {
public:
  const char *what() const noexcept { return "Bad any_cast"; }
};

template <typename T> using U = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T> T any_cast(const any &operand) {
  auto ptr = any_cast<U<T>>(&operand);
  if (ptr == nullptr)
    throw bad_any_cast();

  return static_cast<T>(*ptr);
}

template <typename T> T any_cast(any &operand) {
  auto ptr = any_cast<U<T>>(&operand);
  if (ptr == nullptr)
    throw bad_any_cast();

  return static_cast<T>(*ptr);
}

template <typename T> T any_cast(any &&operand) {
  auto ptr = any_cast<U<T>>(&operand);
  if (ptr == nullptr)
    throw bad_any_cast();

  return static_cast<T>(std::move(*ptr));
}

template <typename T> const T *any_cast(const any *operand) noexcept {
  if (typeid(T) == operand->type() and operand->has_value()) {
    return static_cast<const T *>(**operand);
  }

  return nullptr;
}

template <typename T> T *any_cast(any *operand) noexcept {
  if (typeid(T) == operand->type() and operand->has_value()) {
    return static_cast<T *>(**operand);
  }

  return nullptr;
}

template <typename T, typename... Args> any make_any(Args &&...args) {
  return any(T(std::forward<Args>(args)...));
}

}; // namespace my_std

struct S {
  S() = default;
  S(const S &) { std::cout << "called S copy\n"; }
  S(S &&) { std::cout << "called S move\n"; }
};

std::ostream &operator<<(std::ostream &os, const S &s) { return os << "S{}"; }

int main() {
  std::vector<my_std::any> vec;
  my_std::any val = 1;
  my_std::any val2(2.0f);

  vec.push_back(val);
  // vec.push_back(val2);

  my_std::any s(S{});
  std::cout << "\n===pushing S onto vec:\n";
  vec.push_back(s);
  std::cout << "===done\n\n";

  std::cout << "type of void: " << typeid(void).name() << '\n';

  std::cout << "type of val: " << val.type().name() << '\n';
  std::cout << "type of val2: " << val2.type().name() << '\n';
  std::cout << "val addr: " << *(val) << "\n";

  std::cout << "vec addr: " << &(vec[0])
            << "; val: " << my_std::any_cast<int>(vec[0]) << '\n';
  std::cout << "vec addr: " << &(vec[1])
            << "; val: " << my_std::any_cast<S>(vec[1]) << '\n';

  std::boolalpha(std::cout);

  my_std::any a0;
  std::cout << "a0.has_value(): " << a0.has_value() << '\n';

  my_std::any a1 = 42;
  std::cout << "a1.has_value(): " << a1.has_value() << '\n';
  std::cout << "a1 = " << my_std::any_cast<int>(a1) << '\n';
  a1.reset();
  std::cout << "a1.has_value(): " << a1.has_value() << '\n';

  auto a2 = my_std::make_any<std::string>("Milky Way");
  std::cout << "a2.has_value(): " << a2.has_value() << '\n';
  std::cout << "a2 = \"" << my_std::any_cast<std::string &>(a2) << "\"\n";
  a2.reset();
  std::cout << "a2.has_value(): " << a2.has_value() << '\n';

  // Simple example

  a1 = my_std::any(12);

  std::cout << "1) a1 is int: " << my_std::any_cast<int>(a1) << '\n';

  try {
    auto s = my_std::any_cast<std::string>(a1); // throws
  } catch (const my_std::bad_any_cast &e) {
    std::cout << "2) " << e.what() << '\n';
  }

  // Pointer example

  if (int *i = my_std::any_cast<int>(&a1))
    std::cout << "3) a1 is int: " << *i << '\n';
  else if (std::string *s = my_std::any_cast<std::string>(&a1))
    std::cout << "3) a1 is std::string: " << *s << '\n';
  else
    std::cout << "3) a1 is another type or unset\n";

  // // Advanced example

  a1 = std::string("hello");

  auto &ra = my_std::any_cast<std::string &>(a1); //< reference
  ra[1] = 'o';

  std::cout << "4) a1 is string: " << my_std::any_cast<std::string const &>(a1)
            << '\n'; //< const reference

  auto s1 =
      my_std::any_cast<std::string &&>(std::move(a1)); //< rvalue reference

  // Note: s1 is a move-constructed std::string:
  static_assert(std::is_same_v<decltype(s1), std::string>);

  a2 = my_std::any(S{});
  std::cout << "\n\n===testing any_cast<S &&>\n";
  auto s2 = my_std::any_cast<S &&>(std::move(a2)); //< rvalue reference

  // Note: s2 is a move-constructed std::string:
  static_assert(std::is_same_v<decltype(s2), S>);

  // // Note: the std::string in a1 is left in valid but unspecified state
  std::cout << "5) a1.size(): "
            << my_std::any_cast<std::string>(&a1)->size() //< pointer
            << '\n';

  std::cout << "6) s1: " << s1 << '\n';
}
