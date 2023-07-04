#pragma once

#include <cstddef>
#include <typeinfo>
#include <variant>

namespace my_std {

// references:
// - spec: https://en.cppreference.com/w/cpp/utility/variant
// - code: https://ojdip.net/2013/10/implementing-a-variant-type-in-cpp/
//
// The class template std::variant represents a type-safe union.
//
// * An instance of std::variant at any given time either holds a value of one
// of its alternative types, or in the case of error - no value (this state is
// hard to achieve, see valueless_by_exception). As with unions, if a variant
// holds a value of some object type T, the object representation of T is
// allocated directly within the object representation of the variant itself.
// Variant is not allowed to allocate additional (dynamic) memory.
//
// * A variant is not permitted to hold references, arrays, or the type void.
// Empty variants are also ill-formed (std::variant<std::monostate> can be used
// instead).
//
// * A variant is permitted to hold the same type more than once, and to hold
// differently cv-qualified versions of the same type.
//
// * Consistent with the behavior of unions during aggregate initialization, a
// default-constructed variant holds a value of its first alternative, unless
// that alternative is not default-constructible (in which case the variant is
// not default-constructible either). The helper class std::monostate can be
// used to make such variants default-constructible. Template parameters Types
// - the types that may be stored in this variant. All types must meet the
// Destructible requirements (in particular, array types and non-object types
// are not allowed).

template <typename... Types> class variant {
public:
  static inline size_t invalid_type() { return typeid(void); }

  constexpr variant() noexcept() : type_info(invalid_type()) {}

  // deleted copy constructors
  variant(const variant<Types...> &other) {
    // deep copy using type's copy constructor
    helper_t::copy(other.type_info, &other.data, &data);
  }
  variant<Types...> &operator=(const variant<Types...> &other) {
    // deep copy using type's copy constructor
    helper_t::copy(other.type_info, &other.data, &data);

    return *this;
  }

  // ok to move, make previous type invalid
  variant(variant<Types...> &&other)
      : type_info(other.type_id), data(other.data) {
    other.type_info = invalid_type();
  }
  variant<Types...> &operator=(variant<Types...> &&other) {
    type_info = other.type_info;
    data = other.data;
    other.type_info = invalid_type();
    return *this;
  }

  // destructor calls recursive helper which calls destructor of the correct
  // type
  ~variant() noexcept { helper_t::destroy(type_info, &data); }

  template <typename T, typename... Args> void emplace(Args &&...args) {
    // destroy current object
    helper_t::destroy(type_info, &data);

    // placement new on new object
    new (&data) T(std::forward<Args>(args)...);
    type_info = typeid(T);
  }

  template <typename T> T &get() {
    if (type_info == typeid(T)) {
      return *reinterpret_cast<T *>(&data);
    } else {
      throw std::bad_cast();
    }
  }

  template <typename T> bool is() { return type_info == typeid(T); }

  bool valid() { return type_info != invalid_type(); }

private:
  using helper_t = variant_helper<Types...>;

  // storing current type;
  std::typeinfo type_info;
  // variant takes up as much space as the largest type, aligned properly
  alignas(Types...) std::byte data[std::max({sizeof(Types)...})];
};

template <typename F, typename... Types> struct variant_helper {
  inline static void copy(size_t old_t, const void *old_v, void *new_v) {
    if (old_t == typeid(F))
      new (new_v) F(*reinterpret_cast<const F *>(old_v));
    else
      variant_helper<Types...>::copy(old_t, old_v, new_v);
  }

  inline static void destroy(size_t id, void *data) {
    if (id == typeid(F)) {
      reinterpret_cast<F *>(data)->~F();
    } else {
      variant_helper<Types...>::destroy(id, data);
    }
  }
};

template <typename F> struct variant_helper {
  inline static void copy(size_t old_t, const void *old_v, void *new_v) {
    if (old_t == typeid(F))
      new (new_v) F(*reinterpret_cast<const F *>(old_v));
  }

  inline static void destroy(size_t id, void *data) {
    if (id == typeid(F)) {
      reinterpret_cast<F *>(data)->~F();
    }
  }
};

}; // namespace my_std