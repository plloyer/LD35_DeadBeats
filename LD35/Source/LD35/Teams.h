#pragma once

#include "EngineMinimal.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ETeamEnum : uint8
{
	Team1 			UMETA(DisplayName = "Team1"),
	Team2 			UMETA(DisplayName = "Team2"),
	Team3			UMETA(DisplayName = "Team3"),
	Team4			UMETA(DisplayName = "Team4"),
	TeamCount,
	Neutral			UMETA(DisplayName = "Neutral")
};

#define TEAM_COUNT static_cast<int>(ETeamEnum::TeamCount)