// Fill out your copyright notice in the Description page of Project Settings.

#include "CityLot.h"
#include "CityBlock.h"
#include "CustomMath.h"
#include "RoadHalfEdge.h"
#include "RoadNode.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

ACityLot::ACityLot()
	: CityBlock(nullptr), CityZoneType(ECityZoneType::Invalid)
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent->SetRelativeLocation(FVector(0, 0, -1.0f));
	ZoneColorProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ZoneColorProceduralMeshComponent"));
	ZoneColorProceduralMeshComponent->SetupAttachment(RootComponent);
	ZoneColorProceduralMeshComponent->SetRelativeLocation(FVector(0, 0, 5.0f));
	ZoneFloorProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ZoneFloorProceduralMeshComponent"));
	ZoneFloorProceduralMeshComponent->SetupAttachment(RootComponent);

	LotComponent = CreateDefaultSubobject<ULotComponent>(TEXT("LotComponent"));
}

TArray<FRoadHalfEdge*> ACityLot::RemoveStrayRoads()
{
	// Save all half edges removed
	TArray<FRoadHalfEdge*> RoadHalfEdgesRemovedArray;
	RoadHalfEdgesRemovedArray.Reserve(BorderingRoadHalfEdgesArray.Num());
	
	for (int i = 0; i < BorderingRoadHalfEdgesArray.Num(); i++)
	{
		int32 j = (i + 1) % BorderingRoadHalfEdgesArray.Num();
		FRoadHalfEdge* CurrHalfEdge = BorderingRoadHalfEdgesArray[i];
		FRoadHalfEdge* NextHalfEdge = BorderingRoadHalfEdgesArray[j];

		// If it loops back on the same edge, it must be a stray road inside the city lot.
		if (CurrHalfEdge->GetOriginRoadNode() == NextHalfEdge->GetNextRoadNode())
		{
			// Remove element ahead first
			if (i < j)
			{
				BorderingRoadHalfEdgesArray.RemoveAt(j);
				BorderingRoadHalfEdgesArray.RemoveAt(i);
			}
			else
			{
				BorderingRoadHalfEdgesArray.RemoveAt(i);
				BorderingRoadHalfEdgesArray.RemoveAt(j);
			}

			// Append to half edges removed
			RoadHalfEdgesRemovedArray.Add(CurrHalfEdge);
			RoadHalfEdgesRemovedArray.Add(NextHalfEdge);

			// Set previous half edge's next half edge to the next one
			int NewCurrHalfEdgeIndex = (i - 1 + BorderingRoadHalfEdgesArray.Num()) % BorderingRoadHalfEdgesArray.Num();
			int NewNextHalfEdgeIndex = i % BorderingRoadHalfEdgesArray.Num();
			FRoadHalfEdge* NewCurrHalfEdge = BorderingRoadHalfEdgesArray[NewCurrHalfEdgeIndex];
			FRoadHalfEdge* NewNextHalfEdge = BorderingRoadHalfEdgesArray[NewNextHalfEdgeIndex];
			NewCurrHalfEdge->SetNextRoadHalfEdge(NewNextHalfEdge);

			i = FMath::Max(i - 2, -1); // -2 because current half edge is deleted and it will be incremented
		}
	}

	if (RoadHalfEdgesRemovedArray.Num() > 0)
	{
		bIsTriangulated = false;
	}

	return RoadHalfEdgesRemovedArray;
}

void ACityLot::Triangulate()
{
	if (bIsTriangulated)
	{
		return;
	}
	
	// Reset
	TriangleIndicesArray.Empty();
	RoadNodesPosArray = GetBorderingRoadNodesPositionsArray();
	
	TArray<int32> RoadNodeIndicesArray;
	
	for (int i = 0; i < RoadNodesPosArray.Num(); i++)
	{
		RoadNodeIndicesArray.Add(i);
	}
		
	while (RoadNodeIndicesArray.Num() > 3)
	{
		for (int i = 0; i < RoadNodeIndicesArray.Num(); i++)
		{
			int p = (i - 1 + RoadNodeIndicesArray.Num()) % RoadNodeIndicesArray.Num();
			int n = (i + 1) % RoadNodeIndicesArray.Num();
			FVector PrevRoadNodePos = RoadNodesPosArray[RoadNodeIndicesArray[p]];
			FVector CurrRoadNodePos = RoadNodesPosArray[RoadNodeIndicesArray[i]];
			FVector NextRoadNodePos = RoadNodesPosArray[RoadNodeIndicesArray[n]];
			float CrossZ = CustomMath::CrossZ(PrevRoadNodePos - CurrRoadNodePos, NextRoadNodePos - CurrRoadNodePos);
			
			if (CrossZ >= 0)
			{
				continue; // Is greater than 180 degrees
			}

			bool bIsEar = true;

			for (int j = 0; j < RoadNodeIndicesArray.Num(); j++)
			{
				if (j == p || j == i || j == n)
				{
					continue; // Is part of the possible ear
				}

				FVector OtherRoadNodePos = RoadNodesPosArray[RoadNodeIndicesArray[j]];
				
				if (CustomMath::IsPointInTriangle(OtherRoadNodePos, PrevRoadNodePos, CurrRoadNodePos, NextRoadNodePos))
				{
					bIsEar = false;
					break;
				}
			}

			if (!bIsEar)
			{
				continue; // Is not a valid ear
			}

			TriangleIndicesArray.Add(RoadNodeIndicesArray[n]);
			TriangleIndicesArray.Add(RoadNodeIndicesArray[i]);
			TriangleIndicesArray.Add(RoadNodeIndicesArray[p]);
			RoadNodeIndicesArray.RemoveAt(i);
			break;
		}
	}

	TriangleIndicesArray.Add(RoadNodeIndicesArray[2]);
	TriangleIndicesArray.Add(RoadNodeIndicesArray[1]);
	TriangleIndicesArray.Add(RoadNodeIndicesArray[0]);

	bIsTriangulated = true;
}

