// Fill out your copyright notice in the Description page of Project Settings.


#include "CityLotSubdivision.h"

CityLotSubdivision::CityLotSubdivision(const TSharedPtr<TDoubleLinkedList<FVector>>& NewSubdivisonVerts,
									   TArray<FVector> NewBoundingBoxVerts,
									   ECityZoneType NewCityZoneType,
									   FVector NewBuildingSpawnPosition,
									   FVector NewBuildingSpawnNormals,
									   FRotator NewBuildingSpawnRotation)
{
	CreateDeepCopyOfLinkedList(NewSubdivisonVerts);
	
	BoundingBoxVertices = NewBoundingBoxVerts;
	BuildingSpawnPosition = NewBuildingSpawnPosition;
	BuildingSpawnNormals = NewBuildingSpawnNormals;
	BuildingSpawnRotation = NewBuildingSpawnRotation;
	CityZoneType = NewCityZoneType;
}

CityLotSubdivision::~CityLotSubdivision()
{
	
}

void CityLotSubdivision::CreateDeepCopyOfLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes)
{
	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIterator(nodes->GetHead()); listIterator; ++listIterator)
	{
		SubdivisionVertices.AddTail(listIterator.GetNode()->GetValue());
	}
}

void CityLotSubdivision::SpawnBuilding(ULotComponent* lotComponent, ARoadNetwork* network)
{
	BuildingSpawnPosition -= BuildingSpawnNormals * 5.0f;

	lotComponent->SpawnBuildingFromComponent(BuildingSpawnPosition, BuildingSpawnRotation, CityZoneType, network);
}