// Fill out your copyright notice in the Description page of Project Settings.


#include "Detail.h"

ADetail::ADetail()
{
	PrimaryActorTick.bCanEverTick = true;

	InstancedStaticMeshComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(FName("InstancedStaticMeshComponent"));
	RootComponent = InstancedStaticMeshComponent;
	
}

void ADetail::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADetail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


UHierarchicalInstancedStaticMeshComponent* ADetail::GetInstancedStaticMesh()
{
	return InstancedStaticMeshComponent;
}

void ADetail::SemanticSegmentationMode(UMaterialInterface* SemanticMaterial)
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