void ACityLot::SpawnFloor()
{
	if (CityZoneType == ECityZoneType::HighRiseCommercial || CityZoneType == ECityZoneType::HighRiseResidential || CityZoneType == ECityZoneType::Industrial)
	{
		Triangulate();

		ZoneFloorProceduralMeshComponent->CreateMeshSection(
			0,
			RoadNodesPosArray,
			TriangleIndicesArray,
			TArray<FVector>(),
			TArray<FVector2D>(),
			TArray<FColor>(),
			TArray<FProcMeshTangent>(),
			false);
		ZoneFloorProceduralMeshComponent->SetMaterial(0, ZoneFloorMaterial);
	}
}

void ACityLot::ShowCityZoningColor()
{
	if (ZoneMaterials.Num() < 6)
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't create CityZoneMesh because there is not enough materials!"));
		return;
	}

	if (ZoneColorProceduralMeshComponent->GetProcMeshSection(0) != nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to show city zoning color in CityLot but it is already shown!"));
	}

	Triangulate();

	ZoneColorProceduralMeshComponent->CreateMeshSection(
		0,
		RoadNodesPosArray,
		TriangleIndicesArray,
		TArray<FVector>(),
		TArray<FVector2D>(),
		TArray<FColor>(),
		TArray<FProcMeshTangent>(),
		false);
	ZoneColorProceduralMeshComponent->SetMaterial(0, ZoneMaterials[static_cast<int32>(CityZoneType) - 1]);
}

void ACityLot::HideCityZoningColor()
{
	if (ZoneColorProceduralMeshComponent->GetProcMeshSection(0) == nullptr)
	{
		// UE_LOG(LogTemp, Error, TEXT("Tried to hide city zoning color in CityLot but it is already hidden!"));
	}

	ZoneColorProceduralMeshComponent->ClearAllMeshSections();
}

void ACityLot::SpawnBuildings(int32 depth, ARoadNetwork* network)
{
	InitializeInsetPolygon();

	// Find centroid
	FVector Center = GetCentroidPosition();

	// Find shortest distance of the center to inset
	float ShortestDist = TNumericLimits<float>::Max();
	
	for (int i = 0; i < InsetPositions.Num(); i++)
	{
		float Length = FVector::Dist(Center, InsetPositions[i]);
		
		if (Length < ShortestDist)
		{
			ShortestDist = Length;
		}
	}

	spawnType = network->RetrieveRandomSpawnTypeFromCityZone(CityZoneType);

	TSharedPtr<TDoubleLinkedList<FVector>> SubdivisionList;

	SubdivisionQueue.Enqueue(ConvertVectorArrayToDoubleLinkedListSharedPtr(InsetPositions));

	while (!SubdivisionQueue.IsEmpty()) {
		SubdivisionQueue.Dequeue(SubdivisionList);
		CreateSubdivisions(SubdivisionList, depth, network);
	}

	SpawnBuildingsInSubdivision(network);
}

void ACityLot::SpawnBuildingsInSubdivision(ARoadNetwork* network)
{
	for (int i = 0; i < Subdivisions.Num(); i++) {
		Subdivisions[i]->SpawnBuilding(LotComponent, network);
	}

	if (network->CanSpawnFences() && spawnType == EBuildingSpawnType::Compound) {
		LotComponent->SpawnFenceFromComponent(this->GetAllBorderingRoadHalfEdges(), InsetPositions, network);
	}
}

void ACityLot::InitializeInsetPolygon()
{
	TArray<FVector> RoadNodePosArray = GetBorderingRoadNodesPositionsArray();
	InsetPositions = CustomMath::GetInsetOfPolygon(RoadNodePosArray, 3900);//4900
}

FVector ACityLot::FindNearestPointOfPositionOnBorder(FVector Position)
{
	FVector OutClosestPoint;
	float ShortestDistSqr = TNumericLimits<float>::Max();

	for (int i = 0; i < BorderingRoadHalfEdgesArray.Num(); i++)
	{
		FRoadHalfEdge* HalfEdge = BorderingRoadHalfEdgesArray[i];
		FVector CurrNodePosA = HalfEdge->GetOriginRoadNode()->GetActorLocation();
		FVector CurrNodePosB = HalfEdge->GetNextRoadNode()->GetActorLocation();
		FVector CurrClosestPoint = UKismetMathLibrary::FindClosestPointOnSegment(Position, CurrNodePosA, CurrNodePosB);
		float CurrClosestPointDistSqr = FVector::DistSquared(Position, CurrClosestPoint);

		if (CurrClosestPointDistSqr < ShortestDistSqr)
		{
			ShortestDistSqr = CurrClosestPointDistSqr;
			OutClosestPoint = CurrClosestPoint;
		}
	}

	return OutClosestPoint;
}

FVector ACityLot::GetFaceNormals(const FVector& Midpoint, const FVector& A, const FVector& B)
{
	FVector DirA = Midpoint - A;
	DirA.Normalize();
	FVector DirB = Midpoint - B;
	DirB.Normalize();
	return (DirA + DirB) * 0.5f * -1.0f;
}

