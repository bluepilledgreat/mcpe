#pragma once
#include "common/math/Vector2.hpp"

template<typename T>
class Rect2D
{
public:
	Vector2<T> min;
	Vector2<T> max;

public:
	Rect2D()
		: min(0, 0)
		, max(0, 0)
	{
	}

	template<typename U>
	Rect2D(const Vector2<U>& wh)
		: min(0, 0)
		, max(wh)
	{
	}

	template<typename U>
	Rect2D(const Vector2<U>& min, const Vector2<U>& max)
		: min(min)
		, max(max)
	{
	}

	T width() const
	{
		return max.x - min.x;
	}

	T height() const
	{
		return max.y - min.y;
	}

	T x0() const
	{
		return min.x;
	}

	T x1() const
	{
		return max.x;
	}

	T y0() const
	{
		return min.y;
	}

	T y1() const
	{
		return max.y;
	}

	Vector2<T> x0y0() const
	{
		return min;
	}

	Vector2<T> x1y0() const
	{
		return Vector2<T>(max.x, min.y);
	}

	Vector2<T> x0y1() const
	{
		return Vector2<T>(min.x, max.y);
	}

	Vector2<T> x1y1() const
	{
		return max;
	}

	Vector2<T> wh() const
	{
		return max - min;
	}

	Vector2<T> center() const
	{
		return (max + min) / 2;
	}

	bool contains(const Vector2<T>& v) const
	{
		return
			v.x >= min.x &&
			v.y >= min.y &&
			v.x <= max.x &&
			v.y <= max.y;
	}
};

typedef Rect2D<float> Rect2DF;
typedef Rect2D<int> Rect2DI;
