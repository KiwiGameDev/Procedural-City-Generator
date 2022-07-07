// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "FLineSegment.h"

struct FRoadHalfEdge;

class CustomMath
{
public:
	static FVector GetDirection(const FVector& To, const FVector& From);
	static bool GetIntersection(const FLineSegment& LineA, const FLineSegment& LineB, FVector& OutIntersection);
	static float GetClockwiseAngleBetweenVectors(const FVector& From, const FVector& To);
	static float CrossZ(const FVector& A, const FVector& B);
	static bool IsPointInTriangle(const FVector& P, const FVector& A, const FVector& B, const FVector& C);
	static float Sign(const FVector& A, const FVector& B, const FVector& C);
	static float GetAreaOfTriangle(const FVector& A, const FVector& B, const FVector& C);
	static bool IsHalfEdgeCycleClockwise(const TArray<FRoadHalfEdge*> HalfEdgesArray);
	static TArray<FVector> GetInsetOfPolygon(const TArray<FVector>& VertexPositions, float InsetSize);
};