void ACityLot::CreateSubdivisions(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions, int Depth, ARoadNetwork* network)
{
	TSharedPtr<TDoubleLinkedList<FVector>> PosList(new TDoubleLinkedList<FVector>);
	
	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> Iter(NodePositions->GetHead()); Iter; ++Iter)
	{
		FVector Pos = Iter.GetNode()->GetValue();
		PosList->AddTail(Pos);
	}

	TArray<FVector> boxVertices = CreateOrientedBoundingBoxFromLinkedList(PosList);

	bool bOdd = FMath::RandBool();
	int StartIndex = 0;
	
	if (boxVertices.Num() > 4)
	{
		TArray<ABuilding*>  SpawnableBuildings = network->RetrieveBuildingsFromCityZone(CityZoneType);

		if (SpawnableBuildings.Num() <= 0) return;

		float MAX_AREA_THRESHOLD = SpawnableBuildings[0]->GetBuildingLotArea();
		float MIN_AREA_THRESHOLD = SpawnableBuildings[0]->GetBuildingLotArea();
		const float ASPECT_RATIO = 0.75f;
		float EXTRA_ALLOWANCE_PERCENT = 0.20f;
		
		for (int i = 0; i < SpawnableBuildings.Num(); i++)
		{
			float buildingLotArea = SpawnableBuildings[i]->GetBuildingLotArea();
			if (MAX_AREA_THRESHOLD < buildingLotArea)
				MAX_AREA_THRESHOLD = buildingLotArea;
			else if (MIN_AREA_THRESHOLD < buildingLotArea)
				MIN_AREA_THRESHOLD = buildingLotArea;
		}
		
		float ALLOWANCE = MAX_AREA_THRESHOLD * EXTRA_ALLOWANCE_PERCENT;
		MAX_AREA_THRESHOLD += ALLOWANCE;
		float area = boxVertices[4].X * boxVertices[4].Y;

		if (area * 0.5f < MIN_AREA_THRESHOLD || area < MAX_AREA_THRESHOLD)
		{
			FVector Pos = GetMidpointFromLinkedList(NodePositions);
			FVector Norm = GetBuildingDirectionToNearestEdge(Pos);
			FRotator Rot = Norm.Rotation();

			if (spawnType == EBuildingSpawnType::Compound)
			{
				FVector ClosestPoint = FindNearestPointOfPositionOnBorder(Pos);
				FVector Diff = ClosestPoint - Pos;
				FVector Dir;
				float Length;
				Diff.ToDirectionAndLength(Dir, Length);

				if (boxVertices[4].X > boxVertices[4].Y && Length > boxVertices[4].Y * 2.0f)
				{
					return;
				}
				if (boxVertices[4].X < boxVertices[4].Y && Length > boxVertices[4].X * 2.0f)
				{
					return;
				}
			}
			
			Subdivisions.Add(new CityLotSubdivision(NodePositions, boxVertices, CityZoneType, Pos, Norm, Rot));
			
			return;
		}
	}

	if (boxVertices[4].X > boxVertices[4].Y)
	{
		StartIndex = 1;
	}
	else
	{
		StartIndex = 0;
	}
	
	FVector MidPoint;
	FVector a = boxVertices[StartIndex];
	FVector b = boxVertices[StartIndex + 1];
	MidPoint = (a + b) * 0.5f;

	FVector Endpoint;
	FVector c = boxVertices[StartIndex + 2];
	FVector d = boxVertices[(StartIndex + 3) % 4];
	Endpoint = (c + d) * 0.5f;

	FVector extendDir = Endpoint - MidPoint;
	extendDir.Normalize();
	
	TArray<FVector> intersectionPoints = GetIntersectionPointsFromPolygon(PosList, MidPoint, Endpoint);

	if (intersectionPoints.Num() == 0)	return;

	SubdivisionQueue.Enqueue(CreateNewSharedPtrFromIntersectionSideA(PosList, MidPoint, Endpoint));
	SubdivisionQueue.Enqueue(CreateNewSharedPtrFromIntersectionSideB(PosList, MidPoint, Endpoint));
}

TArray<FVector> ACityLot::CreateBoundingBoxFromLinkedList(TSharedPtr<TDoubleLinkedList<FVector>> const& nodes)
{
	TArray<FVector> out;
	FVector tL; //highest Y lowest X
	FVector bL; //lowest Y lowest X
	FVector tR; //highest Y highest X
	FVector bR; //lowest Y highest X

	float HighestX = GetMaxFloatFromLinkedList(nodes, true);
	float LowestX = GetMinFloatFromLinkedList(nodes, true);
	float HighestY = GetMaxFloatFromLinkedList(nodes, false);
	float LowestY = GetMinFloatFromLinkedList(nodes, false);

	bL = FVector(LowestX, LowestY, 0);
	out.Add(bL);
	tL = FVector(LowestX, HighestY, 0);
	out.Add(tL);
	tR = FVector(HighestX, HighestY, 0);
	out.Add(tR);
	bR = FVector(HighestX, LowestY, 0);
	out.Add(bR);

	return out;
}

