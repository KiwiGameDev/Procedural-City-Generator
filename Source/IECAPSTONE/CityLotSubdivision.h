// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityZone.h"
#include "LotComponent.h"

class ARoadNetwork;

class IECAPSTONE_API CityLotSubdivision
{
public:
	CityLotSubdivision(const TSharedPtr<TDoubleLinkedList<FVector>>& NewSubdivisonVerts,
					   TArray<FVector> NewBoundingBoxVerts,
					   ECityZoneType NewCityZoneType,
					   FVector NewBuildingSpawnPosition,
					   FVector NewBuildingSpawnNormals,
					   FRotator NewBuildingSpawnRotation);
	~CityLotSubdivision();

	void CreateDeepCopyOfLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes);

	void SpawnBuilding(ULotComponent* lotComponent, ARoadNetwork* network);

private:
	TDoubleLinkedList<FVector> SubdivisionVertices;
	TArray<FVector> BoundingBoxVertices;

	FVector BuildingSpawnPosition;
	FVector BuildingSpawnNormals;
	FRotator BuildingSpawnRotation;

	ECityZoneType CityZoneType;
};