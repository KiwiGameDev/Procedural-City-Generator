// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadNetwork.h"
#include "CityBlock.h"
#include "CityLot.h"
#include "CustomMath.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"
#include "RoadHalfEdge.h"
#include "RoadNode.h"
#include "StreetMap.h"
#include "RoadActor.h"
#include "RoadNetworkFactory.h"
#include "Components/DecalComponent.h"
#include "Detail.h"
#include "Kismet/GameplayStatics.h"

ARoadNetwork::ARoadNetwork()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bTickEvenWhenPaused = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
	
	SidewalkProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("SidewalkProceduralMeshComponent"));
}

void ARoadNetwork::BeginPlay()
{
	Super::BeginPlay();

	ClearRoadNetwork();
	SpawnAllBuildingInstanceActors();

	// Spawn Parent Detail
	for (TSubclassOf<ADetail> detail : DetailsDataAsset->GetDetailArray())
	{
		this->DetailInstance.Add(GetWorld()->SpawnActor<ADetail>(detail, FVector::ZeroVector, FRotator::ZeroRotator));
	}
}

void ARoadNetwork::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
}

void ARoadNetwork::GenerateRandomRoadNetwork()
{
	RandomizeSeed();
	GenerateRoadNetwork();
}

void ARoadNetwork::GenerateRoadNetwork()
{
	// Cleanup and Reset
	ClearRoadNetwork();
	FMath::RandInit(Seed);

	// Create base cities
	CreateCityFromOSM();

	// Cleanup & initialize base cities
	RemoveDisconnectedRoadsInBaseCities();
	MergeSameRoadHalfEdgesInBaseCities();
	SortRoadHalfEdgePairsInBaseCities();

	// Generate final city
	MergeBaseCitiesToFinalCity();
	
	// DEBUG: Draw debug lines of roads
	DrawDebugRoadsOfFinalCity();
	DrawDebugRoadNodesOfFinalCity();
}

void ARoadNetwork::GenerateCityLots()
{
	FlushPersistentDebugLines(GetWorld());

	FinalCity->Initialize(this);

	// DEBUG: Draw debug lines of roads
	DrawDebugRoadsOfFinalCity();
	DrawDebugRoadNodesOfFinalCity();
}

void ARoadNetwork::GenerateBuildings()
{
	if (FinalCity == nullptr)
	{
		return;
	}

	DestroyAllBuildingInstances();
	
	FinalCity->SpawnBuildings(SubdivideDepth, this);
	FinalCity->HideAllCityLotsZoningColor();
}

void ARoadNetwork::callSpawnDetails()
{
	if (this->FinalCity == nullptr)
	{
		return;
	}
	
	if (DetailsDataAsset == nullptr)
	{
		return;
	}

	if (this->DetailInstance.Num() == 0)
	{
		return;
	}
	
	FinalCity->callSpawnDetails(this, DetailsDensity);
}