TSharedPtr<FBoundsAndMinMax> ACityLot::CreateBoundingBoxFromArray(const TArray<FVector>& Positions)
{
	TSharedPtr<FBoundsAndMinMax, ESPMode::Fast> ret(new FBoundsAndMinMax);
	TArray<FVector> out;
	FVector tL;
	FVector bL;
	FVector tR;
	FVector bR;

	float highestX = TNumericLimits<float>::Lowest();
	float lowestX = TNumericLimits<float>::Max();
	float hightestY = TNumericLimits<float>::Lowest();
	float lowestY = TNumericLimits<float>::Max();

	for (FVector Pos : Positions)
	{
		float current = Pos.X;
		if (current > highestX)
		{
			highestX = current;
		}
	}
	
	for (FVector Pos : Positions)
	{
		float current = Pos.Y;
		if (current > hightestY)
		{
			hightestY = current;
		}
	}

	for (FVector Pos : Positions)
	{
		float current = Pos.X;
		if (current < lowestX)
		{
			lowestX = current;
		}
	}

	for (FVector Pos : Positions)
	{
		float current = Pos.Y;
		if (current < lowestY)
		{
			lowestY = current;
		}
	}
	
	bL = FVector(lowestX, lowestY, 0);
	out.Add(bL);
	tL = FVector(lowestX, hightestY, 0);
	out.Add(tL);
	tR = FVector(highestX, hightestY, 0);
	out.Add(tR);
	bR = FVector(highestX, lowestY, 0);
	out.Add(bR);

	ret->BoundingBox = out;
	ret->maxX = highestX;
	ret->maxY = hightestY;
	ret->minX = lowestX;
	ret->minY = lowestY;
	
	return ret;
}

TArray<FVector> ACityLot::CreateOrientedBoundingBoxFromLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions)
{
	TArray<FVector> Out;
	FVector tL;
	FVector bL;
	FVector tR;
	FVector bR;

	FVector pointA, pointB;

	float SmallestArea = TNumericLimits<float>::Max();
	float SelectedHighestX = TNumericLimits<float>::Lowest();
	float SelectedLowestX = TNumericLimits<float>::Max();
	float SelectedHighestY = TNumericLimits<float>::Lowest();
	float SelectedLowestY = TNumericLimits<float>::Max();
	float SelectedAngle = 0.0f;
	FVector SelectedPivot;

	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIterator(NodePositions->GetHead()); listIterator; ++listIterator)
	{
		// Get points
		pointA = listIterator.GetNode()->GetValue();

		if (listIterator.GetNode()->GetNextNode())
		{
			pointB = listIterator.GetNode()->GetNextNode()->GetValue();
		}
		else
		{
			pointB = NodePositions->GetHead()->GetValue();
		}
		
		FVector temp = pointB - pointA;
		temp.Normalize();

		float angle = acosf(FVector::DotProduct(FVector(1, 0, 0), temp));

		TSharedPtr<TDoubleLinkedList<FVector>> CopiedNodePositions = CreateDeepCopyOfLinkedList(NodePositions);

		// Rotate to make straight based on Pivot (pointA)
		for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> rotatingPointItr(CopiedNodePositions->GetHead()); rotatingPointItr; ++rotatingPointItr)
		{
			// Get the point that needs to rotate
			FVector rotatingPoint = rotatingPointItr.GetNode()->GetValue();
			float rpX = rotatingPoint.X;
			float rpY = rotatingPoint.Y;

			rpX -= pointA.X;
			rpY -= pointA.Y;

			rotatingPointItr.GetNode()->GetValue().X = (rpX * cos(angle)) - (rpY * sin(angle)) + pointA.X;
			rotatingPointItr.GetNode()->GetValue().Y = (rpX * sin(angle)) + (rpY * cos(angle)) + pointA.Y;
		}

		// Get the bounding points
		float highestX = this->GetMaxFloatFromLinkedList(CopiedNodePositions, true);
		float lowestX = this->GetMinFloatFromLinkedList(CopiedNodePositions, true);
		float highestY = this->GetMaxFloatFromLinkedList(CopiedNodePositions, false);
		float lowestY = this->GetMinFloatFromLinkedList(CopiedNodePositions, false);

		// Calculate area
		float area = (highestX - lowestX) * (highestX - lowestX) * (highestY - lowestY) * (highestY - lowestY);

		// Check if smallest area
		if (SmallestArea > area)
		{
			SmallestArea = area;

			SelectedHighestX = highestX;
			SelectedLowestX = lowestX;
			SelectedHighestY = highestY;
			SelectedLowestY = lowestY;

			SelectedAngle = angle;

			SelectedPivot = pointA;
		}
	}
	
	float rpX, rpY;

	// Should be a function
	bL = FVector(SelectedLowestX, SelectedLowestY, 0);
	rpX = bL.X - SelectedPivot.X;
	rpY = bL.Y - SelectedPivot.Y;
	bL.X = (rpX * cos(-SelectedAngle)) - (rpY * sin(-SelectedAngle));
	bL.Y = (rpX * sin(-SelectedAngle)) + (rpY * cos(-SelectedAngle));
	bL.X += SelectedPivot.X;
	bL.Y += SelectedPivot.Y;

	tL = FVector(SelectedLowestX, SelectedHighestY, 0);
	rpX = tL.X - SelectedPivot.X;
	rpY = tL.Y - SelectedPivot.Y;
	tL.X = (rpX * cos(-SelectedAngle)) - (rpY * sin(-SelectedAngle));
	tL.Y = (rpX * sin(-SelectedAngle)) + (rpY * cos(-SelectedAngle));
	tL.X += SelectedPivot.X;
	tL.Y += SelectedPivot.Y;

	tR = FVector(SelectedHighestX, SelectedHighestY, 0);
	rpX = tR.X - SelectedPivot.X;
	rpY = tR.Y - SelectedPivot.Y;
	tR.X = (rpX * cos(-SelectedAngle)) - (rpY * sin(-SelectedAngle));
	tR.Y = (rpX * sin(-SelectedAngle)) + (rpY * cos(-SelectedAngle));
	tR.X += SelectedPivot.X;
	tR.Y += SelectedPivot.Y;

	bR = FVector(SelectedHighestX, SelectedLowestY, 0);
	rpX = bR.X - SelectedPivot.X;
	rpY = bR.Y - SelectedPivot.Y;
	bR.X = (rpX * cos(-SelectedAngle)) - (rpY * sin(-SelectedAngle));
	bR.Y = (rpX * sin(-SelectedAngle)) + (rpY * cos(-SelectedAngle));
	bR.X += SelectedPivot.X;
	bR.Y += SelectedPivot.Y;

	Out.Add(bL);
	Out.Add(tL);
	Out.Add(tR);
	Out.Add(bR);
	Out.Add(FVector(SelectedHighestX - SelectedLowestX, SelectedHighestY - SelectedLowestY, 0));

	return Out;
}

