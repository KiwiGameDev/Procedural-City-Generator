#include "CustomMath.h"
#include "RoadHalfEdge.h"
#include "RoadNode.h"

FVector CustomMath::GetDirection(const FVector& To, const FVector& From)
{
	FVector Out = To - From;
	Out.Normalize();
	return Out;
}

bool CustomMath::GetIntersection(const FLineSegment& LineA, const FLineSegment& LineB, FVector& OutIntersection)
{
	FVector P1(LineA.PointB.X - LineA.PointA.X, LineA.PointB.Y - LineA.PointA.Y, 0.0f);
	FVector P2(LineB.PointB.X - LineB.PointA.X, LineB.PointB.Y - LineB.PointA.Y, 0.0f);

	float Denom = -P2.X * P1.Y + P1.X * P2.Y;
	float S = (-P1.Y * (LineA.PointA.X - LineB.PointA.X) + P1.X * (LineA.PointA.Y - LineB.PointA.Y)) / Denom;
	float T = (P2.X * (LineA.PointA.Y - LineB.PointA.Y) - P2.Y * (LineA.PointA.X - LineB.PointA.X)) / Denom;

	if (S >= 0 && S <= 1 && T >= 0 && T <= 1)
	{
		// Intersection detected
		OutIntersection.X = LineA.PointA.X + (T * P1.X);
		OutIntersection.Y = LineA.PointA.Y + (T * P1.Y);
		OutIntersection.Z = 0.0f;
		return true;
	}

	OutIntersection = FVector(0.0f, 0.0f, 0.0f);
	return false;
}

float CustomMath::GetClockwiseAngleBetweenVectors(const FVector& From, const FVector& To)
{
	float FromAngle = FMath::Atan2(From.Y, From.X);
	float ToAngle = FMath::Atan2(To.Y, To.X);
	float AngleBetween = ToAngle - FromAngle;

	if (AngleBetween < 0.0f)
	{
		AngleBetween += 2.0f * PI;
	}

	return AngleBetween;
}

float CustomMath::CrossZ(const FVector& A, const FVector& B)
{
	return A.X * B.Y - A.Y * B.X;
}

bool CustomMath::IsPointInTriangle(const FVector& P, const FVector& A, const FVector& B, const FVector& C)
{
	float A1 = Sign(P, A, B);
	float A2 = Sign(P, B, C);
	float A3 = Sign(P, C, A);
	bool bHasNeg = (A1 < 0) || (A2 < 0) || (A3 < 0);
	bool bHasPos = (A1 > 0) || (A2 > 0) || (A3 > 0);
	
	return !(bHasNeg && bHasPos);
}

float CustomMath::Sign(const FVector& A, const FVector& B, const FVector& C)
{
	return (A.X - C.X) * (B.Y - C.Y) - (B.X - C.X) * (A.Y - C.Y);
}

float CustomMath::GetAreaOfTriangle(const FVector& A, const FVector& B, const FVector& C)
{
	return FMath::Abs(A.X * (B.Y - C.Y) + B.X * (C.Y - A.Y) + C.X * (A.Y - B.Y)) * 0.5f;
}

bool CustomMath::IsHalfEdgeCycleClockwise(const TArray<FRoadHalfEdge*> HalfEdgesArray)
{
	float Sum = 0.0f;
		
	for (FRoadHalfEdge* HalfEdgeToSum : HalfEdgesArray)
	{
		ARoadNode* RoadNodeA = HalfEdgeToSum->GetOriginRoadNode();
		ARoadNode* RoadNodeB = HalfEdgeToSum->GetNextRoadNode();
		FVector LocA = RoadNodeA->GetActorLocation();
		FVector LocB = RoadNodeB->GetActorLocation();

		Sum += (LocB.X - LocA.X) * (LocB.Y + LocA.Y);
	}

	return Sum < 0;
}

TArray<FVector> CustomMath::GetInsetOfPolygon(const TArray<FVector>& VertexPositions, float InsetSize)
{
	int NodePositionsCount = VertexPositions.Num();
	TArray<FVector> Out;

	for (int i = 0; i < NodePositionsCount; i++)
	{
		int PrevNodeIndex = (i - 1 + NodePositionsCount) % NodePositionsCount;
		int NextNodeIndex = (i + 1) % NodePositionsCount;

		FVector NextDir = VertexPositions[NextNodeIndex] - VertexPositions[i];
		NextDir.Normalize();
		float nnnX = NextDir.Y;
		float nnnY = NextDir.X * -1;

		FVector PrevDir = VertexPositions[i] - VertexPositions[PrevNodeIndex];
		PrevDir.Normalize();
		float npnX = PrevDir.Y;
		float npnY = PrevDir.X * -1;

		float BisectorX = (nnnX + npnX) * -1;
		float BisectorY = (nnnY + npnY) * -1;

		FVector Bisector = FVector(BisectorX, BisectorY, 0);
		Bisector.Normalize();

		float InsetSizeScale = 0.5f;
		float InsetSizeMult = 1.0f + FMath::Clamp(1.0f - FVector::DotProduct(PrevDir, NextDir), 0.0f, 1.0f) * InsetSizeScale;

		FVector InsetPos = FVector(
			VertexPositions[i].X + Bisector.X * InsetSize * InsetSizeMult,
			VertexPositions[i].Y + Bisector.Y * InsetSize * InsetSizeMult,
			0);
		Out.Add(InsetPos);
	}
	
	return Out;
}
