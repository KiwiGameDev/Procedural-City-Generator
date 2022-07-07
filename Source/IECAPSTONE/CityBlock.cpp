// Fill out your copyright notice in the Description page of Project Settings.

#include "CityBlock.h"
#include "CityLot.h"
#include "CustomMath.h"
#include "RoadHalfEdge.h"
#include "RoadNode.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"
#include "RoadNetworkFactory.h"
#include "Kismet/KismetRenderingLibrary.h"

ACityBlock::ACityBlock()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMeshComponent"));
	ProceduralMeshComponent->SetRelativeLocation(FVector(0,0,-100.0f));
}

void ACityBlock::InjectRoads(const TArray<FRoadHalfEdge*>& NewRoadHalfEdges, const TArray<ARoadNode*>& NewRoadNodes)
{
	RoadHalfEdgesArray = NewRoadHalfEdges;
	RoadNodesArray = NewRoadNodes;

	// Attach road nodes to city block actor
	for (ARoadNode* RoadNode : RoadNodesArray)
	{
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, false);
		RoadNode->AttachToActor(this, AttachmentRules);
	}
}

void ACityBlock::RemoveDisconnectedRoads()
{
	if (RoadNodesArray.Num() == 0)
	{
		return;
	}
	
	TArray<TSet<ARoadNode*>> RoadSubnetworksArray;

	// Split road nodes into sets where all nodes in the set are connected
	for (ARoadNode* OriginRoadNode : RoadNodesArray)
	{
		bool bFoundRoadNode = false;

		// Check if road node is already in a road set
		for (TSet<ARoadNode*>& RoadSet : RoadSubnetworksArray)
		{
			if (RoadSet.Contains(OriginRoadNode))
			{
				bFoundRoadNode = true;
				break;
			}
		}

		// If road node not found, create new road set
		if (!bFoundRoadNode)
		{
			TSet<ARoadNode*> NewRoadSubnetwork;
			NewRoadSubnetwork.Add(OriginRoadNode);
			RemoveDisconnectedRoadsRecursive(OriginRoadNode, NewRoadSubnetwork);
			RoadSubnetworksArray.Add(NewRoadSubnetwork);
		}
	}

	// Remove least amount of road nodes in road set. Get most amount and delete everything else
	int MostRoadNodesIndex = -1;
	int MostRoadNodesCount = 0;

	for (int i = 0; i < RoadSubnetworksArray.Num(); i++)
	{
		const TSet<ARoadNode*>& RoadSubnetwork = RoadSubnetworksArray[i];

		if (RoadSubnetwork.Num() > MostRoadNodesCount)
		{
			MostRoadNodesIndex = i;
			MostRoadNodesCount = RoadSubnetwork.Num();
		}
	}

	// Delete every road set less than most road set
	for (int i = 0; i < RoadSubnetworksArray.Num(); i++)
	{
		if (i == MostRoadNodesIndex)
		{
			continue;
		}

		const TSet<ARoadNode*>& RoadSet = RoadSubnetworksArray[i];

		// Delete this road subgraph's road nodes (and implicitly, its half edges)
		for (ARoadNode* RoadNode : RoadSet)
		{
			DestroyRoadNodeAndConnectedHalfEdges(RoadNode);
		}
	}
}

// Time Complexity: O(n^2)
// n: Number of road half edges
void ACityBlock::MergeSameRoadHalfEdges()
{
	TArray<FRoadHalfEdge*> FoundRoadHalfEdgesArray;

	for (int i = 0; i < RoadHalfEdgesArray.Num(); i++)
	{
		FRoadHalfEdge* CurrentRoadHalfEdge = RoadHalfEdgesArray[i];
		bool bIsAlreadyInSet = false;
		
		for (FRoadHalfEdge* ExistingRoadHalfEdge : FoundRoadHalfEdgesArray)
		{
			if (*CurrentRoadHalfEdge == *ExistingRoadHalfEdge)
			{
				DestroyRoadHalfEdge(CurrentRoadHalfEdge);
				bIsAlreadyInSet = true;
				i--;
				break;
			}
		}

		if (!bIsAlreadyInSet)
		{
			FoundRoadHalfEdgesArray.Add(CurrentRoadHalfEdge);
		}
	}
}

