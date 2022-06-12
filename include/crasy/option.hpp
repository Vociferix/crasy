#ifndef CRASY_OPTION_HPP
#define CRASY_OPTION_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/detail.hpp>
#include <optional>

namespace crasy {

using nullopt_t = std::nullopt_t;

inline constexpr nullopt_t nullopt = std::nullopt;

template <typename T>
class option {
  private:
    std::optional<T> opt_;

    template <typename O, typename F>
    static constexpr auto map(O&& opt, F&& map_func) {
        using ret_t = decltype(map_func(*std::forward<O>(opt)));
        option<detail::remove_rvalue_reference_t<ret_t>> ret;
        if (opt.has_value()) { ret.emplace(map_func(*std::forward<O>(opt))); }
        return ret;
    }

  public:
    using value_type = T;
    using reference = T&;
    using const_reference = std::add_const_t<T>&;
    using rvalue_reference = T&&;
    using const_rvalue_reference = std::add_const_t<T>&&;
    using pointer = T*;
    using const_pointer = std::add_const_t<T>*;

    constexpr option() = default;
    constexpr option(const option&) = default;
    constexpr option(option&&) = default;
    constexpr ~option() = default;
    constexpr option& operator=(const option&) = default;
    constexpr option& operator=(option&&) = default;

    constexpr option([[maybe_unused]] std::nullopt_t null) : option() {}

    template <typename U>
    constexpr explicit option(std::optional<U> opt) : opt_(std::move(opt)) {}

    template <std::convertible_to<T> U>
    constexpr option(U&& value) : opt_(std::forward<U>(value)) {}

    template <typename... Args>
    constexpr explicit(sizeof...(Args) == 0)
        option(std::in_place_t in_place, Args&&... args)
        : opt_(in_place, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr option(std::in_place_t in_place,
                     std::initializer_list<U> ilist,
                     Args&&... args)
        : opt_(in_place, ilist, std::forward<Args>(args)...) {}

    constexpr operator bool() const { return opt_.has_value(); }

    constexpr bool has_value() const { return opt_.has_value(); }

    constexpr reference operator*() & noexcept { return *opt_; }

    constexpr const_reference operator*() const& noexcept { return *opt_; }

    constexpr rvalue_reference operator*() && { return *std::move(opt_); }

    constexpr const_rvalue_reference operator*() const&& {
        return *static_cast<const std::optional<T>&&>(opt_);
    }

    constexpr pointer operator->() noexcept { return opt_.operator->(); }

    constexpr const_pointer operator->() const noexcept {
        return opt_.operator->();
    }

    constexpr reference value() & { return opt_.value(); }

    constexpr const_reference value() const& { return opt_.value(); }

    constexpr rvalue_reference value() && { return std::move(opt_).value(); }

    constexpr const_rvalue_reference value() const&& {
        return static_cast<const std::optional<T>&&>(opt_).value();
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) const& {
        return opt_.value_or(std::forward<U>(default_value));
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) && {
        return std::move(opt_).value_or(std::forward<U>(default_value));
    }

    constexpr void swap(option& other) { opt_.swap(other.opt_); }

    constexpr void reset() { opt_.reset(); }

    template <typename... Args>
    constexpr T& emplace(Args&&... args) {
        return opt_.emplace(std::forward<Args>(args)...);
    }

    template <typename U, typename... Args>
    constexpr T& emplace(std::initializer_list<U> ilist, Args&&... args) {
        return opt_.emplace(ilist, std::forward<Args>(args)...);
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
        return map(static_cast<const option&&>(*this),
                   std::forward<F>(map_func));
    }

    template <typename U>
    constexpr bool operator==(const option<U>& rhs) const {
        return opt_ == rhs.opt_;
    }

    template <typename U>
    constexpr bool operator!=(const option<U>& rhs) const {
        return opt_ != rhs.opt_;
    }

    template <typename U>
    constexpr auto operator<=>(const option<U>& rhs) const {
        return opt_ <=> rhs.opt_;
    }

    friend constexpr bool operator==(const option& lhs, std::nullopt_t rhs) {
        return lhs.opt_ == rhs;
    }

    friend constexpr bool operator==(std::nullopt_t lhs, const option& rhs) {
        return lhs == rhs.opt_;
    }

    friend constexpr bool operator!=(const option& lhs, std::nullopt_t rhs) {
        return lhs.opt_ != rhs;
    }

    friend constexpr bool operator!=(std::nullopt_t lhs, const option& rhs) {
        return lhs != rhs.opt_;
    }

    template <typename U>
    constexpr operator std::optional<U>() const& {
        return std::optional<U>(opt_);
    }

    template <typename U>
    constexpr operator std::optional<U>() && {
        return std::optional<U>(std::move(opt_));
    }
};

template <typename T>
class option<T&> {
  private:
    T* opt_{nullptr};

