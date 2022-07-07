// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseNode.h"
#include "DrawDebugHelpers.h"

// Sets default values
ABaseNode::ABaseNode()
{
	PrimaryActorTick.bCanEverTick = true;
	this->splineComponent = CreateDefaultSubobject<USplineComponent>(FName("SplineComponent"));
	this->splineComponent->SetupAttachment(this->RootComponent);
	this->splineComponent->SetVisibility(true);
	this->splineComponent->SetMobility(EComponentMobility::Movable);
	const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("/Game/Meshes/StaticMeshes/roadPlane"));
	this->static_mesh = MeshObj.Object;
}

void ABaseNode::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (splineComponent->bSplineHasBeenEdited)
	{
		splineComponent->UpdateSpline();
	}
}

void ABaseNode::initialize(int32 Road, FVector Loc, FRotator Rot, TArray<FVector> PositionList)
{
	this->roadIndex = Road;
	this->location = Loc;
	this->rotation = Rot;
	int counter = 0;

	for (int i = 1; i < PositionList.Num(); i++)
	{
		splineComponent->AddSplinePoint(PositionList[i], ESplineCoordinateSpace::World, true);
	}
	
	int SplinePointsNum = splineComponent->GetNumberOfSplinePoints(); //default at 2;
	FString SplinePointsNumString = FString::FromInt(SplinePointsNum);

	for (int i = 0; i < SplinePointsNum -1; i++)
	{
		FString number = FString::FromInt(i);
		FString hehe = "meshComponent" + FString::FromInt(i);
		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), FName(*hehe));//FName("SplineMeshComponent")
		splineMeshComponentsList.Add(SplineMeshComponent);
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
		splineComponent->GetLocalLocationAndTangentAtSplinePoint(i+1, EndLoc, EndTangent);

		SplineMeshComponent->SetStartAndEnd(StartLoc, StartTangent, EndLoc, EndTangent, true);
		
		RegisterAllComponents();
		counter++;
	}
	
	FString IntAsString = FString::FromInt(splineMeshComponentsList.Num());
	DrawDebugString(this->GetWorld(), FVector(0, 0, 5), TEXT("spline component count: " + IntAsString), 0, FColor::Cyan, -1, false, 4);

	splineComponent->UpdateSpline();

	for(int i = 0; i< splineMeshComponentsList.Num();i++)
	{
		if (i == 0 || i == 1)
		{
			DrawDebugSphere(this->GetWorld(), splineMeshComponentsList[i]->GetStartPosition(), 100.0f, 10, FColor::Yellow, true);
		}
	}
}

void ABaseNode::createRoad(TArray<FVector>& PositionList)
{
	for (int i = 0; i < PositionList.Num(); i++)
	{
		USplineMeshComponent* meshComponent = CreateDefaultSubobject<USplineMeshComponent>(FName("SplineMeshComponent"));
		splineMeshComponentsList.Add(meshComponent);
	}
}

