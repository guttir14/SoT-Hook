#pragma once
#include <Windows.h>
#include <UE4/UE4.h>
#include <string>

#ifdef _MSC_VER
#pragma pack(push, 0x8)
#endif

struct FNameEntry
{
	uint32_t Index;
	uint32_t pad;
	FNameEntry* HashNext;
	char AnsiName[1024];

	const int GetIndex() const { return Index >> 1; }
	const char* GetAnsiName() const { return AnsiName; }
};

class TNameEntryArray
{
public:

	bool IsValidIndex(uint32_t index) const { return index < NumElements; }

	FNameEntry const* GetById(uint32_t index) const { return *GetItemPtr(index); }

	FNameEntry const* const* GetItemPtr(uint32_t Index) const {
		const auto ChunkIndex = Index / 16384;
		const auto WithinChunkIndex = Index % 16384;
		const auto Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	FNameEntry** Chunks[128];
	uint32_t NumElements = 0;
	uint32_t NumChunks = 0;
};

struct FName
{
	int ComparisonIndex = 0;
	int Number = 0;

	static inline TNameEntryArray* GNames = nullptr;

	static const char* GetNameByIdFast(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	static std::string GetNameById(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	}

	const char* GetNameFast() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	const std::string GetName() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	};

	inline bool operator==(const FName& other) const {
		return ComparisonIndex == other.ComparisonIndex;
	};

	FName() {}

	FName(const char* find) {
		for (auto i = 6000u; i < GNames->NumElements; i++)
		{
			auto name = GetNameByIdFast(i);
			if (!name) continue;
			if (strcmp(name, find) == 0) {
				ComparisonIndex = i;
				return;
			};
		}
	}
};

struct FUObjectItem
{
	class UObject* Object;
	int Flags;
	int ClusterIndex;
	int SerialNumber;
	int pad;
};

struct TUObjectArray
{
	FUObjectItem* Objects;
	int MaxElements;
	int NumElements;

    class UObject* GetByIndex(int index) { return Objects[index].Object; }
};

class UClass;
class UObject
{
public:
	UObject(UObject* addr) { *this = addr; }
	static inline TUObjectArray* GObjects = nullptr;
	void* Vtable; // 0x0
	int ObjectFlags; // 0x8
	int InternalIndex; // 0xC
	UClass* Class; // 0x10
	FName Name; // 0x18
	UObject* Outer; // 0x20

	std::string GetName() const;

	const char* GetNameFast() const;

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
	static T* GetObjectCasted(uint32_t index)
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
	int ArrayDim;
	int ElementSize;
	uint64_t PropertyFlags;
	char pad[0xC];
	int Offset_Internal;
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
	int PropertySize;
	int MinAlignment;
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
	int FunctionFlags;
	uint16_t RepOffset;
	uint8_t NumParms;
	char pad;
	uint16_t ParmsSize;
	uint16_t ReturnValueOffset;
	uint16_t RPCId;
	uint16_t RPCResponseId;
	UProperty* FirstPropertyToInit;
	UFunction* EventGraphFunction; //0x00A0
	int EventGraphCallOffset;
	char pad_0x00AC[0x4]; //0x00AC
	void* Func; //0x00B0
};


inline void ProcessEvent(void* obj, UFunction* function, void* parms) 
{
	auto vtable = *reinterpret_cast<void***>(obj);
	reinterpret_cast<void(*)(void*, UFunction*, void*)>(vtable[59])(obj, function, parms);
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
		Max = Count = *other ? static_cast<int>(std::wcslen(other)) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};
	FString(const wchar_t* other, int count)
	{
		Data = const_cast<wchar_t*>(other);;
		Max = Count = count;
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


enum class EPlayerActivityType : uint8_t
{
	None = 0,
	Bailing = 1,
	Cannon = 2,
	Cannon_END = 3,
	Capstan = 4,
	Capstan_END = 5,
	CarryingBooty = 6,
	CarryingBooty_END = 7,
	Dead = 8,
	Dead_END = 9,
	Digging = 10,
	Dousing = 11,
	EmptyingBucket = 12,
	Harpoon = 13,
	Harpoon_END = 14,
	LoseHealth = 15,
	Repairing = 16,
	Sails = 17,
	Sails_END = 18,
	Wheel = 19,
	Wheel_END = 20
};

struct FPirateDescription
{
	
};

struct APlayerState {
	char pad[0x0478]; // 0x0
	FString PlayerName; // 0x0478

	EPlayerActivityType GetPlayerActivity()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaPlayerState.GetPlayerActivity");
		EPlayerActivityType activity;
		ProcessEvent(this, fn, &activity);
		return activity;
	}

	FPirateDescription GetPirateDesc()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaPlayerState.GetPirateDesc");
		FPirateDescription desc;
		ProcessEvent(this, fn, &desc);
		return desc;
	}
	
};

