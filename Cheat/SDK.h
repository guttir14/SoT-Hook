#pragma once
#include <unordered_set>
#include <Windows.h>
#include "CoreStruct.hpp"
#include <string>
#include <math.h> 


struct FName
{
	INT32 ComparisonIndex;
	INT32 Number;

	static inline TNameEntryArray** GNames = nullptr;

	static std::string GetNameById(INT32 Id) {
		auto NameEntry = (*GNames)->GetById(Id);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	}

	const std::string GetName() const {
		auto NameEntry = (*GNames)->GetById(ComparisonIndex);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	};

	inline bool operator==(const FName& other) const {
		return ComparisonIndex == other.ComparisonIndex;
	};
};

struct FUObjectItem
{
	class UObject* Object;
	INT32 Flags;
	INT32 ClusterIndex;
	INT32 SerialNumber;
	INT32 pad;
};

struct TUObjectArray
{
	FUObjectItem* Objects;
	INT32 MaxElements;
	INT32 NumElements;

	inline class UObject* GetByIndex(INT32 index)
	{
		return Objects[index].Object;
	}
};

class UClass;


class UObject
{
public:
	UObject(UObject* addr) { *this = addr; }
	static inline TUObjectArray* GObjects = nullptr;
	void* Vtable;
	int32_t ObjectFlags;
	int32_t InternalIndex;
	UClass* Class;
	FName Name;
	UObject* Outer;

	std::string GetName() const;

	std::string GetFullName() const;

	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (int i = 0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);

			if (object == nullptr)
			{
				continue;
			}

			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	}

	static UClass* FindClass(const std::string& name)
	{
		return FindObject<UClass>(name);
	}

	template<typename T>
	static T* GetObjectCasted(std::size_t index)
	{
		return static_cast<T*>(GObjects->GetByIndex(index));
	}

	bool IsA(UClass* cmp) const;

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class CoreUObject.Object");
		return ptr;
	}
};

class UField : public UObject
{
public:
	using UObject::UObject;
	UField* Next;
};

class UProperty : public UField
{
public:
	int32_t ArrayDim;
	int32_t ElementSize;
	uint64_t PropertyFlags;
	char pad[0xC];
	int32_t Offset_Internal;
	UProperty* PropertyLinkNext;
	UProperty* NextRef;
	UProperty* DestructorLinkNext;
	UProperty* PostConstructLinkNext;
};


class UStruct : public UField
{
public:
	using UField::UField;

	UStruct* SuperField;
	UField* Children;
	int32_t PropertySize;
	int32_t MinAlignment;
	TArray<uint8_t> Script;
	UProperty* PropertyLink;
	UProperty* RefLink;
	UProperty* DestructorLink;
	UProperty* PostConstructLink;
	TArray<UObject*> ScriptObjectReferences;
};

class UFunction : public UStruct
{
public:
	uint32_t FunctionFlags;
	uint16_t RepOffset;
	uint8_t NumParms;
	char pad;
	uint16_t ParmsSize;
	uint16_t ReturnValueOffset;
	uint16_t RPCId;
	uint16_t RPCResponseId;
	UProperty* FirstPropertyToInit;
	UFunction* EventGraphFunction; //0x00A0
	int32_t EventGraphCallOffset;
	char pad_0x00AC[0x4]; //0x00AC
	void* Func; //0x00B0
};

template<typename Fn>
inline void ProcessEvent(Fn* obj, UFunction* function, void* parms)
{
	auto vtable = *reinterpret_cast<void***>(obj);
	reinterpret_cast<void(*)(Fn*, class UFunction*, void*)>(vtable[59])(obj, function, parms);
}

class UClass : public UStruct
{
public:
	using UStruct::UStruct;
	unsigned char                                      UnknownData00[0x138];                                     // 0x0088(0x0138) MISSED OFFSET

	template<typename T>
	inline T* CreateDefaultObject()
	{
		return static_cast<T*>(CreateDefaultObject());
	}
};

class FString : public TArray<wchar_t>
{
public:
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? static_cast<int32_t>(std::wcslen(other)) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline const wchar_t* wide() const
	{
		return Data;
	}

	int multi(char* name, int size) const
	{
		return WideCharToMultiByte(CP_UTF8, 0, Data, Count, name, size, nullptr, nullptr) - 1;
	}
};


struct APlayerState {
	char pad[0x460];
	FString PlayerName;
};

struct APlayerCameraManager {
	FVector GetCameraLocation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraLocation");
		FVector location;
		ProcessEvent(this, fn, &location);
		return location;
	};
	FRotator GetCameraRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraRotation");
		FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}
};

