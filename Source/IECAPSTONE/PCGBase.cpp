// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGBase.h"
#include <string>
#include "Kismet/GameplayStatics.h"
#include "KismetProceduralMeshLibrary.h"
#include "MeshDescription.h"
#include "DrawDebugHelpers.h"
#include "StreetMapComponent.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green,text)
#define printFString(text, fstring) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta, FString::Printf(TEXT(text), fstring))

APCGBase::APCGBase()
{
	PrimaryActorTick.bCanEverTick = false;

	pmc = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMeshComponent"));
	pmc->SetupAttachment(RootComponent);
}

void APCGBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void APCGBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void APCGBase::OnCreateCity()
{
	
}

void APCGBase::OnCreateSubdivisions()
{
	ResetAll();

	if (EndpointReference == nullptr)
	{
		return;
	}
	
	UGameplayStatics::GetAllActorsOfClassWithTag(this->GetWorld(), EndpointReference, FName("endpoint"), actor_storage);

	for (int i = 0; i < actor_storage.Num(); i++)
	{
		vertices.Add(actor_storage[i]->GetActorLocation());
	}
	
	FlushPersistentDebugLines(this->GetWorld());
	
	FVector Midpoint;
	float TempX = 0;
	float TempY = 0;
	float TempZ = 0;
	
	for (int i = 0; i < vertices.Num(); i++)
	{
		TempX += vertices[i].X;
		TempY += vertices[i].Y;
		TempZ += vertices[i].Z;
	}

	Midpoint.Set(TempX / vertices.Num(), TempY / vertices.Num(), TempZ / vertices.Num());

	TArray<FVector> InsetPoints;
	
	for(int i = 0 ; i < vertices.Num(); i++)
	{
		FVector Subtracted = Midpoint - vertices[i];
		float Magnitude = sqrt(pow(Subtracted.X,2) + pow(Subtracted.Y,2) + pow(Subtracted.Z,2));
		FVector UnitVec = Subtracted / Magnitude;
		InsetPoints.Add(vertices[i] + (UnitVec) * 100.f);
	}

	TArray<FVector> InnerPoints;
	
	for (int i = 0; i < InsetPoints.Num(); i++)
	{
		FVector Subtracted = Midpoint - InsetPoints[i];
		float Magnitude = sqrt(pow(Subtracted.X, 2) + pow(Subtracted.Y, 2) + pow(Subtracted.Z, 2));
		FVector UnitVec = Subtracted / Magnitude;
		InnerPoints.Add(InsetPoints[i] + UnitVec * 500.f);
	}

	TArray<FFaceInfo> FaceInfos;
	
	for (int i = 0 ; i < InsetPoints.Num(); i++)
	{
		FVector CurrentPoint = vertices[i];
		FVector NextPoint = vertices[(i + 1) % vertices.Num()];

		FVector InnerCurrentPoint = InnerPoints[i];
		FVector InnerNextPoint = InnerPoints[(i + 1) % InnerPoints.Num()];
		FVector MidpointZ = GetMidpoint(InnerCurrentPoint, InnerNextPoint);
		FVector Normal = GetFaceNormals(MidpointZ, CurrentPoint, NextPoint);
		FFaceInfo FaceInfo;
		FaceInfo.normal = Normal;
		FaceInfos.Add(FaceInfo);
	}

	TArray<FVector> InnerMidpoints;
	TArray<FVector> BuildingPoints;
	
	for (int i = 0; i < InsetPoints.Num(); i++)
	{
		int next = (i + 1) % InsetPoints.Num();
		FVector InsetStart = InsetPoints[i];
		FVector InsetEnd = InsetPoints[next];
		FVector InnerStart = InnerPoints[i];
		FVector InnerEnd = InnerPoints[next];
		FVector Subtracted = InsetEnd - InsetStart;
		float FrontFaceLength = sqrt(pow(Subtracted.X, 2) + pow(Subtracted.Y, 2) + pow(Subtracted.Z, 2));
		FVector subtracted2 = InnerEnd - InnerStart;
		float backFaceLength = sqrt(pow(subtracted2.X, 2) + pow(subtracted2.Y, 2) + pow(subtracted2.Z, 2));
		int buildingCount = floor(backFaceLength / BUILDING_WIDTH);

		FVector InsetDirection = GetDirection(InsetEnd, InsetStart);
		FVector InnerDirection = GetDirection(InnerEnd, InnerStart);
	
		FVector FrontFaceStep = Subtracted  * (BUILDING_WIDTH / FrontFaceLength);
		FVector BackFaceStep = subtracted2  * (BUILDING_WIDTH / FrontFaceLength) ;

		FVector CurrentInsetPoint = InsetPoints[i];
		FVector CurrentInnerPoint = InnerPoints[i];
		FVector PointInit = CurrentInsetPoint + InsetDirection * BUILDING_WIDTH;
		FVector PointInit1 = CurrentInnerPoint + (InnerDirection * BUILDING_WIDTH);
		bool bHasStarted = false;
		
		for (int j = 0 ; j < buildingCount; j ++)
		{
			FVector Point0;
			FVector Point1;
			FVector Point2;
			FVector Point3;

			if (!bHasStarted)
			{
				Point0 = CurrentInsetPoint;
				Point1 = CurrentInnerPoint;
				bHasStarted = true;
			}
			else
			{
				Point0 = Point3;
				Point1 = Point2;
			}
			
			Point2 = Point1 + InnerDirection * BUILDING_WIDTH;
			Point3 = Point0 + InsetDirection * (BUILDING_WIDTH * (FrontFaceLength/backFaceLength));

			BuildingPoints.Add(Point0);
			BuildingPoints.Add(Point1);
			BuildingPoints.Add(Point2);
			BuildingPoints.Add(Point3);
		}
	}

	TArray<FVector> SpawnCenter;
	
	for(int i = 0; i < BuildingPoints.Num(); i += 4)
	{
		FVector Temp = GetMidpointFour(BuildingPoints[i], BuildingPoints[(i + 1) % BuildingPoints.Num()], BuildingPoints[(i + 2) % BuildingPoints.Num()], BuildingPoints[(i + 3) % BuildingPoints.Num()]);
		SpawnCenter.Add(Temp);
	}

	int StartIndex = 0;
	
	for (int j = 0 ; j < FaceInfos.Num(); j++)
	{
		if (j == FaceInfos.Num() - 1)
		{
			FString IntAsString = FString::FromInt(SpawnCenter.Num());
			DrawDebugString(this->GetWorld(), FVector(0, 0, 5), TEXT("building count: " + IntAsString), 0, FColor::Cyan, -1, false, 4);
		}
		
		for (int i = StartIndex; i < SpawnCenter.Num(); i++)
		{
			FVector position = SpawnCenter[i];
			FRotator rotation = FRotator(0.0f, 0.0f, 0.0f);
			//rotation = FRotationMatrix::MakeFromX(faceInfos[j].normal).Rotator();
			float angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector(0, 1, 0), FaceInfos[j].normal)));
			rotation = FRotator(0, angle, 0);
			
			FActorSpawnParameters SpawnInfo;
			AActor* Temp = GetWorld()->SpawnActor(BuildingToSpawn->GetClass(), &position, &rotation, SpawnInfo);
			Temp->SetActorScale3D(FVector(FMath::RandRange(0.8f, 1.3f), FMath::RandRange(1.0f, 1.4f) , FMath::RandRange(2.5f, 4.5f)));
			spawnedBuildings.Add(Temp);
			StartIndex++;
		}
	}
}

