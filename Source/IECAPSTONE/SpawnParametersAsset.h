// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityZone.h"
#include "Engine/DataAsset.h"
#include "Detail.h"
#include "SpawnParametersAsset.generated.h"

class ABuilding;
class ADetail;

UENUM()
enum class EBuildingSpawnType
{
	Standard,
	Compound
};

USTRUCT(BlueprintType)
struct FBuildingGroups
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building")
		TArray<TSubclassOf<ABuilding>> BuildingList;
};

USTRUCT(BlueprintType)
struct FDetailGroups
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Detail")
		TArray<int> DetailIndexList;
};

USTRUCT(BlueprintType)
struct FCityZoneSpawnParameters
{
	GENERATED_BODY()

public:
	const TArray<TSubclassOf<ABuilding>>& GetRandomBuildingGroup() const;
	const TArray<TSubclassOf<ABuilding>>& GetBuildingGroup(int index) const;

	const TArray<int> GetRandomDetailGroup() const;

	const EBuildingSpawnType GetRandomSpawnType() const;

	bool CheckIfCanSpawnSidewalk();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Group")
		TArray<FBuildingGroups> BuildingGroupArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Detail Group")
		TArray<FDetailGroups> DetailGroupArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spawn Building Type")
		TArray<EBuildingSpawnType> SpawnTypeArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Miscellaneous")
		bool CanSpawnSidewalk;
};

/**
 * 
 */
UCLASS()
class IECAPSTONE_API USpawnParametersAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	const FCityZoneSpawnParameters GetSpawnParameterOfCityZoneType(ECityZoneType CityZoneType) const;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spawn Parameters")
		FCityZoneSpawnParameters LowRiseResidentialSpawnParameters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spawn Parameters")
		FCityZoneSpawnParameters HighRiseResidentialSpawnParameters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spawn Parameters")
		FCityZoneSpawnParameters LowRiseCommercialSpawnParameters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spawn Parameters")
		FCityZoneSpawnParameters HighRiseCommercialSpawnParameters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spawn Parameters")
		FCityZoneSpawnParameters IndustrialSpawnParameters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spawn Parameters")
		FCityZoneSpawnParameters LandmarkSpawnParameters;

	const FCityZoneSpawnParameters EmptySpawnParameters;
};