class AController {

public:
	struct ProjectWorldLocationToScreen_Params
	{
		FVector WorldLocation;
		FVector2D ScreenLocation;
		bool ReturnValue = false;
	};

	char pad_0000[0x460]; //0x0000
	void* Pawn; //0x0460
	char pad_0468[8]; //0x0468
	class ACharacter* Character; //0x0470
	void* PlayerState; //0x0478
	void* TransformComponent; //0x0480
	FVector ControlRotation; //0x0488
	char pad_04A0[52]; //0x0494
	void* AcknowledgedPawn; //0x04C8
	char pad_04D0[0x10]; //0x04D0
	void* MyHUD; //0x04E0
	APlayerCameraManager* PlayerCameraManager; //0x04E8
	char pad[0x138]; // 0x4F0
	FRotator RotationInput; // 0x618


	bool ProjectWorldLocationToScreen(const FVector& WorldLocation, FVector2D* ScreenLocation) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
		ProjectWorldLocationToScreen_Params params;
		params.WorldLocation = WorldLocation;
		ProcessEvent(this, fn, &params);
		*ScreenLocation = params.ScreenLocation;
		return params.ReturnValue;
	}

	FRotator GetControlRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetControlRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	FRotator GetDesiredRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetDesiredRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	void AddYawInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddYawInput");
		ProcessEvent(this, fn, &Val);
	}

	void AddPitchInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddPitchInput");
		ProcessEvent(this, fn, &Val);
	}

	void FOV(float NewFOV) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.FOV");
		ProcessEvent(this, fn, &NewFOV);
	}

	bool LineOfSightTo(ACharacter* Other, const struct FVector& ViewPoint, bool bAlternateChecks ) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.LineOfSightTo");
		struct {
			ACharacter* Other = nullptr;
			FVector ViewPoint;
			bool bAlternateChecks = false;
			bool ReturnValue = false;
		} params;
		params.Other = Other;
		params.ViewPoint = ViewPoint;
		params.bAlternateChecks = bAlternateChecks;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}
};

struct UHealthComponent {
	float GetCurrentHealth() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HealthComponent.GetCurrentHealth");
		float health = 0.f;
		ProcessEvent(this, fn, &health);
		return health;
	};
};
struct USkeletalMeshComponent {
	char pad[0x590];
	TArray<FTransform> SpaceBasesArray[2];

	FName GetBoneName(int BoneIndex)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.SkinnedMeshComponent.GetBoneName");
		struct
		{
			int BoneIndex;
			FName ReturnValue;
		} params;
		params.BoneIndex = BoneIndex;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	FTransform K2_GetComponentToWorld() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.SceneComponent.K2_GetComponentToWorld"); 
		FTransform CompToWorld;
		ProcessEvent(this, fn, &CompToWorld);
		return CompToWorld;
	}
};

struct AShipInternalWater {
	float GetNormalizedWaterAmount() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipInternalWater.GetNormalizedWaterAmount");
		float params = 0.f;
		ProcessEvent(this, fn, &params);
		return params;
	}
};

struct AHullDamage {
	char pad[0x4a0];
	TArray<class ACharacter*> ActiveHullDamageZones;
};

struct UDrowningComponent {
	float GetOxygenLevel() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.DrowningComponent.GetOxygenLevel");
		float oxygen;
		ProcessEvent(this, fn, &oxygen);
		return oxygen;
	}
};



struct UItemDesc {
	char pad[0x0028];
	FString* Title; // 0x0028(0x0038)
};

struct AItemInfo {
	char pad[0x04B0];
	UItemDesc* Desc; // 0x04B0(0x0008) (Net, ZeroConstructor, IsPlainOldData)
};

struct UWieldedItemComponent {
	char pad[0x0298]; // 0x0
	ACharacter* CurrentlyWieldedItem; // 0x0298
};

struct FWeaponProjectileParams {
	char pad[0x10];
	float Velocity;
};

struct FProjectileWeaponParameters {
	char pad[0x50];
	float ProjectileMaximumRange;  // 0x0050
	char pad2[0x1C]; // 0x54
	FWeaponProjectileParams AmmoParams; // 0x0070
};

struct AProjectileWeapon {
	char pad[0x0838]; // 0
	FProjectileWeaponParameters WeaponParameters; // 0x0838
};

struct UWorldMapIslandDataAsset {
	char pad[0x48];
	FVector WorldSpaceCameraPosition; // 0x48
};

