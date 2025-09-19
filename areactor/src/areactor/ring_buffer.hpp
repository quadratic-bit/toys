#pragma once
#include <cassert>
#include <cstddef>
#include <cstdlib>

template <typename T>
class RingBuffer {
	T* _data;
	size_t _head;
public:
	size_t capacity, size;

	RingBuffer() : _data(0), _head(0), capacity(0), size(0) {}
	explicit RingBuffer(size_t cap) : _head(0), capacity(cap), size(0) {
		_data = (T*)std::malloc(sizeof(T) * capacity);
	}

	~RingBuffer() {
		std::free(_data);
	}

	void push(const T &v) {
		if (capacity == 0) return;
		_data[_head] = v;
		_head = (_head + 1) % capacity;
		if (size < capacity) ++size;
	}

	const T &at(size_t i) const {
		size_t idx = (_head + capacity - size + i) % capacity;
		return _data[idx];
	}

	T mean(int n) const {
		if ((size_t)n > size) n = size;

		T total = 0;
		for (int i = (int)size - 1; i >= (int)size - n; --i) {
			total += at(i);
		}
		return total / (T)n;
	}

	bool full() const {
		return size == capacity;
	}
};
