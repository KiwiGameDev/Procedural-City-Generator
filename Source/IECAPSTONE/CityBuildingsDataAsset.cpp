// Fill out your copyright notice in the Description page of Project Settings.

#include "CityBuildingsDataAsset.h"

const TArray<TSubclassOf<ABuilding>>& UCityBuildingsDataAsset::GetBuildingsOfCityZoneType(ECityZoneType CityZoneType) const
{
	if (CityZoneType == ECityZoneType::LowRiseResidential)
	{
		return LowRiseResidentialBuildingsArray;
	}

	if (CityZoneType == ECityZoneType::HighRiseResidential)
	{
		return HighRiseResidentialBuildingsArray;
	}

	if (CityZoneType == ECityZoneType::LowRiseCommercial)
	{
		return LowRiseCommercialBuildingsArray;
	}

	if (CityZoneType == ECityZoneType::HighRiseCommercial)
	{
		return HighRiseCommercialBuildingsArray;
	}

	if (CityZoneType == ECityZoneType::Industrial)
	{
		return IndustrialBuildingsArray;
	}

	if (CityZoneType == ECityZoneType::Landmark)
	{
		return LandmarkBuildingsArray;
	}

	return EmptyBuildingArray;
}

const TArray<TSubclassOf<ABuilding>>& UCityBuildingsDataAsset::GetLowRiseResidentialBuildings() const
{
	return LowRiseResidentialBuildingsArray;
}

const TArray<TSubclassOf<ABuilding>>& UCityBuildingsDataAsset::GetHighRiseResidentialBuildings() const
{
	return HighRiseResidentialBuildingsArray;
}

const TArray<TSubclassOf<ABuilding>>& UCityBuildingsDataAsset::GetLowRiseCommercialBuildings() const
{
	return LowRiseCommercialBuildingsArray;
}

const TArray<TSubclassOf<ABuilding>>& UCityBuildingsDataAsset::GetHighRiseCommercialBuildings() const
{
	return HighRiseCommercialBuildingsArray;
}

const TArray<TSubclassOf<ABuilding>>& UCityBuildingsDataAsset::GetIndustrialBuildings() const
{
	return IndustrialBuildingsArray;
}

const TArray<TSubclassOf<ABuilding>>& UCityBuildingsDataAsset::GetLandmarkBuildings() const
{
	return LandmarkBuildingsArray;
}