struct FMinimalViewInfo {
	FVector Location;
	FRotator Rotation;
	char UnknownData00[0x10];
	float FOV;
};

struct FCameraCacheEntry {
	float TimeStamp;
	char pad[0xC];
	FMinimalViewInfo POV;
};

struct APlayerCameraManager {
	char pad[0x04D0];
	FCameraCacheEntry CameraCache;

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


struct FKey
{
	FName KeyName;
	unsigned char UnknownData00[0x18] = {};

	FKey() {};
	FKey(const char* InName) : KeyName(FName(InName)) {}
};

struct AController {

	char pad_0000[0x0488]; //0x0000
	class ACharacter* Character; //0x0488
	char pad_0480[0x70]; // 0x490
	APlayerCameraManager* PlayerCameraManager; //0x0500
	char pad_04f8[0x1049]; // 0x0508
	bool IdleDisconnectEnabled; // 0x1551(0x0001)

	void SendToConsole(FString& cmd){
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.SendToConsole");
		ProcessEvent(this, fn, &cmd);
	}

	bool ProjectWorldLocationToScreen(const FVector& WorldLocation, FVector2D& ScreenLocation) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
		struct
		{
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool ReturnValue = false;
		} params;

		params.WorldLocation = WorldLocation;
		ProcessEvent(this, fn, &params);
		ScreenLocation = params.ScreenLocation;
		return params.ReturnValue;
	}

	bool WasInputKeyJustPressed(const FKey& Key) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.WasInputKeyJustPressed");
		struct
		{
			FKey Key;
			bool ReturnValue = false;
		} params;

		params.Key = Key;
		ProcessEvent(this, fn, &params);
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

	bool LineOfSightTo(ACharacter* Other, const FVector& ViewPoint, const bool bAlternateChecks ) {
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
	float GetMaxHealth() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HealthComponent.GetMaxHealth");
		float health = 0.f;
		ProcessEvent(this, fn, &health);
		return health;
	}
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
	int CurrentEditableSpaceBases;
	int CurrentReadSpaceBases;

	FName GetBoneName(int BoneIndex)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.SkinnedMeshComponent.GetBoneName");
		struct
		{
			int BoneIndex = 0;
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



