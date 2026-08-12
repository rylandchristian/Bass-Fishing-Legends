#include "Game/BFLPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

ABFLPlayerController::ABFLPlayerController()
{
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void ABFLPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (OverrideMappingContext)
	{
		AddMappingContext(OverrideMappingContext, MappingPriority);
	}
}

void ABFLPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void ABFLPlayerController::AddMappingContext(UInputMappingContext* MappingContext, int32 Priority)
{
	if (!MappingContext)
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(MappingContext, Priority);
	}
}
