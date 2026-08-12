#include "Game/BFLStatics.h"

#include "Game/BFLGameMode.h"
#include "Game/BFLGameSettings.h"
#include "Materials/MaterialInstanceDynamic.h"

ABFLGameMode* UBFLStatics::GetBFLGameMode(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetAuthGameMode<ABFLGameMode>();
	}
	return nullptr;
}

float UBFLStatics::GetWaterHeight(const UObject* WorldContextObject)
{
	if (const ABFLGameMode* GM = GetBFLGameMode(WorldContextObject))
	{
		return GM->GetWaterHeight();
	}
	return UBFLGameSettings::Get()->WaterHeight;
}

float UBFLStatics::GetLakeRadius(const UObject* WorldContextObject)
{
	if (const ABFLGameMode* GM = GetBFLGameMode(WorldContextObject))
	{
		return GM->GetLakeRadius();
	}
	return UBFLGameSettings::Get()->LakeRadius;
}

FVector UBFLStatics::GetLakeCenter(const UObject* WorldContextObject)
{
	if (const ABFLGameMode* GM = GetBFLGameMode(WorldContextObject))
	{
		return GM->GetLakeCenter();
	}
	return FVector::ZeroVector;
}

ABFLBaitActor* UBFLStatics::GetActiveBait(const UObject* WorldContextObject)
{
	if (ABFLGameMode* GM = GetBFLGameMode(WorldContextObject))
	{
		return GM->GetActiveBait();
	}
	return nullptr;
}

FVector UBFLStatics::ClampToLake(const FVector& Location, const FVector& Center, float Radius, float WaterZ, float DepthOffset)
{
	FVector Flat = Location;
	Flat.Z = 0.f;
	FVector CenterFlat(Center.X, Center.Y, 0.f);
	FVector Offset = Flat - CenterFlat;
	const float Dist = Offset.Size();
	if (Dist > Radius && Dist > KINDA_SMALL_NUMBER)
	{
		Offset *= (Radius / Dist);
	}
	return FVector(CenterFlat.X + Offset.X, CenterFlat.Y + Offset.Y, WaterZ + DepthOffset);
}

FVector UBFLStatics::RandomPointInLake(const FVector& Center, float Radius, float WaterZ, float DepthOffset, float MinRadiusFraction)
{
	const float Angle = FMath::FRand() * 2.f * PI;
	const float MinR = Radius * FMath::Clamp(MinRadiusFraction, 0.f, 0.95f);
	const float R = FMath::Sqrt(FMath::FRand()) * (Radius - MinR) + MinR;
	return FVector(Center.X + FMath::Cos(Angle) * R, Center.Y + FMath::Sin(Angle) * R, WaterZ + DepthOffset);
}

bool UBFLStatics::IsOverWater(const FVector& Location, const FVector& Center, float Radius)
{
	const FVector2D A(Location.X, Location.Y);
	const FVector2D C(Center.X, Center.Y);
	return FVector2D::DistSquared(A, C) <= FMath::Square(Radius);
}

bool UBFLStatics::PredictWaterHit(FVector Start, FVector Velocity, float GravityZ, float WaterZ, float MaxTime, FVector& OutHit)
{
	const float Dt = 1.f / 60.f;
	FVector Pos = Start;
	FVector Vel = Velocity;
	float Time = 0.f;
	while (Time < MaxTime)
	{
		const FVector Next = Pos + Vel * Dt;
		Vel.Z += GravityZ * Dt;
		if (Pos.Z > WaterZ && Next.Z <= WaterZ)
		{
			const float Alpha = (Pos.Z - WaterZ) / FMath::Max(Pos.Z - Next.Z, KINDA_SMALL_NUMBER);
			OutHit = FMath::Lerp(Pos, Next, Alpha);
			OutHit.Z = WaterZ;
			return true;
		}
		Pos = Next;
		Time += Dt;
	}
	return false;
}

UStaticMesh* UBFLStatics::GetEngineMesh(const TCHAR* Path)
{
	return LoadObject<UStaticMesh>(nullptr, Path);
}

UMaterialInterface* UBFLStatics::GetBasicShapeMaterial()
{
	if (UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
	{
		return Mat;
	}
	return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
}

UMaterialInstanceDynamic* UBFLStatics::MakeTintedMeshMaterial(UObject* Outer, const FLinearColor& Color, bool bUnlit)
{
	UMaterialInterface* Parent = GetBasicShapeMaterial();
	if (!Parent)
	{
		return nullptr;
	}

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Parent, Outer);
	if (!MID)
	{
		return nullptr;
	}

	// BasicShapeMaterial uses "Color"; DefaultMaterial uses "BaseColor". Set both.
	MID->SetVectorParameterValue(TEXT("Color"), Color);
	MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
	MID->SetScalarParameterValue(TEXT("Metallic"), 0.f);
	MID->SetScalarParameterValue(TEXT("Roughness"), bUnlit ? 1.f : 0.55f);
	return MID;
}
