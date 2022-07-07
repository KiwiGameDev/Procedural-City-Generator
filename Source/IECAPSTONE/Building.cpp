// Fill out your copyright notice in the Description page of Project Settings.

#include "Building.h"

ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	InstancedStaticMeshComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(FName("InstancedStaticMeshComponent"));
	RootComponent = InstancedStaticMeshComponent;

	FString path = "/Game/Textures/yellow.uasset";
}

void ABuilding::SetBuildingAreaFromStaticMeshBounds()
{
	if (InstancedStaticMeshComponent->GetStaticMesh() == nullptr)
	{
		return;
	}
	
	FBoxSphereBounds bounds = InstancedStaticMeshComponent->GetStaticMesh()->GetBounds();
	box = bounds.GetBox();
	FVector size = box.GetSize();

	BuildingArea = (size.X * size.Y);
}

void ABuilding::SemanticSegmentationMode(UMaterialInterface* SemanticMaterial)
{
	if (!bHasSemanticSegmentation)
	{
		for (int i = 0; i < InstancedStaticMeshComponent->GetMaterials().Num(); i++)
		{
			OriginalMaterials.Add(InstancedStaticMeshComponent->GetMaterials()[i]);
		}

		bHasSemanticSegmentation = true;
	}
	
	bIsSemanticSegmentation = !bIsSemanticSegmentation;

	if (bIsSemanticSegmentation)
	{
		for (int i = 0; i < InstancedStaticMeshComponent->GetMaterials().Num(); i++)
		{
			this->InstancedStaticMeshComponent->SetMaterial(i, SemanticMaterial);
		}
	}
	else
	{
		for (int i = 0; i < InstancedStaticMeshComponent->GetMaterials().Num(); i++)
		{
			this->InstancedStaticMeshComponent->SetMaterial(i, OriginalMaterials[i]);
		}
	}
}

void ABuilding::SetBuildingLotArea(float newArea)
{
	BuildingArea = newArea;
}

float ABuilding::GetBuildingLotArea()
{
	return BuildingArea;
}

FBox ABuilding::SetBoxTransform(FTransform transform)
{
	return box.TransformBy(transform);
}

UHierarchicalInstancedStaticMeshComponent* ABuilding::GetInstancedStaticMesh()
{
	return InstancedStaticMeshComponent;
}

bool ABuilding::GetIsAllowedToSpawn() const
{
	return bIsAllowedToSpawn;
}