struct UIslandDataAssetEntry {
	char pad[0x40];
	UWorldMapIslandDataAsset* WorldMapData; // 0x0040
	char pad2[0x68];
	FString* LocalisedName; // 0x00B0
};

struct UIslandDataAsset
{
	char pad[0x0048];
	TArray<UIslandDataAssetEntry*> IslandDataEntries; // 0x0048
};

struct AIslandService {
	char pad[0x4d0];
	UIslandDataAsset* IslandDataAsset; // 0x4d0
};

struct AAthenaGameState {
	char pad[0x640];
	UINT64* WindService;
	UINT64* PlayerManagerService;
	UINT64* ShipService;
	UINT64* WatercraftService;
	UINT64* TimeService;
	UINT64* WaterService;
	UINT64* StormService;
	UINT64* CrewService;
	UINT64* ContestZoneService;
	UINT64* ContestRowboatsService;
	AIslandService* IslandService; // 0x0690
};

class ACharacter : public UObject {
public:
	char pad1[0x450]; // 48
	APlayerState* PlayerState;  // 0x478
	char pad2[0x10];
	AController* Controller; // 0x490
	char pad3[0x28];
	USkeletalMeshComponent* Mesh; // 0x4c0
	char pad4[0x3B8]; // 0x4c8
	UWieldedItemComponent* WieldedItemComponent; // 0x880
	char pad5[0x20]; // 0x888
	UHealthComponent* HealthComponent; // 0x08A8
	char pad6[0x410]; // 0x8B0
	UDrowningComponent* DrowningComponent; // 0xCC0

	void GetActorBounds(bool bOnlyCollidingComponents, struct FVector* Origin, struct FVector* BoxExtent) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorBounds");
		struct
		{
			bool bOnlyCollidingComponents = false;
			FVector Origin;
			FVector BoxExtent;
		} params;

		if (!Origin || !BoxExtent) return;

		params.bOnlyCollidingComponents = bOnlyCollidingComponents;

		ProcessEvent(this, fn, &params);

		*Origin = params.Origin;
		*BoxExtent = params.BoxExtent;
	}

	ACharacter* GetAttachParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetAttachParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}

	ACharacter* GetWieldedItem() {
		if (!WieldedItemComponent) return nullptr;
		return WieldedItemComponent->CurrentlyWieldedItem;
	}

	FVector& GetVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetVelocity");
		FVector velocity;
		ProcessEvent(this, fn, &velocity);
		return velocity;
	}

	AItemInfo* GetItemInfo() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ItemProxy.GetItemInfo");
		AItemInfo* info = nullptr;
		ProcessEvent(this, fn, &info);
		return info;
	}

	void CureAllAilings() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.CureAllAilings");
		ProcessEvent(this, fn, nullptr);
	}

	void Kill(uint8_t DeathType) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.Kill");
		ProcessEvent(this, fn, &DeathType);
	}
	bool IsDead() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsDead");
		bool isDead = true;
		ProcessEvent(this, fn, &isDead);
		return isDead;
	}

	

	bool IsInWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsInWater");
		bool isInWater = false;
		ProcessEvent(this, fn, &isInWater);
		return isInWater;
	}

	

	FRotator& K2_GetActorRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorRotation");
		FRotator params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector& K2_GetActorLocation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorLocation");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	bool GetBone(int id, FVector* pos, FMatrix& componentToWorld) {
		auto bones = Mesh->SpaceBasesArray[0];
		if (id >= bones.Count) return false;
		const auto& bone = bones[id];
		auto boneMatrix = bone.ToMatrixWithScale();
		auto world = boneMatrix * componentToWorld;
		*pos = { world.M[3][0], world.M[3][1], world.M[3][2] };
		return true;
	}

	bool GetBone2(int id, FVector* pos) {
		if (!Mesh) return false;
		auto bones = Mesh->SpaceBasesArray[1];
		if (id >= bones.Count) return false;
		const auto& bone = bones[id];
		auto boneMatrix = bone.ToMatrixWithScale();
		auto compMatrix = Mesh->K2_GetComponentToWorld().ToMatrixWithScale();
		auto world = boneMatrix * compMatrix;
		*pos = { world.M[3][0], world.M[3][1], world.M[3][2] };
		return true;
	}

	bool isSkeleton() {
		static auto obj = UObject::FindClass("Class Athena.AthenaAICharacter"); 
		return IsA(obj);
	}
	bool isPlayer() {
		static auto obj = UObject::FindClass("Class Athena.AthenaPlayerCharacter");
		return IsA(obj);
	}
	bool isShip() {
		static auto obj = UObject::FindClass("Class Athena.Ship");
		return IsA(obj);
	}

	bool isFarShip() {
		static auto obj = UObject::FindClass("Class Athena.ShipNetProxy");
		return IsA(obj);
	}

	bool isItem() {
		static auto obj = UObject::FindClass("Class Athena.ItemProxy");
		return IsA(obj);
	}

	bool isWeapon() {
		static auto obj = UObject::FindClass("Class Athena.ProjectileWeapon");
		return IsA(obj);
	}

	bool isBarrel() {
		static auto obj = UObject::FindClass("Class Athena.StorageContainer");
		return IsA(obj);
	}
	bool isBuriedTreasure() {
		static auto obj = UObject::FindClass("Class Athena.BuriedTreasureLocation");
		return IsA(obj);
	}
	AHullDamage* GetHullDamage() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetHullDamage");
		AHullDamage* params  = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}
	AShipInternalWater* GetInternalWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetInternalWater");
		AShipInternalWater* params = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}
};