float ACityLot::GetMaxFloatFromLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& Nodes, bool bIsX)
{
	float Max = TNumericLimits<float>::Lowest();

	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIterator(Nodes->GetHead()); listIterator; ++listIterator)
	{
		if (bIsX)
		{
			if (listIterator.GetNode()->GetValue().X > Max)
			{
				Max = listIterator.GetNode()->GetValue().X;
			}
		}
		else
		{
			if (listIterator.GetNode()->GetValue().Y > Max)
			{
				Max = listIterator.GetNode()->GetValue().Y;
			}
		}
	}

	return Max;
}

float ACityLot::GetMinFloatFromLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes, bool bIsX)
{
	float min = TNumericLimits<float>::Max();

	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIterator(nodes->GetHead()); listIterator; ++listIterator)
	{
		if (bIsX)
		{
			if (listIterator.GetNode()->GetValue().X < min)
			{
				min = listIterator.GetNode()->GetValue().X;
			}
		}
		else
		{
			if (listIterator.GetNode()->GetValue().Y < min)
			{
				min = listIterator.GetNode()->GetValue().Y;
			}
		}
	}
	return min;
}

TSharedPtr<TDoubleLinkedList<FVector>> ACityLot::ConvertVectorArrayToDoubleLinkedListSharedPtr(TArray<FVector> Positions)
{
	TSharedPtr<TDoubleLinkedList<FVector>, ESPMode::Fast> Temp(new TDoubleLinkedList<FVector>);

	for (FVector Pos : Positions)
	{
		Temp.Get()->AddTail(Pos);
	}

	return Temp;
}

TSharedPtr<TDoubleLinkedList<FVector>> ACityLot::CreateDeepCopyOfLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes)
{
	TSharedPtr<TDoubleLinkedList<FVector>, ESPMode::Fast> Temp(new TDoubleLinkedList<FVector>);
	
	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIterator(nodes->GetHead()); listIterator; ++listIterator)
	{
		Temp->AddTail(listIterator.GetNode()->GetValue());
	}
	
	return Temp;
}

TSharedPtr<TDoubleLinkedList<FVector>> ACityLot::CreateNewSharedPtrFromIntersectionSideA(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes, FVector pointAStart, FVector pointAEnd)
{
	TSharedPtr<TDoubleLinkedList<FVector>, ESPMode::Fast> FinalList(new TDoubleLinkedList<FVector>);
	
	FVector PointBStart;
	FVector PointBEnd;
	FVector IntersectionPoint = FVector::ZeroVector;
	TArray<FVector> Points;
	
	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector>listIterator(nodes->GetHead()); listIterator; ++listIterator)
	{
		PointBStart = listIterator.GetNode()->GetValue();

		if (listIterator.GetNode()->GetNextNode() != nullptr)
		{
			PointBEnd = listIterator.GetNode()->GetNextNode()->GetValue();
		}
		else
		{
			PointBEnd = nodes->GetHead()->GetValue();
		}

		FVector dir = pointAEnd - pointAStart;
		dir.Normalize();
		
		if (FMath::SegmentIntersection2D(pointAStart + dir * -5000, pointAEnd + dir * 5000, PointBStart, PointBEnd, IntersectionPoint))
		{
			FinalList->AddTail(IntersectionPoint);
			for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIteratorNext(listIterator.GetNode()->GetNextNode()); listIteratorNext; ++listIteratorNext)
			{
				FinalList->AddTail(listIteratorNext.GetNode()->GetValue());

				FVector pointCStart = listIteratorNext.GetNode()->GetValue();
				FVector pointCEnd;
				
				if (listIteratorNext.GetNode()->GetNextNode() == nullptr)
				{
					pointCEnd = nodes->GetHead()->GetValue();
				}
				else
				{
					pointCEnd = listIteratorNext.GetNode()->GetNextNode()->GetValue();
				}

				FVector interectionPoint2 = FVector::ZeroVector;
				
				if (FMath::SegmentIntersection2D(pointAStart + dir * -5000, pointAEnd + dir * 5000, pointCStart, pointCEnd, interectionPoint2))
				{
					FinalList->AddTail(interectionPoint2);
					
					return FinalList;
				}
			}
			
			break;
		}
	}

	return FinalList;
}

