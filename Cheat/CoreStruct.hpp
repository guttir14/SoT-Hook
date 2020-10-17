#pragma once
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <emmintrin.h>


#ifdef _MSC_VER
	#pragma pack(push, 0x8)
#endif

template<class T>
class TArray
{
	friend class FString;

public:
	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline T& operator[](int i) const
	{
		return Data[i];
	};

	inline bool IsValidIndex(int i) const
	{
		return i < Count;
	}

	T* Data;
	int Count;
	int Max;
};

class FNameEntry
{
public:
	int Index;
	int pad;
	FNameEntry* HashNext;
	union
	{
		char AnsiName[1024];
		wchar_t WideName[1024];
	};

	inline const int GetIndex() const
	{
		return Index >> 1;
	}

	inline bool IsWide() const
	{
		return Index & 1;
	}

	inline const char* GetAnsiName() const
	{
		return AnsiName;
	}

	inline const wchar_t* GetWideName() const
	{
		return WideName;
	}
};

constexpr int ChunkTableSize = (2 * 1024 * 1024 + 16384 - 1) / 16384;

class TNameEntryArray
{
public:

	inline bool IsValidIndex(int index) const
	{
		return index < NumElements && index >= 0;
	}

	FNameEntry const* const& GetById(int index) const
	{
		return *GetItemPtr(index);
	}

	inline FNameEntry const* const* GetItemPtr(int Index) const
	{
		const auto ChunkIndex = Index / 16384;
		const auto WithinChunkIndex = Index % 16384;
		const auto Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	void Resolve() {
		for (auto i = 0; Chunks[i]; i++) {
			NumChunks++;
		}
		NumElements = NumChunks * 16384;
	}

	FNameEntry** Chunks[ChunkTableSize];
	static inline int NumElements = 0;
	static inline int NumChunks = 0;
};

struct FVector
{
	float X, Y, Z;

    inline FVector() : X(0.f), Y(0.f), Z(0.f) {}

    inline FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

	float Length(void) const { return sqrtf(X * X + Y * Y + Z * Z); }

	float DistTo(FVector v) const { return (*this - v).Length(); }

    inline FVector operator + (const FVector& other) const { return FVector(X + other.X, Y + other.Y, Z + other.Z); }

    inline FVector operator - (const FVector& other) const { return FVector(X - other.X, Y - other.Y, Z - other.Z); }

    inline FVector operator * (float scalar) const { return FVector(X * scalar, Y * scalar, Z * scalar); }

    inline FVector& operator=  (const FVector& other) { X  = other.X; Y  = other.Y; Z  = other.Z; return *this; }

    inline FVector& operator+= (const FVector& other) { X += other.X; Y += other.Y; Z += other.Z; return *this; }

    inline FVector& operator-= (const FVector& other) { X -= other.X; Y -= other.Y; Z -= other.Z; return *this; }

    inline FVector& operator*= (const float other)    { X *= other;   Y *= other;   Z *= other;   return *this; }

	friend bool operator==(const FVector& first, const FVector& second)
	{
		return first.X == second.X && first.Y == second.Y && first.Z == second.Z;
	}

	friend bool operator!=(const FVector& first, const FVector& second)
	{
		return !(first == second);
	}

};

// ScriptStruct CoreUObject.Vector2D
// 0x0008
struct FVector2D
{
	float X, Y;
	inline FVector2D()
		: X(0), Y(0)
	{ }

	inline FVector2D(float x, float y)
		: X(x),
		  Y(y)
	{ }

	inline float Length() {
		return sqrtf(X * X + Y * Y);
	}

	inline FVector2D operator + (const FVector2D& other) const
	{
		return FVector2D(X + other.X, Y + other.Y);
	}

	inline FVector2D operator - (const FVector2D& other) const
	{
		return FVector2D(X - other.X, Y - other.Y);
	}

	inline FVector2D operator * (float scalar) const
	{
		return FVector2D(X * scalar, Y * scalar);
	}

	inline FVector2D& operator=  (const FVector2D& other)
	{
		X  = other.X;
		Y  = other.Y;
		return *this;
	}

	inline FVector2D& operator+= (const FVector2D& other)
	{
		X += other.X;
		Y += other.Y;
		return *this;
	}

