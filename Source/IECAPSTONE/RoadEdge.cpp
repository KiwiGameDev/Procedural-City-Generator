#include "RoadEdge.h"

FRoadEdge::FRoadEdge()
	: HalfEdgeA(nullptr), HalfEdgeB(nullptr), RoadType(ERoadType::Invalid)
{
	
}

FRoadEdge::FRoadEdge(FRoadHalfEdge* HalfEdgeA, FRoadHalfEdge* HalfEdgeB, ERoadType RoadType)
	: HalfEdgeA(HalfEdgeA), HalfEdgeB(HalfEdgeB), RoadType(RoadType)
{
	
}

FRoadHalfEdge* FRoadEdge::GetHalfEdgeA() const
{
	return HalfEdgeA;
}

FRoadHalfEdge* FRoadEdge::GetHalfEdgeB() const
{
	return HalfEdgeB;
}

ERoadType FRoadEdge::GetRoadType() const
{
	return RoadType;
}