void ACityBlock::SortRoadHalfEdgesInPairs()
{
	TArray<FRoadHalfEdge*> SortedRoadHalfEdgesArray;
	SortedRoadHalfEdgesArray.Reserve(RoadHalfEdgesArray.Num());

	for (FRoadHalfEdge* ToCopy : RoadHalfEdgesArray)
	{
		bool bDoesCopyExist = false;
		
		for (int i = 0; i < SortedRoadHalfEdgesArray.Num(); i++)
		{
			FRoadHalfEdge* ToCompare = SortedRoadHalfEdgesArray[i];
			
			if (ToCopy->GetOriginRoadNode() == ToCompare->GetNextRoadNode() && ToCopy->GetNextRoadNode() == ToCompare->GetOriginRoadNode())
			{
				SortedRoadHalfEdgesArray[i + 1] = ToCopy;
				bDoesCopyExist = true;
				break;
			}
		}

		if (!bDoesCopyExist)
		{
			SortedRoadHalfEdgesArray.Add(ToCopy);
			SortedRoadHalfEdgesArray.Add(nullptr);
		}
	}

	// DEBUG: Check if any difference between sorted and unsorted. Original array may have already been sorted already.
	if (RoadHalfEdgesArray.Num() != SortedRoadHalfEdgesArray.Num())
	{
		// UE_LOG(LogTemp, Error, TEXT("Tried to sort road half edges but sorted array has different number of half edges!"));
		return;
	}

	bool bIsSortedArraySame = true;
	
	for (int i = 0; i < RoadHalfEdgesArray.Num(); i++)
	{
		if (RoadHalfEdgesArray[i] != SortedRoadHalfEdgesArray[i])
		{
			// UE_LOG(LogTemp, Warning, TEXT("Sorted road half edges are different from original array."));
			bIsSortedArraySame = false;
			break;
		}
	}

	if (bIsSortedArraySame)
	{
		// UE_LOG(LogTemp, Log, TEXT("Sorted road half edges of city is the same with the original array."));
	}
	
	RoadHalfEdgesArray = SortedRoadHalfEdgesArray;
}

void ACityBlock::Initialize(ARoadNetwork* RoadNetwork)
{
	InitializeRoadHalfEdges();
	InitializeCityLots();
	RemoveStrayRoadsInCityLots();
	SortRoadHalfEdgesInPairs();
	// RemoveSmallCityLots();
	InitializeCityZones(RoadNetwork);
	SpawnCityLotFloors();
}

void ACityBlock::InitializeRoadHalfEdges()
{
	for (FRoadHalfEdge* CurrentHalfEdge : RoadHalfEdgesArray)
	{
		CurrentHalfEdge->Initialize();
	}
}

void ACityBlock::RemoveDisconnectedRoadsRecursive(ARoadNode* RoadNode, TSet<ARoadNode*>& RoadSet)
{
	for (FRoadHalfEdge* EmanatingHalfEdge : RoadNode->GetEmanatingRoadHalfEdges())
	{
		ARoadNode* NextRoadNode = EmanatingHalfEdge->GetNextRoadNode();
		bool bIsAlreadyInRoadSet = false;
		RoadSet.Add(NextRoadNode, &bIsAlreadyInRoadSet);

		if (!bIsAlreadyInRoadSet)
		{
			RemoveDisconnectedRoadsRecursive(NextRoadNode, RoadSet);
		}
	}
}

void ACityBlock::InitializeCityLots()
{
	if (CityLotBlueprint.Get() == nullptr)
	{
		return;
	}

	DestroyAllCityLots();
	
	TSet<FRoadHalfEdge*> FoundHalfEdges;
	
	for (FRoadHalfEdge* HalfEdge : RoadHalfEdgesArray)
	{
		if (FoundHalfEdges.Contains(HalfEdge))
		{
			continue;
		}
		
		if (HalfEdge->GetNextRoadNode() == nullptr)
		{
			continue;
		}

		TArray<FRoadHalfEdge*> CityLotHalfEdges;
		FRoadHalfEdge* CurrentHalfEdge = HalfEdge;
		FoundHalfEdges.Add(CurrentHalfEdge);
		CityLotHalfEdges.Add(CurrentHalfEdge);

		// Go around the lot clockwise until root node
		while((CurrentHalfEdge = CurrentHalfEdge->GetNextRoadHalfEdge()) != nullptr && CityLotHalfEdges[0] != CurrentHalfEdge)
		{
			FoundHalfEdges.Add(CurrentHalfEdge);
			CityLotHalfEdges.Add(CurrentHalfEdge);
		}

		// City lots can't have less than 3 road (half) edges
		if (CityLotHalfEdges.Num() < 3)
		{
			continue;
		}

		// If road's direction is clockwise, create CityLot
		float Sum = 0.0f;
		
		for (FRoadHalfEdge* HalfEdgeToSum : CityLotHalfEdges)
		{
			ARoadNode* RoadNodeA = HalfEdgeToSum->GetOriginRoadNode();
			ARoadNode* RoadNodeB = HalfEdgeToSum->GetNextRoadNode();
			FVector LocA = RoadNodeA->GetActorLocation();
			FVector LocB = RoadNodeB->GetActorLocation();

			Sum += (LocB.X - LocA.X) * (LocB.Y + LocA.Y);
		}

		if (Sum < 0)
		{
			// Is clockwise -> Instantiate this city lot
			ACityLot* NewCityLot = GetWorld()->SpawnActor<ACityLot>(CityLotBlueprint.Get());
			NewCityLot->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
			NewCityLot->SetCityBlock(this);
			NewCityLot->SetBorderingHalfEdges(CityLotHalfEdges);

			for (FRoadHalfEdge* RoadHalfEdge : CityLotHalfEdges)
			{
				RoadHalfEdge->SetCityLot(NewCityLot);
			}
			
			CityLotsList.AddTail(NewCityLot);
		}
	}

	// Check if network is valid. Euler's formula: v - e + f = 1. EXCLUDES outside face.
	int RoadNodesCount = RoadNodesArray.Num();
	int RoadEdgesCount = RoadHalfEdgesArray.Num() / 2;
	int CityLotsCount = CityLotsList.Num();
	int EulerFormulaSum = RoadNodesCount - RoadEdgesCount + CityLotsCount;

	if (EulerFormulaSum == 1)
	{
		// UE_LOG(LogTemp, Log, TEXT("Road network generation successful!"));
	}
	else
	{
		// UE_LOG(LogTemp, Error, TEXT("Invalid road network! v - e + f = %d"), EulerFormulaSum);
		// UE_LOG(LogTemp, Error, TEXT("Vertices: %d"), RoadNodesCount);
		// UE_LOG(LogTemp, Error, TEXT("Edges: %d"), RoadEdgesCount);
		// UE_LOG(LogTemp, Error, TEXT("Half Edges: %d"), RoadHalfEdgesArray.Num());
		// UE_LOG(LogTemp, Error, TEXT("Faces: %d"), CityLotsCount);
	}

	bIsCityLotsInitialized = true;
}

