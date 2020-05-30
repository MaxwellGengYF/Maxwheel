#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <type_traits>
#include <algorithm>

using uint64 = uint64_t;
template <typename T>
class ArrayList
{

private:

	T* arr;
	uint64 mSize;
	uint64 mCapacity;
public:
	void reserve(uint64 newCapacity) noexcept
	{
		if (newCapacity <= mCapacity) return;
		T* newArr = (T*)malloc(newCapacity * sizeof(T));
		memcpy(newArr, arr, sizeof(T) * mSize);
		if(arr) free(arr);
		mCapacity = newCapacity;
		arr = newArr;
	}
	T* data() const noexcept { return arr; }
	uint64 size() const noexcept { return mSize; }
	uint64 capacity() const noexcept { return mCapacity; }
	struct Iterator
	{
		friend class ArrayList<T>;
	private:
		const ArrayList<T>* lst;
		uint64 index;
		Iterator(const ArrayList<T>* lst, uint64 index) noexcept : lst(lst), index(index) {}
	public:
		bool operator==(const Iterator& ite) const noexcept
		{
			return index == ite.index;
		}
		bool operator!=(const Iterator& ite) const noexcept
		{
			return index != ite.index;
		}
		void operator++() noexcept
		{
			index++;
		}
		void operator++(int) noexcept
		{
			index++;
		}
		Iterator operator+(uint64 value) const noexcept
		{
			return Iterator(lst, index + value);
		}
		Iterator operator-(uint64 value) const noexcept
		{
			return Iterator(lst, index - value);
		}
		Iterator& operator+=(uint64 value) noexcept
		{
			index += value;
			return *this;
		}
		Iterator& operator-=(uint64 value) noexcept
		{
			index -= value;
			return *this;
		}
		T* operator->() const noexcept
		{
#if defined(DEBUG) || defined(_DEBUG)
			if (index >= lst->mSize) throw "Out of Range!";
#endif
			return &(*lst).arr[index];
		}
		T& operator*() const noexcept
		{
#if defined(DEBUG) || defined(_DEBUG)
			if (index >= lst->mSize) throw "Out of Range!";
#endif
			return (*lst).arr[index];
		}

	};
	ArrayList(uint64 mSize) noexcept : mSize(mSize), mCapacity(mCapacity)
	{
		arr = (T*)malloc(sizeof(T) * mCapacity);
	}
	ArrayList() noexcept : mCapacity(0), mSize(0), arr(nullptr)
	{
		
	}
	~ArrayList() noexcept
	{
		if (arr) free(arr);
	}
	bool empty() const noexcept
	{
		return mSize == 0;
	}

	void push_back(const T& value) noexcept
	{
		if (mSize >= mCapacity)
		{
			uint64 newCapacity = mCapacity * 1.5;
			if (newCapacity < mCapacity + 1) newCapacity = mCapacity + 1;
			reserve(newCapacity);
		}
		arr[mSize] = value;
		mSize++;
	}

	void push_back_all(const T* values, uint64 count)
	{
		if (mSize + count > mCapacity)
		{
			uint64 newCapacity = mCapacity * 1.5;
			newCapacity = std::max({
				mCapacity + 1, newCapacity, count + mSize });
			reserve(newCapacity);
		}
		memcpy(arr + mSize, values, count * sizeof(T));
		mSize += count;
	}
	template <typename Func>
	void push_back_with(const Func& functor, uint64 count)
	{
		if (mSize + count > mCapacity)
		{
			uint64 newCapacity = mCapacity * 1.5;
			newCapacity = std::max({
				mCapacity + 1, newCapacity, count + mSize });
			reserve(newCapacity);
		}
		auto ptr = arr + mSize;
		for (uint64 i = 0; i < count; ++i)
		{
			ptr[i] = functor(i);
		}
		mSize += count;
	}

	void push_back_all(const std::initializer_list<T>& list)
	{
		push_back_all(list.begin(), list.size());
	}

	Iterator begin() const noexcept
	{
		return Iterator(this, 0);
	}
	Iterator end() const noexcept
	{
		return Iterator(this, mSize);
	}
	void erase(const Iterator& ite) noexcept
	{
#if defined(DEBUG) || defined(_DEBUG)
		if (ite.index >= mSize) throw "Out of Range!";
#endif
		if (ite.index < mSize - 1)
		{
			memmove(arr + ite.index, arr + ite.index + 1, (mSize - ite.index - 1) * sizeof(T));
		}
		mSize--;
	}
	void clear() noexcept
	{
		mSize = 0;
	}
	void resize(uint64 newSize) noexcept
	{
		if (newSize > mCapacity)
		{
			reserve(newSize);
		}
		mSize = newSize;
	}
	T& operator[](uint64 index) noexcept
	{
#if defined(DEBUG) || defined(_DEBUG)
		if (index >= mSize) throw "Out of Range!";
#endif
		return arr[index];
	}
	const T& operator[](uint64 index) const noexcept
	{
#if defined(DEBUG) || defined(_DEBUG)
		if (index >= mSize) throw "Out of Range!";
#endif
		return arr[index];
	}
};