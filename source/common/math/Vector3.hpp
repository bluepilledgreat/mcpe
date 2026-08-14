#pragma once
#include <limits>

template<typename T>
class Vector3
{
public:
	T x;
	T y;
	T z;

public:
	Vector3(float x, float y, float z)
		: x(static_cast<T>(x))
		, y(static_cast<T>(y))
		, z(static_cast<T>(z))
	{
	}

	Vector3(int x, int y, int z)
		: x(static_cast<T>(x))
		, y(static_cast<T>(y))
		, z(static_cast<T>(z))
	{
	}

public:
	static Vector3 MinFinite()
	{
		return Vector3<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), std::numeric_limits<T>::min());
	}

	static Vector3 MaxFinite()
	{
		return Vector3<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
	}
};

typedef Vector3<float> Vector3F;
typedef Vector3<int> Vector3I;
