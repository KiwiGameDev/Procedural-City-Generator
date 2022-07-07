// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityZone.h"
#include "Engine/DataAsset.h"
#include "CityBuildingsDataAsset.generated.h"

class ABuilding;

UCLASS()
class IECAPSTONE_API UCityBuildingsDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	const TArray<TSubclassOf<ABuilding>>& GetBuildingsOfCityZoneType(ECityZoneType CityZoneType) const;
	const TArray<TSubclassOf<ABuilding>>& GetLowRiseResidentialBuildings() const;
	const TArray<TSubclassOf<ABuilding>>& GetHighRiseResidentialBuildings() const;
	const TArray<TSubclassOf<ABuilding>>& GetLowRiseCommercialBuildings() const;
	const TArray<TSubclassOf<ABuilding>>& GetHighRiseCommercialBuildings() const;
	const TArray<TSubclassOf<ABuilding>>& GetIndustrialBuildings() const;
	const TArray<TSubclassOf<ABuilding>>& GetLandmarkBuildings() const;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "City Buildings")
	TArray<TSubclassOf<ABuilding>> LowRiseResidentialBuildingsArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "City Buildings")
	TArray<TSubclassOf<ABuilding>> HighRiseResidentialBuildingsArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "City Buildings")
	TArray<TSubclassOf<ABuilding>> LowRiseCommercialBuildingsArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "City Buildings")
	TArray<TSubclassOf<ABuilding>> HighRiseCommercialBuildingsArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "City Buildings")
	TArray<TSubclassOf<ABuilding>> IndustrialBuildingsArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "City Buildings")
	TArray<TSubclassOf<ABuilding>> LandmarkBuildingsArray;

	const TArray<TSubclassOf<ABuilding>> EmptyBuildingArray;
};
