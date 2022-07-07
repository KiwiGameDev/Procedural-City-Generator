// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityBuildingsDataAsset.h"
#include "Components/ActorComponent.h"
#include "CityZone.h"
#include "RoadNetwork.h"
#include "SidewalkActor.h"
#include "FenceActor.h"
#include "LotComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class IECAPSTONE_API ULotComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	ULotComponent();

	void SpawnBuildingFromComponent(FVector Position, FRotator Rotation, ECityZoneType CityZoneType, ARoadNetwork* Network);
	void SpawnDetailFromComponent(int detailIndex, FVector Position, FRotator Rotation, ARoadNetwork* network);
	void SpawnSidewalkFromComponent(TArray<FRoadHalfEdge*>const& edgeList,TArray<FVector> inset, ARoadNetwork* roadnetwork);
	void SpawnFenceFromComponent(TArray<FRoadHalfEdge*>const& edgeList,TArray<FVector> inset, ARoadNetwork* RoadNetwork);

	void SemanticSegmentationMode(UMaterialInterface* SidewalkSemanticMaterial, UMaterialInterface* FenceSemanticMaterial);
	
	void ClearAllSpawnedBuildingsInLotComponent();
	void TurnOffAllPhysicsAfterSpawn();
	
	UFUNCTION(BlueprintCallable)
	void CleanupOverlappingActors();

	void ClearAllSidewalksInLotComponent();
	void ClearAllFencesInLotComponent();

	void DestroyAll();
	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Buildings")
	TArray<AActor*> BuildingsSpawnedArray;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Buildings")
	UCityBuildingsDataAsset* CityBuildingsDataAsset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Details")
	UDetailsDataAsset* DetailsDataAsset;

	TArray<AActor*> SidewalkSpawnedArray;
	TArray<AActor*> FencesSpawnedArray;
};
