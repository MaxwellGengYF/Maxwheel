#pragma once
#include <type_traits>
#include <stdint.h>
#include <typeinfo>
#include <unordered_map>
using UINT = uint32_t;
template <typename T, UINT size>
class Storage
{
	alignas(T) char c[size * sizeof(T)];
};
template <typename T>
class Storage<T, 0>
{};

template <typename T, bool autoDispose = false>
class StackObject;

template <typename T>
class StackObject<T, false>
{
private:
	alignas(T) char storage[sizeof(T)];
public:
	template <typename... Args>
	constexpr void New(Args&&... args) noexcept
	{
		new (storage)T(std::forward<Args>(args)...);
	}
	template <typename... Args>
	constexpr void InPlaceNew(Args&&... args) noexcept
	{
		new (storage)T{ std::forward<Args>(args)... };
	}
	constexpr void operator=(const StackObject<T>& value)
	{
		*(T*)storage = value.operator*();
	}
	constexpr void operator=(StackObject<T>&& value)
	{
		*(T*)storage = std::move(*value);
	}
	constexpr void Delete() noexcept
	{
		((T*)storage)->~T();
	}
	constexpr T& operator*() const noexcept
	{
		return *(T*)storage;
	}
	constexpr T* operator->() const noexcept
	{
		return (T*)storage;
	}
	constexpr T* GetPtr() const noexcept
	{
		return (T*)storage;
	}
	constexpr operator T* () const noexcept
	{
		return (T*)storage;
	}
	bool operator==(const StackObject<T>&) const = delete;
	bool operator!=(const StackObject<T>&) const = delete;
	StackObject() {}
	StackObject(const StackObject<T>& value)
	{
		New(value.operator*());
	}
};

template <typename T>
class StackObject<T, true>
{
private:
	StackObject<T, false> stackObj;
	bool initialized = false;
public:
	template <typename... Args>
	constexpr void New(Args&&... args) noexcept
	{
		if (initialized) return;
		stackObj.New(std::forward<Args>(args)...);
		initialized = true;
	}
	template <typename... Args>
	constexpr void InPlaceNew(Args&&... args) noexcept
	{
		if (initialized) return;
		stackObj.InPlaceNew(std::forward<Args>(args)...);
		initialized = true;
	}

	constexpr operator bool() const noexcept
	{
		return initialized;
	}

	constexpr void Delete() noexcept
	{
		if (initialized)
		{
			initialized = false;
			stackObj.Delete();
		}
	}

	constexpr void operator=(const StackObject<T, true>& value) noexcept
	{
		if (initialized)
		{
			stackObj.Delete();
		}
		initialized = value.initialized;
		if (initialized)
		{
			stackObj = value.stackObj;
		}
	}
	constexpr void operator=(StackObject<T>&& value) noexcept
	{
		if (initialized)
		{
			stackObj.Delete();
		}
		initialized = value.initialized;
		if (initialized)
		{
			stackObj = std::move(value.stackObj);
		}
	}
	constexpr T& operator*() const noexcept
	{
		return *stackObj;
	}
	constexpr T* operator->() const noexcept
	{
		return stackObj.operator->();
	}
	constexpr T* GetPtr() const noexcept
	{
		return stackObj.GetPtr();
	}
	constexpr operator T* () const noexcept
	{
		return stackObj;
	}
	bool operator==(const StackObject<T>&) const = delete;
	bool operator!=(const StackObject<T>&) const = delete;
	StackObject() {}
	StackObject(const StackObject<T, true>& value)
	{
		stackObj.New(value.operator*());
	}
	~StackObject()
	{
		if (initialized)
			stackObj.Delete();
	}
};

template <typename F, typename T>
struct LoopClass;

template <typename F, size_t... idx>
struct LoopClass<F, std::index_sequence<idx...>>
{
	static void Do(const F& f)
	{
		char c[] = { (f(idx), 0)... };
	}
};


template <typename F, unsigned int count>
struct LoopClassEarlyBreak
{
	static bool Do(const F& f)
	{
		if (!LoopClassEarlyBreak<F, count - 1>::Do((f))) return false;
		return f(count);
	}
};

template <typename F>
struct LoopClassEarlyBreak<F, 0>
{
	static bool Do(const F& f)
	{
		return f(0);
	}
};

template <typename F, unsigned int count>
void InnerLoop(const F& function)
{
	LoopClass<typename std::remove_cvref_t<F>, std::make_index_sequence<count>>::Do(function);
}

