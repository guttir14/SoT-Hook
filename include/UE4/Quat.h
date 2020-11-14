#pragma once

struct FQuat
{
public:

	float X, Y, Z, W;

	FQuat() : X(0.f), Y(0.f), Z(0.f), W(0.f) {};

	FQuat::FQuat(float InX, float InY, float InZ, float InW) : X(InX), Y(InY), Z(InZ), W(InW) {}

	FQuat(const struct FRotator& const R);

	FVector RotateVector(const struct FVector& const V) const;
};