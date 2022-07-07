// Fill out your copyright notice in the Description page of Project Settings.


#include "SidewalkActor.h"

#include "DrawDebugHelpers.h"
#include "RoadNode.h"

// Sets default values
ASidewalkActor::ASidewalkActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	this->sceneComponent = CreateDefaultSubobject<USceneComponent>(FName("SceneComponent"));
	RootComponent = sceneComponent;
	
	this->splineComponent = CreateDefaultSubobject<USplineComponent>(FName("SplineComponent"));
	this->splineComponent->SetupAttachment(this->RootComponent);
	this->splineComponent->SetVisibility(true);
	this->splineComponent->SetMobility(EComponentMobility::Movable);

	const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("/Game/Megascans/3D_Assets/Sidewalk/Modular_Concrete_Median_LOD0_vcrnfetdw"));
	this->static_mesh = MeshObj.Object;
}

void ASidewalkActor::initialize(TArray<FVector> positions)
{
	const float road_size = 10000;
	float CurrentSplineEndToEndDistance = 0;
	FVector Vec = FVector::ZeroVector;

	float SplineLength = splineComponent->GetSplineLength();;
	SplineLength /= road_size;
	int NumSplineInDist = 0;
	TArray<FVector> SidewalkPos;

	for (int i = 0; i < positions.Num(); i++) //include the last one
	{
		FVector CurrentPosition;
		FVector NextPosition;
		
		if (SidewalkPos.Num() == 0)
		{
			CurrentPosition = positions[i];
			NextPosition = positions[i + 1];
		}
		else
		{
			CurrentPosition = SidewalkPos[SidewalkPos.Num() - 1];
			NextPosition = positions[(i + 1) % positions.Num()];
		}

		Vec = (NextPosition - CurrentPosition);
		CurrentSplineEndToEndDistance = Vec.Size();
		NumSplineInDist = FMath::Floor(CurrentSplineEndToEndDistance / road_size);
		
		FVector SplineStep = Vec * (road_size / CurrentSplineEndToEndDistance);
		FVector Finaldir;
		
		for (int j = 0; j < NumSplineInDist; j++)
		{
			FVector splinePointLoc = CurrentPosition + (SplineStep * j);
			splinePointLoc + FVector(0, 0, 4);
			
			if (j!=0 && i == 0)
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

		SidewalkPos.Add(NextPosition);
		splineComponent->AddSplinePoint(NextPosition, ESplineCoordinateSpace::World, true);
	}

	splineComponent->AddSplinePoint(SidewalkPos[0], ESplineCoordinateSpace::World, true);
	
	int SplinePointsNum = splineComponent->GetNumberOfSplinePoints();
	
	for (int i = 0; i < SplinePointsNum; i++)
	{
		FString number = FString::FromInt(i);
		FString hehe = "meshComponent" + FString::FromInt(i);
		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), FName(*hehe));
		SplineMeshComponentsList.Add(SplineMeshComponent);

		SplineMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		SplineMeshComponent->SetupAttachment(splineComponent);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->SetVisibility(true);
		SplineMeshComponent->SetStaticMesh(this->static_mesh);
		SplineMeshComponent->SetForwardAxis(ESplineMeshAxis::X, true);
		
		FVector StartLoc;
		FVector EndLoc;
		FVector StartTangent;
		FVector EndTangent;

		splineComponent->GetLocalLocationAndTangentAtSplinePoint(i, StartLoc, StartTangent);
		splineComponent->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndLoc, EndTangent);
		
		SplineMeshComponent->SetStartAndEnd(StartLoc, StartTangent * 0.1, EndLoc, EndTangent * 0.1, true);//startTangent

		RegisterAllComponents();
	}
	
	splineComponent->UpdateSpline();
}

void ASidewalkActor::SemanticSegmentationMode(UMaterialInterface* SemanticMaterial)
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

void ASidewalkActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASidewalkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

