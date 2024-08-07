#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <new>

template <size_t N>
class Arena {
 public:
  Arena(){};
  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;
  Arena& operator=(Arena&&) = delete;

  void set_object_size(size_t size) {
    if (_object_size < size)
      _object_size = size;
  }

  char* allocate(size_t n) {
    if (n == 0) {
      return nullptr;
    }
    // the deque logic below won't work for multiple allocs at once
    (void)n;
    char* p;

    if (_recycling_plant.empty()) {
      // checking if we reached the end of preallocated buffer
      if ((_ptr + _object_size) > (_buffer + N)) {
        p = static_cast<char*>(::operator new(_object_size));
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

  void deallocate(char* p, size_t n) {
    (void)n;

    if (ptr_is_from_buffer(p)) {
      // recycling the mem address for reuse
      _recycling_plant.push_back(p);
    } else {
      // can't reuse dynamically allocated
      ::operator delete(p);
    }
  }

  bool ptr_is_from_buffer(char* p) {
    return (p >= _buffer) && (p <= _buffer + N);
  }

 private:
  // allocated on the heap
  char _buffer[N];
  char* _ptr = _buffer;
  size_t _object_size = 1;
  std::deque<char*> _recycling_plant = {};
};

template <typename T, size_t N>
class Allocator {
 public:
  using buffer_t = Arena<N>;
  using value_type = T;
  using pointer = T*;
  using size_type = size_t;

  Allocator() noexcept {
    if (_buffer == nullptr) {
      _buffer = std::make_shared<buffer_t>();
    }
    this->_buffer->set_object_size(sizeof(T));
  };

  template <typename U>
  Allocator(const Allocator<U, N>& other) : _buffer(other._buffer){};

  void set_object_size(size_t n) { _buffer->set_object_size(n); }

  template <typename T1>
  struct rebind {
    typedef Allocator<T1, N> other;
  };

  pointer allocate(std::size_t n) {
    return reinterpret_cast<pointer>(_buffer->allocate(n));
  }

  void deallocate(pointer p, size_t n) {
    _buffer->deallocate(reinterpret_cast<char*>(p), n);
  }

  template <typename T1, std::size_t N1>
  bool operator==(const Allocator<T1, N1>& other) const {
    return (N == N1) && (_buffer == other._buffer);
  }

  template <typename T1, std::size_t N1>
  bool operator!=(const Allocator<T1, N1>& other) const {
    return not(*this == other);
  }

  template <typename T1, size_t N1>
  friend class Allocator;

 private:
  std::shared_ptr<buffer_t> _buffer;
};