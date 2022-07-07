// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadActor.h"
#include "DrawDebugHelpers.h"
#include "CustomMath.h"
#include "RoadNode.h"
#include "CableComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARoadActor::ARoadActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	this->splineComponent = CreateDefaultSubobject<USplineComponent>(FName("SplineComponent"));
	this->splineComponent->SetupAttachment(this->RootComponent);
	this->splineComponent->SetVisibility(true);
	this->splineComponent->SetMobility(EComponentMobility::Movable);
	this->splineComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//this->splineComponent->RemoveSplinePoint(1, true);
	const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("/Game/Meshes/StaticMeshes/Road_Experimental_1"));
	const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj1(TEXT("/Game/Meshes/StaticMeshes/Road_Experimental_1"));
	const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj2(TEXT("/Game/Meshes/StaticMeshes/testRoad1"));
	this->static_mesh = MeshObj.Object;
	this->static_mesh1 = MeshObj1.Object;
	this->static_mesh2 = MeshObj2.Object;
	this->RoadID = 0;

	static ConstructorHelpers::FObjectFinder<UClass> ItemBlueprintClass(TEXT("Class'/Game/Blueprints/BP_ElectricPole.BP_ElectricPole_C'"));

	if (ItemBlueprintClass.Object)
	{
		ElectricPoleBP = ItemBlueprintClass.Object;
	}
	
	this->SetActorEnableCollision(true);
}

ARoadActor::~ARoadActor()
{
	
}

void ARoadActor::initialize(TArray<FRoadHalfEdge*> const& edgeList, int NewRoadID)
{
	RoadID = NewRoadID;
	roadType = edgeList[0]->GetRoadType();
	roadHalfEdgesList = edgeList;

	static const float MAX_SPLINE_DISTANCE = 6000;
	int Counter = 0;
	float SplineLength = splineComponent->GetSplineLength();
	SplineLength /= MAX_SPLINE_DISTANCE;
	
	TArray<FRoadHalfEdge*> edgesToSetFalseRoadMesh;

	for (int i = 0; i < roadHalfEdgesList.Num() - 1; i++)
	{
		splineComponent->AddSplinePoint(roadHalfEdgesList[i]->GetNextRoadNode()->GetActorLocation(), ESplineCoordinateSpace::World, true);
	}
	
	//last spline 
	splineComponent->AddSplinePoint(roadHalfEdgesList[roadHalfEdgesList.Num() - 1]->GetNextRoadNode()->GetActorLocation(), ESplineCoordinateSpace::World, true);

	int R = 0;
	UStaticMesh* TempMesh = nullptr;

	switch (roadType)
	{
	case ERoadType::Highway:
	{
		TempMesh = this->static_mesh;
		break;
	}
	case ERoadType::MajorRoad:
	{
		TempMesh = this->static_mesh1;
		break;
	}
	case ERoadType::MinorRoad:
	{
		TempMesh = this->static_mesh2;
		break;
	}
	case ERoadType::Invalid:
	{
		TempMesh = this->static_mesh;
		break;
	}
	default:
	{
		break;
	}
	}

	int SplinePointsNum = splineComponent->GetNumberOfSplinePoints();
	
	for (int i = 0; i < SplinePointsNum; i++)
	{
		FString number = FString::FromInt(i);
		FString hehe = "meshComponent" + FString::FromInt(i);
		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), FName(*hehe));//FName("SplineMeshComponent")
		splineMeshComponentsList.Add(SplineMeshComponent);

		SplineMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		SplineMeshComponent->SetupAttachment(splineComponent);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// SplineMeshComponent->SetCollisionProfileName(FName("OverlapAll"), true);
		SplineMeshComponent->bUseDefaultCollision = true;
		SplineMeshComponent->SetVisibility(true);
		SplineMeshComponent->SetStaticMesh(TempMesh);
		SplineMeshComponent->SetForwardAxis(ESplineMeshAxis::X, true);
		
		FVector startLoc;
		FVector endLoc;
		FVector startTangent;
		FVector endTangent;

		splineComponent->GetLocalLocationAndTangentAtSplinePoint(i, startLoc, startTangent);
		splineComponent->GetLocalLocationAndTangentAtSplinePoint(i + 1, endLoc, endTangent);
		SplineMeshComponent->SetStartAndEnd(startLoc, startTangent * 0.1, endLoc, endTangent * 0.1, true);//startTangent
		SplineMeshComponent->UpdateCollisionFromStaticMesh();
		SplineMeshComponent->RecreateCollision();
		// SplineMeshComponent->RegisterComponent();
		RegisterAllComponents();
		
		Counter++;
	}
	FString IntAsString = FString::FromInt(splineMeshComponentsList.Num());
	splineComponent->UpdateSpline();
	
	for (FRoadHalfEdge* edges : edgesToSetFalseRoadMesh)
	{
		edges->SetHalfEdgeMeshBool(true);
	}
}

