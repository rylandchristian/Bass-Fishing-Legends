#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BFLBaitActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ABFLFishActor;

/**
 * Bobber / bait that sits on the water after a cast.
 * Fish query this actor (via the GameMode) and swim toward it.
 */
UCLASS(Blueprintable)
class BASSFISHINGLEGENDS_API ABFLBaitActor : public AActor
{
	GENERATED_BODY()

public:
	ABFLBaitActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "BFL|Bait")
	float GetAttractRadius() const { return AttractRadius; }

	UFUNCTION(BlueprintPure, Category = "BFL|Bait")
	bool IsAvailable() const { return bAvailable && !HookedFish; }

	UFUNCTION(BlueprintCallable, Category = "BFL|Bait")
	void SetAvailable(bool bInAvailable) { bAvailable = bInAvailable; }

	UFUNCTION(BlueprintCallable, Category = "BFL|Bait")
	void HookFish(ABFLFishActor* Fish);

	UFUNCTION(BlueprintCallable, Category = "BFL|Bait")
	void ReleaseHook();

	UFUNCTION(BlueprintPure, Category = "BFL|Bait")
	ABFLFishActor* GetHookedFish() const { return HookedFish; }

	UFUNCTION(BlueprintPure, Category = "BFL|Bait")
	FVector GetBobberLocation() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Bait")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Bait")
	TObjectPtr<UStaticMeshComponent> BobberMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Bait")
	TObjectPtr<UStaticMeshComponent> HookMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Bait")
	float AttractRadius = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Bait")
	float Lifetime = 45.f;

protected:
	UPROPERTY()
	TObjectPtr<ABFLFishActor> HookedFish;

	bool bAvailable = true;
	float Age = 0.f;
	float BobPhase = 0.f;
};
