#pragma once
#include "common.hpp"


#ifndef HEXA_MATH_VECTOR_HPP
#define HEXA_MATH_VECTOR_HPP

namespace HEXA_MATH_NAMESPACE
{
#define BINARY_OP_VEC2(op) \
	constexpr Vector2 operator##op(const Vector2& b) const { return { x##op b.x, y##op b.y }; } \
	constexpr Vector2& operator##op##=(const Vector2& b) { x##op##= b.x; y##op##= b.y; return *this; }

#define BINARY_OP_VEC3(op) \
	constexpr Vector3 operator##op(const Vector3& b) const { return { x##op b.x, y##op b.y, z##op b.z }; } \
	constexpr Vector3& operator##op##=(const Vector3& b) { x##op##= b.x; y##op##= b.y; z##op##= b.z; return *this; }

#define BINARY_OP_VEC4(op) \
	constexpr Vector4 operator##op(const Vector4& b) const { return { x##op b.x, y##op b.y, z##op b.z, w##op b.w }; } \
	constexpr Vector4& operator##op##=(const Vector4& b) { x##op##= b.x; y##op##= b.y; z##op##= b.z; w##op##= b.w; return *this; }

#define UNARY_OP_VEC2(op) \
	constexpr Vector2 operator##op() const { return { op x, op y }; }

#define UNARY_OP_VEC3(op) \
	constexpr Vector3 operator##op() const { return { op x, op y, op z }; }

#define UNARY_OP_VEC4(op) \
	constexpr Vector4 operator##op() const { return { op x, op y, op z, op w }; }

	struct Vector2
	{
		static constexpr size_t Count = 2;

		float x, y;

		constexpr Vector2() : x(0.0f), y(0.0f) {}
		constexpr Vector2(float x, float y) : x(x), y(y) {}
		constexpr Vector2(float v) : x(v), y(v) {}

		BINARY_OP_VEC2(+);
		BINARY_OP_VEC2(-);
		BINARY_OP_VEC2(*);
		BINARY_OP_VEC2(/);

		UNARY_OP_VEC2(-);

		constexpr bool operator==(const Vector2& b) const { return x == b.x && y == b.y; }
		constexpr bool operator!=(const Vector2& b) const { return !(*this == b); }

		constexpr float& operator[](size_t index) { return (&x)[index]; }
		constexpr const float& operator[](size_t index) const { return (&x)[index]; }
	};

	struct Vector3
	{
		static constexpr size_t Count = 3;

		float x, y, z;

		constexpr Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
		constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
		constexpr Vector3(float v) : x(v), y(v), z(v) {}

		BINARY_OP_VEC3(+);
		BINARY_OP_VEC3(-);
		BINARY_OP_VEC3(*);
		BINARY_OP_VEC3(/);

		UNARY_OP_VEC3(-);

		constexpr bool operator==(const Vector3& b) const { return x == b.x && y == b.y && z == b.z; }
		constexpr bool operator!=(const Vector3& b) const { return !(*this == b); }

		constexpr float& operator[](size_t index) { return (&x)[index]; }
		constexpr const float& operator[](size_t index) const { return (&x)[index]; }
	};

	struct Vector4
	{
		static constexpr size_t Count = 4;

		float x, y, z, w;

		constexpr Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
		constexpr Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		constexpr Vector4(float v) : x(v), y(v), z(v), w(v) {}

		BINARY_OP_VEC4(+);
		BINARY_OP_VEC4(-);
		BINARY_OP_VEC4(*);
		BINARY_OP_VEC4(/);

		UNARY_OP_VEC4(-);

		constexpr bool operator==(const Vector4& b) const { return x == b.x && y == b.y && z == b.z && w == b.w; }
		constexpr bool operator!=(const Vector4& b) const { return !(*this == b); }

		constexpr float& operator[](size_t index) { return (&x)[index]; }
		constexpr const float& operator[](size_t index) const { return (&x)[index]; }
	};

#undef BINARY_OP_VEC2
#undef BINARY_OP_VEC3
#undef BINARY_OP_VEC4

#undef UNARY_OP_VEC2
#undef UNARY_OP_VEC3
#undef UNARY_OP_VEC4

}

#endif

HEXA_PRISM_NAMESPACE_BEGIN