	bool GetBone(const uint32_t id, const FMatrix& componentToWorld, FVector& pos) {
		auto bones = SpaceBasesArray[CurrentReadSpaceBases];
		if (id >= bones.Count) return false;
		const auto& bone = bones[id];
		auto boneMatrix = bone.ToMatrixWithScale();
		auto world = boneMatrix * componentToWorld;
		pos = { world.M[3][0], world.M[3][1], world.M[3][2] };
		return true;
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
	char pad[0x04B8];
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

struct AFauna {
	char pad1[0x0898];
	FString* DisplayName; // 0x0898
};

enum class ESwimmingCreatureType : uint8_t
{
	SwimmingCreature = 0,
	Shark = 1,
	TinyShark = 2,
	Siren = 3,
	ESwimmingCreatureType_MAX = 4
};

struct ASharkPawn {
	char pad1[0x0550];
	USkeletalMeshComponent* Mesh; // 0x0550
	char pad2[0x5C]; // 0x0558
	ESwimmingCreatureType SwimmingCreatureType; // 0x05B4
};


struct FAIEncounterSpecification
{
	char pad[0x80];
	FString* LocalisableName; // 0x0080
};


struct UItemDesc {
	char pad[0x0028];
	FString* Title; // 0x0028(0x0038)
};

struct AItemInfo {
	char pad[0x04C8];
	UItemDesc* Desc; // 0x04C8
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
	char pad[0x0054];
	float ProjectileMaximumRange;  // 0x0054
	char pad2[0x18]; // 0x58
	FWeaponProjectileParams AmmoParams; // 0x0070
};

struct AProjectileWeapon {
	char pad[0x0848]; // 0
	FProjectileWeaponParameters WeaponParameters; // 0x0848
};

struct UWorldMapIslandDataAsset {
	char pad[0x48];
	FVector WorldSpaceCameraPosition; // 0x48
};

struct UIslandDataAssetEntry {
	char pad[0x40];
	UWorldMapIslandDataAsset* WorldMapData; // 0x0040
	char pad2[0x68]; // 0x48
	FString* LocalisedName; // 0x00B0
};

struct UIslandDataAsset
{
	char pad[0x0048];
	TArray<UIslandDataAssetEntry*> IslandDataEntries; // 0x0048
};

struct AIslandService {
	char pad[0x04E8];
	UIslandDataAsset* IslandDataAsset; // 0x04E8
};

struct ASlidingDoor {
	char pad_0x0[0x05CC];
	FVector InitialDoorMeshLocation; // 0x0514
	void OpenDoor() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.SkeletonFortDoor.OpenDoor");
		ProcessEvent(this, fn, nullptr);
	}
};

struct USceneComponent {
	FVector K2_GetComponentLocation() {
		FVector location;
		static auto fn = UObject::FindObject<UFunction>("Function Engine.SceneComponent.K2_GetComponentLocation");
		ProcessEvent(this, fn, &location);
		return location;
	}
};

struct APuzzleVault {
	char pad[0x1090];
	ASlidingDoor* OuterDoor; // 0x1080
};

struct FCrew
{
	char pad[0x20];
	TArray<APlayerState*> Players;                                                  // 0x0020(0x0010) (ZeroConstructor)
	char pad2[0x50];
};

struct ACrewService {
	char pad[0x0638];
	TArray<FCrew> Crews; // 0x0638
};

struct AShipService
{
	int GetNumShips()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipService.GetNumShips");
		int num;
		ProcessEvent(this, fn, &num);
		return num;
	}
};

struct AKrakenService
{
	char pad[0x05B8];
	class AKraken* Kraken; // 0x05B8(0x0008)
	bool IsKrakenActive() {
		static auto fn = UObject::FindObject<UFunction>("Function Kraken.KrakenService.IsKrakenActive");
		bool isActive;
		ProcessEvent(this, fn, &isActive);
		return isActive;
	}

	void RequestKrakenWithLocation(const FVector& SpawnLocation, ACharacter* SpawnedForActor) {
		static auto fn = UObject::FindObject<UFunction>("Function Kraken.KrakenService.RequestKrakenWithLocation");
		struct {
			FVector SpawnLocation;
			ACharacter* SpawnedForActor = nullptr;
		} params;
		params = { SpawnLocation, SpawnedForActor };
		ProcessEvent(this, fn, &params);
	}

	void DismissKraken()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Kraken.KrakenService.DismissKraken");
		ProcessEvent(this, fn, nullptr);
	}
};

