// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SceneComponent.h"
#include "RoadHalfEdge.h"
#include "RoadNetwork.h"
#include "GameFramework/Actor.h"
#include "SidewalkActor.generated.h"

UCLASS()
class IECAPSTONE_API ASidewalkActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASidewalkActor();
	void initialize(TArray<FVector> positions);

	void SemanticSegmentationMode(UMaterialInterface* SemanticMaterial);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USplineComponent* splineComponent;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USceneComponent* sceneComponent;
	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator::ZeroRotator;

	TArray<FRoadHalfEdge*> roadHalfEdgesList;
	TArray<USplineMeshComponent*> SplineMeshComponentsList;
	TArray<FVector> position_List;
	UStaticMesh* static_mesh = nullptr;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	TArray<UMaterialInterface*> OriginalMaterials;

	bool bHasSemanticSegmentation = false;
	bool bIsSemanticSegmentation = false;
};
