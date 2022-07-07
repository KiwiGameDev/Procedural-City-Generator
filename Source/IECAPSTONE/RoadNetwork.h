// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoadType.h"
#include "StreetMap.h"
#include "Building.h"
#include "CityZone.h"
#include "CityBuildingsDataAsset.h"
#include "SpawnParametersAsset.h"
#include "DetailsDataAsset.h"
#include "Engine/DecalActor.h"
#include "Detail.h"
#include "GameFramework/Actor.h"
#include "RoadNetwork.generated.h"

class UProceduralMeshComponent;
class UStreetMap;
class ABuilding;
class ACityBlock;
class ACityLot;
struct FRoadHalfEdge;
struct FCityZoneSpawnParameters;
class ARoadNode;
class ARoadActor;

USTRUCT(BlueprintType)
struct FRoadMap
{
	GENERATED_BODY()
	
	TArray<FRoadHalfEdge*> HalfEdges;
};

UCLASS()
class IECAPSTONE_API ARoadNetwork : public AActor
{
	GENERATED_BODY()

public:	
	ARoadNetwork();

	TArray<ABuilding*> RetrieveBuildingsFromCityZone(ECityZoneType CityZoneType);
	const TArray<int> RetrieveRandomDetailsGroupFromCityZone(ECityZoneType CityZoneType);
	const EBuildingSpawnType RetrieveRandomSpawnTypeFromCityZone(ECityZoneType CityZoneType);
	const TArray<ADetail*>& RetrieveDetails();
	TSubclassOf<AActor> GetSidewalkActor() { return this->SidewalkActor; };
	TSubclassOf<AActor> GetFenceActor() { return this->FenceActor; };
	float GetPopulationDensitySettings() const;
	float GetWealthDensitySettings() const;
	float GetIndustryDensitySettings() const;
	bool CanSpawnFences() { return this->SpawnFences; };

	UPROPERTY(VisibleAnywhere, Category = "Road Network")
	USceneComponent* SceneComponent;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(CallInEditor, Category="Road Network")
	void GenerateRandomRoadNetwork();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void GenerateRoadNetwork();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void GenerateCityLots();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void GenerateBuildings();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Road Network")
	void callSpawnDetails();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void GenerateRoadMesh();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Road Network")
	void GenerateElectricPoles();
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void GenerateSidewalk();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void RegenerateFinalCity();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void RandomizeOSM();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void RandomizeSeed();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void ClearRoadNetwork();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Road Network")
	void ClearBuildings();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void RemoveDebugLines();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void ToggleDataTextureMapOfFinalCity();
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Road Network")
	void ToggleLotsZoneColorsOfFinalCity();
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Road Network")
	void SemanticSegmentationMode(bool retainSemanticMode);

	virtual bool ShouldTickIfViewportsOnly() const override;
	virtual void Destroyed() override;

	// Base building instances
	const TArray<ABuilding*> EmptyBuildingInstances;
	
	UPROPERTY(BlueprintReadOnly, Category = "DataAsset")
	TArray<ABuilding*> LowRiseResidentialBuildingInstances;
	UPROPERTY(BlueprintReadOnly, Category = "DataAsset")
	TArray<ABuilding*> HighRiseResidentialBuildingInstances;
	UPROPERTY(BlueprintReadOnly, Category = "DataAsset")
	TArray<ABuilding*> LowRiseCommercialBuildingInstances;
	UPROPERTY(BlueprintReadOnly, Category = "DataAsset")
	TArray<ABuilding*> HighRiseCommercialBuildingInstances;
	UPROPERTY(BlueprintReadOnly, Category = "DataAsset")
	TArray<ABuilding*> IndustrialBuildingInstances;
	UPROPERTY(BlueprintReadOnly, Category = "DataAsset")
	TArray<ABuilding*> LandmarkBuildingInstances;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Road Network")
	TArray<UStreetMap*> AllStreetMapsArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Road Network")
	TArray<UStreetMap*> StreetMapsArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Road Network")
	int32 Seed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Road Network")
	float RoadNetworkScaleX = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Network")
	float RoadNetworkScaleY = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Network")
	float PopulationDensity = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Network")
	float WealthDensity = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Network")
	float IndustryDensity = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Network")
	float DetailsDensity = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Network")
	bool SpawnElectricPoles = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Network")
	bool SpawnFences = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Network")
	bool SpawnSidewalk = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Network")
	bool bHasSemanticSegmentation = false;

private:
	void CreateCityFromOSM();
	void MergeSameRoadHalfEdgesInBaseCities();
	void RemoveDisconnectedRoadsInBaseCities();
	void SortRoadHalfEdgePairsInBaseCities();
	void MergeBaseCitiesToFinalCity();

