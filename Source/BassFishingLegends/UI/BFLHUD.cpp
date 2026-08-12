#include "UI/BFLHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Fish/BFLFishActor.h"
#include "Fishing/BFLFishingComponent.h"
#include "Game/BFLGameMode.h"
#include "Game/BFLStatics.h"
#include "Player/BFLBoatPawn.h"

ABFLHUD::ABFLHUD()
{
}

void ABFLHUD::ShowNotification(const FText& Message, float Duration)
{
	Notification = Message;
	NotificationTimeLeft = Duration;
}

void ABFLHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float Dt = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
	NotificationTimeLeft = FMath::Max(0.f, NotificationTimeLeft - Dt);

	const ABFLGameMode* GM = UBFLStatics::GetBFLGameMode(this);
	const ABFLBoatPawn* Boat = Cast<ABFLBoatPawn>(GetOwningPawn());
	const UBFLFishingComponent* Fishing = Boat ? Boat->GetFishing() : nullptr;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	// Score panel — top left.
	DrawRect(Panel, 24.f, 20.f, 280.f, 78.f);
	const int32 Score = GM ? GM->GetScore() : 0;
	const int32 Caught = GM ? GM->GetFishCaught() : 0;
	DrawShadowText(36.f, 26.f, FString::Printf(TEXT("SCORE  %d"), Score), Accent, 1.35f);
	DrawShadowText(36.f, 58.f, FString::Printf(TEXT("Fish caught  %d"), Caught), FLinearColor::White, 1.0f);

	// Controls — top right.
	DrawShadowText(W - 430.f, 22.f, TEXT("WASD / Stick  —  boat"), FLinearColor(0.85f, 0.9f, 1.f, 0.8f), 0.9f);
	DrawShadowText(W - 430.f, 44.f, TEXT("Hold LMB / RT  —  cast  /  reel"), FLinearColor(0.85f, 0.9f, 1.f, 0.8f), 0.9f);
	DrawShadowText(W - 430.f, 66.f, TEXT("RMB / LT / F  —  cancel"), FLinearColor(0.85f, 0.9f, 1.f, 0.8f), 0.9f);
	DrawShadowText(W - 430.f, 88.f, TEXT("Mouse / Right stick  —  look"), FLinearColor(0.85f, 0.9f, 1.f, 0.8f), 0.9f);

	if (Fishing)
	{
		const EFishingState State = Fishing->GetState();

		if (State == EFishingState::ChargingCast)
		{
			const float BarW = 360.f;
			DrawBar((W - BarW) * 0.5f, H - 120.f, BarW, 22.f, Fishing->GetCastPower(),
				FLinearColor(0.05f, 0.05f, 0.05f, 0.75f),
				FLinearColor(0.95f, 0.78f, 0.15f, 1.f),
				TEXT("CAST POWER"));
		}

		if (State == EFishingState::Waiting)
		{
			DrawShadowText(W * 0.5f - 140.f, H - 110.f, TEXT("Waiting for a bite..."), FLinearColor(0.7f, 0.85f, 1.f), 1.1f);
		}

		if (State == EFishingState::Casting)
		{
			DrawShadowText(W * 0.5f - 80.f, H - 110.f, TEXT("Casting..."), FLinearColor(0.85f, 0.9f, 1.f), 1.1f);
		}

		if (State == EFishingState::Fighting)
		{
			DrawShadowText(W * 0.5f - 70.f, H * 0.18f, TEXT("FISH ON!"), FLinearColor(1.f, 0.82f, 0.15f), 2.1f);

			if (const ABFLFishActor* Fish = Fishing->GetHookedFish())
			{
				const FString Info = FString::Printf(TEXT("%s   %.1f lb"),
					*Fish->GetSpeciesDef().DisplayName.ToString(),
					Fish->GetWeightLbs());
				DrawShadowText(W * 0.5f - 110.f, H * 0.18f + 48.f, Info, FLinearColor::White, 1.15f);
			}

			const float Tension = Fishing->GetTension();
			FLinearColor TensionColor = FLinearColor(0.2f, 0.8f, 0.25f);
			if (Tension > 0.55f)
			{
				TensionColor = FLinearColor(0.95f, 0.75f, 0.12f);
			}
			if (Tension > 0.78f)
			{
				TensionColor = FLinearColor(0.9f, 0.12f, 0.1f);
			}

			const float BarW = 520.f;
			DrawBar((W - BarW) * 0.5f, H - 160.f, BarW, 28.f, Tension,
				FLinearColor(0.05f, 0.05f, 0.05f, 0.8f), TensionColor, TEXT("TENSION  (ease off in the red)"));

			DrawBar((W - BarW) * 0.5f, H - 112.f, BarW, 18.f, Fishing->GetReelProgress(),
				FLinearColor(0.05f, 0.05f, 0.05f, 0.8f),
				FLinearColor(0.25f, 0.65f, 0.95f, 1.f),
				TEXT("REEL"));

			const FString Hint = Fishing->IsReeling()
				? TEXT("Reeling... release if tension spikes")
				: TEXT("Hold LMB / RT to reel");
			DrawShadowText((W - BarW) * 0.5f, H - 80.f, Hint, FLinearColor(0.9f, 0.9f, 0.9f), 0.95f);
		}

		if (State == EFishingState::Failed)
		{
			DrawShadowText(W * 0.5f - 90.f, H * 0.28f, TEXT("LINE SNAPPED"), FLinearColor(0.95f, 0.2f, 0.18f), 1.8f);
		}
	}

	if (NotificationTimeLeft > 0.f && !Notification.IsEmpty())
	{
		const float Alpha = FMath::Clamp(NotificationTimeLeft / 0.4f, 0.f, 1.f);
		DrawShadowText(W * 0.5f - 220.f, H * 0.36f, Notification.ToString(), FLinearColor(1.f, 0.95f, 0.55f, Alpha), 1.45f);
	}

	// Last few catches.
	if (GM && GM->GetCatchLog().Num() > 0)
	{
		DrawShadowText(28.f, H - 160.f, TEXT("Catch log"), FLinearColor(0.75f, 0.8f, 0.7f), 0.9f);
		const TArray<FBFLCatchRecord>& Log = GM->GetCatchLog();
		const int32 Start = FMath::Max(0, Log.Num() - 4);
		int32 Row = 0;
		for (int32 i = Log.Num() - 1; i >= Start; --i, ++Row)
		{
			const FBFLCatchRecord& R = Log[i];
			DrawShadowText(28.f, H - 138.f + Row * 20.f,
				FString::Printf(TEXT("%s   %.1f lb   +%d"), *R.DisplayName.ToString(), R.WeightLbs, R.ScoreAwarded),
				FLinearColor(0.85f, 0.9f, 0.8f, 0.85f),
				0.85f);
		}
	}
}

void ABFLHUD::DrawBar(float X, float Y, float W, float H, float Fill01, FLinearColor Back, FLinearColor Fill, const FString& Label)
{
	DrawRect(Back, X, Y, W, H);
	const float Inner = FMath::Clamp(Fill01, 0.f, 1.f) * (W - 4.f);
	DrawRect(Fill, X + 2.f, Y + 2.f, Inner, H - 4.f);
	DrawShadowText(X, Y - 22.f, Label, FLinearColor::White, 0.9f);
}

void ABFLHUD::DrawShadowText(float X, float Y, const FString& Text, FLinearColor Color, float Scale)
{
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	DrawText(Text, FLinearColor(0.f, 0.f, 0.f, Color.A * 0.7f), X + 2.f, Y + 2.f, Font, Scale, false);
	DrawText(Text, Color, X, Y, Font, Scale, false);
}