	inline FVector2D& operator-= (const FVector2D& other)
	{
		X -= other.X;
		Y -= other.Y;
		return *this;
	}

	inline FVector2D& operator*= (const float other)
	{
		X *= other;
		Y *= other;
		return *this;
	}

	friend bool operator==(const FVector2D& one, const FVector2D& two)
	{
		return one.X == two.X && one.Y == two.Y;
	}

	friend bool operator!=(const FVector2D& one, const FVector2D& two)
	{
		return !(one == two);
	}

	friend bool operator>(const FVector2D& one, const FVector2D& two)
	{
		return one.X > two.X && one.Y > two.Y;
	}

	friend bool operator<(const FVector2D& one, const FVector2D& two)
	{
		return one.X < two.X && one.Y < two.Y;
	}

};

// ScriptStruct CoreUObject.Rotator
// 0x000C
struct FRotator
{
	float                                              Pitch;                                                    // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
	float                                              Yaw;                                                      // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)
	float                                              Roll;                                                     // 0x0008(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData)

	inline FRotator()
		: Pitch(0), Yaw(0), Roll(0)
	{ }

	inline FRotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}

	inline FRotator operator + (const FRotator& other) const { return FRotator(Pitch + other.Pitch, Yaw + other.Yaw, Roll + other.Roll); }

	inline FRotator operator - (const FRotator& other) const { return FRotator(Pitch - other.Pitch,Yaw - other.Yaw, Roll - other.Roll); }

	inline FRotator operator * (float scalar) const { return FRotator(Pitch * scalar,Yaw * scalar, Roll * scalar); }

	inline FRotator& operator=  (const FRotator& other) { Pitch = other.Pitch; Yaw = other.Yaw; Roll = other.Roll; return *this; }

	inline FRotator& operator+= (const FRotator& other) { Pitch += other.Pitch; Yaw += other.Yaw; Roll += other.Roll; return *this; }

	inline FRotator& operator-= (const FRotator& other) { Pitch -= other.Pitch; Yaw -= other.Yaw; Roll -= other.Roll; return *this; }

	inline FRotator& operator*= (const float other) { Yaw *= other; Pitch *= other; Roll *= other; return *this; }

	inline float Length() {
		return sqrtf(Pitch * Pitch + Yaw * Yaw);
	}

	friend bool operator==(const FRotator& first, const FRotator& second)
	{ 
		return first.Pitch == second.Pitch && first.Yaw == second.Yaw && first.Roll == second.Roll;
	}

	friend bool operator!=(const FRotator& first, const FRotator& second)
	{
		return !(first == second);
	}

};



struct FLinearColor
{
	float R;
	float G; 
	float B; 
	float A;

	inline FLinearColor()
		: R(0), G(0), B(0), A(0)
	{ }

	inline FLinearColor(float r, float g, float b, float a)
		: R(r),
		  G(g),
		  B(b),
		  A(a)
	{ }

	inline FLinearColor(float r, float g, float b)
		: R(r),
		  G(g),
		  B(b),
		  A(1.f)
	{ }

	bool operator!=(const FLinearColor& other)
	{ 
		return R != other.R || G != other.G || B != other.B || A != other.A;
	}

	bool operator==(const FLinearColor& other)
	{
		return R == other.R && G == other.G && B == other.B && A == other.A;
	}

};

struct FQuat
{
	/** Holds the quaternion's X-component. */
	float X;

	/** Holds the quaternion's Y-component. */
	float Y;

	/** Holds the quaternion's Z-component. */
	float Z;

	/** Holds the quaternion's W-component. */
	float W;

};

typedef __m128	VectorRegister;
typedef __m128i VectorRegisterInt;
typedef __m128d VectorRegisterDouble;

FORCEINLINE VectorRegister VectorMultiply(const VectorRegister& Vec1, const VectorRegister& Vec2)
{
	return _mm_mul_ps(Vec1, Vec2);
}

