#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BFLCastProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UBFLFishingComponent;

/** Simple ballistic lure. Hits when it crosses the water plane. */
UCLASS()
class BASSFISHINGLEGENDS_API ABFLCastProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABFLCastProjectile();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	void Launch(const FVector& InVelocity, float InWaterZ, UBFLFishingComponent* InOwnerFishing);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Cast")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Cast")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Cast")
	float GravityZ = -2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Cast")
	float MaxLifetime = 6.f;

protected:
	FVector Velocity = FVector::ZeroVector;
	float WaterZ = 0.f;
	float Age = 0.f;
	bool bHasHit = false;

	UPROPERTY()
	TWeakObjectPtr<UBFLFishingComponent> OwnerFishing;

	void HitWater(const FVector& HitLocation);
};
