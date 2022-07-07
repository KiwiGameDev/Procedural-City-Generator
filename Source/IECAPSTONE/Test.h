// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityLot.h"
#include "FLineSegment.h"
#include "GameFramework/Actor.h"
#include "Test.generated.h"

class ACityBlock;
class ARoadNode;

UCLASS()
class IECAPSTONE_API ATest : public AActor
{
	GENERATED_BODY()

public:
	ATest();

protected:
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(CallInEditor, Category="Test")
	void GeneratePerlinNoiseTexture();

	UFUNCTION(CallInEditor, Category="Test")
	void InitializeCity();
	
	UFUNCTION(CallInEditor, Category="Test")
	void CreateBasicCity();

	UFUNCTION(CallInEditor, Category="Test")
	void CreateSquareCityWithStrayInsideRoad();

	UFUNCTION(CallInEditor, Category="Test")
	void CreateComplexCityFromBaseCities();
	
	UFUNCTION(CallInEditor, Category="Test")
	void GenerateCityLots();

	UFUNCTION(CallInEditor, Category="Test")
	void ClearAll();

private:
	void DebugDrawCity();
	
	virtual bool ShouldTickIfViewportsOnly() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test")
	UTextureRenderTarget2D* NoiseRenderTarget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test")
	UMaterial* NoiseGenerationMaterial;

	UPROPERTY(EditAnywhere, Category="Test")
	TSubclassOf<ACityBlock> CityBlockBlueprint;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test")
	float DebugLineThickness = 4.0f;

	UPROPERTY(VisibleInstanceOnly, Category="Test")
	ACityBlock* FinalCity = nullptr;

	TArray<ACityBlock*> AllBaseCitiesArray;
};
