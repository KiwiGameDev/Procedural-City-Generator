// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadNode.generated.h"

struct FRoadHalfEdge;

UCLASS()
class IECAPSTONE_API ARoadNode : public AActor
{
	GENERATED_BODY()
	
public:	
	ARoadNode();

	void ConnectRoadHalfEdge(FRoadHalfEdge* RoadHalfEdge);
	void DisconnectRoadHalfEdge(FRoadHalfEdge* RoadHalfEdge);

	void OnPositionChanged();

	const TArray<FRoadHalfEdge*>& GetEmanatingRoadHalfEdges() const;

private:
	TArray<FRoadHalfEdge*> EmanatingRoadHalfEdgesArray;
};
