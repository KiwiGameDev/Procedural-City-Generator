// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "RoadHalfEdge.h"
#include "RoadActor.generated.h"


UCLASS()
class IECAPSTONE_API ARoadActor : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ARoadActor();
	~ARoadActor();
	void initialize(TArray<FRoadHalfEdge*>const& edgeList, int RoadID);
	void SpawnElectricPolesFromRoadActor();
	void createRoad(TArray<FVector>& positionList);
	USplineComponent* getSplineComponent();
	
	bool CheckPoleSpawnPointIfValid(FVector position);
	void SemanticSegmentationMode(UMaterialInterface* SemanticRoadMaterial, UMaterialInterface* SemanticPoleMaterial);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Info")
	int RoadID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Info")
	ERoadType roadType = ERoadType::Invalid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Info")
	TArray<USplineMeshComponent*> splineMeshComponentsList;

	void DestroyPoleListActors();
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	USplineComponent* splineComponent = nullptr;
	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator::ZeroRotator;

	TArray<FRoadHalfEdge*> roadHalfEdgesList;
	
	TArray<FVector> position_List;
	UStaticMesh* static_mesh = nullptr;
	UStaticMesh* static_mesh1 = nullptr;
	UStaticMesh* static_mesh2 = nullptr;
	TSubclassOf<AActor> ElectricPoleBP = nullptr;
	TArray<AActor*> ElectricPoleList;
	
	UMaterialInterface* OriginalRoadMaterial;
	UMaterialInterface* OriginalPoleMaterial;
	bool bIsSemanticSegmentation = false;
};
