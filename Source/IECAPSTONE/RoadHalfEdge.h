// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoadType.h"
#include "RoadHalfEdge.generated.h"

class ACityLot;
class ARoadNode;

USTRUCT()
struct FRoadHalfEdge 
{
	GENERATED_BODY()

	FRoadHalfEdge();
	FRoadHalfEdge(ARoadNode* NewOriginRoadNode);
	FRoadHalfEdge(const FRoadHalfEdge& Copy);

	void Initialize();

	void OnOriginRoadNodePositionChanged();

	void SetOriginRoadNode(ARoadNode* NewOriginRoadNode);
	void SetPairRoadHalfEdge(FRoadHalfEdge* NewPairHalfEdge);
	void SetNextRoadHalfEdge(FRoadHalfEdge* NewNextHalfEdge);
	void SetCityLot(ACityLot* NewCityLot);
	void SetRoadType(ERoadType NewRoadType);
	void SetRoadID(int NewRoadID);
	void SetHalfEdgeMeshBool(bool value);

	ARoadNode* GetOriginRoadNode() const;
	ARoadNode* GetNextRoadNode() const;
	FRoadHalfEdge* GetPairRoadHalfEdge() const;
	FRoadHalfEdge* GetNextRoadHalfEdge() const;
	ACityLot* GetCityLot() const;
	ERoadType GetRoadType() const;
	int GetRoadID() const;
	bool GetHasRoadMesh() const;

	bool operator==(const FRoadHalfEdge& Other) const;

	~FRoadHalfEdge();
	
private:
	ARoadNode* OriginRoadNode;
	FRoadHalfEdge* PairHalfEdge;
	FRoadHalfEdge* NextHalfEdge;
	ACityLot* CityLot;
	ERoadType RoadType;
	int RoadID;
	bool hasRoadMesh = false;
};
