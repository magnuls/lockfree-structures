#ifndef TREIBER_STACK_HPP
#define TREIBER_STACK_HPP

#include <types.hpp>

#include <algorithm>
#include <atomic>
#include <concepts>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

/*
 * we will update the treiber stack once we introduce the hazard
 * Pointer
 */

#ifdef __cpp_lib_hardware_interference_size
inline constexpr std::size_t CacheLine =
    std::hardware_destructive_interference_size;
#else
inline constexpr std::size_t CacheLine = 64;
#endif

template<typename T, typename Allocator = std::allocator<T>>
    requires std::is_nothrow_destructible_v<T>
class TreiberStack {
  public:
    struct Node {
        Node* next;
        T value;
    };
    TreiberStack() = default;
    TreiberStack(const TreiberStack&) = delete;
    TreiberStack& operator=(const TreiberStack&) = delete;

    // push pre allocated Node
    void push(Node* node) {
        node->next = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(
            node->next, node, std::memory_order_release,
            std::memory_order_relaxed)) {
        }
    }
    // Pop returns node or nullptr
    Node* pop() {
        Node* h = head_.load(std::memory_order_relaxed);
        while (h && h->next, std::memory_order_acquire,
               std::memory_order_relaxed) {
        }
        return h;
    }
    bool empty() const {
        return head_.load(std::memory_order_relaxed) == nullptr;
    }

  private:
    std::atomic<Node*> head_{nullptr};
};

#endif
