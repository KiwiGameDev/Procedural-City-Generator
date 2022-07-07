// Fill out your copyright notice in the Description page of Project Settings.

#include "TextFileManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

bool UTextFileManager::SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool bAllowOverwriting = false)
{
	SaveDirectory += "\\";
	SaveDirectory += FileName;

	if (!bAllowOverwriting)
	{
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*SaveDirectory))
		{
			return false;
		}
	}

	FString FinalString = "";

	for (FString& Text : SaveText)
	{
		FinalString += Text;
		FinalString += LINE_TERMINATOR;
	}

	return FFileHelper::SaveStringToFile(FinalString, *SaveDirectory);
}

TArray<FString> UTextFileManager::ReadTextFileAsStringArray(FString FilePath)
{
	TArray<FString> Output;
	
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		return Output;
	}
	
	FFileHelper::LoadFileToStringArray(Output, *FilePath);

	return Output;
}
