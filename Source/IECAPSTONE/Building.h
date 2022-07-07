// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Classes/Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Building.generated.h"

UCLASS()
class IECAPSTONE_API ABuilding : public AActor
{
	GENERATED_BODY()
	
public:	
	ABuilding();

	UFUNCTION(BlueprintCallable)
	void SetBuildingAreaFromStaticMeshBounds();

	UFUNCTION(BlueprintCallable)
	void SemanticSegmentationMode(UMaterialInterface* SemanticMaterial);
	
	void SetBuildingLotArea(float newArea);
	
	float GetBuildingLotArea();

	FBox SetBoxTransform(FTransform transform);

	UHierarchicalInstancedStaticMeshComponent* GetInstancedStaticMesh();

	bool GetIsAllowedToSpawn() const;
	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Component")
	UHierarchicalInstancedStaticMeshComponent* InstancedStaticMeshComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Building Info")
	float BuildingArea = 0;

	FBox box;

	UPROPERTY(BlueprintReadWrite, Category = "Building Info")
	bool bIsAllowedToSpawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building Info")
	FString Name;

private:
	TArray<UMaterialInterface*> OriginalMaterials;

	bool bHasSemanticSegmentation = false;
	bool bIsSemanticSegmentation = false;
};