	void DestroyFinalCity();
	void DestroyAllBuildingInstances();
	void DestroyAllVisualActors();
	void SpawnAllBuildingInstanceActors();

	// DEBUG
	void DrawDebugRoadsOfBaseCities();
	void DrawDebugRoadsOfFinalCity();
	void DrawDebugRoadNodesOfFinalCity();
	void DrawDebugCityZoneMeshOfFinalCity();

	static ERoadType GetRoadTypeFromOSM(EStreetMapRoadType StreetMapRoadType);
	bool FindOrCreateRoadNodeAt(const FVector& Position, const TArray<ARoadNode*>& RoadNodesArray, ARoadNode*& RoadNode) const;

	UPROPERTY(VisibleAnywhere, Category="Road Network")
	UProceduralMeshComponent* SidewalkProceduralMeshComponent;

	UPROPERTY(EditAnywhere, Category="Road Network")
	UMaterialInterface* SemanticSidewalkMaterial;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	UMaterialInterface* SemanticLowRiseBuildingMaterial;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	UMaterialInterface* SemanticHighRiseBuildingMaterial;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	UMaterialInterface* SemanticRoadMaterial;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	UMaterialInterface* SemanticPoleMaterial;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	UMaterialInterface* SemanticDetailMaterial;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	UMaterialInterface* SemanticLandscapeMaterial;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	UMaterialInterface* SemanticStoplightMaterial;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	UMaterialInterface* SemanticFenceMaterial;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	TArray<AActor*> SemanticSegmentationActorsToDisable;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	AActor* Landscape;
	
	UPROPERTY(EditAnywhere, Category="Road Network")
	float DebugLinesRadius = 16.0f;

	UPROPERTY(EditAnywhere, Category="Road Network")
	TSubclassOf<ACityBlock> CityBlockBlueprint;
	
	UPROPERTY(EditAnywhere, Category = "Road Network")
	TSubclassOf<ARoadActor> RoadMeshBlueprintToSpawn;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	TSubclassOf<ADecalActor> PedestrianDecalActor;
	
	UPROPERTY(EditAnywhere, Category = "Road Network")
	TSubclassOf<AActor> StoplightClass;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	TSubclassOf<AActor> SidewalkActor;

	UPROPERTY(EditAnywhere, Category = "Road Network")
	TSubclassOf<AActor> FenceActor;
	
	UPROPERTY(EditAnywhere, Category = "Road Network")
	int32 SubdivideDepth = 0;

	UPROPERTY(EditAnywhere, Category = "Buildings")
	UCityBuildingsDataAsset* CityBuildingsDataAsset;
	
	UPROPERTY(EditAnywhere, Category = "DataAsset")
	UDetailsDataAsset* DetailsDataAsset;

	UPROPERTY(EditAnywhere, Category = "DataAsset")
	USpawnParametersAsset* SpawnParametersAsset;
	
	UPROPERTY(EditAnywhere, Category = "DataAsset")
	bool SemanticSegmentationModeBool = false;

	TArray<ACityBlock*> AllBaseCitiesArray;
	TArray<ARoadActor*> AllRoadMeshArray;
	TArray<ADecalActor*> AllDecalActors;
	TArray<AActor*> AllStoplightActors;

	// Landscape 
	UMaterialInterface* OriginalLandScapeMaterial;
	TArray<TArray<UMaterialInterface*>> OriginalStoplightMaterials;
	
	// Base Detail Instances
	TArray<ADetail*> DetailInstance;
	
	ACityBlock* FinalCity = nullptr;
};