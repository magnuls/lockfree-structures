# lock-free-ds

Lock-free data structures 

## Build and test

```
cmake -S . -B build                                   # normal build
cmake -S . -B build-tsan -DLFDS_SANITIZER=thread      # data race errors
cmake -S . -B build-asan -DLFDS_SANITIZER=address     # memory errors
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests are in `tests/test_*.cpp`

## SPSC ring buffer

SPSC capacity is a power of 2

When capacity c = 2^k, pos % c equals pos & (c - 1). With a runtime capacity, % compiles to divide instruction
~20-40+ cycles, while the & takes 1 cycle.
The positions are unsigned n bit integers that wrap from 2^n back to 0.
The slot mapping stays continuous across the wrap only if 2^n mod c = 0, meaning c divides 2^n
The only divisors of 2^n are powers of two, so c must be a power of two.

`explicit SpscRingBuffer<T, Allocator = std::allocator<T>>(usize capacity, const Allocator& a = Allocator());`  
Allocates storage for capacity items. Throws std::invalid_argument if capacity is not a power of two.
Requires std::is_nothrow_destructible<T>::value == true. Not copyable or movable.

`void emplace(Args&&... args);`  
Enqueue an item using inplace construction. Blocks if queue is full.

`bool try_emplace(Args&&... args);`  
Try to enqueue an item using inplace construction. Returns true on success and false if queue is full.

`void push(const T& v);`  
Enqueue an item using copy construction. Blocks if queue is full.

`void push(T&& v);`  
Enqueue an item using move construction. Blocks if queue is full.

`bool try_push(const T& v);`  
Try to enqueue an item using copy construction. Returns true on success and false if queue is full.

`bool try_push(T&& v);`  
Try to enqueue an item using move construction. Returns true on success and false if queue is full.

`void pop(T& out);`  
Dequeue first item of queue by moving it into out. Blocks if queue is empty.
Requires std::is_move_assignable<T>::value == true.

`void pop();`  
Dequeue and discard first item of queue. Blocks if queue is empty.

`bool try_pop(T& out);`  
Try to dequeue first item of queue by moving it into out. Returns true on success and false if queue is empty.
Requires std::is_move_assignable<T>::value == true.

`bool try_pop();`  
Try to dequeue and discard first item of queue. Returns true on success and false if queue is empty.

`usize size() const;`  
Return the number of items available in the queue.

`bool empty() const;`  
Return true if queue is currently empty.

`bool full() const;`  
Return true if queue is currently full.

`usize capacity() const;`  
Return the capacity of the queue.

Only a single writer thread can perform enqueue operations and only a single reader thread can perform dequeue operations.
Any other usage is invalid.