void ACityBlock::RemoveStrayRoadsInCityLots()
{
	for (ACityLot* CityLot : CityLotsList)
	{
		TArray<FRoadHalfEdge*> HalfEdgesRemoved = CityLot->RemoveStrayRoads();

		if (HalfEdgesRemoved.Num() == 0)
		{
			continue;
		}
		
		for (int i = 0; i < HalfEdgesRemoved.Num(); i += 2)
		{
			FRoadHalfEdge* CurrRoadHalfEdgeToRemove = HalfEdgesRemoved[i];
			FRoadHalfEdge* PairRoadHalfEdgeToRemove = HalfEdgesRemoved[i + 1];
			ARoadNode* RoadNodeToRemove = CurrRoadHalfEdgeToRemove->GetNextRoadNode();

			RoadHalfEdgesArray.RemoveSingle(CurrRoadHalfEdgeToRemove);
			RoadHalfEdgesArray.RemoveSingle(PairRoadHalfEdgeToRemove);
			RoadNodesArray.RemoveSingle(RoadNodeToRemove);
			
			delete CurrRoadHalfEdgeToRemove;
			delete PairRoadHalfEdgeToRemove;
			RoadNodeToRemove->Destroy();
		}
	}
}

void ACityBlock::TriangulateCityLots()
{
	for (ACityLot* CityLot : CityLotsList)
	{
		CityLot->Triangulate();
	}
}