struct AAthenaGameState {
	char pad[0x0658];
	UINT64* WindService; // 0x0658
	UINT64* PlayerManagerService; // 0x0660
	AShipService* ShipService; // 0x0668
	UINT64* WatercraftService; // 0x0670
	UINT64* TimeService; // 0x0678
	UINT64* WaterService; // 0x0680
	UINT64* StormService; // 0x0688
	ACrewService* CrewService; // 0x0690
	UINT64* ContestZoneService;
	UINT64* ContestRowboatsService;
	class AIslandService* IslandService; // 0x06A8(0x0008)
	class ANPCService* NPCService; // 0x06B0(0x0008)
	class ASkellyFortService* SkellyFortService; // 0x06B8(0x0008)
	class AAIDioramaService* AIDioramaService; // 0x06C0(0x0008)
	class AAshenLordEncounterService* AshenLordEncounterService; // 0x06C8(0x0008)
	class AAggressiveGhostShipsEncounterService* AggressiveGhostShipsEncounterService; // 0x06D0(0x0008)
	class ATallTaleService* TallTaleService; // 0x06D8(0x0008)
	class AAIShipObstacleService* AIShipObstacleService; // 0x06E0(0x0008) 
	class AAIShipService* AIShipService; // 0x06E8(0x0008)
	class AAITargetService* AITargetService; // 0x06F0(0x0008)
	class UShipLiveryCatalogueService* ShipLiveryCatalogueService; // 0x06F8(0x0008)
	class AContestManagerService* ContestManagerService; // 0x0700(0x0008)
	class ADrawDebugService* DrawDebugService; // 0x0708(0x0008)
	class AWorldEventZoneService* WorldEventZoneService; // 0x0710(0x0008)
	class UWorldResourceRegistry* WorldResourceRegistry; // 0x0718(0x0008)
	class AKrakenService* KrakenService; // 0x0720(0x0008) 
	class UPlayerNameService* PlayerNameService; // 0x0728(0x0008)
	class ATinySharkService* TinySharkService; // 0x0730(0x0008)
	class AProjectileService* ProjectileService; // 0x0738(0x0008)
	class UServerNotificationsService* ServerNotificationsService; // 0x0740(0x0008)
	class AAIManagerService* AIManagerService; // 0x0748(0x0008)
	class AAIEncounterService* AIEncounterService; // 0x0750(0x0008)
	class AAIEncounterGenerationService* AIEncounterGenerationService; // 0x0758(0x0008)
	class UEncounterService* EncounterService; // 0x0760(0x0008)
	class UGameEventSchedulerService* GameEventSchedulerService; // 0x0768(0x0008)
	class UHideoutService* HideoutService; // 0x0770(0x0008)
	class UAthenaStreamedLevelService* StreamedLevelService; // 0x0778(0x0008) 
	class ULocationProviderService* LocationProviderService; // 0x0780(0x0008)
	class AHoleService* HoleService; // 0x0788(0x0008) 
	class ULoadoutService* LoadoutService; // 0x0790(0x0008)
	class UOcclusionService* OcclusionService; // 0x0798(0x0008)
	class UPetsService* PetsService; // 0x07A0(0x0008) 

};

struct UCharacterMovementComponent {
	FVector GetCurrentAcceleration() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.CharacterMovementComponent.GetCurrentAcceleration");
		FVector acceleration;
		ProcessEvent(this, fn, &acceleration);
		return acceleration;
	}
};


struct AHarpoonLauncher {
	char pad[0x0BA0];
	FRotator rotation;

	void Server_RequestAim(float InPitch, float InYaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HarpoonLauncher.Server_RequestAim");
		struct {
			float InPitch;
			float InYaw;
		} params;
		params.InPitch = InPitch;
		params.InYaw = InYaw;
		ProcessEvent(this, fn, &params);
	}
};


class ACharacter : public UObject {
public:
	