void APCGBase::ResetAll()
{
	vertices.Reset();
	actor_storage.Empty();
	triangles.Reset();
	uvs.Reset();
	normals.Reset();
	tangents.Reset();
	colors.Reset();

	FlushPersistentDebugLines(this->GetWorld());
}

int cuurentIndex = 0;

void APCGBase::GenerateNodesFromStreetMap()
{
	if (this->osmActor == nullptr)
	{
		return;
	}

	UStreetMapComponent* StreetMapComponent = this->osmActor->GetStreetMapComponent();
	const TArray<FStreetMapNode>& Nodes = StreetMapComponent->GetStreetMap()->GetNodes();
	const TArray<FStreetMapRoad>& Nodes2 = StreetMapComponent->GetStreetMap()->GetRoads();
	
	TArray<FStreetMapVertex> streetmapNodes = StreetMapComponent->GetRawMeshVertices();
	FVector Position;
	FVector2D Position2d;
	FString IntAsString = FString::FromInt(Nodes2.Num());
	printFString("number of roads: %s", *IntAsString);
	TArray<FVector> RoadVerts;
	
	for (int i = 0; i< Nodes2.Num();i++)
	{
		TArray<FVector2D> pointsInRoad= Nodes2[i].RoadPoints;
		TArray<int32> NodeIndices = Nodes2[i].NodeIndices;

		for (int k = 0; k < NodeIndices.Num();k++)
		{
			if(NodeIndices[k] != -1)
			{
				Position2d = Nodes[NodeIndices[k]].GetLocation(*StreetMapComponent->GetStreetMap());
				Position = FVector(Position2d, 0);
			}
		}
		
		TArray<FVector> RoadGeneratorStuff;
		for (int j = 0; j<pointsInRoad.Num(); j++)
		{
			Position2d = pointsInRoad[j];
			RoadVerts.Add(FVector(Position2d,0));
			RoadGeneratorStuff.Add(FVector(Position2d, 0));
		}

		if(nodeToSpawn == nullptr)
		{
			return;
		}

		FVector Direction = GetDirection(RoadGeneratorStuff[1], RoadGeneratorStuff[0]);
		FRotator Rotator = Direction.Rotation();
		FTransform SpawnTransform(Rotator, RoadGeneratorStuff[0]);
		auto DefferedActor = CastChecked<ABaseNode>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, nodeToSpawn, SpawnTransform));
		
		if (DefferedActor != nullptr)
		{
			DefferedActor->initialize(i, RoadGeneratorStuff[0], Rotator, RoadGeneratorStuff);//roadGeneratorStuff[0]
			DefferedActor->FinishSpawning(SpawnTransform);
			roadNetworkRefs.Add(DefferedActor);
		}
	}
}

