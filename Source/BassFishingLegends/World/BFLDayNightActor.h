#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BFLDayNightActor.generated.h"

class ADirectionalLight;

/** Rotates the sun around a 24-hour clock. Disable from Project Settings if you want a fixed time. */
UCLASS(Blueprintable)
class BASSFISHINGLEGENDS_API ABFLDayNightActor : public AActor
{
	GENERATED_BODY()

public:
	ABFLDayNightActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "BFL|World")
	void SetTimeOfDay(float Hours);

	UFUNCTION(BlueprintPure, Category = "BFL|World")
	float GetTimeOfDay() const { return TimeOfDayHours; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|World")
	bool bEnabled = true;

	/** Hours on a 24-hour clock. 6 = dawn, 12 = noon, 18 = dusk. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|World", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float TimeOfDayHours = 9.5f;

	/** Real minutes for a full day. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|World", meta = (ClampMin = "1.0"))
	float DayLengthMinutes = 12.f;

protected:
	void ApplySun();
	ADirectionalLight* FindSun() const;

	UPROPERTY()
	TWeakObjectPtr<ADirectionalLight> CachedSun;
};