void ARoadNetwork::GenerateRoadMesh()
{
	if (FinalCity == nullptr)
	{
		return;
	}
	
	const TDoubleLinkedList<ACityLot*>& CityLots = FinalCity->GetAllCityLots();
	const TArray<FRoadHalfEdge*>& HalfEdgesList = FinalCity->GetAllRoadHalfEdges();
	const TArray<ARoadNode*>& NodesList = FinalCity->GetAllRoadNodes();
	TMap<int, FRoadMap*> RoadMapList;
	
	for (int i = 0; i < HalfEdgesList.Num(); i += 2)
	{
		int id = HalfEdgesList[i]->GetRoadID();
		
		if (!RoadMapList.Contains(id))
		{
			FRoadMap* roadMap = new FRoadMap();
			RoadMapList.Add(id, roadMap);
			RoadMapList[id]->HalfEdges.Add(HalfEdgesList[i]);
		}
		else
		{
			RoadMapList[id]->HalfEdges.Add(HalfEdgesList[i]);
		}
	}
	
	TArray<int> keyList;
	RoadMapList.GetKeys(keyList);
	
	for(int i = 0 ; i < keyList.Num(); i++)
	{

		const TArray<FRoadHalfEdge*> edgesList = RoadMapList[keyList[i]]->HalfEdges;
		
		if(edgesList.Num() > 0)
		{
			FRoadHalfEdge* firstEdge = edgesList[0];
			FVector direction = CustomMath::GetDirection(firstEdge->GetNextRoadNode()->GetActorLocation(), firstEdge->GetOriginRoadNode()->GetActorLocation());
			FRotator rotation = direction.Rotation();
			FTransform spawnTransform(rotation, firstEdge->GetOriginRoadNode()->GetActorLocation());

			ARoadActor* DeferredActor = CastChecked<ARoadActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, RoadMeshBlueprintToSpawn, spawnTransform));
			if (DeferredActor != nullptr)
			{
				DeferredActor->initialize(edgesList, keyList[i]);
				DeferredActor->FinishSpawning(spawnTransform);
				AllRoadMeshArray.Add(DeferredActor);
			}
		}
	}
	
	TArray<ARoadNode*> RoadNodes = FinalCity->GetAllRoadNodes();
	
	for (ARoadNode* Node : RoadNodes)
	{
		// Get all emanating roads and check if it's more than 2
		TArray<FRoadHalfEdge*> edgeList = Node->GetEmanatingRoadHalfEdges();
		if(edgeList.Num() >= 4)
		{
			for (int i = 0; i < edgeList.Num(); i++)
			{
				if(edgeList[i]->GetRoadType() == ERoadType::MajorRoad) //just do it for all for testing purposes
				{
					FVector decalPos;
					FRotator decalRot;
					FVector a = edgeList[i]->GetOriginRoadNode()->GetActorLocation();
					FVector b = edgeList[i]->GetNextRoadNode()->GetActorLocation();
					FVector dir = b - a;
					dir.Normalize();

					float len;
					float off = 4000;
					decalPos = a + dir * off;
					dir = dir.RotateAngleAxis(90, dir.ZAxisVector);
					dir.ToDirectionAndLength(dir, len);
					decalRot = dir.ToOrientationRotator();

					FTransform finalTransform(decalRot, decalPos);
					FActorSpawnParameters spawnParams;

					ADecalActor* decalActor = GetWorld()->SpawnActor<ADecalActor>(PedestrianDecalActor.Get(), decalPos, decalRot, spawnParams);
					if (decalActor)
					{
						/*DrawDebugDirectionalArrow(this->GetWorld(), a, decalPos, 20, FColor::Red, true, -1, 0,
							50);*/
						decalActor->GetDecal()->DecalSize = FVector(32.0f, 64.0f, 64.0f);
						AllDecalActors.Add(decalActor);
					}

					FVector c = edgeList[i]->GetNextRoadNode()->GetActorLocation();
					FVector d = a - c;
					FVector m = d.RotateAngleAxis(90, d.ZAxisVector);
					FVector dirr;
					m.ToDirectionAndLength(dirr, len);
					FVector stopPos = decalPos + dirr * 3000;
					FRotator stopRot = decalRot;

					AActor* stoplight = GetWorld()->SpawnActor<AActor>(StoplightClass.Get(), stopPos, stopRot, spawnParams);
					AllStoplightActors.Add(stoplight);
				}
			}
		}
	}
}

void ARoadNetwork::GenerateElectricPoles()
{
	if (!SpawnElectricPoles || AllRoadMeshArray.Num() == 0)
	{
		return;
	}
	
	for (auto roads: AllRoadMeshArray)
	{
		if (roads != nullptr)
		{
			roads->SpawnElectricPolesFromRoadActor();
		}
	}
}

void ARoadNetwork::GenerateSidewalk()
{
	if (!SpawnSidewalk || FinalCity == nullptr)
	{
		return;
	}

	for (ACityLot* CityLot : FinalCity->GetAllCityLots())
	{
		if (SpawnParametersAsset->GetSpawnParameterOfCityZoneType(CityLot->GetCityZoneType()).CanSpawnSidewalk)
		{
			CityLot->SpawnSidewalkFromComponent(this);
		}
	}
}

void ARoadNetwork::RegenerateFinalCity()
{
	if (FinalCity == nullptr)
	{
		return;
	}
	
	bool bWasFinalCityLotsInitialized = FinalCity->IsCityLotsInitialized();
	bool bWasFinalCityLotZoneColorsVisible = FinalCity->IsCityCityLotZoneColorsVisible();

	if (AllBaseCitiesArray.Find(FinalCity) == INDEX_NONE)
	{
		DestroyFinalCity();
		MergeBaseCitiesToFinalCity();
	}
	else
	{
		DestroyAllBuildingInstances();
		DestroyAllVisualActors();
	}
	
	if (bWasFinalCityLotsInitialized)
	{
		GenerateCityLots();

		if (bWasFinalCityLotZoneColorsVisible)
		{
			FinalCity->ShowAllCityLotsZoningColor();
		}
	}
}

