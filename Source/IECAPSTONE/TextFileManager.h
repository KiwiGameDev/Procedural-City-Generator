// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TextFileManager.generated.h"

UCLASS()
class IECAPSTONE_API UTextFileManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Save"))
	static bool SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool bAllowOverwriting);

	
	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Read"))
	static TArray<FString> ReadTextFileAsStringArray(FString FilePath);
};
