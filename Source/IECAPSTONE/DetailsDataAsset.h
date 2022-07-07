// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Detail.h"
#include "DetailsDataAsset.generated.h"

UCLASS()
class IECAPSTONE_API UDetailsDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	const TArray<TSubclassOf<ADetail>>& GetDetailArray() const;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Detail")
		TArray<TSubclassOf<ADetail>> DetailArray;
	
};
