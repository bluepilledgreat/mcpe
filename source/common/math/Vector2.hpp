#pragma once

template<typename T>
class Vector2
{
public:
	T x;
	T y;

public:
	Vector2()
		: x(static_cast<T>(0))
		, y(static_cast<T>(0))
	{
	}

	Vector2(float x, float y)
		: x(static_cast<T>(x))
		, y(static_cast<T>(y))
	{
	}

	Vector2(int x, int y)
		: x(static_cast<T>(x))
		, y(static_cast<T>(y))
	{
	}

public:
	template<typename U>
	Vector2<T> operator+(const Vector2<U>& other) const
	{
		return Vector2<T>(x + static_cast<T>(other.x), y + static_cast<T>(other.y));
	}

	template<typename U>
	Vector2<T>& operator+=(const Vector2<U>& other)
	{
		x += static_cast<T>(other.x);
		y += static_cast<T>(other.y);
		return *this;
	}

	template<typename U>
	Vector2<T>& operator-=(const Vector2<U>& other)
	{
		x -= static_cast<T>(other.x);
		y -= static_cast<T>(other.y);
		return *this;
	}

	template<typename U>
	Vector2<T> operator*(U s) const
	{
		return Vector2<T>(static_cast<T>(x * s), static_cast<T>(y * s));
	}

	template<typename U>
	Vector2<T> operator/(U s) const
	{
		return Vector2<T>(static_cast<T>(x / s), static_cast<T>(y / s));
	}
};

typedef Vector2<float> Vector2F;
typedef Vector2<int> Vector2I;
