// Asserts must fire even in RelWithDebInfo / Release, which define
// NDEBUG.
#undef NDEBUG
#include <cassert>

#include <lfds/spsc_ring_buffer.hpp>

#include <stdexcept>
#include <thread>

//  compile time checks

static_assert(std::atomic<usize>::is_always_lock_free);

static_assert(!std::is_copy_constructible_v<SpscRingBuffer<int>>);
static_assert(!std::is_copy_assignable_v<SpscRingBuffer<int>>);
static_assert(!std::is_move_constructible_v<SpscRingBuffer<int>>);
static_assert(!std::is_move_assignable_v<SpscRingBuffer<int>>);

static_assert(is_power_of_two(1));
static_assert(is_power_of_two(64));
static_assert(!is_power_of_two(0));
static_assert(!is_power_of_two(6));

// single thread runtime checks

static void test_rejects_non_power_of_two() {
    bool threw = false;
    try {
        SpscRingBuffer<int> q(6);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

static void test_fresh_queue() {
    SpscRingBuffer<int> q(8);
    assert(q.capacity() == 8);
    assert(q.size() == 0);
    assert(q.empty());
    assert(!q.full());
}

static void test_fill_then_drain_fifo() {
    SpscRingBuffer<int> q(8);
    for (int i = 0; i < 8; ++i) {
        assert(q.try_push(i));
    }
    assert(q.full());
    assert(q.size() == 8);
    assert(!q.try_push(99)); // full and must refulse

    for (int i = 0; i < 8; ++i) {
        int out = -1;
        assert(q.try_pop(out));
        assert(out == i);
    }
    assert(q.empty());
    int out = -1;
    assert(!q.try_pop(out)); // empty and must refuse
    assert(out == -1);       // and must not touch out
}

static void test_discard_pop() {
    SpscRingBuffer<int> q(4);
    q.push(1);
    q.push(2);
    assert(q.size() == 2);
    assert(q.try_pop()); // discards 1
    assert(q.size() == 1);
    int out = 0;
    assert(q.try_pop(out));
    assert(out == 2);
    assert(!q.try_pop());
}

static void test_wraparound() {
    // push and pop many more items than capacity so the positions
    // wrap around the buffer several times order must survive
    SpscRingBuffer<int> q(4);
    int next_in = 0;
    int next_out = 0;
    for (int round = 0; round < 25; ++round) {
        q.push(next_in++);
        q.push(next_in++);
        q.push(next_in++);
        int out = -1;
        assert(q.try_pop(out));
        assert(out == next_out++);
        assert(q.try_pop(out));
        assert(out == next_out++);
        assert(q.try_pop(out));
        assert(out == next_out++);
    }
    assert(q.empty());
}

static void test_emplace_and_move_only() {
    struct MoveOnly {
        int v;
        explicit MoveOnly(int x) : v(x) {}
        MoveOnly(MoveOnly&&) = default;
        MoveOnly& operator=(MoveOnly&&) = default;
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly& operator=(const MoveOnly&) = delete;
    };
    SpscRingBuffer<MoveOnly> q(2);
    q.emplace(7);
    assert(q.try_push(MoveOnly{8}));
    assert(!q.try_emplace(9)); // full
    MoveOnly out{0};
    q.pop(out);
    assert(out.v == 7);
    assert(q.try_pop(out));
    assert(out.v == 8);
    assert(q.empty());
}

static void test_destroy_while_non_empty() {
    // Destructor must destroy live elements and release storage
    // Leaks double-destroys are caught by the ASAN build
    SpscRingBuffer<int> q(4);
    q.push(1);
    q.push(2);
    q.push(3);
}

// two threads runtime check

static void test_producer_consumer_threads() {
    constexpr int N = 100000;
    SpscRingBuffer<int> q(16);

    std::thread producer([&] {
        for (int i = 0; i < N; ++i) {
            q.push(i); // blocks when full
        }
    });

    for (int expected = 0; expected < N; ++expected) {
        int got = -1;
        q.pop(got); // blocks when empty
        assert(got == expected);
    }
    producer.join();
    assert(q.empty());
}

int main() {
    test_rejects_non_power_of_two();
    test_fresh_queue();
    test_fill_then_drain_fifo();
    test_discard_pop();
    test_wraparound();
    test_emplace_and_move_only();
    test_destroy_while_non_empty();
    test_producer_consumer_threads();
    return 0;
}
