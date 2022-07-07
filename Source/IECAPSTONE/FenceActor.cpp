// Fill out your copyright notice in the Description page of Project Settings.


#include "FenceActor.h"

// Sets default values
AFenceActor::AFenceActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	this->sceneComponent = CreateDefaultSubobject<USceneComponent>(FName("SceneComponent"));
	RootComponent = sceneComponent;

	this->splineComponent = CreateDefaultSubobject<USplineComponent>(FName("SplineComponent"));
	this->splineComponent->SetupAttachment(this->RootComponent);
	this->splineComponent->SetVisibility(true);
	this->splineComponent->SetMobility(EComponentMobility::Movable);

	const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("/Game/Models/Props/Fence/back_fence_small"));
	this->static_mesh = MeshObj.Object;
}

void AFenceActor::Initialize(TArray<FVector> positions)
{
	const float RoadSize = 7000;
	float CurrentSplineEndToEndDistance = 0;
	FVector haha = FVector::ZeroVector;

	float SplineLength = splineComponent->GetSplineLength();
	SplineLength /= RoadSize;
	int NumSplineInDist = 0;
	
	TArray<FVector> SidewalkPos;

	for (int i = 0; i < positions.Num(); i++) //include the last one
	{
		FVector currentPosition;
		FVector nextPosition;

		if (SidewalkPos.Num() == 0)
		{
			currentPosition = positions[i];
			nextPosition = positions[i + 1];
		}
		else
		{
			currentPosition = SidewalkPos[SidewalkPos.Num() - 1];
			nextPosition = positions[(i + 1) % positions.Num()];
		}

		haha = (nextPosition - currentPosition);//+1) % positionList.Num()
		CurrentSplineEndToEndDistance = haha.Size();
		//numSplineInDist = FMath::CeilToInt(currentSplineEndToEndDistance / road_size);//floor
		NumSplineInDist = FMath::Floor(CurrentSplineEndToEndDistance / RoadSize);//floor

		FVector SplineStep = haha * (RoadSize / CurrentSplineEndToEndDistance);
		FVector Finaldir;

		for (int j = 0; j < NumSplineInDist; j++)
		{
			FVector splinePointLoc = currentPosition + (SplineStep * j);
			splinePointLoc + FVector(0, 0, 4);

			if (j != 0 && i == 0)// //skip position 1 i != 0 && 
			{
				SidewalkPos.Add(splinePointLoc);
				splineComponent->AddSplinePoint(splinePointLoc, ESplineCoordinateSpace::World, true);
			}
			else
			{
				SidewalkPos.Add(splinePointLoc);
				splineComponent->AddSplinePoint(splinePointLoc, ESplineCoordinateSpace::World, true);
			}
		}

		SidewalkPos.Add(nextPosition);
		splineComponent->AddSplinePoint(nextPosition, ESplineCoordinateSpace::World, true);
	}
	
	splineComponent->AddSplinePoint(SidewalkPos[0], ESplineCoordinateSpace::World, true);

	int splinePointsNum = splineComponent->GetNumberOfSplinePoints(); //default at 2;
	int randNum = rand() % splinePointsNum;
	
	for (int i = 0; i < splinePointsNum; i++)
	{
		if (i == randNum) continue;

		FString number = FString::FromInt(i);
		FString hehe = "meshComponent" + FString::FromInt(i);
		USplineMeshComponent* meshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), FName(*hehe));
		SplineMeshComponentsList.Add(meshComponent);

		meshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		meshComponent->SetupAttachment(splineComponent);
		meshComponent->SetMobility(EComponentMobility::Movable);
		meshComponent->SetVisibility(true);
		meshComponent->SetStaticMesh(this->static_mesh);
		meshComponent->SetForwardAxis(ESplineMeshAxis::X, true);

		FVector startLoc;
		FVector endLoc;
		FVector startTangent;
		FVector endTangent;

		splineComponent->GetLocalLocationAndTangentAtSplinePoint(i, startLoc, startTangent);
		splineComponent->GetLocalLocationAndTangentAtSplinePoint(i + 1, endLoc, endTangent);
		
		meshComponent->SetStartAndEnd(startLoc, startTangent * 0.1, endLoc, endTangent * 0.1, true);//startTangent

		RegisterAllComponents();
	}

	splineComponent->UpdateSpline();
}

void AFenceActor::SemanticSegmentationMode(UMaterialInterface* SemanticMaterial)
{
	if (!bHasSemanticSegmentation)
	{
		for (int i = 0; i < SplineMeshComponentsList[0]->GetMaterials().Num(); i++)
		{
			OriginalMaterials.Add(SplineMeshComponentsList[0]->GetMaterials()[i]);
		}

		bHasSemanticSegmentation = true;
	}

	bIsSemanticSegmentation = !bIsSemanticSegmentation;

	if (bIsSemanticSegmentation)
	{
		for (USplineMeshComponent* SplineMeshComponent : SplineMeshComponentsList)
		{
			for (int i = 0; i < SplineMeshComponent->GetMaterials().Num(); i++)
			{
				SplineMeshComponent->SetMaterial(i, SemanticMaterial);
			}
		}
	}
	else
	{
		for (USplineMeshComponent* SplineMeshComponent : SplineMeshComponentsList)
		{
			for (int i = 0; i < SplineMeshComponent->GetMaterials().Num(); i++)
			{
				SplineMeshComponent->SetMaterial(i, OriginalMaterials[i]);
			}
		}
	}
}

void AFenceActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFenceActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

