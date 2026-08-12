#pragma once

#include "CoreMinimal.h"
#include "BFLTypes.h"
#include "GameFramework/Pawn.h"
#include "BFLBoatPawn.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USceneComponent;
class UCableComponent;
class UBFLFishingComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Small fishing boat. WASD / left stick to drive, mouse / right stick to look,
 * hold left mouse / right trigger to charge a cast.
 */
UCLASS(Blueprintable)
class BASSFISHINGLEGENDS_API ABFLBoatPawn : public APawn
{
	GENERATED_BODY()

public:
	ABFLBoatPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintPure, Category = "BFL|Boat")
	UBFLFishingComponent* GetFishing() const { return Fishing; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Boat")
	TObjectPtr<UBoxComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Boat")
	TObjectPtr<UStaticMeshComponent> HullMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Boat")
	TObjectPtr<UStaticMeshComponent> CabinMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Boat")
	TObjectPtr<UStaticMeshComponent> RodMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Boat")
	TObjectPtr<USceneComponent> RodTip;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Boat")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Boat")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Boat")
	TObjectPtr<UCableComponent> FishingLine;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Boat")
	TObjectPtr<UBFLFishingComponent> Fishing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Boat")
	float MaxSpeed = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Boat")
	float Acceleration = 2.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Boat")
	float TurnSpeed = 85.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Boat")
	float LookYawSpeed = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Boat")
	float LookPitchSpeed = 70.f;

	/** Optional designer IMC. Leave empty to use the runtime keyboard/gamepad map. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Input")
	TObjectPtr<UInputMappingContext> MappingContext;

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void CastStarted(const FInputActionValue& Value);
	void CastCompleted(const FInputActionValue& Value);
	void CancelPressed(const FInputActionValue& Value);

	void EnsureRuntimeInput();
	void AddMappingToController();
	void BuildDefaultMappings(UInputMappingContext* IMC);
	void ApplyPlaceholderMeshes();
	void DrawCastPreview() const;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> CastAction;

	UPROPERTY()
	TObjectPtr<UInputAction> CancelAction;

	FVector2D MoveInput = FVector2D(0.f, 0.f);
	FVector2D LookInput = FVector2D(0.f, 0.f);
	float Throttle = 0.f;
	float CameraYaw = 0.f;
	float CameraPitch = -18.f;
	float BobPhase = 0.f;
};
