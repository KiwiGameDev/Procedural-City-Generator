// Fill out your copyright notice in the Description page of Project Settings.

#include "LotComponent.h"

#include <string>

#include "Building.h"
#include "CustomMath.h"
#include "Detail.h"
#include "DrawDebugHelpers.h"
#include "RoadNode.h"
#include "Kismet/GameplayStatics.h"

ULotComponent::ULotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVector zoffset = FVector(0, 0, 100);

void ULotComponent::SpawnBuildingFromComponent(FVector Position, FRotator Rotation, ECityZoneType CityZoneType, ARoadNetwork* Network)
{
	FActorSpawnParameters SpawnParameters;
	int MaxTries = 5;
	TArray<TSubclassOf<ABuilding>> SpawnableBuildingsArray = CityBuildingsDataAsset->GetBuildingsOfCityZoneType(CityZoneType);
	TArray<ABuilding*>  SpawnableBuildings = Network->RetrieveBuildingsFromCityZone(CityZoneType);
	TSubclassOf<ABuilding> BuildingToSpawn;
	AActor* NewBuilding = nullptr;

	if (SpawnableBuildings.Num() == 0)
	{
		return;
	}
	
	while (MaxTries != 0)
	{
		ABuilding* Instance = SpawnableBuildings[FMath::Rand() % SpawnableBuildings.Num()];
		FTransform transform(Rotation, Position, FVector(FMath::RandRange(0.9f, 1.0f), FMath::RandRange(0.9f, 1.0f), FMath::RandRange(0.9f, 1.0f)));
		int32 InstanceID = Instance->GetInstancedStaticMesh()->AddInstance(transform);
		MaxTries--;
		break;
	}

}

void ULotComponent::SpawnDetailFromComponent(int detailIndex,FVector Position, FRotator Rotation, ARoadNetwork* network)
{
	TArray<ADetail*> SpawnableDetail = network->RetrieveDetails();
	ADetail* Detail = SpawnableDetail[detailIndex];
	FRotator Rot = FRotator(0, FMath::RandRange(0, 360),0);
	FTransform Transform(Rot, Position, FVector::OneVector);
	Detail->GetInstancedStaticMesh()->AddInstance(Transform);
}

void ULotComponent::SpawnSidewalkFromComponent(TArray<FRoadHalfEdge*> const& edgesList, TArray<FVector> inset, ARoadNetwork* roadnetwork)
{
	if (roadnetwork->GetSidewalkActor() == nullptr)
	{
		return;
	}
	if (inset.Num() == 0)
	{
		return;
	}
	
	TArray<FVector> SendPositions;

	for(int i = 0 ; i<edgesList.Num(); i++)
	{
		FVector Position = (edgesList[i]->GetOriginRoadNode()->GetActorLocation() + inset[i]) / 2;
		SendPositions.Add(Position);
	}
	
	if (!(edgesList.Num() <= 1))
	{
		FVector Direction = CustomMath::GetDirection(SendPositions[1], SendPositions[0]);
		FRotator Rotation = Direction.Rotation();
		
		FTransform SpawnTransform(Rotation, SendPositions[0]);
		ASidewalkActor* DeferredActor = Cast<ASidewalkActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, roadnetwork->GetSidewalkActor(), SpawnTransform));
		
		if (DeferredActor != nullptr)
		{
			DeferredActor->initialize(SendPositions);
			DeferredActor->FinishSpawning(SpawnTransform);
			SidewalkSpawnedArray.Add(DeferredActor);
		}
	}
}

void ULotComponent::SpawnFenceFromComponent(TArray<FRoadHalfEdge*> const& edgesList, TArray<FVector> inset, ARoadNetwork* RoadNetwork)
{
	if (RoadNetwork->GetFenceActor() == nullptr)
	{
		return;
	}
	if (inset.Num() == 0)
	{
		return;
	}
	
	TArray<FVector> SendPositions;

	for (int i = 0; i < edgesList.Num(); i++)
	{
		FVector temp = (edgesList[i]->GetOriginRoadNode()->GetActorLocation() + inset[i]) / 2;
		SendPositions.Add(temp);
	}

	if (!(edgesList.Num() <= 1))
	{
		FVector Direction = CustomMath::GetDirection(SendPositions[1], SendPositions[0]);
		FRotator Rotation = Direction.Rotation();
		FTransform spawnTransform(Rotation, SendPositions[0]);

		AFenceActor* DeferredActor = Cast<AFenceActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, RoadNetwork->GetFenceActor(), spawnTransform));
		if (DeferredActor != nullptr)
		{
			DeferredActor->Initialize(SendPositions);
			DeferredActor->FinishSpawning(spawnTransform);
			FencesSpawnedArray.Add(DeferredActor);
		}
	}
}

void ULotComponent::SemanticSegmentationMode(UMaterialInterface* SidewalkSemanticMaterial, UMaterialInterface* FenceSemanticMaterial)
{
	for (AActor* Actor : SidewalkSpawnedArray)
	{
		ASidewalkActor* SidewalkActor = Cast<ASidewalkActor>(Actor);
		SidewalkActor->SemanticSegmentationMode(SidewalkSemanticMaterial);
	}

	for (AActor* Actor : FencesSpawnedArray)
	{
		AFenceActor* FenceActor = Cast<AFenceActor>(Actor);
		FenceActor->SemanticSegmentationMode(FenceSemanticMaterial);
	}
}

void ULotComponent::ClearAllSpawnedBuildingsInLotComponent()
{
	if (BuildingsSpawnedArray.Num() == 0)
	{
		return;
	}
	
	for (int i = 0; i < BuildingsSpawnedArray.Num(); i++)
	{
		if (BuildingsSpawnedArray[i] != nullptr)
		{
			BuildingsSpawnedArray[i]->Destroy();
		}
	}

	BuildingsSpawnedArray.Empty();
}

void ULotComponent::TurnOffAllPhysicsAfterSpawn()
{
	for(int i = 0; i <BuildingsSpawnedArray.Num(); i++)
	{
		UStaticMeshComponent* StaticMeshComponent = BuildingsSpawnedArray[i]->FindComponentByClass<UStaticMeshComponent>();
		
		if(StaticMeshComponent != nullptr)
		{
			StaticMeshComponent->SetSimulatePhysics(false);
		}

		BuildingsSpawnedArray[i]->SetActorEnableCollision(false);
	}
}

void ULotComponent::CleanupOverlappingActors()
{
	for (int i = 0; i < BuildingsSpawnedArray.Num(); i++)
	{
		AActor* Building = BuildingsSpawnedArray[i];
		TArray<AActor*> OverlappingActors;
		
		Building->GetOverlappingActors(OverlappingActors, ABuilding::StaticClass());
		
		if (OverlappingActors.Num() != 0)
		{
			Building->Destroy();
		}
	}
}

void ULotComponent::ClearAllSidewalksInLotComponent()
{
	for (AActor* Sidewalk : this->SidewalkSpawnedArray)
	{
		if(Sidewalk != nullptr)
		{
			Sidewalk->Destroy();
		}
	}

	SidewalkSpawnedArray.Empty();
}

void ULotComponent::ClearAllFencesInLotComponent()
{
	for (AActor* Actor : this->FencesSpawnedArray)
	{
		if (Actor != nullptr)
		{
			Actor->Destroy();
		}
	}

	this->FencesSpawnedArray.Empty();
}

void ULotComponent::DestroyAll()
{
	ClearAllSidewalksInLotComponent();
	ClearAllFencesInLotComponent();
}