	char pad1[0x468]; // 0x28
	APlayerState* PlayerState;  // 0x0490
	char pad2[0x10]; // 0x0498
	AController* Controller; // 0x04A8
	char pad3[0x28]; // 0x4B0
	USkeletalMeshComponent* Mesh; // 0x04D8
	UCharacterMovementComponent* CharacterMovement; // 0x04E0
	char pad4[0x3B8]; // 0x4E8
	UWieldedItemComponent* WieldedItemComponent; // 0x08A0
	char pad5[0x20]; // 0x08A8
	UHealthComponent* HealthComponent; // 0x08C8
	char pad6[0x428]; // 0x8D0
	UDrowningComponent* DrowningComponent; // 0x0CF8

	void ReceiveTick(float DeltaSeconds)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.ActorComponent.ReceiveTick");
		ProcessEvent(this, fn, &DeltaSeconds);
	}

	void GetActorBounds(bool bOnlyCollidingComponents, FVector& Origin, FVector& BoxExtent) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorBounds");
		struct
		{
			bool bOnlyCollidingComponents = false;
			FVector Origin;
			FVector BoxExtent;
		} params;

		params.bOnlyCollidingComponents = bOnlyCollidingComponents;

		ProcessEvent(this, fn, &params);

		Origin = params.Origin;
		BoxExtent = params.BoxExtent;
	}

	ACharacter* GetCurrentShip() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.GetCurrentShip");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}

	ACharacter* GetAttachParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetAttachParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	};

	ACharacter* GetWieldedItem() {
		if (!WieldedItemComponent) return nullptr;
		return WieldedItemComponent->CurrentlyWieldedItem;
	}

	FVector GetVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetVelocity");
		FVector velocity;
		ProcessEvent(this, fn, &velocity);
		return velocity;
	}

	AItemInfo* GetItemInfo()  {
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

	
	FRotator K2_GetActorRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorRotation");
		FRotator params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector K2_GetActorLocation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorLocation");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	inline bool isSkeleton() {
		static auto obj = UObject::FindClass("Class Athena.AthenaAICharacter"); 
		return IsA(obj);
	}

	inline bool isPlayer() {
		static auto obj = UObject::FindClass("Class Athena.AthenaPlayerCharacter");
		return IsA(obj);
	}
	inline bool isPuzzleVault() {
		static auto obj = UObject::FindClass("Class Athena.PuzzleVault");
		return IsA(obj);
	}
	inline bool isShip() {
		static auto obj = UObject::FindClass("Class Athena.Ship");
		return IsA(obj);
	}

	inline bool isHarpoon() {
		static auto obj = UObject::FindClass("Class Athena.HarpoonLauncher");
		return IsA(obj);
	}

	inline bool isFarShip() {
		static auto obj = UObject::FindClass("Class Athena.ShipNetProxy");
		return IsA(obj);
	}

	inline bool isItem() {
		static auto obj = UObject::FindClass("Class Athena.ItemProxy");
		return IsA(obj);
	}

	inline bool isShipwreck() {
		static auto obj = UObject::FindClass("Class Athena.Shipwreck");
		return IsA(obj);
	
	}

	inline bool isShark() {
		static auto obj = UObject::FindClass("Class Athena.SharkPawn");
		return IsA(obj);
	}

	inline bool isAnimal() {
		static auto obj = UObject::FindClass("Class AthenaAI.Fauna");
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
	bool isWorldSettings() {
		static auto obj = UObject::FindClass("Class Engine.WorldSettings");
		return IsA(obj);
	}
	bool isBuriedTreasure() {
		static auto obj = UObject::FindClass("Class Athena.BuriedTreasureLocation");
		return IsA(obj);
	}

	FAIEncounterSpecification GetAIEncounterSpec() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaAICharacter.GetAIEncounterSpec");
		FAIEncounterSpecification spec;
		ProcessEvent(this, fn, &spec);
		return spec;
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


#ifdef _MSC_VER
#pragma pack(pop)
#endif
