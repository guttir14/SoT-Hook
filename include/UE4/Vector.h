#pragma once
#include "Vector4.h"
#include <corecrt_math.h>

struct FVector
{
	float X, Y, Z;

	__forceinline FVector() : X(0.f), Y(0.f), Z(0.f) {}

	__forceinline FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

	__forceinline FVector::FVector(float InF) : X(InF), Y(InF), Z(InF) { }

	__forceinline FVector(const FVector4& V) : X(V.X), Y(V.Y), Z(V.Z) {};

	__forceinline float Size() const { return sqrtf(X * X + Y * Y + Z * Z); }

	__forceinline float Sum() const { return X + Y + Z; }

	__forceinline float Size2D() const { return sqrtf(X * X + Y * Y); }

	__forceinline float SizeSquared() const { return X * X + Y * Y + Z * Z; }

	__forceinline float DistTo(const FVector& V) const { return (*this - V).Size(); }

	__forceinline FVector operator+(const FVector& other) const { return FVector(X + other.X, Y + other.Y, Z + other.Z); }

	__forceinline FVector operator-(const FVector& other) const { return FVector(X - other.X, Y - other.Y, Z - other.Z); }

	__forceinline FVector operator*(const FVector& V) const { return FVector(X * V.X, Y * V.Y, Z * V.Z); }

	__forceinline FVector operator/(const FVector& V) const { return FVector(X / V.X, Y / V.Y, Z / V.Z); }

	__forceinline bool operator==(const FVector& V) const { return X == V.X && Y == V.Y && Z == V.Z; }

	__forceinline bool operator!=(const FVector& V) const { return X != V.X || Y != V.Y || Z != V.Z; }

	__forceinline FVector operator-() const { return FVector(-X, -Y, -Z); }

	__forceinline FVector operator+(float Bias) const { return FVector(X + Bias, Y + Bias, Z + Bias); }

	__forceinline FVector operator-(float Bias) const { return FVector(X - Bias, Y - Bias, Z - Bias); }

	__forceinline FVector operator*(float Scale) const { return FVector(X * Scale, Y * Scale, Z * Scale); } const

	__forceinline FVector operator/(float Scale) const { const float RScale = 1.f / Scale; return FVector(X * RScale, Y * RScale, Z * RScale); }

	__forceinline FVector operator=(const FVector& V) { X = V.X; Y = V.Y; Z = V.Z; return *this; }

	__forceinline FVector operator+=(const FVector& V) { X += V.X; Y += V.Y; Z += V.Z; return *this; }

	__forceinline FVector operator-=(const FVector& V) { X -= V.X; Y -= V.Y; Z -= V.Z; return *this; }

	__forceinline FVector operator*=(const FVector& V) { X *= V.X; Y *= V.Y; Z *= V.Z; return *this; }

	__forceinline FVector operator/=(const FVector& V) { X /= V.X; Y /= V.Y; Z /= V.Z; return *this; }

	__forceinline FVector operator*=(float Scale) { X *= Scale; Y *= Scale; Z *= Scale; return *this; }

	__forceinline FVector operator/=(float V) { const float RV = 1.f / V; X *= RV; Y *= RV; Z *= RV; return *this; }

	__forceinline float operator|(const FVector& V) const { return X * V.X + Y * V.Y + Z * V.Z; }

	__forceinline FVector FVector::operator^(const FVector& V) const { return FVector(Y * V.Z - Z * V.Y,Z * V.X - X * V.Z,X * V.Y - Y * V.X); }

	static const FVector ZeroVector;

	static const FVector OneVector;
};
