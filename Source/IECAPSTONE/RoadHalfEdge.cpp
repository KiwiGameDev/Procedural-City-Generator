// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadHalfEdge.h"

#include "CityLot.h"
#include "CustomMath.h"
#include "RoadNode.h"

FRoadHalfEdge::FRoadHalfEdge()
	: OriginRoadNode(nullptr), PairHalfEdge(nullptr), NextHalfEdge(nullptr), CityLot(nullptr), RoadType(ERoadType::Invalid), RoadID(0)
{

}

FRoadHalfEdge::FRoadHalfEdge(ARoadNode* NewOriginRoadNode)
	: OriginRoadNode(NewOriginRoadNode), PairHalfEdge(nullptr), NextHalfEdge(nullptr), CityLot(nullptr), RoadType(ERoadType::Invalid), RoadID(0)
{
	OriginRoadNode->ConnectRoadHalfEdge(this);
}

FRoadHalfEdge::FRoadHalfEdge(const FRoadHalfEdge& Copy)
{
	OriginRoadNode = Copy.OriginRoadNode;
	PairHalfEdge = Copy.PairHalfEdge;
	NextHalfEdge = Copy.NextHalfEdge;
	CityLot = Copy.CityLot;
	RoadType = Copy.RoadType;
	RoadID = Copy.RoadID;
}

void FRoadHalfEdge::Initialize()
{
	if (GetNextRoadHalfEdge() != nullptr)
	{
		// UE_LOG(LogTemp, Warning, TEXT("Next road half edge already been found!"));
		return;
	}

	ARoadNode* CenterNode = GetNextRoadNode();

	// Case of when zero emanating road half edge. Should actually be impossible. 
	if (CenterNode->GetEmanatingRoadHalfEdges().Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Road node has zero emanating road half edges despite being connected!"));
		SetNextRoadHalfEdge(GetPairRoadHalfEdge());
		return;
	}

	// Case of when only 1 emanating road half edge
	if (CenterNode->GetEmanatingRoadHalfEdges().Num() == 1)
	{
		SetNextRoadHalfEdge(CenterNode->GetEmanatingRoadHalfEdges()[0]);
		return;
	}
	
	ARoadNode* OriginNode = GetOriginRoadNode();
	FVector CenterPos = CenterNode->GetActorLocation();
	FVector OriginPosRelative = OriginNode->GetActorLocation() - CenterPos;
	FRoadHalfEdge* NewNextHalfEdge = nullptr;
	float MaxAngle = 2.0f * PI;

	for (FRoadHalfEdge* OtherHalfEdge : CenterNode->GetEmanatingRoadHalfEdges())
	{
		ARoadNode* OtherNode = OtherHalfEdge->GetNextRoadNode();

		if (OriginNode == OtherNode)
		{
			continue;
		}

		FVector OtherPos = OtherNode->GetActorLocation();
		FVector OtherPosRelative = OtherPos - CenterPos;
		float Angle = CustomMath::GetClockwiseAngleBetweenVectors(OtherPosRelative, OriginPosRelative);

		if (Angle < MaxAngle)
		{
			NewNextHalfEdge = OtherHalfEdge;
			MaxAngle = Angle;
		}
	}
	
	if (NewNextHalfEdge != nullptr)
	{
		SetNextRoadHalfEdge(NewNextHalfEdge);
	}
}

void FRoadHalfEdge::OnOriginRoadNodePositionChanged()
{
	if (CityLot != nullptr)
	{
		CityLot->OnTopologyChanged();
	}
}

void FRoadHalfEdge::SetOriginRoadNode(ARoadNode* NewOriginRoadNode)
{
	OriginRoadNode->DisconnectRoadHalfEdge(this);
	OriginRoadNode = NewOriginRoadNode;
	OriginRoadNode->ConnectRoadHalfEdge(this);
}

void FRoadHalfEdge::SetPairRoadHalfEdge(FRoadHalfEdge* NewPairHalfEdge)
{
	PairHalfEdge = NewPairHalfEdge;
}

void FRoadHalfEdge::SetNextRoadHalfEdge(FRoadHalfEdge* NewNextHalfEdge)
{
	NextHalfEdge = NewNextHalfEdge;
}

void FRoadHalfEdge::SetCityLot(ACityLot* NewCityLot)
{
	CityLot = NewCityLot;
}

void FRoadHalfEdge::SetRoadType(ERoadType NewRoadType)
{
	 RoadType = NewRoadType;
}

void FRoadHalfEdge::SetRoadID(int NewRoadID) 
{
	RoadID = NewRoadID;
}

void FRoadHalfEdge::SetHalfEdgeMeshBool(bool value)
{
	hasRoadMesh = value;
}

ARoadNode* FRoadHalfEdge::GetOriginRoadNode() const
{
	return OriginRoadNode;
}

ARoadNode* FRoadHalfEdge::GetNextRoadNode() const
{
	return PairHalfEdge->GetOriginRoadNode();
}

FRoadHalfEdge* FRoadHalfEdge::GetPairRoadHalfEdge() const
{
	return PairHalfEdge;
}

FRoadHalfEdge* FRoadHalfEdge::GetNextRoadHalfEdge() const
{
	return NextHalfEdge;
}

ACityLot* FRoadHalfEdge::GetCityLot() const
{
	return CityLot;
}

ERoadType FRoadHalfEdge::GetRoadType() const
{
	return RoadType;
}

int FRoadHalfEdge::GetRoadID() const
{
	return RoadID;
}

bool FRoadHalfEdge::GetHasRoadMesh() const
{
	return hasRoadMesh;
}

bool FRoadHalfEdge::operator==(const FRoadHalfEdge& Other) const
{
	return GetNextRoadNode() == Other.GetNextRoadNode() && OriginRoadNode == Other.OriginRoadNode;
}

FRoadHalfEdge::~FRoadHalfEdge()
{
	if (OriginRoadNode != nullptr)
	{
		OriginRoadNode->DisconnectRoadHalfEdge(this);
	}
}
