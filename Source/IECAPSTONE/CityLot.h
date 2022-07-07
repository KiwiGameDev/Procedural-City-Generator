// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityZone.h"
#include "LotComponent.h"
#include "CityLotSubdivision.h"
#include "CityLot.generated.h"

class UCityBuildingsDataAsset;
class UProceduralMeshComponent;
class ARoadNetwork;
class ACityBlock;
struct FRoadHalfEdge;
class ARoadNode;

USTRUCT(BlueprintType)
struct FBoundsAndMinMax
{
	GENERATED_BODY()
	
	float maxX;
	float maxY;
	float minX;
	float minY;
	TArray<FVector> BoundingBox;
};


USTRUCT(BlueprintType)
struct FInner
{
	GENERATED_BODY()

	FVector InnerPointPosition;
	TArray<int> InsetPolygonIndex;
};

UCLASS()
class IECAPSTONE_API ACityLot : public AActor
{
	GENERATED_BODY()
	
public:
	ACityLot();

	TArray<FRoadHalfEdge*> RemoveStrayRoads();
	void Triangulate();
	void SpawnFloor();
	void ShowCityZoningColor();
	void HideCityZoningColor();
	void SpawnBuildings(int32 depth, ARoadNetwork* network);
	void SpawnBuildingsInSubdivision(ARoadNetwork* network);

	// Building Spawning
	void InitializeInsetPolygon();
	FVector FindNearestPointOfPositionOnBorder(FVector Position);
	FVector GetFaceNormals(const FVector& Midpoint, const FVector& A, const FVector& B);
	void CreateSubdivisions(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions, int Depth, ARoadNetwork* network);
	TArray<FVector> CreateBoundingBoxFromLinkedList(TSharedPtr<TDoubleLinkedList<FVector>> const& nodes);
	TSharedPtr<FBoundsAndMinMax> CreateBoundingBoxFromArray(const TArray<FVector>& Positions);
	TArray<FVector> CreateOrientedBoundingBoxFromLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions);
	float GetMaxFloatFromLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& Nodes, bool bIsX);
	float GetMinFloatFromLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes, bool bIsX);
	TSharedPtr<TDoubleLinkedList<FVector>> ConvertVectorArrayToDoubleLinkedListSharedPtr(TArray<FVector> Positions);
	TSharedPtr<TDoubleLinkedList<FVector>> CreateDeepCopyOfLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes);
	TSharedPtr<TDoubleLinkedList<FVector>> CreateNewSharedPtrFromIntersectionSideA(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes, FVector pointAStart, FVector pointAEnd);
	TSharedPtr<TDoubleLinkedList<FVector>> CreateNewSharedPtrFromIntersectionSideB(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes, FVector pointAStart, FVector pointAEnd);
	void DrawDebugSharedPtrListVectors(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions);
	FVector GetIntersectionPointFromArray(const TArray<FVector>& Positions, FVector bisectorOrigin, FVector current);
	TArray<FVector> GetIntersectionPointsFromPolygon(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions, FVector PointAStart, FVector PointAEnd);
	FVector GetBuildingDirectionToNearestEdge(FVector Position);
	void TurnOffBuildingPhysics();
	void SpawnDetails(ARoadNetwork* roadNetwork, float spawnDensity);
	bool CheckPointIfVeryClose(FVector point, TArray<FVector> positions);
	
	void OnTopologyChanged();
	
	void DebugDrawCityZoneMesh(UWorld* World) const;

	void SetCityBlock(ACityBlock* NewCityBlock);
	void SetBorderingHalfEdges(const TArray<FRoadHalfEdge*>& NewBorderingRoadHalfEdgesArray);
	void SetCityZoneType(ECityZoneType NewCityZoneType);
	
	FVector GetMidpointFromLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions);
	const TArray<FRoadHalfEdge*>& GetAllBorderingRoadHalfEdges() const;
	TArray<FVector> GetBorderingRoadNodesPositionsArray() const;
	TArray<FVector> GetInsetPositionsArray();
	ECityZoneType GetCityZoneType() const;
	float GetArea();
	FVector GetCentroidPosition() const;
	bool IsTriangulated() const;
	bool IsHalfEdgesCountValid() const;
	bool IsHalfEdgesCycleValid() const;
	bool IsPointInsidePolygon(const TArray<FVector>& PolygonPositions, const FVector& Point);

	void SpawnSidewalkFromComponent(ARoadNetwork* RoadNetwork);
	void SemanticSegmentationMode(UMaterialInterface* SidewalkSemanticMaterial, UMaterialInterface* FenceSemanticMaterial);
	
	virtual void Destroyed() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lot Component")
	ULotComponent* LotComponent;

	void ClearSubdivisions();

private:
	UPROPERTY(EditAnywhere)
	TArray<UMaterialInterface*> ZoneMaterials;
	
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ZoneColorProceduralMeshComponent;
	
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ZoneFloorProceduralMeshComponent;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* ZoneFloorMaterial;
	
	UPROPERTY(VisibleInstanceOnly)
	ACityBlock* CityBlock;
	
	UPROPERTY(VisibleInstanceOnly)
	TArray<AActor*> BuildingsSpawned;
	
	// Mesh Triangulation
	UPROPERTY(VisibleInstanceOnly)
	bool bIsTriangulated = false;
	
	UPROPERTY(VisibleInstanceOnly)
	TArray<FVector> RoadNodesPosArray;
	
	UPROPERTY(VisibleInstanceOnly)
	TArray<int32> TriangleIndicesArray;
	
	TArray<FRoadHalfEdge*> BorderingRoadHalfEdgesArray;

	FVector zOffset = FVector(0, 0, 300);
	ECityZoneType CityZoneType;
	int MAX_DEPTH = 6;
	TArray<FVector> InsetPositions;

	TArray<CityLotSubdivision*> Subdivisions;
	TQueue<TSharedPtr<TDoubleLinkedList<FVector>>> SubdivisionQueue;

	bool isVillageCompound;
	EBuildingSpawnType spawnType;

	// Semantic Segmentation
	bool bIsSemanticSegmentation = false;
};