TSharedPtr<TDoubleLinkedList<FVector>> ACityLot::CreateNewSharedPtrFromIntersectionSideB(const TSharedPtr<TDoubleLinkedList<FVector>>& nodes, FVector pointAStart, FVector pointAEnd)
{
	TSharedPtr<TDoubleLinkedList<FVector>> FinalList(new TDoubleLinkedList<FVector>);
	FVector pointBStart;
	FVector pointBEnd;
	FVector intersectionPoint = FVector::ZeroVector;
	TArray<FVector> Points;
	
	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIterator(nodes->GetHead()); listIterator; ++listIterator)
	{
		//add head to the list. side b always start from the node head
		FVector vec = listIterator.GetNode()->GetValue();
		FinalList->AddTail(vec);

		//check for an intersection
		pointBStart = listIterator.GetNode()->GetValue();
		if (listIterator.GetNode()->GetNextNode() != nullptr)
		{
			pointBEnd = listIterator.GetNode()->GetNextNode()->GetValue();
		}
		else
		{
			pointBEnd = nodes->GetHead()->GetValue();
		}
		
		//found intersection //add tail after the added node
		FVector dir = pointAEnd - pointAStart;
		dir.Normalize();
		//	DrawDebugLine(this->GetWorld(), pointAStart + dir * -5000, pointAEnd + dir * 5000,
		//		FColor::Red, true, -1, -1, 48.0f);
		if (FMath::SegmentIntersection2D(pointAStart + dir * -5000, pointAEnd + dir * 5000, pointBStart, pointBEnd, intersectionPoint))
		{
			FinalList->AddTail(intersectionPoint);

			for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIteratorNext(listIterator.GetNode()->GetNextNode()); listIteratorNext; ++listIteratorNext)
			{
				//check for intersection
				FVector pointCStart = listIteratorNext.GetNode()->GetValue();
				FVector pointCEnd;
				if (listIteratorNext.GetNode()->GetNextNode() == nullptr)
				{
					pointCEnd = nodes->GetHead()->GetValue();
				}
				else
				{
					pointCEnd = listIteratorNext.GetNode()->GetNextNode()->GetValue();
				}

				FVector interectionPoint2 = FVector::ZeroVector;
				
				if (FMath::SegmentIntersection2D(pointAStart + dir * -5000, pointAEnd + dir * 5000, pointCStart, pointCEnd, interectionPoint2))
				{
					FinalList->AddTail(interectionPoint2);
					
					for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIteratorFinal(listIteratorNext.GetNode()->GetNextNode()); listIteratorFinal; ++listIteratorFinal)
					{
						FinalList->AddTail(listIteratorFinal.GetNode()->GetValue());
					}
					
					return FinalList;
				}
			}
			
			break;
		}
	}

	return FinalList;
}

FVector ACityLot::GetIntersectionPointFromArray(const TArray<FVector>& Positions, FVector bisectorOrigin, FVector bisectorLine)
{
	FVector a;
	FVector b;
	FVector lineC;
	FVector lineA;
	FVector lineB;
	FVector intersection = FVector::ZeroVector;
	
	for (int i = 0; i < Positions.Num(); i++)
	{
		a = Positions[i];
		b = Positions[(i + (Positions.Num() - 1)) % Positions.Num()]; //next point from a
		
		if (a != bisectorOrigin && b != bisectorOrigin)
		{
			lineA = a - b; //a-b
			lineC = a - bisectorOrigin;
			FVector crossAB = FVector::CrossProduct(bisectorLine, lineA);
			FVector crossBC = FVector::CrossProduct(lineC, lineA);
			
			if (FMath::SegmentIntersection2D(bisectorOrigin, bisectorLine, a, b, intersection))
			{
				break;
			}
		}
	}
	return intersection;
}

TArray<FVector> ACityLot::GetIntersectionPointsFromPolygon(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions, FVector PointAStart, FVector PointAEnd)
{
	FVector Dir = PointAEnd - PointAStart;
	Dir.Normalize();
	TArray<FVector> Points;
	
	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIterator(NodePositions->GetHead()); listIterator; ++listIterator)
	{
		FVector PointBStart = listIterator.GetNode()->GetValue();
		FVector PointBEnd;

		if (listIterator.GetNode()->GetNextNode() != nullptr)
		{
			PointBEnd = listIterator.GetNode()->GetNextNode()->GetValue();
		}
		else
		{
			PointBEnd = NodePositions->GetHead()->GetValue();
		}
		
		FVector intersectionPoint = FVector::ZeroVector;

		if (FMath::SegmentIntersection2D(PointAStart + Dir * -5000, PointAEnd + Dir * 5000, PointBStart, PointBEnd, intersectionPoint))
		{
			Points.Add(intersectionPoint);
		}
	}

	return Points;
}

FVector ACityLot::GetBuildingDirectionToNearestEdge(FVector Position)
{
	FVector ClosestPoint = FindNearestPointOfPositionOnBorder(Position);
	FVector Diff = ClosestPoint - Position;
	FVector Dir;
	float Length;
	Diff.ToDirectionAndLength(Dir, Length);
	return Dir;
}

void ACityLot::TurnOffBuildingPhysics()
{
	if (LotComponent == nullptr)
	{
		return;
	}

	LotComponent->TurnOffAllPhysicsAfterSpawn();
}

void ACityLot::OnTopologyChanged()
{
	bIsTriangulated = false;
}

