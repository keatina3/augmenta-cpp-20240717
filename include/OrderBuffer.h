#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <new>

// no need to include much of the boiler plate stuff
// as these should be handled by allocator_traits
template <typename T, size_t N>
class Allocator {
  class Arena {
   public:
    Arena(){};
    // TODO: fix this constructor
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena& operator=(Arena&&) = delete;

    void set_object_size(size_t size) {
      if (_object_size < size)
        _object_size = size;
    }

    uint8_t* allocate(size_t n) {
      if (n == 0) {
        return nullptr;
      }
      // the deque logic below won't work for multiple allocs at once
      (void)n;
      uint8_t* p;

      if (_recycling_plant.empty()) {
        // checking if we reached the end of preallocated buffer
        if ((_ptr + _object_size) > (_buffer + N)) {
          p = static_cast<uint8_t*>(::operator new(_object_size));
        } else {
          p = _ptr;
          _ptr += _object_size;
        }
      } else {
        // reusing old mem address
        p = _recycling_plant.front();
        _recycling_plant.pop_front();
      }

      return p;
    }

    // TODO: what about the value n?
    uint8_t* deallocate(uint8_t* p, size_t n) {
      (void)n;

      if (ptr_is_from_buffer(p, 1)) {
        // recycling the mem address for reuse
        _recycling_plant.push_back(p);
      } else {
        ::operator delete(p);
      }
    }

    bool ptr_is_from_buffer(uint8_t* p) {
      return (p >= _buffer) && (p <= _buffer + N);
    }

   private:
    // allocated on the heap
    uint8_t _buffer[N];
    uint8_t* _ptr = _buffer;
    size_t _object_size = 1;
    std::deque<uint8_t*> _recycling_plant;
  };

  using buffer_t = Arena;
  using value_type = T;
  using pointer = T*;

 public:
  Allocator() noexcept { _buffer = std::make_shared<buffer_t>(); };

  template <typename U>
  Allocator(const Allocator<U, N>& other) : _buffer(other._buffer) {}

  pointer allocate(std::size_t n) {
    return reinterpret_cast<pointer>(_buffer.allocate);
  }

  void deallocate(pointer p, size_t n) { ::operator delete(p); }

  // TODO: unsure if I need construct and destroy.
  // come back to check this
  template <typename... Args>
  void construct(pointer p, Args... args) {
    new (p) T(std::forward<Args>(args)...);
  }

  void destroy(pointer p) { new (p) T(); }

  template <typename T1, std::size_t N1>
  constexpr bool operator==(const Allocator<T1, N1>&) noexcept const {
    return (N == N1) && (_buffer == _buffer);
  }

  template <typename T1, std::size_t N1>
  constexpr bool operator!=(const Allocator<T1, N1>&) noexcept const {
    return not(*this == other);
  }

 private:
  std::shared_ptr<buffer_t> _buffer;
};