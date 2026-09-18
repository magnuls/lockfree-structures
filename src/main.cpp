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
template<typename T, usize N>
class SpscRingBuffer {
    static_assert(is_power_of_two(N));
    static_assert(std::atomic<usize>::is_always_lock_free);

  public:
    SpscRingBuffer() = default;
    ~SpscRingBuffer() {
        // Destroy everything in the range of
        //[consumer_pos_, producer_pos_)
    }

    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;
    SpscRingBuffer(SpscRingBuffer&&) = delete;
    SpscRingBuffer& operator=(SpscRingBuffer&&) = delete;

    template<typename... Args>
    bool try_emplace(Args&&... args) {}

    bool try_push(const T& v) {}
    bool try_push(T&& v) {}

    bool try_pop(T& out) {}
    std::optional<T> try_pop() {}

  private:
    T* slot(usize pos) {}

    alignas(T) std::byte storage_[N * sizeof(T)];

    struct alignas(CacheLine) {
        std::atomic<usize> producer_pos_{};
        usize cached_consumer{};
    };
    struct alignas(CacheLine) {
        std::atomic<usize> consumer_pos_{};
        usize cached_producer{};
    };
};

int main() {

    return 0;
}