void ARoadNetwork::RandomizeOSM()
{
	if (AllStreetMapsArray.Num() == 0)
	{
		return;
	}
	
	StreetMapsArray[0] = AllStreetMapsArray[FMath::Rand() % AllStreetMapsArray.Num()];
}

void ARoadNetwork::CreateCityFromOSM()
{
	// Convert OSM roads to road half-edges
	for (UStreetMap* StreetMap : StreetMapsArray)
	{
		const TArray<FStreetMapRoad>& StreetMapRoads = StreetMap->GetRoads();
		TArray<FRoadHalfEdge*> RoadHalfEdgesArray;
		TArray<ARoadNode*> RoadNodesArray;

		for (int i = 0; i < StreetMapRoads.Num(); i++)
		{
			const FStreetMapRoad& Road = StreetMapRoads[i];
			const TArray<FVector2D>& RoadPositionsArray = Road.RoadPoints;

			if (RoadPositionsArray.Num() < 2)
			{
				continue;
			}

			// Instantiate road nodes or find it if already exists
			FVector2D ModifiedRoadPos = RoadPositionsArray[0];
			ModifiedRoadPos.X *= RoadNetworkScaleX;
			ModifiedRoadPos.Y *= RoadNetworkScaleY;

			FVector CurrentRoadNodePos = FVector(ModifiedRoadPos, 0.0f);
			ARoadNode* CurrentRoadNode = nullptr;

			if (!FindOrCreateRoadNodeAt(CurrentRoadNodePos, RoadNodesArray, CurrentRoadNode))
			{
				RoadNodesArray.Add(CurrentRoadNode);
			}
			
			for (int j = 1; j < RoadPositionsArray.Num(); j++)
			{
				ModifiedRoadPos = RoadPositionsArray[j];
				ModifiedRoadPos.X *= RoadNetworkScaleX;
				ModifiedRoadPos.Y *= RoadNetworkScaleY;

				FVector NextRoadPos = FVector(ModifiedRoadPos, 0.0f);
				ARoadNode* NextRoadNode = nullptr;
				
				if (!FindOrCreateRoadNodeAt(NextRoadPos, RoadNodesArray, NextRoadNode))
				{
					RoadNodesArray.Add(NextRoadNode);
				}

				const ERoadType RoadType = GetRoadTypeFromOSM(Road.RoadType);
				const int32 RoadID = Road.GetRoadIndex(*StreetMap);
				FRoadEdge RoadEdge = RoadNetworkFactory::CreateRoadEdge(CurrentRoadNode, NextRoadNode, RoadType, RoadID);
				RoadHalfEdgesArray.Add(RoadEdge.GetHalfEdgeA());
				RoadHalfEdgesArray.Add(RoadEdge.GetHalfEdgeB());

				CurrentRoadNode = NextRoadNode;
			}
		}
		
		ACityBlock* CityBlock = RoadNetworkFactory::CreateCityBlock(CityBlockBlueprint, GetWorld(), RoadHalfEdgesArray, RoadNodesArray);
		AllBaseCitiesArray.Add(CityBlock);
	}
}

void ARoadNetwork::RemoveDisconnectedRoadsInBaseCities()
{
	for (ACityBlock* CityBlock : AllBaseCitiesArray)
	{
		CityBlock->RemoveDisconnectedRoads();
	}
}

void ARoadNetwork::MergeSameRoadHalfEdgesInBaseCities()
{
	for (ACityBlock* CityBlock : AllBaseCitiesArray)
	{
		CityBlock->MergeSameRoadHalfEdges();
	}
}

void ARoadNetwork::SortRoadHalfEdgePairsInBaseCities()
{
	for (ACityBlock* CityBlock : AllBaseCitiesArray)
	{
		CityBlock->SortRoadHalfEdgesInPairs();
	}
}

