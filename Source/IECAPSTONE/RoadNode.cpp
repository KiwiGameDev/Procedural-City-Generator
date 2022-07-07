// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadNode.h"

#include "RoadHalfEdge.h"

ARoadNode::ARoadNode()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
}

void ARoadNode::ConnectRoadHalfEdge(FRoadHalfEdge* RoadHalfEdge)
{
	EmanatingRoadHalfEdgesArray.Add(RoadHalfEdge);
}

void ARoadNode::DisconnectRoadHalfEdge(FRoadHalfEdge* RoadHalfEdge)
{
	EmanatingRoadHalfEdgesArray.RemoveSingle(RoadHalfEdge);
}

void ARoadNode::OnPositionChanged()
{
	for (FRoadHalfEdge* HalfEdge : EmanatingRoadHalfEdgesArray)
	{
		HalfEdge->OnOriginRoadNodePositionChanged();
	}
}

const TArray<FRoadHalfEdge*>& ARoadNode::GetEmanatingRoadHalfEdges() const
{
	return EmanatingRoadHalfEdgesArray;
}
