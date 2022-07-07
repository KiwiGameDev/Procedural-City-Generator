#include "RoadNetworkFactory.h"
#include "CityBlock.h"
#include "RoadHalfEdge.h"
#include "RoadNode.h"

ARoadNode* RoadNetworkFactory::CreateRoadNodeAt(UWorld* World, const FVector& Position)
{
	ARoadNode* RoadNode = World->SpawnActor<ARoadNode>(Position, FRotator::ZeroRotator);
	return RoadNode;
}

FRoadEdge RoadNetworkFactory::CreateRoadEdge(ARoadNode* RoadNodeA, ARoadNode* RoadNodeB, ERoadType RoadType, int RoadID)
{
	FRoadHalfEdge* RoadHalfEdgeA = new FRoadHalfEdge(RoadNodeA);
	FRoadHalfEdge* RoadHalfEdgeB = new FRoadHalfEdge(RoadNodeB);
	RoadHalfEdgeA->SetPairRoadHalfEdge(RoadHalfEdgeB);
	RoadHalfEdgeB->SetPairRoadHalfEdge(RoadHalfEdgeA);
	RoadHalfEdgeA->SetRoadType(RoadType);
	RoadHalfEdgeB->SetRoadType(RoadType);
	RoadHalfEdgeA->SetRoadID(RoadID);
	RoadHalfEdgeB->SetRoadID(RoadID);
	return FRoadEdge(RoadHalfEdgeA, RoadHalfEdgeB, RoadType);
}

ACityBlock* RoadNetworkFactory::CreateVerticalCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World)
{
	TArray<FRoadHalfEdge*> RoadHalfEdges;
	TArray<ARoadNode*> RoadNodesArray;

	ARoadNode* Node1 = CreateRoadNodeAt(World, FVector(0.0f, -10000.0f, 0.0f));
	ARoadNode* Node2 = CreateRoadNodeAt(World, FVector(0.0f, 10000.0f, 0.0f));
	FRoadEdge Edge1 = CreateRoadEdge(Node1, Node2, ERoadType::MinorRoad, 0);

	RoadNodesArray.Add(Node1);
	RoadNodesArray.Add(Node2);
	RoadHalfEdges.Add(Edge1.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge1.GetHalfEdgeB());
	
	return CreateCityBlock(CityBlockBlueprint, World, RoadHalfEdges, RoadNodesArray);
}

ACityBlock* RoadNetworkFactory::CreateHorizontalCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World)
{
	TArray<FRoadHalfEdge*> RoadHalfEdges;
	TArray<ARoadNode*> RoadNodesArray;

	ARoadNode* Node1 = CreateRoadNodeAt(World, FVector(-10000.0f, 0.0f, 0.0f));
	ARoadNode* Node2 = CreateRoadNodeAt(World, FVector(10000.0f, 0.0f, 0.0f));
	FRoadEdge Edge1 = CreateRoadEdge(Node1, Node2, ERoadType::MinorRoad, 0);

	RoadNodesArray.Add(Node1);
	RoadNodesArray.Add(Node2);
	RoadHalfEdges.Add(Edge1.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge1.GetHalfEdgeB());
	
	return CreateCityBlock(CityBlockBlueprint, World, RoadHalfEdges, RoadNodesArray);
}

ACityBlock* RoadNetworkFactory::CreateSlantedCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World)
{
	TArray<FRoadHalfEdge*> RoadHalfEdges;
	TArray<ARoadNode*> RoadNodesArray;

	ARoadNode* Node1 = CreateRoadNodeAt(World, FVector(-3000.0f, -4000.0f, 0.0f));
	ARoadNode* Node2 = CreateRoadNodeAt(World, FVector(4000.0f, 3000.0f, 0.0f));
	FRoadEdge Edge1 = CreateRoadEdge(Node1, Node2, ERoadType::MinorRoad, 0);

	RoadNodesArray.Add(Node1);
	RoadNodesArray.Add(Node2);
	RoadHalfEdges.Add(Edge1.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge1.GetHalfEdgeB());
	
	return CreateCityBlock(CityBlockBlueprint, World, RoadHalfEdges, RoadNodesArray);
}

ACityBlock* RoadNetworkFactory::CreateSquareCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World)
{
	TArray<FRoadHalfEdge*> RoadHalfEdges;
	TArray<ARoadNode*> RoadNodesArray;
	
	ARoadNode* Node1 = CreateRoadNodeAt(World, FVector(-5000.0f, -5000.0f, 0.0f));
	ARoadNode* Node2 = CreateRoadNodeAt(World, FVector(5000.0f, -5000.0f, 0.0f));
	ARoadNode* Node3 = CreateRoadNodeAt(World, FVector(5000.0f, 5000.0f, 0.0f));
	ARoadNode* Node4 = CreateRoadNodeAt(World, FVector(-5000.0f, 5000.0f, 0.0f));
	FRoadEdge Edge1 = CreateRoadEdge(Node1, Node2, ERoadType::MinorRoad, 0);
	FRoadEdge Edge2 = CreateRoadEdge(Node2, Node3, ERoadType::MinorRoad, 0);
	FRoadEdge Edge3 = CreateRoadEdge(Node3, Node4, ERoadType::MinorRoad, 0);
	FRoadEdge Edge4 = CreateRoadEdge(Node4, Node1, ERoadType::MinorRoad, 0);

	RoadNodesArray.Add(Node1);
	RoadNodesArray.Add(Node2);
	RoadNodesArray.Add(Node3);
	RoadNodesArray.Add(Node4);
	RoadHalfEdges.Add(Edge1.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge1.GetHalfEdgeB());
	RoadHalfEdges.Add(Edge2.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge2.GetHalfEdgeB());
	RoadHalfEdges.Add(Edge3.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge3.GetHalfEdgeB());
	RoadHalfEdges.Add(Edge4.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge4.GetHalfEdgeB());
	
	return CreateCityBlock(CityBlockBlueprint, World, RoadHalfEdges, RoadNodesArray);
}

