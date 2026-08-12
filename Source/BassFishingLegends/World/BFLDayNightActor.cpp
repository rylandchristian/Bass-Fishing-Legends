#include "World/BFLDayNightActor.h"

#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "EngineUtils.h"
#include "Game/BFLGameSettings.h"

ABFLDayNightActor::ABFLDayNightActor()
{
	PrimaryActorTick.bCanEverTick = true;

	if (const UBFLGameSettings* Settings = UBFLGameSettings::Get())
	{
		bEnabled = Settings->bEnableDayNight;
		DayLengthMinutes = Settings->DayLengthMinutes;
	}
}

void ABFLDayNightActor::BeginPlay()
{
	Super::BeginPlay();
	CachedSun = FindSun();
	ApplySun();
}

void ABFLDayNightActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bEnabled)
	{
		return;
	}

	const float DaySeconds = FMath::Max(DayLengthMinutes, 1.f) * 60.f;
	TimeOfDayHours = FMath::Fmod(TimeOfDayHours + (24.f / DaySeconds) * DeltaSeconds, 24.f);
	ApplySun();
}

void ABFLDayNightActor::SetTimeOfDay(float Hours)
{
	TimeOfDayHours = FMath::Fmod(Hours + 24.f, 24.f);
	ApplySun();
}

void ABFLDayNightActor::ApplySun()
{
	ADirectionalLight* Sun = CachedSun.Get();
	if (!Sun)
	{
		CachedSun = FindSun();
		Sun = CachedSun.Get();
	}
	if (!Sun)
	{
		return;
	}

	// 6:00 = sunrise on the horizon, 12:00 = overhead.
	const float Elev = (TimeOfDayHours - 6.f) * 15.f;
	const float Azimuth = 30.f + TimeOfDayHours * 8.f;
	Sun->SetActorRotation(FRotator(-Elev, Azimuth, 0.f));

	UDirectionalLightComponent* Light = Cast<UDirectionalLightComponent>(Sun->GetLightComponent());
	if (!Light)
	{
		return;
	}

	const float Dayness = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(FMath::Clamp(Elev, 0.f, 180.f))), 0.f, 1.f);
	const float Intensity = FMath::Lerp(0.35f, 10.f, Dayness);
	Light->SetIntensity(Intensity);

	const FLinearColor Noon(1.f, 0.97f, 0.9f);
	const FLinearColor Dusk(1.f, 0.55f, 0.28f);
	const FLinearColor Night(0.25f, 0.32f, 0.55f);
	FLinearColor Color = FMath::Lerp(Dusk, Noon, Dayness);
	if (Elev < 0.f)
	{
		Color = FMath::Lerp(Dusk, Night, FMath::Clamp((-Elev) / 40.f, 0.f, 1.f));
	}
	Light->SetLightColor(Color);
}

ADirectionalLight* ABFLDayNightActor::FindSun() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ADirectionalLight* Fallback = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		if (It->ActorHasTag(FName(TEXT("BFL_Sun"))))
		{
			return *It;
		}
		if (!Fallback)
		{
			Fallback = *It;
		}
	}
	return Fallback;
}
