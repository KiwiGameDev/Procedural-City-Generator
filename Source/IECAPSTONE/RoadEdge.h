#pragma once

#include "CoreMinimal.h"
#include "RoadType.h"
#include "RoadEdge.generated.h"

struct FRoadHalfEdge;

USTRUCT()
struct FRoadEdge
{
	GENERATED_BODY()
	
	FRoadEdge();
	FRoadEdge(FRoadHalfEdge* HalfEdgeA, FRoadHalfEdge* HalfEdgeB, ERoadType RoadType);

	FRoadHalfEdge* GetHalfEdgeA() const;
	FRoadHalfEdge* GetHalfEdgeB() const;
	ERoadType GetRoadType() const;
	
private:
	FRoadHalfEdge* HalfEdgeA;
	FRoadHalfEdge* HalfEdgeB;
	ERoadType RoadType;
};
