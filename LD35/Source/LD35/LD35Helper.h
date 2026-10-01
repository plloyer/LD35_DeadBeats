// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Teams.h"
#include "LD35Helper.generated.h"

class ALD35Character;
class ALD35SpawnLocation;
class ACapturePointController;

UCLASS()
class LD35_API ULD35Helper : public UObject
{
	GENERATED_BODY()
	
public:

	// Characters
	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static TArray<ALD35Character*> InSameTeam(const TArray<ALD35Character*>& Range, const ALD35Character* Actor, bool bInverse = false);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static TArray<ALD35Character*> Alive(const TArray<ALD35Character*>& Range, bool bInverse = false);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static TArray<ALD35Character*> InArea(const TArray<ALD35Character*>& Range, const FVector& Location, const float Radius, bool bInverse = false);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static TArray<ALD35Character*> PrioritizeObjectsOfType(const TArray<ALD35Character*>& Range, TSubclassOf<ALD35Character> PriorityClass);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static TArray<ALD35Character*> SortByDistance(const TArray<ALD35Character*>& Range, const FVector& Location, bool bIsAscending = true);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static ALD35Character* SelectFirstWithLineOfSight(const TArray<ALD35Character*>& Range, const ALD35Character* Reference);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static ALD35Character* SelectClosest(const TArray<ALD35Character*>& Range, const FVector& Location);

	// Altars
	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static TArray<ACapturePointController*> NotCaptured(const TArray<ACapturePointController*>& Range, const ALD35Character* Character);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static ACapturePointController* SelectClosestAltar(const TArray<ACapturePointController*>& Range, const FVector& Location);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static ACapturePointController* SelectRandomAltar(const TArray<ACapturePointController*>& Range);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static bool IsTeamEmpty(ETeamEnum Team);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static bool IsTeamReady(ETeamEnum Team);

	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static ETeamEnum GetFirstEmptyTeam(UWorld* World);
	
	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static ACapturePointController* GetFirstCapturePointOfTeam(UWorld* World, ETeamEnum Team);

	// Spawn points
	UFUNCTION(BlueprintPure, Category = "Static Helpers")
	static ALD35SpawnLocation* GetFirstSpawnLocationOfTeam(UWorld* World, ETeamEnum Team);
	

	// Generic
	static class ALD35GameState& GetGameState(UWorld* World);

	static ACameraActor* GetCameraByName(const FString& CameraName);

	template<typename T>
	static FString EnumToString(const FString& enumName, const T value, const FString& defaultValue)
	{
		UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, *enumName, true);
		return pEnum
			? ExpandEnumString(pEnum->GetNameByIndex(static_cast<uint8>(value)).ToString(), enumName)
			: defaultValue;
	}

	static FString ExpandEnumString(const FString& name, const FString& enumName)
	{
		FString expanded(name);
		FString spaceLetter("");
		FString spaceNumber("");
		FString search("");
		expanded.ReplaceInline(*enumName, TEXT(""), ESearchCase::CaseSensitive);
		expanded.ReplaceInline(TEXT("::"), TEXT(""), ESearchCase::CaseSensitive);
		for (TCHAR letter = 'A'; letter <= 'Z'; ++letter)
		{
			search = FString::Printf(TEXT("%c"), letter);
			spaceLetter = FString::Printf(TEXT(" %c"), letter);
			expanded.ReplaceInline(*search, *spaceLetter, ESearchCase::CaseSensitive);
		}
		for (TCHAR number = '0'; number <= '9'; ++number)
		{
			search = FString::Printf(TEXT("%c"), number);
			spaceNumber = FString::Printf(TEXT(" %c"), number);
			expanded.ReplaceInline(*search, *spaceNumber, ESearchCase::CaseSensitive);
		}
		expanded.ReplaceInline(TEXT("_"), TEXT(" -"), ESearchCase::CaseSensitive);
		expanded = expanded.RightChop(1).Trim().TrimTrailing();
		return expanded;
	}
};
