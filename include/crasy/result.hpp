#ifndef CRASY_RESULT_HPP
#define CRASY_RESULT_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/detail.hpp>
#include <exception>
#include <system_error>
#include <tuple>
#include <variant>

namespace crasy {

template <typename T, typename E = std::system_error>
class result;

template <typename... Args>
class ok_result_args;

template <typename... Args>
class err_result_args;

template <typename... Args>
constexpr ok_result_args<Args...> ok(Args&&... args);

template <typename... Args>
constexpr err_result_args<Args...> err(Args&&... args);

template <typename... Args>
class ok_result_args {
  private:
    std::tuple<Args...> args_;

    template <typename, typename>
    friend class result;

    template <typename... A>
    friend constexpr ok_result_args<A...> ok(A&&...);

  public:
    explicit constexpr ok_result_args([[maybe_unused]] std::in_place_t in_place,
                                      Args&&... args)
        : args_(std::forward<Args>(args)...) {}
};

template <typename... Args>
class err_result_args {
  private:
    std::tuple<Args...> args_;

    template <typename, typename>
    friend class result;

    template <typename... A>
    friend constexpr err_result_args<A...> err(A&&...);

  public:
    explicit constexpr err_result_args(
        [[maybe_unused]] std::in_place_t in_place,
        Args&&... args)
        : args_(std::forward<Args>(args)...) {}
};

template <typename... Args>
constexpr ok_result_args<Args...> ok(Args&&... args) {
    return ok_result_args<Args...>{std::in_place, std::forward<Args>(args)...};
}

template <typename... Args>
constexpr err_result_args<Args...> err(Args&&... args) {
    return err_result_args<Args...>{std::in_place, std::forward<Args>(args)...};
}

template <typename T, typename E>
class result {
  public:
    using ok_type = typename detail::type_kinds<T>::value_type;
    using ok_reference = typename detail::type_kinds<T>::reference;
    using ok_const_reference = typename detail::type_kinds<T>::const_reference;
    using ok_rvalue_reference =
        typename detail::type_kinds<T>::rvalue_reference;
    using ok_const_rvalue_reference =
        typename detail::type_kinds<T>::const_rvalue_reference;
    using ok_pointer = typename detail::type_kinds<T>::pointer;
    using ok_const_pointer = typename detail::type_kinds<T>::const_pointer;

    using err_type = typename detail::type_kinds<E>::value_type;
    using err_reference = typename detail::type_kinds<E>::reference;
    using err_const_reference = typename detail::type_kinds<E>::const_reference;
    using err_rvalue_reference =
        typename detail::type_kinds<E>::rvalue_reference;
    using err_const_rvalue_reference =
        typename detail::type_kinds<E>::const_rvalue_reference;
    using err_pointer = typename detail::type_kinds<E>::pointer;
    using err_const_pointer = typename detail::type_kinds<E>::const_pointer;

    constexpr result(const result&) = default;
    constexpr result(result&&) = default;
    constexpr ~result() = default;
    constexpr result& operator=(const result&) = default;
    constexpr result& operator=(result&&) = default;

    template <typename... Args>
    constexpr result(ok_result_args<Args...> ok)
        : result(std::move(ok.args_),
                 std::true_type{},
                 std::make_index_sequence<sizeof...(Args)>{}) {}

    template <typename... Args>
    constexpr result(err_result_args<Args...> err)
        : result(std::move(err.args_),
                 std::false_type{},
                 std::make_index_sequence<sizeof...(Args)>{}) {}

    constexpr bool is_ok() const { return value_.index() == ok_index; }

    constexpr bool is_err() const { return value_.index() == err_index; }

    constexpr operator bool() const { return is_ok(); }

    constexpr bool operator!() const { return is_err(); }

    constexpr ok_reference ok() & { return std::get<ok_holder>(value_).get(); }

    constexpr ok_const_reference ok() const& {
        return std::get<ok_holder>(value_).get();
    }

    constexpr ok_rvalue_reference ok() && {
        return std::get<ok_holder>(std::move(value_)).get();
    }

    constexpr ok_const_rvalue_reference ok() const&& {
        return std::get<ok_holder>(
                   static_cast<const std::variant<ok_holder, err_holder>&&>(
                       value_))
            .get();
    }

    constexpr err_reference err() & {
        return std::get<err_holder>(value_).get();
    }

    constexpr err_const_reference err() const& {
        return std::get<err_holder>(value_).get();
    }

    constexpr err_rvalue_reference err() && {
        return std::get<err_holder>(std::move(value_)).get();
    }

    constexpr err_const_rvalue_reference err() const&& {
        return std::get<err_holder>(
                   static_cast<const std::variant<ok_holder, err_holder>&&>(
                       value_))
            .get();
    }

    constexpr err_result_args<E> propagate() & {
        return static_cast<const result&>(*this).propagate();
    }

