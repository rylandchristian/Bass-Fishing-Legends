#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BFLPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class BASSFISHINGLEGENDS_API ABFLPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABFLPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Optional designer-authored IMC. If null, the boat builds a runtime default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Input")
	TObjectPtr<UInputMappingContext> OverrideMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Input")
	int32 MappingPriority = 0;

	UFUNCTION(BlueprintCallable, Category = "BFL|Input")
	void AddMappingContext(UInputMappingContext* MappingContext, int32 Priority = 0);
};
