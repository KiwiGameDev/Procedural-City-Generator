// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnParametersAsset.h"

const TArray<TSubclassOf<ABuilding>>& FCityZoneSpawnParameters::GetRandomBuildingGroup() const
{
	return BuildingGroupArray[rand() % BuildingGroupArray.Num()].BuildingList;
}

const TArray<TSubclassOf<ABuilding>>& FCityZoneSpawnParameters::GetBuildingGroup(int index) const
{
	return BuildingGroupArray[index].BuildingList;
}

const TArray<int> FCityZoneSpawnParameters::GetRandomDetailGroup() const
{
	return DetailGroupArray[rand() % DetailGroupArray.Num()].DetailIndexList;
}

const EBuildingSpawnType FCityZoneSpawnParameters::GetRandomSpawnType() const
{
	return SpawnTypeArray[rand() % SpawnTypeArray.Num()];
}

bool FCityZoneSpawnParameters::CheckIfCanSpawnSidewalk()
{
	return CanSpawnSidewalk;
}

const FCityZoneSpawnParameters USpawnParametersAsset::GetSpawnParameterOfCityZoneType(ECityZoneType CityZoneType) const {
	if (CityZoneType == ECityZoneType::LowRiseResidential)
	{
		return LowRiseResidentialSpawnParameters;
	}

	if (CityZoneType == ECityZoneType::HighRiseResidential)
	{
		return HighRiseResidentialSpawnParameters;
	}

	if (CityZoneType == ECityZoneType::LowRiseCommercial)
	{
		return LowRiseCommercialSpawnParameters;
	}

	if (CityZoneType == ECityZoneType::HighRiseCommercial)
	{
		return HighRiseCommercialSpawnParameters;
	}

	if (CityZoneType == ECityZoneType::Industrial)
	{
		return IndustrialSpawnParameters;
	}

	if (CityZoneType == ECityZoneType::Landmark)
	{
		return LandmarkSpawnParameters;
	}

	return EmptySpawnParameters;
}