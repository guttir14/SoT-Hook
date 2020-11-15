#pragma once
#include "Vector.h"
#include "Quat.h"
#include "Matrix.h"
#include "Rotator.h"

struct FTransform
{
	FQuat Rotation;
	FVector Translation;
	char UnknownData00[0x4];
	FVector Scale3D;
	char UnknownData01[0x4];

	/** Default constructor. */
	FTransform() : Rotation(0.f, 0.f, 0.f, 1.f), Translation(0.f), Scale3D(FVector::OneVector) {};

	FMatrix ToMatrixWithScale() const;

	FVector TransformPosition(FVector& V) const;

	FTransform(const FRotator& InRotation) : Rotation(InRotation.Quaternion()), Translation(FVector::ZeroVector), Scale3D(FVector::OneVector) {};
};