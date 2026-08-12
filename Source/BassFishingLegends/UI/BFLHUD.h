#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BFLHUD.generated.h"

class UBFLFishingComponent;
class ABFLGameMode;

/**
 * Immediate-mode HUD. No UMG asset is required — the game is playable after compile.
 * Subclass in Blueprint later if you want a designed widget.
 */
UCLASS()
class BASSFISHINGLEGENDS_API ABFLHUD : public AHUD
{
	GENERATED_BODY()

public:
	ABFLHUD();

	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category = "BFL|UI")
	void ShowNotification(const FText& Message, float Duration = 2.5f);

protected:
	void DrawBar(float X, float Y, float W, float H, float Fill01, FLinearColor Back, FLinearColor Fill, const FString& Label);
	void DrawShadowText(float X, float Y, const FString& Text, FLinearColor Color, float Scale = 1.f);

	FText Notification;
	float NotificationTimeLeft = 0.f;

	UPROPERTY(EditAnywhere, Category = "BFL|UI")
	FLinearColor Panel = FLinearColor(0.02f, 0.04f, 0.06f, 0.62f);

	UPROPERTY(EditAnywhere, Category = "BFL|UI")
	FLinearColor Accent = FLinearColor(0.95f, 0.82f, 0.25f, 1.f);
};