void ACityBlock::RemoveSmallCityLots()
{
	const float E = 350000.0f;

	// Assume there is a small lot somewhere
	bool bIsFoundSmallLot = true;

	// Keep checking all lots because some lots might effect other lots when they are deleted
	while (bIsFoundSmallLot)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Remove small lots!"));
		bIsFoundSmallLot = false;
		
		for (auto CityLotListNode = CityLotsList.GetHead(); CityLotListNode != nullptr; CityLotListNode = CityLotListNode->GetNextNode())
		{
			ACityLot* CurrCityLot = CityLotListNode->GetValue();
			TArray<FRoadHalfEdge*> BorderingHalfEdgesArray = CurrCityLot->GetAllBorderingRoadHalfEdges();
			float Area = CurrCityLot->GetArea();
			float AllowableArea = BorderingHalfEdgesArray.Num() * E;

			if (Area < AllowableArea)
			{
				// Get all road sections that can be removed
				TArray<TArray<int>> RoadSectionsArray;
				TArray<int> CurrRoadSection;

				for (int i = 0; i < BorderingHalfEdgesArray.Num(); i++)
				{
					FRoadHalfEdge* CurrHalfEdge = BorderingHalfEdgesArray[i];
					CurrRoadSection.Add(i);

					// If more than 2 half-edges, this is an intersection
					if (CurrHalfEdge->GetNextRoadNode()->GetEmanatingRoadHalfEdges().Num() > 2)
					{
						// Add current road section to road sections array and empty
						RoadSectionsArray.Add(CurrRoadSection);
						CurrRoadSection = TArray<int>();
					}
				}

				// Special case. It might loop around and the current road section might not have been added yet
				if (CurrRoadSection.Num () > 0)
				{
					RoadSectionsArray[0].Insert(CurrRoadSection, 0);
				}

				// Find which road to delete. Current algorithm just finds longest road section.
				int RoadSectionToDeleteIndex = 0;

				if (RoadSectionsArray.Num() > 1)
				{
					float LongestRoadSectionLength = 0;

					for (int i = 0; i < RoadSectionsArray.Num(); i++)
					{
						const TArray<int>& PossibleRoadSectionToDelete = RoadSectionsArray[i];
						float SumLength = 0;

						for (int CurrHalfEdgeIndex : PossibleRoadSectionToDelete)
						{
							FRoadHalfEdge* CurrHalfEdge = BorderingHalfEdgesArray[CurrHalfEdgeIndex];
							FVector CurrNodePosA = CurrHalfEdge->GetOriginRoadNode()->GetActorLocation();
							FVector CurrNodePosB = CurrHalfEdge->GetNextRoadNode()->GetActorLocation();
							SumLength += FVector::Dist(CurrNodePosA, CurrNodePosB);
						}

						if (SumLength > LongestRoadSectionLength)
						{
							RoadSectionToDeleteIndex = i;
							LongestRoadSectionLength = SumLength;
						}
					}
				}
				else
				{
					//UE_LOG(LogTemp, Error, TEXT("Invalid state in removing small lots!"));
					return;
				}

				// First get starting half edge of other lot to expand from before deleting
				const TArray<int>& RoadSectionToDelete = RoadSectionsArray[RoadSectionToDeleteIndex];
				int LastHalfEdgeOfRoadSectionToDeleteIndex = RoadSectionToDelete.Last();
				FRoadHalfEdge* LastHalfEdgeToDelete = BorderingHalfEdgesArray[LastHalfEdgeOfRoadSectionToDeleteIndex];
				FRoadHalfEdge* LastHalfEdgeToDeletePair = LastHalfEdgeToDelete->GetPairRoadHalfEdge();
				ACityLot* EditedLotToExpand = LastHalfEdgeToDeletePair->GetCityLot();

				if (EditedLotToExpand != nullptr)
				{
					TArray<FRoadHalfEdge*> EditedLotToExpandHalfEdges = EditedLotToExpand->GetAllBorderingRoadHalfEdges();
					int StartingHalfEdgeToDeletePairIndex = EditedLotToExpandHalfEdges.Find(LastHalfEdgeToDeletePair);
					int HalfEdgesToRemoveCount = RoadSectionToDelete.Num();

					// Remove half edges that will be deleted
					if (StartingHalfEdgeToDeletePairIndex + HalfEdgesToRemoveCount <= EditedLotToExpandHalfEdges.Num())
					{
						EditedLotToExpandHalfEdges.RemoveAt(StartingHalfEdgeToDeletePairIndex, HalfEdgesToRemoveCount);
					}
					else
					{
						int FirstPartToDeleteCount = EditedLotToExpandHalfEdges.Num() - StartingHalfEdgeToDeletePairIndex;
						int SecondPartToDeleteCount = HalfEdgesToRemoveCount - FirstPartToDeleteCount;
						EditedLotToExpandHalfEdges.RemoveAt(StartingHalfEdgeToDeletePairIndex, FirstPartToDeleteCount);
						EditedLotToExpandHalfEdges.RemoveAt(0, SecondPartToDeleteCount);

						// Update starting half edge indexed that were moved
						StartingHalfEdgeToDeletePairIndex -= SecondPartToDeleteCount;
					}
					
					// Append the rest of the city lot's half edges to the other city lot's half edges
					int HalfEdgesAppendedCounter = 0;
					
					for (int i = 1; i < RoadSectionsArray.Num(); i++)
					{
						int RoadSectionToAppendIndex = (RoadSectionToDeleteIndex + i) % RoadSectionsArray.Num();
						const TArray<int>& RoadSectionToAppend = RoadSectionsArray[RoadSectionToAppendIndex];

						for (int HalfEdgeToAppendIndex : RoadSectionToAppend)
						{
							FRoadHalfEdge* HalfEdgeToAppend = BorderingHalfEdgesArray[HalfEdgeToAppendIndex];
							HalfEdgeToAppend->SetCityLot(EditedLotToExpand);

							// Append
							int InsertionIndex = StartingHalfEdgeToDeletePairIndex + HalfEdgesAppendedCounter;
							EditedLotToExpandHalfEdges.Insert(HalfEdgeToAppend, InsertionIndex);
							HalfEdgesAppendedCounter++;
						}
					}

					// Set new bordering half edges
					EditedLotToExpand->SetBorderingHalfEdges(EditedLotToExpandHalfEdges);
				}
				else
				{
					// Set this city lot's half edges to null
					for (int i = 1; i < RoadSectionsArray.Num(); i++)
					{
						int RoadSectionToNullIndex = (RoadSectionToDeleteIndex + i) % RoadSectionsArray.Num();
						const TArray<int>& RoadSectionToNull = RoadSectionsArray[RoadSectionToNullIndex];

						for (int HalfEdgeToNullIndex : RoadSectionToNull)
						{
							FRoadHalfEdge* HalfEdgeToNull = BorderingHalfEdgesArray[HalfEdgeToNullIndex];
							HalfEdgeToNull->SetCityLot(nullptr);
						}
					}
				}

				// First, get mutual road nodes. It will be deleted
				ARoadNode* RoadNodeToReinitA = BorderingHalfEdgesArray[RoadSectionToDelete.Last()]->GetNextRoadNode();
				ARoadNode* RoadNodeToReinitB = BorderingHalfEdgesArray[RoadSectionToDelete[0]]->GetOriginRoadNode();

				// Delete chosen road section's half edges
				for (int i = 0; i < RoadSectionToDelete.Num(); i++)
				{
					int HalfEdgeToDeleteIndex = RoadSectionToDelete[i];
					FRoadHalfEdge* CurrHalfEdgeToDelete = BorderingHalfEdgesArray[HalfEdgeToDeleteIndex];
					FRoadHalfEdge* PairHalfEdgeToDelete = CurrHalfEdgeToDelete->GetPairRoadHalfEdge();

					// Delete all nodes in between
					if (i < RoadSectionToDelete.Num() - 1)
					{
						ARoadNode* RoadNodeToDelete = CurrHalfEdgeToDelete->GetNextRoadNode();
						DestroyRoadNode(RoadNodeToDelete);
					}

					DestroyRoadHalfEdge(CurrHalfEdgeToDelete);
					DestroyRoadHalfEdge(PairHalfEdgeToDelete);
				}

				// Set the next road half edge of half edge pointing to old half edges
				for (FRoadHalfEdge* HalfEdge : RoadNodeToReinitA->GetEmanatingRoadHalfEdges())
				{
					HalfEdge->GetPairRoadHalfEdge()->Initialize();
				}
				
				for (FRoadHalfEdge* HalfEdge : RoadNodeToReinitB->GetEmanatingRoadHalfEdges())
				{
					HalfEdge->GetPairRoadHalfEdge()->Initialize();
				}
				
				// Delete city lot
				auto PrevCityLotListNode = CityLotListNode->GetPrevNode();
				CityLotsList.RemoveNode(CityLotListNode);
				CityLotListNode = PrevCityLotListNode;
				CurrCityLot->Destroy();

				//UE_LOG(LogTemp, Warning, TEXT("Deleted small city lot!"));
			}
		}
	}
}

