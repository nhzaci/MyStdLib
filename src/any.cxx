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
