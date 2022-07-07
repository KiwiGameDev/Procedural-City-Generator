// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Detail.generated.h"

UCLASS()
class IECAPSTONE_API ADetail : public AActor
{
	GENERATED_BODY()
	
public:	
	ADetail();

	UFUNCTION(BlueprintCallable)
	void SemanticSegmentationMode(UMaterialInterface* SemanticMaterial);


protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	UHierarchicalInstancedStaticMeshComponent* GetInstancedStaticMesh();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Details")
	UHierarchicalInstancedStaticMeshComponent* InstancedStaticMeshComponent = nullptr;

private:
	TArray<UMaterialInterface*> OriginalMaterials;

	bool bHasSemanticSegmentation = false;
	bool bIsSemanticSegmentation = false;
};