void ACityBlock::InitializeCityZones(ARoadNetwork* RoadNetwork)
{
	UMaterialInstanceDynamic* NoiseGenerationMaterialDynamic = UMaterialInstanceDynamic::Create(NoiseGenerationMaterial, nullptr);
	NoiseGenerationMaterialDynamic->SetScalarParameterValue(TEXT("Red Seed"), FMath::Rand());
	NoiseGenerationMaterialDynamic->SetScalarParameterValue(TEXT("Green Seed"), FMath::Rand());
	NoiseGenerationMaterialDynamic->SetScalarParameterValue(TEXT("Blue Seed"), FMath::Rand());
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), CityZoningRenderTarget, NoiseGenerationMaterialDynamic);
	
	TArray<FFloat16Color> CityZoningValuesArray;
	FTextureRenderTargetResource* CityZoningRenderTargetResource = CityZoningRenderTarget->GameThread_GetRenderTargetResource();
	CityZoningRenderTargetResource->ReadFloat16Pixels(CityZoningValuesArray);

	FBox CityBoundsUpdated = GetCityBounds();
	FVector CityBoundsSize = CityBoundsUpdated.GetSize();

	for (ACityLot* CityLot : CityLotsList)
	{
		FVector CityLotCenter = CityLot->GetCentroidPosition();
		FVector CityLotCenterNormalized = (CityLotCenter - CityBounds.Min) / CityBoundsSize;
		uint32 CityLotCenterTexCoordX = CityLotCenterNormalized.X * CityZoningRenderTargetResource->GetSizeX();
		uint32 CityLotCenterTexCoordY = CityLotCenterNormalized.Y * CityZoningRenderTargetResource->GetSizeY();
		uint32 PixelIndex = CityLotCenterTexCoordY * CityZoningRenderTargetResource->GetSizeX() + CityLotCenterTexCoordX;
		FFloat16Color PixelValue = CityZoningValuesArray[PixelIndex];

		bool bIsPopulationHigh = PixelValue.R < RoadNetwork->GetPopulationDensitySettings();
		bool bIsWealthHigh = PixelValue.G < RoadNetwork->GetWealthDensitySettings();
		bool bIsIndustryHigh = PixelValue.B < RoadNetwork->GetIndustryDensitySettings();
		
		if (bIsIndustryHigh)
		{
			CityLot->SetCityZoneType(ECityZoneType::Industrial);
		}
		else if (bIsPopulationHigh && bIsWealthHigh)
		{
			CityLot->SetCityZoneType(ECityZoneType::HighRiseResidential);
		}
		else if (bIsPopulationHigh && !bIsWealthHigh)
		{
			CityLot->SetCityZoneType(ECityZoneType::LowRiseResidential);
		}
		else if (!bIsPopulationHigh && bIsWealthHigh)
		{
			CityLot->SetCityZoneType(ECityZoneType::HighRiseCommercial);
		}
		else if (!bIsPopulationHigh && !bIsWealthHigh)
		{
			CityLot->SetCityZoneType(ECityZoneType::LowRiseCommercial);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid state for assigning city zone type!"));
		}
	}
}