class UKismetMathLibrary {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetMathLibrary");
	}
	static FRotator NormalizedDeltaRotator(const struct FRotator& A, const struct FRotator& B) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.NormalizedDeltaRotator");

		struct
		{
			struct FRotator                A;
			struct FRotator                B;
			struct FRotator                ReturnValue;
		} params;

		params.A = A;
		params.B = B;

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;

	};
	static FRotator FindLookAtRotation(const FVector& Start, const FVector& Target) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.FindLookAtRotation");
		
		struct {
			FVector Start;
			FVector Target;
			FRotator ReturnValue;
		} params;
		params.Start = Start;
		params.Target = Target;
		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}
	static void DrawDebugBox(UObject* WorldContextObject, const FVector& Center, const FVector& Extent, const FLinearColor& LineColor, const FRotator& Rotation, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			 UObject* WorldContextObject = nullptr;
			 FVector Center;
			 FVector Extent;
			 FLinearColor LineColor;
			 FRotator Rotation;
			float Duration = INFINITY;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Extent = Extent;
		params.LineColor = LineColor;
		params.Rotation = Rotation;
		params.Duration = Duration;
		ProcessEvent(defaultObj, fn, &params);
	}
	static void DrawDebugArrow(UObject* WorldContextObject, const FVector& LineStart, const FVector& LineEnd, float ArrowSize, const FLinearColor& LineColor, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			class UObject* WorldContextObject = nullptr;
			struct FVector LineStart;
			struct FVector LineEnd;
			float ArrowSize = 1.f;
			struct FLinearColor LineColor;
			float Duration = 1.f;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.LineStart = LineStart;
		params.LineEnd = LineEnd;
		params.ArrowSize = ArrowSize;
		params.LineColor = LineColor;
		params.Duration = Duration;

		ProcessEvent(defaultObj, fn, &params);
	}
};

struct UCrewFunctions {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Athena.CrewFunctions");
	}
	static bool AreCharactersInSameCrew(ACharacter* Player1, ACharacter* Player2) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.CrewFunctions.AreCharactersInSameCrew");
		struct
		{
			ACharacter* Player1;
			ACharacter* Player2;
			bool ReturnValue;
		} params;
		params.Player1 = Player1;
		params.Player2 = Player2;
		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}
};

struct UPlayer {
	char UnknownData00[0x30];
	AController* PlayerController;
};

struct UGameInstance {
	char UnknownData00[0x38];
	TArray<UPlayer*> LocalPlayers; // 0x38
};

struct ULevel {
	char UnknownData00[0xA0];
	TArray<ACharacter*> AActors;
};

struct APlayerManagerService {
	char UnknownData00[0x470];
	TArray<struct FWeakActorHandle> AllPlayerControllers;  // 0x470
};

struct AGameState {
	char UnknownData00[0x470];
	FName MatchState; // 0x470
	char pad[0x10]; // 0x478
	TArray<APlayerState*> PlayerArray; // 0x488 
	char UnknownData01[0x1B0]; // 0x498
	APlayerManagerService* PlayerManagerService;
};

struct UWorld {
	static inline UWorld** GWorld = nullptr;
	char pad[0x30]; // 0x0
	ULevel* PersistentLevel; // 0x30
	char pad_0028[0x20];  // 0x38
	AAthenaGameState* GameState; //0x0058
	char pad_0060[0xF0]; //0x0060
	TArray<ULevel*> Levels; //0x0150
	char pad_0160[80]; //0x0160
	ULevel* CurrentLevel; //0x01B0
	char pad_01B8[8]; //0x01B8
	UGameInstance* GameInstance; //0x01C0
};