#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )
#define VectorReplicate( Vec, ElementIndex )	_mm_shuffle_ps( Vec, Vec, SHUFFLEMASK(ElementIndex,ElementIndex,ElementIndex,ElementIndex) )
#define VectorMultiplyAdd( Vec1, Vec2, Vec3 )	_mm_add_ps( _mm_mul_ps(Vec1, Vec2), Vec3 )

struct FMatrix
{
	union
	{
		__declspec(align(16)) float M[4][4];
	};

	FORCEINLINE void VectorMatrixMultiply(void* Result, const void* Matrix1, const void* Matrix2)
	{
		const VectorRegister* A = (const VectorRegister*)Matrix1;
		const VectorRegister* B = (const VectorRegister*)Matrix2;
		VectorRegister* R = (VectorRegister*)Result;
		VectorRegister Temp, R0, R1, R2, R3;

		// First row of result (Matrix1[0] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[0], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[0], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[0], 2), B[2], Temp);
		R0 = VectorMultiplyAdd(VectorReplicate(A[0], 3), B[3], Temp);

		// Second row of result (Matrix1[1] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[1], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[1], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[1], 2), B[2], Temp);
		R1 = VectorMultiplyAdd(VectorReplicate(A[1], 3), B[3], Temp);

		// Third row of result (Matrix1[2] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[2], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[2], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[2], 2), B[2], Temp);
		R2 = VectorMultiplyAdd(VectorReplicate(A[2], 3), B[3], Temp);

		// Fourth row of result (Matrix1[3] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[3], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[3], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[3], 2), B[2], Temp);
		R3 = VectorMultiplyAdd(VectorReplicate(A[3], 3), B[3], Temp);

		// Store result
		R[0] = R0;
		R[1] = R1;
		R[2] = R2;
		R[3] = R3;
	};

	FORCEINLINE FMatrix operator*(const FMatrix& Other) {
		FMatrix Result;
		VectorMatrixMultiply(&Result, this, &Other);
		return Result;
	};
};

struct FTransform
{
	struct FQuat Rotation;
	struct FVector Translation;
	unsigned char UnknownData00[0x4];
	struct FVector Scale3D;
	unsigned char UnknownData01[0x4];


	FORCEINLINE FMatrix ToMatrixWithScale() const
	{
		FMatrix OutMatrix;
		OutMatrix.M[3][0] = Translation.X;
		OutMatrix.M[3][1] = Translation.Y;
		OutMatrix.M[3][2] = Translation.Z;


		const float x2 = Rotation.X + Rotation.X;
		const float y2 = Rotation.Y + Rotation.Y;
		const float z2 = Rotation.Z + Rotation.Z;
		{
			const float xx2 = Rotation.X * x2;
			const float yy2 = Rotation.Y * y2;
			const float zz2 = Rotation.Z * z2;

			OutMatrix.M[0][0] = (1.0f - (yy2 + zz2)) * Scale3D.X;
			OutMatrix.M[1][1] = (1.0f - (xx2 + zz2)) * Scale3D.Y;
			OutMatrix.M[2][2] = (1.0f - (xx2 + yy2)) * Scale3D.Z;
		}
		{
			const float yz2 = Rotation.Y * z2;
			const float wx2 = Rotation.W * x2;

			OutMatrix.M[2][1] = (yz2 - wx2) * Scale3D.Z;
			OutMatrix.M[1][2] = (yz2 + wx2) * Scale3D.Y;
		}
		{
			const float xy2 = Rotation.X * y2;
			const float wz2 = Rotation.W * z2;

			OutMatrix.M[1][0] = (xy2 - wz2) * Scale3D.Y;
			OutMatrix.M[0][1] = (xy2 + wz2) * Scale3D.X;
		}
		{
			const float xz2 = Rotation.X * z2;
			const float wy2 = Rotation.W * y2;

			OutMatrix.M[2][0] = (xz2 + wy2) * Scale3D.Z;
			OutMatrix.M[0][2] = (xz2 - wy2) * Scale3D.X;
		}

		OutMatrix.M[0][3] = 0.0f;
		OutMatrix.M[1][3] = 0.0f;
		OutMatrix.M[2][3] = 0.0f;
		OutMatrix.M[3][3] = 1.0f;

		return OutMatrix;
	}
};

#ifdef _MSC_VER
	#pragma pack(pop)
#endif