void ARoadActor::SpawnElectricPolesFromRoadActor()
{
	int Counter = 0;
	static const float MAX_SPLINE_DISTANCE = 6000;// splineComponent->GetSplineLength()
	float CurrentSplineEndToEndDistance = 0;
	FVector Vec = FVector::ZeroVector;
	int NumSplineInDist = 0;

	float splineLength = splineComponent->GetSplineLength();
	splineLength /= MAX_SPLINE_DISTANCE;

	TArray<FVector> poleSpawnPos;

	for (int i = 0; i < roadHalfEdgesList.Num(); i++)
	{
		FVector CurrentPosition;
		FVector NextPosition;
		
		if (poleSpawnPos.Num() == 0)
		{
			CurrentPosition = this->roadHalfEdgesList[i]->GetOriginRoadNode()->GetActorLocation();
			NextPosition = this->roadHalfEdgesList[i]->GetNextRoadNode()->GetActorLocation();
		}
		else
		{
			CurrentPosition = poleSpawnPos[poleSpawnPos.Num() - 1];
			NextPosition = this->roadHalfEdgesList[i]->GetNextRoadNode()->GetActorLocation();
		}

		Vec = (NextPosition - CurrentPosition);
		CurrentSplineEndToEndDistance = Vec.Size();
		NumSplineInDist = FMath::CeilToInt(CurrentSplineEndToEndDistance / MAX_SPLINE_DISTANCE);

		FVector splineStep = Vec * (MAX_SPLINE_DISTANCE / CurrentSplineEndToEndDistance);

		for (int j = 0; j < NumSplineInDist; j++)
		{
			if (j != 0)
			{
				FVector splinePointLoc = CurrentPosition + (splineStep * j);
				poleSpawnPos.Add(splinePointLoc);
			}
		}
	}

	float pole_offset = 3700;
	for (int i = 0; i < poleSpawnPos.Num(); i++) //include the last one
	{
		FVector loc = poleSpawnPos[i];
		FVector directionVec = poleSpawnPos[(i + 1) % poleSpawnPos.Num()] - poleSpawnPos[i];
		FVector finaldir;
		float len;
		directionVec = directionVec.RotateAngleAxis(90, directionVec.ZAxisVector);
		directionVec.ToDirectionAndLength(finaldir, len);

		FRotator rotation2 = finaldir.Rotation();
		FTransform spawnTransform(rotation2, loc + finaldir * pole_offset);
		FActorSpawnParameters spawnparams;
		
		if (CheckPoleSpawnPointIfValid(loc + finaldir * pole_offset))
		{
			
			AActor* actor = this->GetWorld()->SpawnActor<AActor>(ElectricPoleBP, loc + finaldir * pole_offset, rotation2, spawnparams);
			ElectricPoleList.Add(actor);
		}
	}

	for (int i = 0; i < ElectricPoleList.Num() - 1; i++)
	{
		AActor* current = ElectricPoleList[i];
		AActor* next = ElectricPoleList[i + 1];
		UCableComponent* cable = current->FindComponentByClass<UCableComponent>();
		if (cable != nullptr)
		{
			// FVector pos = next->GetActorLocation() - current->GetActorLocation();
			FTransform transform = next->GetActorTransform().GetRelativeTransform(current->GetTransform());
			transform.SetLocation(FVector(transform.GetLocation().X, transform.GetLocation().Y, 1100));
			cable->EndLocation = transform.GetLocation();//next->GetActorLocation() ;
		}
	}
	
}

void ARoadActor::createRoad(TArray<FVector>& positionList)
{
	
}

USplineComponent* ARoadActor::getSplineComponent()
{
	return this->splineComponent;
}

bool ARoadActor::CheckPoleSpawnPointIfValid(FVector position)
{
	//raycasting
	FHitResult hit;
	FVector start;
	FVector end;
	bool bDidHit = false;
	
	start = position + FVector::UpVector* 500;
	end = position + FVector::DownVector * 500;
	FCollisionQueryParams TraceParams;
	bDidHit = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_WorldDynamic, TraceParams);
	
	if (bDidHit)
	{
		if (hit.ImpactPoint.Z < -1)
		{
			return true;
		}
	}
	
	return false;
}

void ARoadActor::SemanticSegmentationMode(UMaterialInterface* SemanticRoadMaterial, UMaterialInterface* SemanticPoleMaterial)
{
	bIsSemanticSegmentation = !bIsSemanticSegmentation;
	splineComponent->UpdateSpline();

	// Change electric poles
	if (ElectricPoleList.Num() == 0)
	{
		return;
	}
	
	if (OriginalRoadMaterial == nullptr)
	{
		OriginalRoadMaterial = splineMeshComponentsList[0]->GetMaterial(0);
	}
	if (OriginalPoleMaterial == nullptr)
	{
		OriginalPoleMaterial = ElectricPoleList[0]->FindComponentByClass<UStaticMeshComponent>()->GetMaterial(0);
	}

	if (this->bIsSemanticSegmentation)
	{
		for (int i = 0; i < splineMeshComponentsList.Num(); i++)
		{
			splineMeshComponentsList[i]->SetMaterial(0, SemanticRoadMaterial);
		}
		
		for (int i = 0; i < ElectricPoleList.Num(); i++)
		{
			ElectricPoleList[i]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, SemanticPoleMaterial);
		}
	}
	else
	{
		for (int i = 0; i < splineMeshComponentsList.Num(); i++)
		{
			splineMeshComponentsList[i]->SetMaterial(0, OriginalRoadMaterial);
		}
		
		for (int i = 0; i < ElectricPoleList.Num(); i++)
		{
			ElectricPoleList[i]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, OriginalPoleMaterial);
		}
	}
}

void ARoadActor::DestroyPoleListActors()
{
	if (this->ElectricPoleList.Num() == 0)
	{
		return;
	}
	for (AActor* poles : ElectricPoleList)
	{
		if (poles != nullptr)
		{
			poles->Destroy();
		}
	}
	ElectricPoleList.Empty();
}

void ARoadActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARoadActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