    constexpr err_result_args<E> propagate() const& {
        return err_result_args<E>{std::in_place,
                                  std::get<err_holder>(value_).get()};
    }

    constexpr err_result_args<E> propagate() && {
        return err_result_args<E>{
            std::in_place, std::get<err_holder>(std::move(value_)).get()};
    }

    constexpr err_result_args<E> propagate() const&& {
        return static_cast<const result&>(*this).propagate();
    }

    constexpr ok_reference operator*() & {
        if constexpr (std::is_base_of_v<std::exception, E>) {
            if (is_err()) { throw err(); }
        }
        return this->ok();
    }

    constexpr ok_const_reference operator*() const& {
        if constexpr (std::is_base_of_v<std::exception, E>) {
            if (is_err()) { throw err(); }
        }
        return this->ok();
    }

    constexpr ok_rvalue_reference operator*() && {
        if constexpr (std::is_base_of_v<std::exception, E>) {
            if (is_err()) { throw std::move(*this).err(); }
        }
        return std::move(*this).ok();
    }

    constexpr ok_const_rvalue_reference operator*() const&& {
        if constexpr (std::is_base_of_v<std::exception, E>) {
            if (is_err()) { throw err(); }
        }
        return static_cast<const result<T, E>&&>(*this).ok();
    }

    constexpr ok_pointer operator->() {
        if constexpr (std::is_base_of_v<std::exception, E>) {
            if (is_err()) { throw err(); }
        }
        return std::get<ok_index>(value_).ptr();
    }

    constexpr ok_const_pointer operator->() const {
        if constexpr (std::is_base_of_v<std::exception, E>) {
            if (is_err()) { throw err(); }
        }
        return std::get<ok_index>(value_).ptr();
    }

    template <typename F>
    constexpr auto map(F&& map_func) & {
        return map(*this, std::forward<F>(map_func));
    }

    template <typename F>
    constexpr auto map(F&& map_func) const& {
        return map(*this, std::forward<F>(map_func));
    }

    template <typename F>
    constexpr auto map(F&& map_func) && {
        return map(std::move(*this), std::forward<F>(map_func));
    }

    template <typename F>
    constexpr auto map(F&& map_func) const&& {
        return map(static_cast<const result&&>(*this),
                   std::forward<F>(map_func));
    }

    template <typename F>
    constexpr auto map_err(F&& map_func) & {
        return map_err(*this, std::forward<F>(map_func));
    }

    template <typename F>
    constexpr auto map_err(F&& map_func) const& {
        return map_err(*this, std::forward<F>(map_func));
    }

    template <typename F>
    constexpr auto map_err(F&& map_func) && {
        return map_err(std::move(*this), std::forward<F>(map_func));
    }

    template <typename F>
    constexpr auto map_err(F&& map_func) const&& {
        return map_err(static_cast<const result&&>(*this),
                       std::forward<F>(map_func));
    }

  private:
    static inline constexpr std::size_t ok_index = 0;
    static inline constexpr std::size_t err_index = 1;

    template <typename Tup, std::size_t... IDX>
    constexpr result(Tup&& args,
                     [[maybe_unused]] std::true_type is_ok,
                     [[maybe_unused]] std::index_sequence<IDX...> seq)
        : value_(std::in_place_type<ok_holder>,
                 std::in_place,
                 std::forward<std::tuple_element_t<IDX, std::decay_t<Tup>>>(
                     std::get<IDX>(args))...) {}

    template <typename Tup, std::size_t... IDX>
    constexpr result(Tup&& args,
                     [[maybe_unused]] std::false_type is_ok,
                     [[maybe_unused]] std::index_sequence<IDX...> seq)
        : value_(std::in_place_type<err_holder>,
                 std::in_place,
                 std::forward<std::tuple_element_t<IDX, std::decay_t<Tup>>>(
                     std::get<IDX>(args))...) {}

