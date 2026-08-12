#include "Game/BFLGameSettings.h"

UBFLGameSettings::UBFLGameSettings()
{
	CategoryName = TEXT("Game");
}

const UBFLGameSettings* UBFLGameSettings::Get()
{
	return GetDefault<UBFLGameSettings>();
}