void ACityBlock::SpawnCityLotFloors()
{
	for (ACityLot* CityLot : CityLotsList)
	{
		CityLot->SpawnFloor();
	}
}

void ACityBlock::SpawnBuildings(int32 depth, ARoadNetwork* network)
{
	for (ACityLot* CityLot : CityLotsList)
	{
		CityLot->SpawnBuildings(depth, network);
		CityLot->TurnOffBuildingPhysics();
	}
}

void ACityBlock::callSpawnDetails(ARoadNetwork* roadNetwork, float spawnDensity)
{
	if (this->CityLotsList.Num() == 0)
	{
		return;
	}

	for (ACityLot* CityLot : CityLotsList)
	{
		CityLot->SpawnDetails(roadNetwork, spawnDensity);
	}
}

// Sort such that half edges in the array are next one after the other
void ACityBlock::SortRoadHalfEdgesInRoadIDs()
{
	// Get road arrays from road IDs
	TMap<int, TArray<FRoadHalfEdge*>> RoadIDToRoadArrayMap;
	
	for (int i = 0; i < RoadHalfEdgesArray.Num(); i += 2)
	{
		FRoadHalfEdge* CurrentRoadHalfEdge = RoadHalfEdgesArray[i];
		FRoadHalfEdge* CurrentRoadHalfEdgePair = RoadHalfEdgesArray[i + 1];
		int CurrentRoadHalfEdgeID = CurrentRoadHalfEdge->GetRoadID();
	
		if (TArray<FRoadHalfEdge*>* RoadArrayPointer = RoadIDToRoadArrayMap.Find(CurrentRoadHalfEdgeID))
		{
			RoadArrayPointer->Add(CurrentRoadHalfEdge);
			RoadArrayPointer->Add(CurrentRoadHalfEdgePair);
		}
		else
		{
			TArray<FRoadHalfEdge*> NewRoadArray;
			NewRoadArray.Add(CurrentRoadHalfEdge);
			NewRoadArray.Add(CurrentRoadHalfEdgePair);
			RoadIDToRoadArrayMap.Add(CurrentRoadHalfEdgeID, NewRoadArray);
		}
	}
	
	for (TTuple<int, TArray<FRoadHalfEdge*>>& IDToRoadArray : RoadIDToRoadArrayMap)
	{
		TArray<FRoadHalfEdge*>& RoadArray = IDToRoadArray.Value;
		
		// Find the first road and swap to first in array.
		TSet<ARoadNode*> NextRoadNodesSet;
		
		for (int i = 0; i < RoadArray.Num(); i += 2)
		{
			NextRoadNodesSet.Add(RoadArray[i]->GetNextRoadNode());
		}
		
		for (int i = 0; i < RoadArray.Num(); i += 2)
		{
			if (!NextRoadNodesSet.Contains(RoadArray[i]->GetOriginRoadNode()))
			{
				if (i == 0)
				{
					break;
				}

				Swap(RoadArray[0], RoadArray[i]);
				Swap(RoadArray[1], RoadArray[i + 1]);
				break;
			}
		}

		// Sort road array
		for (int i = 0; i < RoadArray.Num() - 2; i += 2)
		{
			ARoadNode* SupposedNextRoadNode = RoadArray[i]->GetNextRoadNode();
			bool bFoundNextRoadHalfEdge = false;

			for (int j = i + 2; j < RoadArray.Num(); j += 2)
			{
				ARoadNode* OtherNextRoadNode = RoadArray[j]->GetOriginRoadNode();

				if (SupposedNextRoadNode == OtherNextRoadNode)
				{
					if (i + 2 != j)
					{
						Swap(RoadArray[i + 2], RoadArray[j]);
						Swap(RoadArray[i + 3], RoadArray[j + 1]);
					}
					
					bFoundNextRoadHalfEdge = true;
					break;
				}
			}
		}
	}

	// Copy to road half edges array
	int i = 0;

	for (const TTuple<int, TArray<FRoadHalfEdge*>>& IDToRoadArray : RoadIDToRoadArrayMap)
	{
		const TArray<FRoadHalfEdge*>& RoadArray = IDToRoadArray.Value;

		for (int j = 0; j < RoadArray.Num(); j++)
		{
			RoadHalfEdgesArray[i++] = RoadArray[j];
		}
	}
}

