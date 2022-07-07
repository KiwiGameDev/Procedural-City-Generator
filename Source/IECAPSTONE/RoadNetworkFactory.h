#pragma once
#include "RoadEdge.h"

class ACityBlock;
struct FRoadHalfEdge;
class ARoadNode;

class RoadNetworkFactory
{
public:
	static ARoadNode* CreateRoadNodeAt(UWorld* World, const FVector& Position);
	static FRoadEdge CreateRoadEdge(ARoadNode* RoadNodeA, ARoadNode* RoadNodeB, ERoadType RoadType, int RoadID);

	static ACityBlock* CreateVerticalCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World);
	static ACityBlock* CreateHorizontalCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World);
	static ACityBlock* CreateSlantedCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World);
	static ACityBlock* CreateSquareCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World);
	static ACityBlock* CreatePolygonCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World, uint16 EdgesCount);
	static ACityBlock* CreateSquareCityWithStrayInsideRoad(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World);
	static ACityBlock* CreateCityBlock(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World, const TArray<FRoadHalfEdge*>& NewRoadHalfEdges, const TArray<ARoadNode*>& NewRoadNodes);
};