ACityBlock* RoadNetworkFactory::CreatePolygonCity(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World, uint16 EdgesCount)
{
	TArray<FRoadHalfEdge*> RoadHalfEdges;
	TArray<ARoadNode*> RoadNodesArray;

	for (int i = 0; i < EdgesCount; i++)
	{
		constexpr float PI2 = PI * 2.0f;
		float Angle = PI2 * i / EdgesCount;
		float CosAngle = FMath::Cos(Angle);
		float SinAngle = FMath::Sin(Angle);
		ARoadNode* NewRoadNode = CreateRoadNodeAt(World, FVector(CosAngle, SinAngle, 0.0f) * 5000.0f);
		RoadNodesArray.Add(NewRoadNode);
	}

	for (int i = 0; i < EdgesCount; i++)
	{
		ARoadNode* CurrentRoadNode = RoadNodesArray[i];
		ARoadNode* NextRoadNode = RoadNodesArray[(i + 1) % EdgesCount];
		FRoadEdge NewRoadEdge = CreateRoadEdge(CurrentRoadNode, NextRoadNode, ERoadType::MinorRoad, 0);
		RoadHalfEdges.Add(NewRoadEdge.GetHalfEdgeA());
		RoadHalfEdges.Add(NewRoadEdge.GetHalfEdgeB());
	}

	return CreateCityBlock(CityBlockBlueprint, World, RoadHalfEdges, RoadNodesArray);;
}

ACityBlock* RoadNetworkFactory::CreateSquareCityWithStrayInsideRoad(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World)
{
	TArray<FRoadHalfEdge*> RoadHalfEdges;
	TArray<ARoadNode*> RoadNodesArray;
	
	ARoadNode* Node1 = CreateRoadNodeAt(World, FVector(-5000.0f, -5000.0f, 0.0f));
	ARoadNode* Node2 = CreateRoadNodeAt(World, FVector(5000.0f, -5000.0f, 0.0f));
	ARoadNode* Node3 = CreateRoadNodeAt(World, FVector(5000.0f, 5000.0f, 0.0f));
	ARoadNode* Node4 = CreateRoadNodeAt(World, FVector(2500.0f, 3500.0f, 0.0f));
	ARoadNode* Node5 = CreateRoadNodeAt(World, FVector(0.0f, 0.0f, 0.0f));
	ARoadNode* Node6 = CreateRoadNodeAt(World, FVector(-5000.0f, 5000.0f, 0.0f));
	FRoadEdge Edge1 = CreateRoadEdge(Node1, Node2, ERoadType::MinorRoad, 0);
	FRoadEdge Edge2 = CreateRoadEdge(Node2, Node3, ERoadType::MinorRoad, 0);
	FRoadEdge Edge3 = CreateRoadEdge(Node3, Node4, ERoadType::MinorRoad, 0);
	FRoadEdge Edge4 = CreateRoadEdge(Node4, Node5, ERoadType::MinorRoad, 0);
	FRoadEdge Edge5 = CreateRoadEdge(Node3, Node6, ERoadType::MinorRoad, 0);
	FRoadEdge Edge6 = CreateRoadEdge(Node6, Node1, ERoadType::MinorRoad, 0);

	RoadNodesArray.Add(Node1);
	RoadNodesArray.Add(Node2);
	RoadNodesArray.Add(Node3);
	RoadNodesArray.Add(Node4);
	RoadNodesArray.Add(Node5);
	RoadNodesArray.Add(Node6);
	RoadHalfEdges.Add(Edge1.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge1.GetHalfEdgeB());
	RoadHalfEdges.Add(Edge2.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge2.GetHalfEdgeB());
	RoadHalfEdges.Add(Edge3.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge3.GetHalfEdgeB());
	RoadHalfEdges.Add(Edge4.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge4.GetHalfEdgeB());
	RoadHalfEdges.Add(Edge5.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge5.GetHalfEdgeB());
	RoadHalfEdges.Add(Edge6.GetHalfEdgeA());
	RoadHalfEdges.Add(Edge6.GetHalfEdgeB());
	
	return CreateCityBlock(CityBlockBlueprint, World, RoadHalfEdges, RoadNodesArray);
}

ACityBlock* RoadNetworkFactory::CreateCityBlock(TSubclassOf<ACityBlock> CityBlockBlueprint, UWorld* World, const TArray<FRoadHalfEdge*>& NewRoadHalfEdges, const TArray<ARoadNode*>& NewRoadNodes)
{
	ACityBlock* CityBlock = World->SpawnActor<ACityBlock>(CityBlockBlueprint.Get());
	CityBlock->InjectRoads(NewRoadHalfEdges, NewRoadNodes);
	return CityBlock;
}