void ACityLot::SpawnDetails(ARoadNetwork* roadNetwork, float spawnDensity)
{
	if (InsetPositions.Num() == 0)
	{
		return;
	}
	
	TSharedPtr<TDoubleLinkedList<FVector>> PosList = ConvertVectorArrayToDoubleLinkedListSharedPtr(InsetPositions);
	TArray<FVector> boxVertices = CreateOrientedBoundingBoxFromLinkedList(PosList);
	float area = boxVertices[4].X * boxVertices[4].Y;

	TSharedPtr<FBoundsAndMinMax, ESPMode::Fast> Bounds = CreateBoundingBoxFromArray(InsetPositions);
	TArray<FVector> BoundingBox = Bounds->BoundingBox;

	TArray<FVector> linetracepoints;
	float AverageDetailsArea = area / 20000000.0f;
	float ZLocation = 1000.0f;

	int numlinetrace = FMath::RoundToInt(AverageDetailsArea * spawnDensity);

	for (int i = 0; i < numlinetrace; i++)
	{
		FVector Pos = FVector(FMath::RandRange(Bounds->minX, Bounds->maxX), FMath::RandRange(Bounds->minY, Bounds->maxY), ZLocation);

		if(IsPointInsidePolygon(InsetPositions, Pos) && !this->CheckPointIfVeryClose(Pos, linetracepoints))
		{
			FString temp = Pos.ToString();
			linetracepoints.Add(Pos);
		}
		else if (i < AverageDetailsArea)
		{
			numlinetrace++;
		}
	}

	FHitResult hit;
	FVector start;
	FVector end;
	bool didHit = false;

	TArray<int> detailList = roadNetwork->RetrieveRandomDetailsGroupFromCityZone(CityZoneType);

	for (int i = 0; i < linetracepoints.Num(); i++)
	{
		start = linetracepoints[i];
		end = linetracepoints[i] + FVector::DownVector * 100000;
		FCollisionQueryParams TraceParams;
		didHit = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Visibility, TraceParams);

		if (didHit && hit.ImpactPoint.Z < 15)
		{
			LotComponent->SpawnDetailFromComponent(detailList[FMath::Rand() % detailList.Num()], hit.ImpactPoint, FRotator::ZeroRotator, roadNetwork);
		}
	}
}

bool ACityLot::CheckPointIfVeryClose(FVector point, TArray<FVector> positions)
{
	for (int i = 0; i<positions.Num(); i++)
	{
		float Distance = FVector::Dist(point, positions[i]);
		
		if (Distance < 500.0f)
		{
			return true;
		}
	}

	return false;
}

void ACityLot::DrawDebugSharedPtrListVectors(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions)
{
	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> listIterator(NodePositions->GetHead()); listIterator; ++listIterator)
	{
		FVector Pos = listIterator.GetNode()->GetValue();
		
		// DrawDebugSphere(CityBlock->GetWorld(), position + zOffset, 100, 10, FColor::Purple, true, -1, 0);
		// DrawDebugString(this->GetWorld(), position + zOffset, position.ToString(), 0, FColor::White, -1, false, 3);
		
		if (listIterator.GetNode()->GetNextNode() != nullptr)
		{
			FVector next = listIterator.GetNode()->GetNextNode()->GetValue();
			FColor color = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), 255);
			// DrawDebugLine(this->GetWorld(), position + zOffset + zOffset, next + zOffset + zOffset, color, true, -1, 0, 48.0f);
		}
		else
		{
			// FVector next = list->GetHead()->GetValue();
			// DrawDebugLine(this->GetWorld(), position, next, FColor::Green, true, -1, -1, 48.0f);
		}
	}
}

void ACityLot::DebugDrawCityZoneMesh(UWorld* World) const
{
	for (int i = 0; i < TriangleIndicesArray.Num(); i += 3)
	{
		FVector A = BorderingRoadHalfEdgesArray[TriangleIndicesArray[i + 0]]->GetOriginRoadNode()->GetActorLocation();
		FVector B = BorderingRoadHalfEdgesArray[TriangleIndicesArray[i + 1]]->GetOriginRoadNode()->GetActorLocation();
		FVector C = BorderingRoadHalfEdgesArray[TriangleIndicesArray[i + 2]]->GetOriginRoadNode()->GetActorLocation();
		// DrawDebugLine(World, A, B, FColor::Green, true, -1, -1, 24.0f);
		// DrawDebugLine(World, B, C, FColor::Green, true, -1, -1, 24.0f);
		// DrawDebugLine(World, C, A, FColor::Green, true, -1, -1, 24.0f);
	}
}

void ACityLot::SetCityBlock(ACityBlock* NewCityBlock)
{
	CityBlock = NewCityBlock;
}

void ACityLot::SetBorderingHalfEdges(const TArray<FRoadHalfEdge*>& NewBorderingRoadHalfEdgesArray)
{
	BorderingRoadHalfEdgesArray = NewBorderingRoadHalfEdgesArray;
	OnTopologyChanged();
}

void ACityLot::SetCityZoneType(ECityZoneType NewCityZoneType)
{
	CityZoneType = NewCityZoneType;
}

FVector ACityLot::GetMidpointFromLinkedList(const TSharedPtr<TDoubleLinkedList<FVector>>& NodePositions)
{
	FVector Out = FVector::ZeroVector;
	int count = 0;
	
	for (TDoubleLinkedListIterator<TDoubleLinkedList<FVector>::TDoubleLinkedListNode, FVector> Iter(NodePositions->GetHead()); Iter; ++Iter)
	{
		Out += Iter.GetNode()->GetValue();
		count++;
	}

	return Out / count;
}

