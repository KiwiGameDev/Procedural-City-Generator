// Fill out your copyright notice in the Description page of Project Settings.


#include "DetailsDataAsset.h"

const TArray<TSubclassOf<ADetail>>& UDetailsDataAsset::GetDetailArray() const
{
	return this->DetailArray;
}
