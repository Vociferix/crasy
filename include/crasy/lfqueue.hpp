#ifndef CRASY_LFQUEUE_HPP
#define CRASY_LFQUEUE_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <atomic>

#include <crasy/option.hpp>

namespace crasy {

template <typename T>
class lfqueue {
  private:
    template <typename U, typename = void>
    struct data_t {
        union {
            U data;
        };
        std::atomic<bool> has_value;

        data_t() : has_value(false) {}

        ~data_t() {
            if constexpr (!std::is_trivially_destructible_v<U>) {
                if (has_value.load()) { data.~U(); }
            }
        }

        template <typename... Args>
        void put_data(Args&&... args) {
            if constexpr (!std::is_trivially_destructible_v<U>) {
                if (has_value.load()) { data.~U(); }
            }
            new (&data) U(std::forward<Args>(args)...);
            has_value.store(true);
        }

        option<U> take_data() {
            if (has_value.exchange(false)) {
                if constexpr (!std::is_trivially_destructible_v<U>) {
                    auto ret = std::move(data);
                    data.~U();
                    return ret;
                } else {
                    return std::move(data);
                }
            } else {
                return std::nullopt;
            }
        }
    };

    template <typename U>
    struct data_t<U&, void> {
        std::atomic<U*> data_{nullptr};

        void put_data(U& value) {
            data_.store(&value, std::memory_order_relaxed);
        }

        option<U&> take_data() {
            return data_.exchange(nullptr, std::memory_order_relaxed);
        }
    };

    struct entry_t {
        data_t<T> data;
        std::atomic<entry_t*> next{nullptr};
    };

    std::atomic<entry_t*> head_{nullptr};
    std::atomic<entry_t*> tail_{nullptr};
    std::atomic<entry_t*> free_{nullptr};

    void del_entry(entry_t* entry) {
        auto next = free_.load(std::memory_order_relaxed);
        do {
            entry->next.store(next, std::memory_order_relaxed);
        } while (!free_.compare_exchange_strong(next, entry,
                                                std::memory_order_relaxed));
    }

    entry_t* new_entry() {
        entry_t* next = nullptr;
        auto entry = free_.load(std::memory_order_relaxed);
        do {
            if (entry == nullptr) { return new entry_t; }
            next = entry->next.load(std::memory_order_relaxed);
        } while (!free_.compare_exchange_strong(entry, next,
                                                std::memory_order_relaxed));
        entry->next.store(nullptr, std::memory_order_relaxed);
        return entry;
    }

  public:
    lfqueue() = default;

    explicit lfqueue(std::size_t initial_capacity) {
        if (initial_capacity > 0) {
            auto entry = new entry_t;
            --initial_capacity;
            for (std::size_t i = 0; i < initial_capacity; ++i) {
                auto tmp = new entry_t;
                tmp->next.store(entry, std::memory_order_relaxed);
                entry = tmp;
            }
            free_.store(entry, std::memory_order_relaxed);
        }
    }

    ~lfqueue() {
        auto entry = tail_.load(std::memory_order_relaxed);
        while (entry != nullptr) {
            auto next = entry->next.load(std::memory_order_relaxed);
            delete entry;
            entry = next;
        }
        entry = free_.load(std::memory_order_relaxed);
        while (entry != nullptr) {
            auto next = entry->next.load(std::memory_order_relaxed);
            delete entry;
            entry = next;
        }
    }

    lfqueue(const lfqueue&) = delete;
    lfqueue(lfqueue&&) = delete;
    lfqueue& operator=(const lfqueue&) = delete;
    lfqueue& operator=(lfqueue&&) = delete;

    template <typename... Args>
    void push(Args&&... args) {
        auto entry = new_entry();
        entry->data.put_data(std::forward<Args>(args)...);
        auto head = head_.exchange(entry, std::memory_order_acquire);
        if (head == nullptr) {
            tail_.store(entry, std::memory_order_release);
        } else {
            head->next.store(entry, std::memory_order_release);
        }
    }

    option<T> pop() {
        entry_t* tail;
        entry_t* next;
        do {
            tail = tail_.load(std::memory_order_acquire);
            if (tail == nullptr) { return std::nullopt; }
            next = tail->next.load(std::memory_order_relaxed);
        } while (!tail_.compare_exchange_strong(tail, next,
                                                std::memory_order_release));
        auto ret = tail->data.take_data();
        if (next == nullptr) {
            tail_.store(tail, std::memory_order_relaxed);
        } else {
            del_entry(tail);
        }
        return ret;
    }
};

} // namespace crasy

#endif