const TArray<FRoadHalfEdge*>& ACityLot::GetAllBorderingRoadHalfEdges() const
{
	return BorderingRoadHalfEdgesArray;
}

TArray<FVector> ACityLot::GetBorderingRoadNodesPositionsArray() const
{
	TArray<FVector> RoadNodesPositions;
	
	for (FRoadHalfEdge* RoadHalfEdge : BorderingRoadHalfEdgesArray)
	{
		RoadNodesPositions.Add(RoadHalfEdge->GetOriginRoadNode()->GetActorLocation());
	}

	return RoadNodesPositions;
}

TArray<FVector> ACityLot::GetInsetPositionsArray()
{
	if (InsetPositions.Num() == 0)
	{
		InitializeInsetPolygon();
	}
	
	return InsetPositions;
}

ECityZoneType ACityLot::GetCityZoneType() const
{
	return CityZoneType;
}

float ACityLot::GetArea()
{
	Triangulate();

	float TriangleAreaSum = 0.0f;

	for (int i = 0; i < TriangleIndicesArray.Num(); i += 3)
	{
		FVector A = BorderingRoadHalfEdgesArray[TriangleIndicesArray[i + 0]]->GetOriginRoadNode()->GetActorLocation();
		FVector B = BorderingRoadHalfEdgesArray[TriangleIndicesArray[i + 1]]->GetOriginRoadNode()->GetActorLocation();
		FVector C = BorderingRoadHalfEdgesArray[TriangleIndicesArray[i + 2]]->GetOriginRoadNode()->GetActorLocation();
		float AreaOfTriangle = CustomMath::GetAreaOfTriangle(A, B, C);
		TriangleAreaSum += AreaOfTriangle;
	}

	return TriangleAreaSum;
}

FVector ACityLot::GetCentroidPosition() const
{
	FVector Sum = FVector::ZeroVector;

	for (FRoadHalfEdge* RoadHalfEdge : BorderingRoadHalfEdgesArray)
	{
		FVector Loc = RoadHalfEdge->GetOriginRoadNode()->GetActorLocation();
		Sum += Loc;
	}

	return Sum / BorderingRoadHalfEdgesArray.Num();
}

bool ACityLot::IsTriangulated() const
{
	return bIsTriangulated;
}

bool ACityLot::IsHalfEdgesCountValid() const
{
	// Edges must be equal or more than 3
	if (BorderingRoadHalfEdgesArray.Num() < 3)
	{
		return false;
	}

	return true;
}

bool ACityLot::IsHalfEdgesCycleValid() const
{
	if (!IsHalfEdgesCountValid())
	{
		return false;
	}

	return CustomMath::IsHalfEdgeCycleClockwise(BorderingRoadHalfEdgesArray);
}


bool ACityLot::IsPointInsidePolygon(const TArray<FVector>& PolygonPositions, const FVector& Point)
{
	FVector Intersection = FVector::ZeroVector;
	TArray<FVector> IntersectionList;
	
	for (int i = 0; i<PolygonPositions.Num(); i++)
	{
		FVector CurrPos = PolygonPositions[i];
		FVector NextPos = PolygonPositions[(i + 1) % PolygonPositions.Num()];
		
		if(FMath::SegmentIntersection2D(CurrPos, NextPos, Point, Point + FVector::RightVector * 1000000, Intersection))
		{
			IntersectionList.Add(Intersection);
		}
	}

	if (IntersectionList.Num() % 2 == 0)
	{
		return false;
	}
	
	return true;
}

void ACityLot::SpawnSidewalkFromComponent(ARoadNetwork* RoadNetwork)
{
	LotComponent->SpawnSidewalkFromComponent(GetAllBorderingRoadHalfEdges(), InsetPositions, RoadNetwork);
}

void ACityLot::SemanticSegmentationMode(UMaterialInterface* SidewalkSemanticMaterial, UMaterialInterface* FenceSemanticMaterial)
{
	// Floor
	if (ZoneFloorProceduralMeshComponent->GetProcMeshSection(0) != nullptr)
	{
		bIsSemanticSegmentation = !bIsSemanticSegmentation;
		
		if (bIsSemanticSegmentation)
		{
			ZoneFloorProceduralMeshComponent->SetMaterial(0, SidewalkSemanticMaterial);
		}
		else
		{
			ZoneFloorProceduralMeshComponent->SetMaterial(0, ZoneFloorMaterial);
		}
	}
	
	// Sidewalks & Fences
	LotComponent->SemanticSegmentationMode(SidewalkSemanticMaterial, FenceSemanticMaterial);
}

void ACityLot::Destroyed()
{
	Super::Destroyed();

	ZoneColorProceduralMeshComponent->ClearAllMeshSections();
	ZoneFloorProceduralMeshComponent->ClearAllMeshSections();
	zOffset = FVector(0, 0, 300);

	for (AActor* Building : BuildingsSpawned)
	{
		Building->Destroy();
	}

	if (this->LotComponent != nullptr)
	{
		LotComponent->ClearAllSpawnedBuildingsInLotComponent();
	}
}

bool ApproximatelyEqual(float a, float b, float epsilon)
{
	return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool EssentiallyEqual(float a, float b, float epsilon)
{
	return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool DefinitelyGreaterThan(float a, float b, float epsilon)
{
	return (a - b) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool DefinitelyLessThan(float a, float b, float epsilon)
{
	return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

void ACityLot::ClearSubdivisions()
{
	Subdivisions.Empty();
	SubdivisionQueue.Empty();
}