    template <typename S, typename F>
    static auto map(S&& self, F&& map_func) {
        if constexpr (std::is_same_v<T, void>) {
            using ok_t = decltype(map_func());
            return std::visit(
                [f = std::forward<F>(map_func)](auto&& val) -> result<ok_t, E> {
                    using holder_t = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<holder_t, ok_holder>) {
                        if constexpr (std::is_same_v<ok_t, void>) {
                            f();
                            return crasy::ok();
                        } else {
                            return crasy::ok(f());
                        }
                    } else {
                        if constexpr (std::is_same_v<E, void>) {
                            return crasy::err();
                        } else {
                            return crasy::err(
                                std::forward<detail::remove_rvalue_reference_t<
                                    decltype(val)>>(val)
                                    .get());
                        }
                    }
                },
                std::forward<S>(self).value_);
        } else {
            using ok_t = decltype(map_func(std::declval<S&&>().ok()));
            return std::visit(
                [f = std::forward<F>(map_func)](auto&& val) -> result<ok_t, E> {
                    using holder_t = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<holder_t, ok_holder>) {
                        if constexpr (std::is_same_v<ok_t, void>) {
                            f(std::forward<detail::remove_rvalue_reference_t<
                                  decltype(val)>>(val)
                                  .get());
                            return crasy::ok();
                        } else {
                            return crasy::ok(f(
                                std::forward<detail::remove_rvalue_reference_t<
                                    decltype(val)>>(val)
                                    .get()));
                        }
                    } else {
                        if constexpr (std::is_same_v<E, void>) {
                            return crasy::err();
                        } else {
                            return crasy::err(
                                std::forward<detail::remove_rvalue_reference_t<
                                    decltype(val)>>(val)
                                    .get());
                        }
                    }
                },
                std::forward<S>(self).value_);
        }
    }

    template <typename S, typename F>
    static auto map_err(S&& self, F&& map_func) {
        if constexpr (std::is_same_v<E, void>) {
            using err_t = decltype(map_func());
            return std::visit(
                [f = std::forward<F>(map_func)](
                    auto&& val) -> result<T, err_t> {
                    using holder_t = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<holder_t, err_holder>) {
                        if constexpr (std::is_same_v<err_t, void>) {
                            f();
                            return crasy::err();
                        } else {
                            return crasy::err(f());
                        }
                    } else {
                        if constexpr (std::is_same_v<T, void>) {
                            return crasy::ok();
                        } else {
                            return crasy::ok(
                                std::forward<detail::remove_rvalue_reference_t<
                                    decltype(val)>>(val)
                                    .get());
                        }
                    }
                },
                std::forward<S>(self).value_);
        } else {
            using err_t = decltype(map_func(std::declval<S&&>().err()));
            return std::visit(
                [f = std::forward<F>(map_func)](
                    auto&& val) -> result<T, err_t> {
                    using holder_t = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<holder_t, err_holder>) {
                        if constexpr (std::is_same_v<err_t, void>) {
                            f(std::forward<detail::remove_rvalue_reference_t<
                                  decltype(val)>>(val)
                                  .get());
                            return crasy::err();
                        } else {
                            return crasy::err(f(
                                std::forward<detail::remove_rvalue_reference_t<
                                    decltype(val)>>(val)
                                    .get()));
                        }
                    } else {
                        if constexpr (std::is_same_v<T, void>) {
                            return crasy::ok();
                        } else {
                            return crasy::ok(
                                std::forward<detail::remove_rvalue_reference_t<
                                    decltype(val)>>(val)
                                    .get());
                        }
                    }
                },
                std::forward<S>(self).value_);
        }
    }

    struct ok_tag {};
    struct err_tag {};

    template <typename U, typename Tag>
    class holder {
      private:
        [[no_unique_address]] U value_;

      public:
        using tag = Tag;

        constexpr holder() = default;
        constexpr holder(const holder&) = default;
        constexpr holder(holder&&) = default;
        constexpr ~holder() = default;
        constexpr holder& operator=(const holder&) = default;
        constexpr holder& operator=(holder&&) = default;

        template <typename... Args>
        explicit constexpr holder([[maybe_unused]] std::in_place_t in_place,
                                  Args&&... args)
            : value_(std::forward<Args>(args)...) {}

        U& get() & { return value_; }
        std::add_const_t<U>& get() const& { return value_; }
        U&& get() && { return std::move(value_); }
        std::add_const_t<U>&& get() const&& {
            return static_cast<std::add_const_t<U>&&>(value_);
        }

        U* ptr() { return &value_; }
        std::add_const_t<U>* ptr() const { return &value_; }
    };

    template <typename U, typename Tag>
    class holder<U&, Tag> {
      private:
        U* value_{nullptr};

      public:
        using tag = Tag;

        constexpr holder() = default;
        constexpr holder(const holder&) = default;
        constexpr holder(holder&&) = default;
        constexpr ~holder() = default;
        constexpr holder& operator=(const holder&) = default;
        constexpr holder& operator=(holder&&) = default;

        template <typename V>
        explicit constexpr holder([[maybe_unused]] std::in_place_t in_place,
                                  V& value)
            : value_(&value) {}

        U& get() const { return *value_; }
        U* ptr() const { return value_; }
    };

    template <typename Tag>
    class holder<void, Tag> {
      public:
        using tag = Tag;

        constexpr holder() = default;
        constexpr holder(const holder&) = default;
        constexpr holder(holder&&) = default;
        constexpr ~holder() = default;
        constexpr holder& operator=(const holder&) = default;
        constexpr holder& operator=(holder&&) = default;

        explicit constexpr holder([[maybe_unused]] std::in_place_t in_place) {}
    };

    using ok_holder = holder<T, ok_tag>;
    using err_holder = holder<E, err_tag>;

    std::variant<ok_holder, err_holder> value_;
};

} // namespace crasy

#endif