class PrismObject
{
	std::atomic<size_t> counter;

public:
	PrismObject() : counter(1)
	{
	}

	void AddRef()
	{
		counter.fetch_add(1, std::memory_order_acq_rel);
	}

	void Release()
	{
		if (counter.fetch_sub(1, std::memory_order_acq_rel) == 1)
		{
			delete this; // TODO: Change to custom allocator solution.
		}
	}

	virtual ~PrismObject() = default;
};

template <typename T>
class PrismObj
{
	template <typename U> friend class PrismObj;
	T* ptr;

public:
	constexpr PrismObj() : ptr(nullptr)
	{
	}

	explicit PrismObj(T* p, bool addRef = true) noexcept : ptr(p)
	{
		if (ptr && addRef) ptr->AddRef();
	}

	PrismObj(const PrismObj& other) noexcept : ptr(other.ptr)
	{
		if (ptr) ptr->AddRef();
	}

	template <typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
	PrismObj(const PrismObj<U>& other) noexcept : ptr(other.Get())
	{
		if (ptr) ptr->AddRef();
	}

	template <typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
	PrismObj(PrismObj<U>& other) noexcept : ptr(other.Get())
	{
		if (ptr) ptr->AddRef();
	}

	PrismObj(PrismObj&& other) noexcept : ptr(other.ptr)
	{
		other.ptr = nullptr;
	}

	~PrismObj() noexcept
	{
		if (ptr) ptr->Release();
		ptr = nullptr;
	}

	PrismObj& operator=(const PrismObj& other)
	{
		if (this != &other)
		{
			if (other.ptr) other.ptr->AddRef();
			if (ptr) ptr->Release();
			ptr = other.ptr;
		}
		return *this;
	}

	template <typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
	PrismObj& operator=(PrismObj<U>& other)
	{
		if (ptr != other.ptr)
		{
			if (other.ptr) other.ptr->AddRef();
			if (ptr) ptr->Release();
			ptr = other.ptr;
		}
		return *this;
	}

	PrismObj& operator=(PrismObj&& other) noexcept
	{
		if (this != &other)
		{
			if (ptr) ptr->Release();
			ptr = other.ptr;
			other.ptr = nullptr;
		}
		return *this;
	}

	PrismObj& operator=(T* p) noexcept
	{
		if (ptr != p)
		{
			if (ptr) ptr->Release();
			p->AddRef();
			ptr = p;
		}
		return *this;
	}

	template <typename U>
	constexpr operator U* () { return ptr; }

	constexpr T* operator->() const { return ptr; }
	constexpr T& operator*() const { return *ptr; }
	constexpr operator bool() const noexcept { return ptr != nullptr; }
	bool operator==(const PrismObj<T>& other) const noexcept { return ptr == other.ptr; }
	bool operator!=(const PrismObj<T>& other) const noexcept { return ptr != other.ptr; }
	bool operator==(T* p) const noexcept { return ptr == p; }
	bool operator!=(T* p) const noexcept { return ptr != p; }

	constexpr T* Get() const { return ptr; }

	PrismObj<T> AddRef()
	{
		return PrismObj<T>(ptr, true);
	}

	T* Detach()
	{
		auto* tmp = ptr;
		ptr = nullptr;
		return tmp;
	}

	void Release()
	{
		if (ptr)
		{
			ptr->Release();
			ptr = nullptr;
		}
	}

	void Reset(T* ptr)
	{
		Release();
		this->ptr = ptr;
	}

	template <typename U>
	PrismObj<U> As() const
	{
		return PrismObj<U>(dynamic_cast<U*>(ptr), true);
	}

	template <typename U>
	U* AsPtr() const		
	{
		return dynamic_cast<U*>(ptr);
	}

	void swap(PrismObj<T>& other) noexcept
	{
		std::swap(ptr, other.ptr);
	}
};

template<typename TCallback>
class EventHandlerList
{
public:
	class EventHandler : public PrismObject
	{
		friend class EventHandlerList;
		EventHandlerList* list;
		TCallback callback;
		EventHandler* next;
		EventHandler* prev;

	public:
		EventHandler(EventHandlerList* list, TCallback callback, EventHandler* next, EventHandler* prev) : list(list), callback(std::move(callback)), next(next), prev(prev)
		{
		}

		EventHandlerList* GetList() const
		{
			return list;
		}

