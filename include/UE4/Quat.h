#pragma once

struct FQuat
{
public:

	/** The quaternion's X-component. */
	float X;

	/** The quaternion's Y-component. */
	float Y;

	/** The quaternion's Z-component. */
	float Z;

	/** The quaternion's W-component. */
	float W;

	__forceinline FQuat() { X = Y = Z = W = 0.f; };

	__forceinline FQuat::FQuat(float InX, float InY, float InZ, float InW)
		: X(InX)
		, Y(InY)
		, Z(InZ)
		, W(InW)
	{
	}

	__forceinline FQuat(const struct FRotator& R);

	__forceinline FVector RotateVector(struct FVector V) const;
};