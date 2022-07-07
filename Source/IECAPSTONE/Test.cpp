// Fill out your copyright notice in the Description page of Project Settings.


#include "Test.h"
#include "CityBlock.h"
#include "CityLot.h"
#include "CustomMath.h"
#include "DrawDebugHelpers.h"
#include "RoadHalfEdge.h"
#include "RoadNetworkFactory.h"
#include "RoadNode.h"
#include "Kismet/KismetRenderingLibrary.h"

ATest::ATest()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

void ATest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// DEBUG: Draw edges
	DebugDrawCity();
}

void ATest::GeneratePerlinNoiseTexture()
{
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), NoiseRenderTarget, NoiseGenerationMaterial);
}

void ATest::InitializeCity()
{
	if (FinalCity == nullptr)
	{
		//////UE_LOG(LogTemp, Error, TEXT("Can't initialize basic city when it doesn't exist!"));
		return;
	}
	
	FinalCity->InitializeRoadHalfEdges();
	FinalCity->InitializeCityLots();
	FinalCity->RemoveStrayRoadsInCityLots();
	FinalCity->TriangulateCityLots();
	FinalCity->RemoveSmallCityLots();
	// FinalCity->InitializeCityZones();

	// DEBUG: Draw triangles
	for (ACityLot* CityLot : FinalCity->GetAllCityLots())
	{
		CityLot->DebugDrawCityZoneMesh(GetWorld());
	}
}

void ATest::CreateBasicCity()
{
	ClearAll();
	
	FinalCity = RoadNetworkFactory::CreatePolygonCity(CityBlockBlueprint, GetWorld(), 8);
}

void ATest::CreateSquareCityWithStrayInsideRoad()
{
	ClearAll();

	FinalCity = RoadNetworkFactory::CreateSquareCityWithStrayInsideRoad(CityBlockBlueprint, GetWorld());
}