void ACityBlock::ToggleDataTextureMap()
{
	if (ProceduralMeshComponent->GetProcMeshSection(0) == nullptr)
	{
		FBox CityBoundsUpdated = GetCityBounds();
		FVector TopLeft = CityBoundsUpdated.Min;
		FVector BotRight = CityBoundsUpdated.Max;
		FVector BotLeft = FVector(TopLeft.X, BotRight.Y, 0);
		FVector TopRight = FVector(BotRight.X, TopLeft.Y, 0);

		TArray<FVector> VertexPositionsArray;
		VertexPositionsArray.Add(TopLeft);
		VertexPositionsArray.Add(TopRight);
		VertexPositionsArray.Add(BotRight);
		VertexPositionsArray.Add(BotLeft);

		TArray<int32> TriangleIndicesArray;
		TriangleIndicesArray.Add(2);
		TriangleIndicesArray.Add(1);
		TriangleIndicesArray.Add(0);
		TriangleIndicesArray.Add(3);
		TriangleIndicesArray.Add(2);
		TriangleIndicesArray.Add(0);

		TArray<FVector2D> UVs;
		UVs.Add(FVector2D(0, 0));
		UVs.Add(FVector2D(1, 0));
		UVs.Add(FVector2D(1, 1));
		UVs.Add(FVector2D(0, 1));

		ProceduralMeshComponent->SetVisibility(true);
		ProceduralMeshComponent->CreateMeshSection(
			0,
			VertexPositionsArray,
			TriangleIndicesArray,
			TArray<FVector>(),
			UVs,
			TArray<FColor>(),
			TArray<FProcMeshTangent>(),
			false);
		ProceduralMeshComponent->SetMaterial(0, CityZoningVisualisationMaterial);
	}
	else
	{
		ProceduralMeshComponent->ClearAllMeshSections();
	}
}

void ACityBlock::ToggleAllCityLotsZoningColor()
{
	if (bIsCityLotZoneColorsVisible)
	{
		for (ACityLot* CityLot : CityLotsList)
		{
			CityLot->HideCityZoningColor();
		}
		
		bIsCityLotZoneColorsVisible = false;
	}
	else
	{
		for (ACityLot* CityLot : CityLotsList)
		{
			CityLot->ShowCityZoningColor();
		}
		
		bIsCityLotZoneColorsVisible = true;
	}
}

void ACityBlock::ShowAllCityLotsZoningColor()
{
	if (bIsCityLotZoneColorsVisible)
	{
		return;
	}
	
	for (ACityLot* CityLot : CityLotsList)
	{
		CityLot->ShowCityZoningColor();
	}
		
	bIsCityLotZoneColorsVisible = true;
}

void ACityBlock::HideAllCityLotsZoningColor()
{
	if (!bIsCityLotZoneColorsVisible)
	{
		return;
	}
	
	for (ACityLot* CityLot : CityLotsList)
	{
		CityLot->HideCityZoningColor();
	}
		
	bIsCityLotZoneColorsVisible = false;
}

void ACityBlock::AddCityLot(ACityLot* CityLot)
{
	CityLotsList.AddTail(CityLot);
}

void ACityBlock::AddRoadHalfEdge(FRoadHalfEdge* RoadHalfEdge)
{
	RoadHalfEdgesArray.Add(RoadHalfEdge);
}

void ACityBlock::AddRoadNode(ARoadNode* RoadNode)
{
	RoadNodesArray.Add(RoadNode);
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, false);
	RoadNode->AttachToActor(this, AttachmentRules);
	bIsCityBoundsDirty = true;
}

void ACityBlock::RemoveCityLot(ACityLot* CityLot)
{
	CityLotsList.RemoveNode(CityLot);
}

void ACityBlock::DestroyRoadHalfEdge(FRoadHalfEdge* RoadHalfEdge)
{
	RoadHalfEdgesArray.RemoveSingle(RoadHalfEdge);
	delete RoadHalfEdge;
}

void ACityBlock::DestroyRoadNode(ARoadNode* RoadNode)
{
	RoadNodesArray.RemoveSingle(RoadNode);
	RoadNode->Destroy();
	
	bIsCityBoundsDirty = true;
}

void ACityBlock::DestroyRoadNodeAndConnectedHalfEdges(ARoadNode* RoadNode)
{
	// Delete emanating half edges
	TArray<FRoadHalfEdge*> EmanatingRoadHalfEdges = RoadNode->GetEmanatingRoadHalfEdges();
			
	for (FRoadHalfEdge* HalfEdge : EmanatingRoadHalfEdges)
	{
		DestroyRoadHalfEdge(HalfEdge);
	}
	
	RoadNodesArray.RemoveSingle(RoadNode);
	RoadNode->Destroy();
	
	bIsCityBoundsDirty = true;
}

ACityBlock* ACityBlock::DeepCopyRoadsAsCityBlock() const
{
	TTuple<TArray<FRoadHalfEdge*>, TArray<ARoadNode*>> Roads = DeepCopyRoads();
	ACityBlock* NewCityBlock = GetWorld()->SpawnActor<ACityBlock>(GetClass(), GetActorLocation(), FRotator::ZeroRotator);
	NewCityBlock->InjectRoads(Roads.Get<0>(), Roads.Get<1>());
	NewCityBlock->bIsCityBoundsDirty = true;

	return NewCityBlock;
}

