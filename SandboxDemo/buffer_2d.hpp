#pragma once

#include <vector>

template<typename T>
struct Buffer2D
{
	Buffer2D(size_t w = 0, size_t h = 0)
	{
		buffer.resize(w * h);
		this->w = w;
		this->h = h;
	}

	void ReSize(size_t w, size_t h)
	{
		buffer.resize(w * h);
		this->w = w;
		this->h = h;
	}

	void Set(int x, int y, const T& v)
	{
		if (x < 0 || x >= w || y < 0 || y >= h) return;
		size_t index = (size_t)y * w + x;
		buffer[index] = v;
	}

	T Get(int x, int y) const
	{
		if (x < 0 || x >= w || y < 0 || y >= h) return {};
		size_t index = (size_t)y * w + x;
		return buffer[index];
	}

	std::vector<T> buffer;
	size_t w;
	size_t h;
};

template<typename T>
struct Buffer2DView
{
	void ReSize(size_t w, size_t h)
	{
		this->w = w;
		this->h = h;
	}

	void Set(int x, int y, const T& v)
	{
		if (x < 0 || x >= w || y < 0 || y >= h) return;
		size_t index = (size_t)y * w + x;
		buffer[index] = v;
	}

	T Get(int x, int y) const
	{
		if (x < 0 || x >= w || y < 0 || y >= h) return {};
		size_t index = (size_t)y * w + x;
		return buffer[index];
	}

	struct Iter
	{
		T* location;
		bool operator!= (const Iter& other) const noexcept
		{
			return this->location != other.location;
		}
		T& operator* () const
		{
			return *location;
		}
		const Iter& operator++() noexcept
		{
			++location;
			return *this;
		}
	};

	Iter begin() const
	{
		return Iter{ buffer };
	}

	Iter end() const
	{
		return Iter{ buffer + w * h };
	}

	T* buffer;
	size_t w;
	size_t h;
};