    template <typename O, typename F>
    static constexpr auto map(O&& opt, F&& map_func) {
        using ret_t = decltype(map_func(*std::forward<O>(opt)));
        option<detail::remove_rvalue_reference_t<ret_t>> ret;
        if (opt.has_value()) { opt.emplace(*std::forward<O>(opt)); }
        return ret;
    }

  public:
    using value_type = T&;
    using reference = T&;
    using const_reference = T&;
    using rvalue_reference = T&;
    using const_rvalue_reference = T&;
    using pointer = T*;
    using const_pointer = T*;

    constexpr option() = default;
    constexpr option(const option&) = default;
    constexpr option(option&&) = default;
    constexpr ~option() = default;
    constexpr option& operator=(const option&) = default;
    constexpr option& operator=(option&&) = default;

    constexpr option([[maybe_unused]] std::nullopt_t null) : option() {}

    constexpr option([[maybe_unused]] std::nullptr_t null) : option() {}

    constexpr option(reference value) : opt_(&value) {}

    template <std::derived_from<T> U>
    constexpr option(const option<U>& other) : opt_(static_cast<U*>(other)) {}

    constexpr option(pointer value) : opt_(value) {}

    constexpr option([[maybe_unused]] std::in_place_t in_place, reference value)
        : opt_(&value) {}

    constexpr operator bool() const { return opt_ != nullptr; }

    constexpr bool has_value() const { return opt_ != nullptr; }

    constexpr operator T*() const { return opt_; }

    constexpr const_reference operator*() const noexcept { return *opt_; }

    constexpr const_pointer operator->() const noexcept { return opt_; }

    constexpr const_reference value() const {
        if (opt_ == nullptr) { throw std::bad_optional_access(); }
        return *opt_;
    }

    constexpr value_type value_or(
        rvalue_reference default_value) const noexcept {
        if (opt_ == nullptr) {
            return default_value;
        } else {
            return *opt_;
        }
    }

    constexpr T& emplace(reference value) {
        opt_ = &value;
        return *opt_;
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
        return map(static_cast<const option&&>(*this),
                   std::forward<F>(map_func));
    }

    constexpr void reset() { opt_ = nullptr; }

    constexpr void swap(option& other) { std::swap(opt_, other.opt_); }

    friend constexpr bool operator==(option lhs, option rhs) {
        return lhs.opt_ == rhs.opt_;
    }

    friend constexpr bool operator!=(option lhs, option rhs) {
        return lhs.opt_ != rhs.opt_;
    }

    friend constexpr bool operator==(option lhs,
                                     [[maybe_unused]] std::nullopt_t rhs) {
        return lhs.opt_ == nullptr;
    }

    friend constexpr bool operator==([[maybe_unused]] std::nullopt_t lhs,
                                     option rhs) {
        return nullptr == rhs.opt_;
    }

    friend constexpr bool operator!=(option lhs,
                                     [[maybe_unused]] std::nullopt_t rhs) {
        return lhs.opt_ != nullptr;
    }

    friend constexpr bool operator!=([[maybe_unused]] std::nullopt_t lhs,
                                     option rhs) {
        return nullptr != rhs.opt_;
    }

    friend constexpr bool operator==(option lhs, std::nullptr_t rhs) {
        return lhs.opt_ == rhs;
    }

    friend constexpr bool operator==([[maybe_unused]] std::nullptr_t lhs,
                                     option rhs) {
        return lhs == rhs.opt_;
    }

    friend constexpr bool operator!=(option lhs, std::nullptr_t rhs) {
        return lhs.opt_ != rhs;
    }

    friend constexpr bool operator!=([[maybe_unused]] std::nullptr_t lhs,
                                     option rhs) {
        return lhs != rhs.opt_;
    }
};

template <typename T, typename... Args>
constexpr option<T> make_option(Args&&... args) {
    return option<T>(std::in_place, std::forward<Args>(args)...);
}

template <typename T, typename U>
requires std::is_lvalue_reference_v<T> && std::is_lvalue_reference_v<U> &&
    requires(typename option<T>::pointer to, typename option<U>::pointer from) {
    to = dynamic_cast<typename option<T>::pointer>(from);
}
constexpr option<T> dynamic_option_cast(option<U> opt) {
    return option<T>(dynamic_cast<typename option<T>::pointer>(
        static_cast<typename option<U>::pointer>(opt)));
}

} // namespace crasy

#endif
