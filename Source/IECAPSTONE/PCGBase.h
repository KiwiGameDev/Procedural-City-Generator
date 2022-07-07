// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Building.h"
#include "StreetMapActor.h"
#include "Engine/SplineMeshActor.h"
#include "BaseNode.h"
#include "PCGBase.generated.h"

USTRUCT()
struct FFaceInfo
{
	GENERATED_BODY()
	FVector normal;
	int buildingCount;
};

UCLASS()

class IECAPSTONE_API APCGBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APCGBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	UFUNCTION(BlueprintCallable, CallInEditor)
		void OnCreateCity();
	UFUNCTION(BlueprintCallable, CallInEditor)
		void OnCreateSubdivisions();
	UFUNCTION(BlueprintCallable, CallInEditor)
		void ResetAll();
	UFUNCTION(BlueprintCallable, CallInEditor)
		void GenerateNodesFromStreetMap();
private:
	AActor* getNearestExistingEndpoint(FVector currentPosition);
	float GetMagnitude(FVector vector);
	FVector GetDirection(FVector a, FVector b);
	FVector GetFaceNormals(FVector midpoint, FVector a, FVector b);
	FVector GetMidpoint(FVector a, FVector b);
	FVector GetMidpointFour(FVector a, FVector b, FVector c, FVector d);

protected:

	TArray<FVector> vertices;
	TArray<AActor*> actor_storage;
	TArray<int32> triangles;
	TArray<FVector2D> uvs;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FColor>colors;
	const float SNAP_DISTANCE = 10.0f;
	const float BUILDING_THICKNESS = 100.0f;
	const float BUILDING_WIDTH = 100.f;
	TArray<AActor*> spawnedBuildings;
	TArray<ABaseNode*> roadNetworkRefs;


protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG Inputs")
		float PopulationDensity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG Inputs")
		float LandSize;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Subdivisions")
		UProceduralMeshComponent* pmc;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG Inputs")
		TSubclassOf<AActor> EndpointReference;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG Inputs")
		ABuilding* BuildingToSpawn;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG Inputs")
		AStreetMapActor* osmActor;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG Inputs")
		ASplineMeshActor* splineMeshActor;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG Inputs")
		TSubclassOf<ABaseNode> nodeToSpawn;
	

};





