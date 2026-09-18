#include "../include/types.hpp"
#include <array>
#include <atomic>
#include <concepts>
#include <iostream>
#include <memory>

using std::cout, std::cin;

template<std::integral T>
constexpr bool is_power_of_two(T x) {
    return x > 0 && (x & (x - 1)) == 0;
}

#ifdef __cpp_lib_hardware_interference_size
inline constexpr std::size_t CacheLine = std::hardware_destructive_interference_size;
#else
inline constexpr std::size_t CacheLine = 64;
#endif

/*
 * Invariants:
   - exactly one thread calls the producer API; exactly one calls the consumer API
   - producer_pos_ is monotonically increasing, written only by the producer
   - consumer_pos_ is monotonically increasing, written only by the consumer
   - consumer_pos_ <= producer_pos_ always, so size == producer_pos_ - consumer_pos_
   - slots in [consumer_pos_, producer_pos_) hold constructed objects; all others are
     raw
 */
template<typename T, typename Allocator = std::allocator<T>>
    requires std::is_nothrow_destructible_v<T>
class SpscRingBuffer {
    // Don't want hidden mutex's, breaks class being lock free
    static_assert(std::atomic<usize>::is_always_lock_free);

  public:
    SpscRingBuffer(usize capacity, const Allocator& a = Allocator())
        : capacity_(capacity), alloc_(a), mask_(capacity - 1) {
        if (!is_power_of_two(capacity_))
            throw std::invalid_argument("capacity must be a power of two");
        // Allocate capacity
        ptr_ = std::allocator_traits<Allocator>::allocate(alloc_, capacity_);
    }
    ~SpscRingBuffer() {
        // Destroy everything in the range of
        //[consumer_pos_, producer_pos_)
    }

    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;
    SpscRingBuffer(SpscRingBuffer&&) = delete;
    SpscRingBuffer& operator=(SpscRingBuffer&&) = delete;

    template<typename... Args>
    // Requires that we can construct T from Args...
        requires std::constructible_from<T, Args...>
    void emplace(Args&&... args) {
        // memory_order_relaxed is used since a thread always sees its own writes in program order
        const usize pro_pos = producer_pos_.load(std::memory_order_relaxed);
        while (pro_pos - cached_consumer == capacity_) {
            consumer_pos_.wait(cached_consumer, std::memory_order_acquire);
            cached_consumer = consumer_pos_.load(std::memory_order_acquire);
        }
        std::allocator_traits<Allocator>::construct(alloc_, std::to_address(get_index(pro_pos)),
                                                    std::forward<Args>(args)...);
        producer_pos_.store(pro_pos + 1, std::memory_order_release);
        producer_pos_.notify_one();
    }

    template<typename... Args>
        requires std::constructible_from<T, Args...>
    bool try_emplace(Args&&... args) {
        const usize pro_pos = producer_pos_.load(std::memory_order_relaxed);
        if (pro_pos - cached_consumer == capacity_) {
            cached_consumer = consumer_pos_.load(std::memory_order_acquire);
            if (pro_pos - cached_consumer == capacity_)
                return false;
        }
        std::allocator_traits<Allocator>::construct(alloc_, std::to_address(get_index(pro_pos)),
                                                    std::forward<Args>(args)...);
        producer_pos_.store(pro_pos + 1, std::memory_order_release);
        producer_pos_.notify_one(); // Notifies a waiting thread
        return true;
    }

    bool try_push(const T& v) {}
    bool try_push(T&& v) {}

    bool try_pop(T& out) {}
    std::optional<T> try_pop() {}

    usize size() const {}
    bool empty() const {}
    bool full() const {}

    static constexpr usize capacity() {}

  private:
    /*
     We use [[no_unique_address]] since allocators are commonly empty
     and the attirbute lets the empty case cost zero bytes while the
     other is unaffected
    */
    [[no_unique_address]] Allocator alloc_;
    // Raw storage for capacity_T and points to first slot
    std::allocator_traits<Allocator>::pointer ptr_;
    usize mask_;
    usize capacity_;
    // Consumer and Producer
    alignas(CacheLine) std::atomic<usize> producer_pos_{};
    alignas(CacheLine) usize cached_consumer{};
    alignas(CacheLine) std::atomic<usize> consumer_pos_{};
    alignas(CacheLine) usize cached_producer{};

    std::allocator_traits<Allocator>::pointer get_index(usize n) {
        return ptr_ + (n & mask_);
    }
};

int main() {
    return 0;
}
