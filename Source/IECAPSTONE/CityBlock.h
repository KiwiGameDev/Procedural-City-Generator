// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoadEdge.h"
#include "RoadNetwork.h"
#include "GameFramework/Actor.h"
#include "CityBlock.generated.h"

class UProceduralMeshComponent;
struct FRoadHalfEdge;
class ACityLot;
class ARoadNode;
class ARoadNetwork;

UCLASS()
class IECAPSTONE_API ACityBlock : public AActor
{
	GENERATED_BODY()
	
public:
	ACityBlock();
	
	void InjectRoads(const TArray<FRoadHalfEdge*>& NewRoadHalfEdges, const TArray<ARoadNode*>& NewRoadNodes);
	void RemoveDisconnectedRoads();
	void MergeSameRoadHalfEdges();
	void SortRoadHalfEdgesInPairs();

	void Initialize(ARoadNetwork* RoadNetwork);
	void InitializeRoadHalfEdges();
	void InitializeCityLots();
	void RemoveStrayRoadsInCityLots();
	void TriangulateCityLots();
	void RemoveSmallCityLots();
	void InitializeCityZones(ARoadNetwork* RoadNetwork);
	void SpawnCityLotFloors();

	void SpawnBuildings(int32 depth, ARoadNetwork* network);
	void callSpawnDetails(ARoadNetwork* roadNetwork, float spawnDensity);
	void SortRoadHalfEdgesInRoadIDs();

	void ToggleDataTextureMap();
	void ToggleAllCityLotsZoningColor();
	void ShowAllCityLotsZoningColor();
	void HideAllCityLotsZoningColor();

	void AddCityLot(ACityLot* CityLot);
	void AddRoadHalfEdge(FRoadHalfEdge* RoadHalfEdge);
	void AddRoadNode(ARoadNode* RoadNode);
	
	void RemoveCityLot(ACityLot* CityLot);
	void DestroyRoadHalfEdge(FRoadHalfEdge* RoadHalfEdge);
	void DestroyRoadNode(ARoadNode* RoadNode);
	void DestroyRoadNodeAndConnectedHalfEdges(ARoadNode* RoadNode);

	ACityBlock* DeepCopyRoadsAsCityBlock() const;
	TTuple<TArray<FRoadHalfEdge*>, TArray<ARoadNode*>> DeepCopyRoads() const;

	const TDoubleLinkedList<ACityLot*>& GetAllCityLots() const;
	const TArray<FRoadHalfEdge*>& GetAllRoadHalfEdges() const;
	const TArray<ARoadNode*>& GetAllRoadNodes() const;
	FBox GetCityBounds();
	bool IsCityLotsInitialized() const;
	bool IsCityCityLotZoneColorsVisible() const;

	virtual void Destroyed() override;
	void DestroyAllCityLots();
	void DestroyAllSubobjects();

private:
	void RemoveDisconnectedRoadsRecursive(ARoadNode* RoadNode, TSet<ARoadNode*>& RoadSet);

	UPROPERTY(EditAnywhere, Category="Road Network")
	TSubclassOf<ACityLot> CityLotBlueprint;

	UPROPERTY(EditAnywhere, Category="Road Network")
	UTextureRenderTarget2D* CityZoningRenderTarget;
	
	UPROPERTY(EditAnywhere, Category="Road Network")
	UMaterialInterface* NoiseGenerationMaterial;

	UPROPERTY(VisibleAnywhere, Category="Road Network")
	bool bIsCityBoundsDirty = true;
	
	UPROPERTY(VisibleInstanceOnly, Category="Road Network")
	FBox CityBounds;
	
	UPROPERTY(EditAnywhere, Category="Road Network")
	UMaterialInterface* CityZoningVisualisationMaterial;
	
	UPROPERTY(EditAnywhere, Category="Road Network")
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(VisibleAnywhere, Category="Road Network")
	bool bIsCityLotsInitialized = false;

	UPROPERTY(VisibleAnywhere, Category="Road Network")
	bool bIsCityLotZoneColorsVisible = false;

	TDoubleLinkedList<ACityLot*> CityLotsList;
	TArray<FRoadHalfEdge*> RoadHalfEdgesArray;
	TArray<ARoadNode*> RoadNodesArray;
};