void ATest::CreateComplexCityFromBaseCities()
{
	ClearAll();

	AllBaseCitiesArray.Add(RoadNetworkFactory::CreateSquareCity(CityBlockBlueprint, GetWorld()));
	AllBaseCitiesArray.Add(RoadNetworkFactory::CreateVerticalCity(CityBlockBlueprint, GetWorld()));
	AllBaseCitiesArray.Add(RoadNetworkFactory::CreateHorizontalCity(CityBlockBlueprint, GetWorld()));
	AllBaseCitiesArray.Add(RoadNetworkFactory::CreateSlantedCity(CityBlockBlueprint, GetWorld()));
	
	// MERGING
	FinalCity = AllBaseCitiesArray[0]->DeepCopyRoadsAsCityBlock();

	TSet<ARoadNode*> AddedRoadNodes;
	const TArray<FRoadHalfEdge*>& FinalHalfEdgesArray = FinalCity->GetAllRoadHalfEdges();

	// Merge all cities into final city
	for (int i = 1; i < AllBaseCitiesArray.Num(); i++)
	{
		// Deep copy edges and nodes
		const ACityBlock* OtherCity = AllBaseCitiesArray[i];
		TTuple<TArray<FRoadHalfEdge*>, TArray<ARoadNode*>> OtherRoads = OtherCity->DeepCopyRoads();
		const TArray<FRoadHalfEdge*>& OtherHalfEdgesArray = OtherRoads.Get<0>();
		const int32 FinalCityOriginalHalfEdgesCount = FinalHalfEdgesArray.Num();

		// Copy roads into final city
		TMap<int32, int32> NewRoadIDMap;
		
		for (FRoadHalfEdge* RoadHalfEdge : OtherHalfEdgesArray)
		{
			ARoadNode* OriginRoadNode = RoadHalfEdge->GetOriginRoadNode();
			
			// Copy nodes into final city
			if (!AddedRoadNodes.Contains(OriginRoadNode))
			{
				FinalCity->AddRoadNode(OriginRoadNode);
				AddedRoadNodes.Add(OriginRoadNode);
			}
			
			// Might have road ID conflicts. Will generate random IDs. (DOES NOT CHECK IF ID ALREADY EXISTS)
			if (int32* IDPointer = NewRoadIDMap.Find(RoadHalfEdge->GetRoadID()))
			{
				RoadHalfEdge->SetRoadID(*IDPointer);
			}
			else
			{
				int32 NewID = FMath::Rand();
				NewRoadIDMap.Add(RoadHalfEdge->GetRoadID(), NewID);
				RoadHalfEdge->SetRoadID(NewID);
			}
			
			// Copy half edges into final city
			FinalCity->AddRoadHalfEdge(RoadHalfEdge);
		}
		
		// Iterate through only every other half edge. Array is sorted in pairs.
		for (int j = 0; j < FinalHalfEdgesArray.Num(); j += 2)
		{
			const int32 OtherHalfEdgesCount = FinalHalfEdgesArray.Num();
			FRoadHalfEdge* FinalCityHalfEdge = FinalHalfEdgesArray[j];
			ARoadNode* FinalCityNodeA = FinalCityHalfEdge->GetOriginRoadNode();

			// Check if intersecting with other city edge
			for (int k = FMath::Max(FinalCityOriginalHalfEdgesCount, j + 2); k < OtherHalfEdgesCount; k += 2)
			{
				// The "next" node can change when found another intersecting node
				ARoadNode* FinalCityNodeB = FinalCityHalfEdge->GetNextRoadNode();

				// Get current road edge's end points
				FVector FinalNodePosA = FinalCityNodeA->GetActorLocation();
				FVector FinalNodePosB = FinalCityNodeB->GetActorLocation();
				FLineSegment FinalLineSegment(FinalNodePosA, FinalNodePosB);

				// Get other road edge's end points
				FRoadHalfEdge* OtherHalfEdge = FinalHalfEdgesArray[k];
				ARoadNode* OtherNodeA = OtherHalfEdge->GetOriginRoadNode();
				ARoadNode* OtherNodeB = OtherHalfEdge->GetNextRoadNode();
				FVector OtherNodePosA = OtherNodeA->GetActorLocation();
				FVector OtherNodePosB = OtherNodeB->GetActorLocation();
				FLineSegment OtherLineSegment(OtherNodePosA, OtherNodePosB);

				// If edges have at least one same node, skip.
				if (FinalNodePosA.Equals(OtherNodePosA) || FinalNodePosA.Equals(OtherNodePosB) || FinalNodePosB.Equals(OtherNodePosA) || FinalNodePosB.Equals(OtherNodePosB))
				{
					continue;
				}
				
				FVector IntersectionPos;

				if (CustomMath::GetIntersection(FinalLineSegment, OtherLineSegment, IntersectionPos))
				{
					// Debug: Log intersection position
					// ////UE_LOG(LogTemp, Warning, TEXT("Found Intersection: X: %f, Y: %f, Z: %f"), IntersectionPos.X, IntersectionPos.Y, IntersectionPos.Z);
					
					// 1. Instantiate new node at intersection
					ARoadNode* IntersectionRoadNode = RoadNetworkFactory::CreateRoadNodeAt(GetWorld(), IntersectionPos);
					FinalCity->AddRoadNode(IntersectionRoadNode);
					AddedRoadNodes.Add(IntersectionRoadNode);

					// 2. Reconnect the end of the first part of existing edge to the intersection node
					FRoadHalfEdge* FinalPairHalfEdge = FinalCityHalfEdge->GetPairRoadHalfEdge();
					FinalPairHalfEdge->SetOriginRoadNode(IntersectionRoadNode);

					// 3. Instantiate new edge pair from intersection node to old node and append to final array
					FRoadEdge NewRoadEdge = RoadNetworkFactory::CreateRoadEdge(IntersectionRoadNode, FinalCityNodeB, FinalCityHalfEdge->GetRoadType(), FinalCityHalfEdge->GetRoadID());
					FinalCity->AddRoadHalfEdge(NewRoadEdge.GetHalfEdgeA());
					FinalCity->AddRoadHalfEdge(NewRoadEdge.GetHalfEdgeB());

					// 4. Repeat step 2 but for other edge
					FRoadHalfEdge* OtherCityPairHalfEdge = OtherHalfEdge->GetPairRoadHalfEdge();
					OtherCityPairHalfEdge->SetOriginRoadNode(IntersectionRoadNode);

					// 5. Repeat step 3 but for other edge
					FRoadEdge NewOtherRoadEdge = RoadNetworkFactory::CreateRoadEdge(IntersectionRoadNode, OtherNodeB, OtherHalfEdge->GetRoadType(), OtherHalfEdge->GetRoadID());
					FinalCity->AddRoadHalfEdge(NewOtherRoadEdge.GetHalfEdgeA());
					FinalCity->AddRoadHalfEdge(NewOtherRoadEdge.GetHalfEdgeB());
				}
			}
		}
	}
}

void ATest::GenerateCityLots()
{
	FinalCity->InitializeRoadHalfEdges();
	FinalCity->InitializeCityLots();
	FinalCity->RemoveStrayRoadsInCityLots();
	FinalCity->SortRoadHalfEdgesInPairs();
	// FinalCity->RemoveSmallCityLots();
	// FinalCity->InitializeCityZones();
	// FinalCity->ShowAllCityLotsZoningColor();
}

void ATest::ClearAll()
{
	FlushPersistentDebugLines(GetWorld());
	
	for (ACityBlock* CityBlock : AllBaseCitiesArray)
	{
		CityBlock->Destroy();
	}
	
	AllBaseCitiesArray.Empty();

	if (FinalCity != nullptr)
	{
		FinalCity->Destroy();
		FinalCity = nullptr;
	}
}

void ATest::DebugDrawCity()
{
	if (FinalCity == nullptr)
	{
		return;
	}
	
	for (int i = 0; i < FinalCity->GetAllRoadHalfEdges().Num(); i += 2)
	{
		FRoadHalfEdge* RoadHalfEdge = FinalCity->GetAllRoadHalfEdges()[i];
		FVector LineStart = RoadHalfEdge->GetOriginRoadNode()->GetActorLocation();
		FVector LineEnd = RoadHalfEdge->GetNextRoadNode()->GetActorLocation();
		
		/*DrawDebugLine(GetWorld(),
			LineStart,
			LineEnd,
			FColor::Red,
			false,
			-1.0f,
			0,
			DebugLineThickness);*/
	}
}

bool ATest::ShouldTickIfViewportsOnly() const
{
	return true;
}
