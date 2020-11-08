#pragma once

constexpr auto PI = 3.1415926535897932f;
constexpr auto FLOAT_NON_FRACTIONAL = 8388608.f /* All single-precision floating point numbers greater than or equal to this have no fractional value. */;
constexpr auto INV_PI = 0.31830988618f;
constexpr auto HALF_PI = 1.57079632679f;
constexpr auto DEG_TO_RAD = PI / 180.f;
constexpr auto RADS_DIVIDED_BY_2 = DEG_TO_RAD / 2.f;

struct FMath {
	static __forceinline void SinCos(float* ScalarSin, float* ScalarCos, float  Value);
	static __forceinline float Fmod(float X, float Y);
	template<class T>
	static __forceinline T Clamp(const T X, const T Min, const T Max) { return X < Min ? Min : X < Max ? X : Max; }
};