template <typename F, unsigned int count>
bool InnerLoopEarlyBreak(const F& function)
{
	return LoopClassEarlyBreak<typename std::remove_cvref_t<F>, count - 1>::Do(function);
}

template <typename ... Types>
struct Tuple;

template<>
struct Tuple<> {};

template <typename T, typename ... Types>
struct Tuple<T, Types...> : public Tuple<Types...>
{
	T value;
};

template <size_t index, typename Tup>
struct TupleGetter;

template <size_t index, typename T, typename ... Types>
struct TupleGetter<index, Tuple<T, Types...>>
{
	using Type = typename TupleGetter<index - 1, Tuple<Types...>>::Type;
	inline static typename Type& Get(Tuple<T, Types...>& value);
};

template <typename T, typename ... Types>
struct TupleGetter<0, Tuple<T, Types...>>
{
	using Type = typename T;
	inline static T& Get(Tuple<T, Types...>& value);
};

template <size_t index, typename T, typename ... Types>
inline typename TupleGetter<index, Tuple<T, Types...>>::Type& TupleGetter<index, Tuple<T, Types...>>::Get(Tuple<T, Types...>& value)
{
	return TupleGetter<index - 1, Tuple<Types...>>::Get(value);
}

template <typename T, typename ... Types>
inline T& TupleGetter<0, Tuple<T, Types...>>::Get(Tuple<T, Types...>& value)
{
	return value.value;
}
#include <type_traits>
#include <stdint.h>
template <typename F>
void QuicksortStack(std::remove_cvref_t<F>* a, int32_t p, int32_t q)
{
	using T = std::remove_cvref_t<F>;
	int32_t i = p;
	int32_t j = q;
	T temp = a[p];

	while (i < j)
	{
		while (a[j] >= temp && j > i) j--;

		if (j > i)
		{
			a[i] = a[j];
			i++;
			while (a[i] <= temp && i < j) i++;
			if (i < j)
			{
				a[j] = a[i];
				j--;
			}
		}
	}
	a[i] = temp;
	if (p < (i - 1)) QuicksortStack<F>(a, p, i - 1);
	if ((j + 1) < q) QuicksortStack<F>(a, j + 1, q);
}

template <typename F, typename CompareFunc>
void QuicksortStackCustomCompare(std::remove_cvref_t<F>* a, const CompareFunc& compareFunc, int32_t p, int32_t q)
{
	using T = std::remove_cvref_t<F>;
	int32_t i = p;
	int32_t j = q;
	T temp = a[p];

	while (i < j)
	{
		while (compareFunc(a[j], temp) >= 0 && j > i) j--;

		if (j > i)
		{
			a[i] = a[j];
			i++;
			while (compareFunc(a[i], temp) <= 0 && i < j) i++;
			if (i < j)
			{
				a[j] = a[i];
				j--;
			}
		}
	}
	a[i] = temp;
	if (p < (i - 1)) QuicksortStackCustomCompare<F, CompareFunc>(a, compareFunc, p, i - 1);
	if ((j + 1) < q) QuicksortStackCustomCompare<F, CompareFunc>(a, compareFunc, j + 1, q);
}
template <typename T>
using PureType_t = typename std::remove_pointer_t<std::remove_cvref_t<T>>;

struct DefaultType
{};
struct Type_Element
{
	const std::type_info& info;
	Type_Element(const std::type_info& t) : info(t)
	{
	}
};

struct Type
{
private:
	StackObject<Type_Element> typeEle;
public:
	Type()
	{
		typeEle.New(typeid(DefaultType));
	}
	Type(const Type& t)
	{
		typeEle.New(t.typeEle->info);
	}
	Type(const std::type_info& info)
	{
		typeEle.New(info);
	}
	bool operator==(const Type& t) const
	{
		return t.typeEle->info == typeEle->info;
	}
	bool operator!=(const Type& t) const
	{
		return t.typeEle->info != typeEle->info;
	}
	void operator=(const Type& t)
	{
		typeEle.New(t.typeEle->info);
	}
	size_t HashCode() const
	{
		return typeEle->info.hash_code();
	}
	const std::type_info& GetType() const
	{
		return typeEle->info;
	}
};

namespace std
{
	template<>
	struct hash<Type>
	{
		size_t operator()(const Type& t) const noexcept
		{
			return t.HashCode();
		}
	};
}