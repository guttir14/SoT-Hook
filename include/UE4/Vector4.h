#pragma once

struct FVector4 {
	float X, Y, Z, W;
	FVector4() { X = Y = Z = W = 0.f; }
	FVector4(float InX, float InY, float InZ, float InW)
		: X(InX)
		, Y(InY)
		, Z(InZ)
		, W(InW)
	{}
};