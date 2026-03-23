# FixedSizePool

A lightweight, high-performance C++ implementation of a fixed-size memory pool. This allocator is designed for scenarios requiring frequent allocation and deallocation of objects of the same size, such as particles in a game engine or nodes in a data structure.

## Features

- **$O(1)$ Performance**: Constant time complexity for both `allocate` and `deallocate` operations.
- **Low Fragmentation**: Reduces heap fragmentation by allocating large "pages" of memory at once.
- **Cache Friendly**: Pre-allocating contiguous blocks improves CPU cache hit rates.
- **Easy Integration**: Can be easily integrated into existing classes by overloading `operator new` and `operator delete`.

## Performance

In benchmarks involving 100,000 objects, this pool typically outperforms the default system allocator (`malloc`/`free`) by significantly reducing the overhead of system calls.

## Quick Start

### 1. Build the Project

This project uses CMake. To build the demo:

```bash
mkdir build
cd build
cmake ..
make
./memoryPool
```

### 2. Basic Usage

You can use the pool as a standalone object or integrate it into a class to manage its memory:

```cpp
#include "FixedSizePool.h"

struct Particle {
    float x, y, z;
    
    // Define a static pool for this class
    static FixedSizePool pool;

    static void* operator new(std::size_t n) {
        return pool.allocate();
    }
    static void operator delete(void* p) {
        pool.deallocate(p);
    }
};

// Initialize: block size, blocks per page
FixedSizePool Particle::pool(sizeof(Particle), 4096);

int main() {
    Particle* p = new Particle(); // Uses memory pool
    delete p;                     // Returns memory to pool
    return 0;
}
```

## How It Works

1.  **Pages**: The pool requests large memory chunks (pages) from the system.
2.  **Blocks**: Each page is sliced into fixed-size blocks aligned to memory boundaries.
3.  **Free List**: A linked list of "free blocks" is maintained using embedded pointers.
4.  **Expansion**: If the free list is empty, the pool automatically allocates a new page to provide more blocks.

## Requirements

- **C++ Standard**: C++17 or higher.
- **Build System**: CMake 3.10+.

