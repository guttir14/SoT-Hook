#pragma once
#include "Vector.h"
#include "Quat.h"
#include "Matrix.h"

struct FTransform
{
	FQuat Rotation;
	FVector Translation;
	char UnknownData00[0x4];
	FVector Scale3D;
	char UnknownData01[0x4];

	/** Default constructor. */
	FTransform();

	FMatrix ToMatrixWithScale() const;

	FVector TransformPosition(const FVector& V) const;

	FTransform(const FRotator& InRotation);
};