void ARoadNetwork::MergeBaseCitiesToFinalCity()
{
	if (AllBaseCitiesArray.Num() == 0)
	{
		if (FinalCity != nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Application is in an invalid state! Final city exists when no base cities exist!"));
			DestroyFinalCity();
		}

		return;
	}

	// Just reference the base city if there are no cities to merge
	if (AllBaseCitiesArray.Num() == 1)
	{
		FinalCity = AllBaseCitiesArray[0];
		return;
	}

	// Delete previous final city if it's not just referencing a base city
	if (FinalCity != nullptr && FinalCity != AllBaseCitiesArray[0])
	{
		FinalCity->Destroy();
	}

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
			ARoadNode* FinalNodeA = FinalCityHalfEdge->GetOriginRoadNode();

			// Check if intersecting with other city edge
			for (int k = FMath::Max(FinalCityOriginalHalfEdgesCount, j + 2); k < OtherHalfEdgesCount; k += 2)
			{
				// The "next" node can change when found another intersecting node
				ARoadNode* FinalNodeB = FinalCityHalfEdge->GetNextRoadNode();

				// Get current road edge's end points
				FVector FinalNodePosA = FinalNodeA->GetActorLocation();
				FVector FinalNodePosB = FinalNodeB->GetActorLocation();
				FLineSegment FinalLineSegment(FinalNodePosA, FinalNodePosB);

				// Get other road edge's end points
				FRoadHalfEdge* OtherHalfEdge = FinalHalfEdgesArray[k];
				ARoadNode* OtherNodeA = OtherHalfEdge->GetOriginRoadNode();
				ARoadNode* OtherNodeB = OtherHalfEdge->GetNextRoadNode();
				FVector OtherNodePosA = OtherNodeA->GetActorLocation();
				FVector OtherNodePosB = OtherNodeB->GetActorLocation();
				FLineSegment OtherLineSegment(OtherNodePosA, OtherNodePosB);

				// If edges have at least one same node, skip.
				if (FinalNodeA == OtherNodeA || FinalNodeA == OtherNodeB || FinalNodeB == OtherNodeA || FinalNodeB == OtherNodeB)
				{
					continue;
				}
				
				FVector IntersectionPos;

				if (CustomMath::GetIntersection(FinalLineSegment, OtherLineSegment, IntersectionPos))
				{
					// Debug: Log intersection position
					// UE_LOG(LogTemp, Warning, TEXT("Found Intersection: X: %f, Y: %f, Z: %f"), IntersectionPos.X, IntersectionPos.Y, IntersectionPos.Z);
					
					// 1. Instantiate new node at intersection
					ARoadNode* IntersectionRoadNode = nullptr;

					bool bIsEqualsFinalNodePosA = FinalNodePosA.Equals(IntersectionPos);
					bool bIsEqualsFinalNodePosB = FinalNodePosB.Equals(IntersectionPos);
					bool bIsEqualsOtherNodePosA = OtherNodePosA.Equals(IntersectionPos);
					bool bIsEqualsOtherNodePosB = OtherNodePosB.Equals(IntersectionPos);
					
					if (bIsEqualsFinalNodePosA)
					{
						IntersectionRoadNode = FinalNodeA;
					}
					else if (bIsEqualsFinalNodePosB)
					{
						IntersectionRoadNode = FinalNodeB;
					}
					else if (bIsEqualsOtherNodePosA)
					{
						IntersectionRoadNode = OtherNodeA;
					}
					else if (bIsEqualsOtherNodePosB)
					{
						IntersectionRoadNode = OtherNodeB;
					}
					else
					{
						IntersectionRoadNode = RoadNetworkFactory::CreateRoadNodeAt(GetWorld(), IntersectionPos);
						FinalCity->AddRoadNode(IntersectionRoadNode);
						AddedRoadNodes.Add(IntersectionRoadNode);
					}

					if (!bIsEqualsFinalNodePosA && !bIsEqualsFinalNodePosB)
					{
						// 2. Reconnect the end of the first part of existing edge to the intersection node
						FRoadHalfEdge* FinalPairHalfEdge = FinalCityHalfEdge->GetPairRoadHalfEdge();
						FinalPairHalfEdge->SetOriginRoadNode(IntersectionRoadNode);

						// 3. Instantiate new edge pair from intersection node to old node and append to final array
						FRoadEdge NewRoadEdge = RoadNetworkFactory::CreateRoadEdge(IntersectionRoadNode, FinalNodeB, FinalCityHalfEdge->GetRoadType(), FinalCityHalfEdge->GetRoadID());
						FinalCity->AddRoadHalfEdge(NewRoadEdge.GetHalfEdgeA());
						FinalCity->AddRoadHalfEdge(NewRoadEdge.GetHalfEdgeB());
					}
					
					if (!bIsEqualsOtherNodePosA && !bIsEqualsOtherNodePosB)
					{
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
}

ERoadType ARoadNetwork::GetRoadTypeFromOSM(EStreetMapRoadType StreetMapRoadType)
{
	if (StreetMapRoadType == Street)
		return ERoadType::MinorRoad;
	if (StreetMapRoadType == MajorRoad)
		return ERoadType::MajorRoad;
	if (StreetMapRoadType == Highway)
		return ERoadType::Highway;
	if (StreetMapRoadType == Other)
		return ERoadType::Other;
	return ERoadType::Invalid;
}

void ARoadNetwork::DrawDebugRoadsOfBaseCities()
{
	if (AllBaseCitiesArray.Num() == 0)
	{
		return;
	}
	
	for (ACityBlock* CityBlock : AllBaseCitiesArray)
	{
		const TArray<FRoadHalfEdge*>& RoadHalfEdgesArray = CityBlock->GetAllRoadHalfEdges();
		
		for (int i = 0; i < RoadHalfEdgesArray.Num(); i += 2)
		{
			FRoadHalfEdge* RoadHalfEdge = RoadHalfEdgesArray[i];
			
			DrawDebugLine(GetWorld(),
				RoadHalfEdge->GetOriginRoadNode()->GetActorLocation(),
				RoadHalfEdge->GetNextRoadNode()->GetActorLocation(),
				FColor::Red,
				true,
				-1.0f,
				0,
				DebugLinesRadius * 0.5f);//
		}
	}
}

void ARoadNetwork::DrawDebugRoadsOfFinalCity()
{
	if (FinalCity == nullptr)
	{
		return;
	}
	
	const TArray<FRoadHalfEdge*>& RoadHalfEdgesArray = FinalCity->GetAllRoadHalfEdges();
		
	for (int i = 0; i < RoadHalfEdgesArray.Num(); i += 2)
	{
		FRoadHalfEdge* RoadHalfEdge = RoadHalfEdgesArray[i];
			
		DrawDebugLine(GetWorld(),
			RoadHalfEdge->GetOriginRoadNode()->GetActorLocation(),
			RoadHalfEdge->GetNextRoadNode()->GetActorLocation(),
			FColor::Red,
			true,
			-1.0f,
			0,
			DebugLinesRadius * 0.5f);//
	}
}

void ARoadNetwork::DrawDebugRoadNodesOfFinalCity()
{
	if (FinalCity == nullptr)
	{
		return;
	}
	
	for (ARoadNode* RoadNode : FinalCity->GetAllRoadNodes())
	{
		DrawDebugSphere(GetWorld(), RoadNode->GetActorLocation(), 128.0f, 8, FColor::Cyan, true);
	}
}

void ARoadNetwork::DrawDebugCityZoneMeshOfFinalCity()
{
	for (ACityLot* CityLot : FinalCity->GetAllCityLots())
	{
		CityLot->DebugDrawCityZoneMesh(GetWorld());
	}
}

bool ARoadNetwork::FindOrCreateRoadNodeAt(const FVector& Position, const TArray<ARoadNode*>& RoadNodesArray, ARoadNode*& RoadNode) const
{			
	for (int k = 0; k < RoadNodesArray.Num(); k++)
	{
		const float E = 1.0f;
		ARoadNode* OtherRoadNode = RoadNodesArray[k];

		// If road is "close enough" to another node, assume it's the same
		if (FVector2D::DistSquared(FVector2D(Position), FVector2D(OtherRoadNode->GetActorLocation())) <= E)
		{
			// Found existing road node
			RoadNode = OtherRoadNode;
			return true;
		}
	}

	// If not found road, instantiate new road node
	RoadNode = RoadNetworkFactory::CreateRoadNodeAt(GetWorld(), Position);
	return false;
}

void ARoadNetwork::ClearRoadNetwork()
{
	FlushPersistentDebugLines(GetWorld());

	DestroyFinalCity();
	
	for (ACityBlock* CityBlock : AllBaseCitiesArray)
	{
		CityBlock->Destroy();
	}
	
	AllBaseCitiesArray.Empty();
}

void ARoadNetwork::ClearBuildings()
{
	if (FinalCity != nullptr)
	{
		for (ACityLot* CityLot : FinalCity->GetAllCityLots())
		{
			if (CityLot != nullptr)
			{
				CityLot->LotComponent->DestroyAll();
				CityLot->ClearSubdivisions();
			}
		}
	}

	DestroyAllBuildingInstances();
}

void ARoadNetwork::RandomizeSeed()
{
	FMath::RandInit(FPlatformTime::Cycles());
	Seed = FMath::Rand();
}

void ARoadNetwork::DestroyFinalCity()
{
	if(FinalCity != nullptr)
	{
		for (ACityLot* CityLot : FinalCity->GetAllCityLots())
		{
			if (CityLot != nullptr)
			{
				CityLot->LotComponent->DestroyAll();
			}
		}
	}
	
	if (FinalCity != nullptr)
	{
		FinalCity->Destroy();
		FinalCity = nullptr;
	}

	DestroyAllBuildingInstances();
	DestroyAllVisualActors();
}

void ARoadNetwork::DestroyAllBuildingInstances()
{
	for (ABuilding* Building : LowRiseResidentialBuildingInstances)
	{
		if (Building != nullptr)
		{
			Building->GetInstancedStaticMesh()->ClearInstances();
		}
	}
	
	for (ABuilding* Building : HighRiseResidentialBuildingInstances)
	{
		if (Building != nullptr)
		{
			Building->GetInstancedStaticMesh()->ClearInstances();
		}
	}

	for (ABuilding* Building : LowRiseCommercialBuildingInstances)
	{
		if (Building != nullptr)
		{
			Building->GetInstancedStaticMesh()->ClearInstances();
		}
	}

	for (ABuilding* Building : HighRiseCommercialBuildingInstances)
	{
		if (Building != nullptr)
		{
			Building->GetInstancedStaticMesh()->ClearInstances();
		}
	}

	for (ABuilding* Building : IndustrialBuildingInstances)
	{
		if (Building != nullptr)
		{
			Building->GetInstancedStaticMesh()->ClearInstances();
		}
	}
	
	for (ABuilding* Building : LandmarkBuildingInstances)
	{
		if (Building != nullptr)
		{
			Building->GetInstancedStaticMesh()->ClearInstances();
		}
	}

	for (ADetail* detail : DetailInstance)
	{
		if (detail != nullptr)
		{
			detail->GetInstancedStaticMesh()->ClearInstances();
		}
	}
}

void ARoadNetwork::DestroyAllVisualActors()
{
	// Delete all decals
	for (ADecalActor* DecalList : AllDecalActors)
	{
		if (DecalList != nullptr)
		{
			DecalList->Destroy();
		}
	}

	for (AActor* Stoplightlist : AllStoplightActors)
	{
		if (Stoplightlist != nullptr)
		{
			Stoplightlist->Destroy();
		}
	}
	
	for (ARoadActor* Roads : AllRoadMeshArray)
	{
		if (Roads != nullptr)
		{
			Roads->DestroyPoleListActors();
			Roads->Destroy();
		}
	}

	SidewalkProceduralMeshComponent->ClearAllMeshSections();

	AllDecalActors.Empty();
	AllStoplightActors.Empty();
	AllRoadMeshArray.Empty();
}

void ARoadNetwork::SpawnAllBuildingInstanceActors()
{
	for (TSubclassOf<ABuilding> Building : CityBuildingsDataAsset->GetLowRiseResidentialBuildings())
	{
		LowRiseResidentialBuildingInstances.Add(GetWorld()->SpawnActor<ABuilding>(Building, FVector::ZeroVector, FRotator::ZeroRotator));
		LowRiseResidentialBuildingInstances.Last()->SetBuildingAreaFromStaticMeshBounds();
	}

	for (TSubclassOf<ABuilding> Building : CityBuildingsDataAsset->GetHighRiseResidentialBuildings())
	{
		HighRiseResidentialBuildingInstances.Add(GetWorld()->SpawnActor<ABuilding>(Building, FVector::ZeroVector, FRotator::ZeroRotator));
		HighRiseResidentialBuildingInstances.Last()->SetBuildingAreaFromStaticMeshBounds();
	}

	for (TSubclassOf<ABuilding> Building : CityBuildingsDataAsset->GetLowRiseCommercialBuildings())
	{
		LowRiseCommercialBuildingInstances.Add(GetWorld()->SpawnActor<ABuilding>(Building, FVector::ZeroVector, FRotator::ZeroRotator));
		LowRiseCommercialBuildingInstances.Last()->SetBuildingAreaFromStaticMeshBounds();
	}

	for (TSubclassOf<ABuilding> Building : CityBuildingsDataAsset->GetHighRiseCommercialBuildings())
	{
		HighRiseCommercialBuildingInstances.Add(GetWorld()->SpawnActor<ABuilding>(Building, FVector::ZeroVector, FRotator::ZeroRotator));
		HighRiseCommercialBuildingInstances.Last()->SetBuildingAreaFromStaticMeshBounds();
	}

	for (TSubclassOf<ABuilding> Building : CityBuildingsDataAsset->GetIndustrialBuildings())
	{
		IndustrialBuildingInstances.Add(GetWorld()->SpawnActor<ABuilding>(Building, FVector::ZeroVector, FRotator::ZeroRotator));
		IndustrialBuildingInstances.Last()->SetBuildingAreaFromStaticMeshBounds();
	}

	for (TSubclassOf<ABuilding> Building : CityBuildingsDataAsset->GetLandmarkBuildings())
	{
		LandmarkBuildingInstances.Add(GetWorld()->SpawnActor<ABuilding>(Building, FVector::ZeroVector, FRotator::ZeroRotator));
		LandmarkBuildingInstances.Last()->SetBuildingAreaFromStaticMeshBounds();
	}
}

TArray<ABuilding*> ARoadNetwork::RetrieveBuildingsFromCityZone(ECityZoneType CityZoneType)
{
	if (CityZoneType == ECityZoneType::LowRiseResidential)
	{
		return LowRiseResidentialBuildingInstances.FilterByPredicate([](ABuilding* Building) { return Building->GetIsAllowedToSpawn(); });
	}
	if (CityZoneType == ECityZoneType::HighRiseResidential)
	{
		return HighRiseResidentialBuildingInstances.FilterByPredicate([](ABuilding* Building) { return Building->GetIsAllowedToSpawn(); });
	}
	if (CityZoneType == ECityZoneType::LowRiseCommercial)
	{
		return LowRiseCommercialBuildingInstances.FilterByPredicate([](ABuilding* Building) { return Building->GetIsAllowedToSpawn(); });
	}
	if (CityZoneType == ECityZoneType::HighRiseCommercial)
	{
		return HighRiseCommercialBuildingInstances.FilterByPredicate([](ABuilding* Building) { return Building->GetIsAllowedToSpawn(); });
	}
	if (CityZoneType == ECityZoneType::Industrial)
	{
		return IndustrialBuildingInstances.FilterByPredicate([](ABuilding* Building) { return Building->GetIsAllowedToSpawn(); });
	}
	if (CityZoneType == ECityZoneType::Landmark)
	{
		return LandmarkBuildingInstances.FilterByPredicate([](ABuilding* Building) { return Building->GetIsAllowedToSpawn(); });
	}
	
	return EmptyBuildingInstances;
}

const TArray<int> ARoadNetwork::RetrieveRandomDetailsGroupFromCityZone(ECityZoneType CityZoneType)
{
	return SpawnParametersAsset->GetSpawnParameterOfCityZoneType(CityZoneType).GetRandomDetailGroup();
}

const EBuildingSpawnType ARoadNetwork::RetrieveRandomSpawnTypeFromCityZone(ECityZoneType CityZoneType)
{
	return SpawnParametersAsset->GetSpawnParameterOfCityZoneType(CityZoneType).GetRandomSpawnType();
}

const TArray<ADetail*>& ARoadNetwork::RetrieveDetails()
{
	return DetailInstance;
}

void ARoadNetwork::RemoveDebugLines()
{
	FlushPersistentDebugLines(GetWorld());
}

void ARoadNetwork::ToggleDataTextureMapOfFinalCity()
{
	if (FinalCity == nullptr)
	{
		return;
	}

	FinalCity->ToggleDataTextureMap();
}

void ARoadNetwork::ToggleLotsZoneColorsOfFinalCity()
{
	if (FinalCity == nullptr)
	{
		return;
	}

	FinalCity->ToggleAllCityLotsZoningColor();
}

void ARoadNetwork::SemanticSegmentationMode(bool retainSemanticMode)
{
	if (FinalCity == nullptr)
		return;
	
	if (!bHasSemanticSegmentation)
	{
		// Landscape
		auto LandscapeStaticMeshComponent = Cast<UStaticMeshComponent>(Landscape->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		OriginalLandScapeMaterial = LandscapeStaticMeshComponent->GetMaterial(0);

		// Stoplight
		for (AActor* StoplightActor : AllStoplightActors)
		{
			auto StoplightStaticMeshComponent = Cast<UStaticMeshComponent>(StoplightActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
			OriginalStoplightMaterials.Add(StoplightStaticMeshComponent->GetMaterials());
		}
		
		bHasSemanticSegmentation = true;
	}
	
	if (!retainSemanticMode || !this->SemanticSegmentationModeBool)
	{
		this->SemanticSegmentationModeBool = !this->SemanticSegmentationModeBool;
	}

	if (this->SemanticSegmentationModeBool)
	{
		for (ABuilding* Building : HighRiseResidentialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticHighRiseBuildingMaterial);
		}
		
		for (ABuilding* Building : LowRiseResidentialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticLowRiseBuildingMaterial);
		}
	
		for (ABuilding* Building : HighRiseCommercialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticHighRiseBuildingMaterial);
		}
	
		for (ABuilding* Building : LowRiseCommercialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticLowRiseBuildingMaterial);
		}
		
		for (ABuilding* Building : IndustrialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticLowRiseBuildingMaterial);
		}

		for (ADetail* Detail : DetailInstance)
		{
			Detail->SemanticSegmentationMode(SemanticDetailMaterial);
		}

		for (int i = 0; i < this->AllRoadMeshArray.Num(); i++)
		{
			AllRoadMeshArray[i]->SemanticSegmentationMode(SemanticRoadMaterial, SemanticPoleMaterial);
		}

		for (AActor* Actor : SemanticSegmentationActorsToDisable)
		{
			Actor->SetActorHiddenInGame(true);
		}

		// Landscape
		UStaticMeshComponent* LandscapeMeshComponent = Cast<UStaticMeshComponent>(Landscape->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		LandscapeMeshComponent->SetMaterial(0, SemanticLandscapeMaterial);
		
		// Sidewalks
		for (ACityLot* CityLot : FinalCity->GetAllCityLots())
		{
			CityLot->SemanticSegmentationMode(SemanticSidewalkMaterial, SemanticFenceMaterial);
		}

		// Stoplights
		for (int i = 0; i < AllStoplightActors.Num(); i++)
		{
			AActor* StoplightActor = AllStoplightActors[i];
			auto StoplightStaticMeshComponent = Cast<UStaticMeshComponent>(StoplightActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

			for (int j = 0; j < OriginalStoplightMaterials.Num(); j++)
			{
				StoplightStaticMeshComponent->SetMaterial(j, SemanticStoplightMaterial);
			}
		}
		
		this->GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
	}
	else
	{
		for (ABuilding* Building : this->HighRiseResidentialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticHighRiseBuildingMaterial);
		}
		
		for (ABuilding* Building : this->LowRiseResidentialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticLowRiseBuildingMaterial);
		}
	
		for (ABuilding* Building : this->HighRiseCommercialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticHighRiseBuildingMaterial);
		}
	
		for (ABuilding* Building : this->LowRiseCommercialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticLowRiseBuildingMaterial);
		}

		for (ABuilding* Building : IndustrialBuildingInstances)
		{
			Building->SemanticSegmentationMode(SemanticLowRiseBuildingMaterial);
		}
		
		for (ADetail* Detail : this->DetailInstance)
		{
			Detail->SemanticSegmentationMode(SemanticDetailMaterial);
		}

		if (!retainSemanticMode)
		{
			for (int i = 0; i < this->AllRoadMeshArray.Num(); i++)
			{
				AllRoadMeshArray[i]->SemanticSegmentationMode(SemanticRoadMaterial, SemanticPoleMaterial);
			}

			for (AActor* Actor : SemanticSegmentationActorsToDisable)
			{
				Actor->SetActorHiddenInGame(false);
			}

			// Landscape
			UStaticMeshComponent* LandscapeMeshComponent = Cast<UStaticMeshComponent>(Landscape->GetComponentByClass(UStaticMeshComponent::StaticClass()));
			LandscapeMeshComponent->SetMaterial(0, OriginalLandScapeMaterial);
		}

		// Sidewalks
		for (ACityLot* CityLot : FinalCity->GetAllCityLots())
		{
			CityLot->SemanticSegmentationMode(SemanticSidewalkMaterial, SemanticFenceMaterial);
		}

		// Stoplights
		for (int i = 0; i < AllStoplightActors.Num(); i++)
		{
			AActor* StoplightActor = AllStoplightActors[i];
			auto StoplightStaticMeshComponent = Cast<UStaticMeshComponent>(StoplightActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

			for (int j = 0; j < OriginalStoplightMaterials[i].Num(); j++)
			{
				StoplightStaticMeshComponent->SetMaterial(j, OriginalStoplightMaterials[i][j]);
			}
		}

		this->GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
	}
}

float ARoadNetwork::GetPopulationDensitySettings() const
{
	return PopulationDensity;
}

float ARoadNetwork::GetWealthDensitySettings() const
{
	return WealthDensity;
}

float ARoadNetwork::GetIndustryDensitySettings() const
{
	return IndustryDensity;
}

bool ARoadNetwork::ShouldTickIfViewportsOnly() const
{
	return true;
}

void ARoadNetwork::Destroyed()
{
	Super::Destroyed();
	
	ClearRoadNetwork();
}