		void Unsubscribe()
		{
			if (list)
			{
				list->Unsubscribe(this);
				list = nullptr;
			}
		}
	};

	class EventHandlerToken
	{
		PrismObj<EventHandler> handler;

	public:
		EventHandlerToken() = default;
		EventHandlerToken(EventHandler* handler) : handler(handler)
		{
		}
		~EventHandlerToken()
		{
			Unsubscribe();
		}

		EventHandlerToken(const EventHandlerToken&) = delete;
		EventHandlerToken& operator=(const EventHandlerToken&) = delete;
		EventHandlerToken(EventHandlerToken&& other) noexcept : handler(std::move(other.handler))
		{
			other.handler = nullptr;
		}
		EventHandlerToken& operator=(EventHandlerToken&& other) noexcept
		{
			if (this != &other)
			{
				Unsubscribe();
				handler = std::move(other.handler);
				other.handler = nullptr;
			}
			return *this;
		}

		void Unsubscribe()
		{
			if (handler)
			{
				handler->Unsubscribe();
				handler = nullptr;
			}
		}
	};

private:
	EventHandler* head;
	std::atomic<size_t> lock;

	void Lock()
	{
		size_t value = lock.load(std::memory_order_relaxed);
		while (!lock.compare_exchange_weak(value, 1, std::memory_order_release, std::memory_order_acquire))
		{
			lock.wait(value, std::memory_order_relaxed);
		}
	}

	void Unlock()
	{
		lock.store(0, std::memory_order_release);
		lock.notify_one();
	}

	struct LockGuard
	{
		EventHandlerList* list;
		explicit LockGuard(EventHandlerList* list) : list(list)
		{
			list->Lock();
		}

		~LockGuard()
		{
			list->Unlock();
		}
	};

public:

	EventHandlerList() : head(nullptr)
	{
	}

	~EventHandlerList()
	{
		LockGuard guard(this);
		EventHandler* current = head;
		while (current)
		{
			EventHandler* next = current->next;
			current->list = nullptr; // Clear out get weak reference behavior, this will prevent any callback to call unsubscribe.
			current->Release();
			current = next;
		}
		head = nullptr;
	}

	EventHandlerToken Subscribe(TCallback callback)
	{
		LockGuard guard(this);
		EventHandler* newHandler = new EventHandler(this, std::move(callback), head, nullptr);
		if (head)
		{
			head->prev = newHandler;
		}
		head = newHandler;
		return EventHandlerToken(newHandler);
	}

	void Unsubscribe(EventHandler* handler)
	{
		LockGuard guard(this);
		if (handler->prev)
		{
			handler->prev->next = handler->next;
		}
		else
		{
			head = handler->next;
		}
		if (handler->next)
		{
			handler->next->prev = handler->prev;
		}
		handler->Release();
	}

	template<typename... TArgs>
	void Invoke(TArgs&&... args)
	{
		LockGuard guard(this);
		EventHandler* current = head;
		while (current)
		{
			current->callback(std::forward<TArgs>(args)...);
			current = current->next;
		}
	}
};

[[nodiscard]] inline void* PrismAlloc(const size_t size)
{
	return malloc(size);
}

inline void PrismFree(void* ptr)
{
	free(ptr);
}

template <typename T>
[[nodiscard]] inline T* PrismAllocT(const size_t count)
{
	return static_cast<T*>(PrismAlloc(sizeof(T) * count));
}

inline void PrismZeroMemory(void* mem, const size_t size)
{
	std::memset(mem, 0, size);
}

template <typename T>
inline void PrismZeroMemoryT(T* mem, const size_t count)
{
	std::memset(mem, 0, sizeof(T) * count);
}

inline void PrismMemoryCopy(void* dst, void* src, size_t size)
{
	std::memcpy(dst, src, size);
}

template <typename T>
inline void PrismMemoryCopyT(T* dst, T* src, size_t count)
{
	std::memcpy(dst, src, sizeof(T) * count);
}

template <typename T, typename... TArgs>
[[nodiscard]] inline PrismObj<T> MakePrismObj(TArgs&&... args)
{
	T* obj = new T(std::forward<TArgs>(args)...); // TODO: Change to custom allocator solution.
	return PrismObj<T>(obj, false);
}

HEXA_PRISM_NAMESPACE_END