TTuple<TArray<FRoadHalfEdge*>, TArray<ARoadNode*>> ACityBlock::DeepCopyRoads() const
{
	TArray<FRoadHalfEdge*> NewRoadHalfEdgesArray;
	TArray<ARoadNode*> NewRoadNodesArray;
	TMap<FRoadHalfEdge*, FRoadHalfEdge*> RoadHalfEdgesOriginalToCopyMap;
	TMap<ARoadNode*, ARoadNode*> RoadNodesOriginalToCopyMap;

	NewRoadHalfEdgesArray.Reserve(RoadHalfEdgesArray.Num());
	NewRoadNodesArray.Reserve(RoadNodesArray.Num());
	RoadHalfEdgesOriginalToCopyMap.Reserve(RoadHalfEdgesArray.Num());
	RoadNodesOriginalToCopyMap.Reserve(RoadNodesArray.Num());

	// Create copy and map original to copy
	for (ARoadNode* RoadNodeOriginal : RoadNodesArray)
	{
		FVector Pos = RoadNodeOriginal->GetActorLocation();
		ARoadNode* RoadNodeCopy = GetWorld()->SpawnActor<ARoadNode>(Pos, FRotator::ZeroRotator);
		NewRoadNodesArray.Add(RoadNodeCopy);
		RoadNodesOriginalToCopyMap.Add(RoadNodeOriginal, RoadNodeCopy);
	}

	// Create copy and map original to copy
	for (FRoadHalfEdge* RoadHalfEdgeOriginal : RoadHalfEdgesArray)
	{
		ARoadNode* OriginRoadNodeOriginal = RoadHalfEdgeOriginal->GetOriginRoadNode();
		FRoadHalfEdge* RoadHalfEdgeCopy = new FRoadHalfEdge(RoadNodesOriginalToCopyMap[OriginRoadNodeOriginal]);
		NewRoadHalfEdgesArray.Add(RoadHalfEdgeCopy);
		RoadHalfEdgeCopy->SetRoadType(RoadHalfEdgeOriginal->GetRoadType());
		RoadHalfEdgeCopy->SetRoadID(RoadHalfEdgeOriginal->GetRoadID());
		RoadHalfEdgesOriginalToCopyMap.Add(RoadHalfEdgeOriginal, RoadHalfEdgeCopy);
	}

	// Remap original references to copy
	for (FRoadHalfEdge* RoadHalfEdgeOriginal : RoadHalfEdgesArray)
	{
		FRoadHalfEdge* RoadHalfEdgeCopy = RoadHalfEdgesOriginalToCopyMap[RoadHalfEdgeOriginal];
		FRoadHalfEdge* RoadHalfEdgePair = RoadHalfEdgesOriginalToCopyMap[RoadHalfEdgeOriginal->GetPairRoadHalfEdge()];
		RoadHalfEdgeCopy->SetPairRoadHalfEdge(RoadHalfEdgePair);
	}

	return TTuple<TArray<FRoadHalfEdge*>, TArray<ARoadNode*>>(NewRoadHalfEdgesArray, NewRoadNodesArray);
}

const TDoubleLinkedList<ACityLot*>& ACityBlock::GetAllCityLots() const
{
	return CityLotsList;
}

const TArray<FRoadHalfEdge*>& ACityBlock::GetAllRoadHalfEdges() const
{
	return RoadHalfEdgesArray;
}

const TArray<ARoadNode*>& ACityBlock::GetAllRoadNodes() const
{
	return RoadNodesArray;
}

FBox ACityBlock::GetCityBounds()
{
	if (!bIsCityBoundsDirty)
	{
		return CityBounds;
	}

	CityBounds.Init();

	for (const ARoadNode* RoadNode : RoadNodesArray)
	{
		const FVector Loc = RoadNode->GetActorLocation();

		if (Loc.X < CityBounds.Min.X)
		{
			CityBounds.Min.X = Loc.X;
		}
		
		if (Loc.X > CityBounds.Max.X)
		{
			CityBounds.Max.X = Loc.X;
		}
		
		if (Loc.Y < CityBounds.Min.Y)
		{
			CityBounds.Min.Y = Loc.Y;
		}
		
		if (Loc.Y > CityBounds.Max.Y)
		{
			CityBounds.Max.Y = Loc.Y;
		}
	}

	bIsCityBoundsDirty = false;
	
	return CityBounds;
}

bool ACityBlock::IsCityLotsInitialized() const
{
	return bIsCityLotsInitialized;
}

bool ACityBlock::IsCityCityLotZoneColorsVisible() const
{
	return bIsCityLotZoneColorsVisible;
}

void ACityBlock::Destroyed()
{
	Super::Destroyed();

	DestroyAllSubobjects();
}

void ACityBlock::DestroyAllSubobjects()
{
	DestroyAllCityLots();
	
	for (FRoadHalfEdge* RoadHalfEdge : RoadHalfEdgesArray)
	{
		delete RoadHalfEdge;
	}

	for (ARoadNode* RoadNode : RoadNodesArray)
	{
		RoadNode->Destroy();
	}

	RoadHalfEdgesArray.Empty();
	RoadNodesArray.Empty();
}

void ACityBlock::DestroyAllCityLots()
{
	for (ACityLot* CityLot : CityLotsList)
	{
		CityLot->Destroy();
	}

	CityLotsList.Empty();
	bIsCityLotsInitialized = false;
	bIsCityLotZoneColorsVisible = false;
}
