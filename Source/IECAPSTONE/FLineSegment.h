#pragma once

#include "CoreMinimal.h"
#include "FLineSegment.generated.h"

USTRUCT(BlueprintType)
struct FLineSegment
{
	GENERATED_BODY()

	FLineSegment();
	FLineSegment(const FVector& PointA, const FVector& PointB);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PointA;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PointB;
};