AActor* APCGBase::getNearestExistingEndpoint(FVector currentPosition)
{
	AActor* TempActor = nullptr;
	AActor* ReturnActor = nullptr;
	FVector ClosestPosition = FVector::ZeroVector;
	float ClosestDistance = 0;
	
	for (int i = 0; i < actor_storage.Num(); i++)
	{
		TempActor = actor_storage[i];
		float Distance = FVector::Dist(currentPosition, TempActor->GetActorLocation());
		
		if (Distance < ClosestDistance || ClosestPosition.IsZero())
		{
			ClosestPosition = TempActor[i].GetActorLocation();
			ClosestDistance = Distance;
			ReturnActor = TempActor;
		}
	}

	if(ClosestDistance <= this->SNAP_DISTANCE)
	{
		return ReturnActor;
	}
	
	return nullptr;
}

float APCGBase::GetMagnitude(FVector vector)
{
	return sqrt(pow(vector.X, 2) + pow(vector.Y, 2) + pow(vector.Z, 2));
}

FVector APCGBase::GetDirection(FVector a, FVector b)
{
	FVector Subtracted = a - b;
	float Magnitude = GetMagnitude(Subtracted);
	return Subtracted / Magnitude;
}

FVector APCGBase::GetFaceNormals(FVector midpoint, FVector a, FVector b)
{
	FVector DirectionA = GetDirection(midpoint, a);
	FVector DirectionB = GetDirection(midpoint, b);
	FVector Out = (DirectionA + DirectionB) * 0.5f;
	Out *= -1;
	
	return Out;
}

FVector APCGBase::GetMidpoint(FVector a, FVector b)
{
	return FVector((a.X + b.X) / 2, (a.Y + b.Y) / 2, (a.Z + b.Z) / 2);
}

FVector APCGBase::GetMidpointFour(FVector a, FVector b, FVector c, FVector d)
{
	return FVector((a.X + b.X + c.X + d.X) / 4, (a.Y + b.Y + c.Y + d.Y) / 4, (a.Z + b.Z + c.Z + d.Z) / 4);
}

