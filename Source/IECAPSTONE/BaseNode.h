// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "BaseNode.generated.h"

UCLASS()
class IECAPSTONE_API ABaseNode : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseNode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public: //class functions
	void initialize(int32 Road, FVector Loc, FRotator Rot, TArray<FVector> PositionList);
	void createRoad(TArray<FVector>& PositionList);
	USplineComponent* getSplineComponent();

private:
	USplineComponent* splineComponent = nullptr;
	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator::ZeroRotator;
	int32 nodeIndex = -1;
	int32 roadIndex = -1;
	TArray<USplineMeshComponent*> splineMeshComponentsList;
	TArray<FVector> position_List;
	UStaticMesh* static_mesh = nullptr;
};


