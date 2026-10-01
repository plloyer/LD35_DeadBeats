// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "LD35.h"

#include "GameFramework/Actor.h"
#include "LD35Helper.h"
#include "LD35Character.h"
#include "LD35PlayerController.h"
#include "LD35PlayerState.h"
#include "LD35GameState.h"
#include "LD35SpawnLocation.h"
#include "CapturePointController.h"

TArray<ALD35Character*> ULD35Helper::InSameTeam(const TArray<ALD35Character*>& Range, const ALD35Character* Character, bool bInverse /*= false*/)
{
	TArray<ALD35Character*> Output;
	for (auto* character : Range)
	{
		if ((character->GetTeamId() == Character->GetTeamId()) ^ bInverse)
		{
			Output.Add(character);
		}
	}
	return Output;
}

TArray<ALD35Character*> ULD35Helper::Alive(const TArray<ALD35Character*>& Range, bool bInverse /*= false*/)
{
	TArray<ALD35Character*> Output;
	for (const auto Character : Range)
	{
		if ((IsValid(Character) && !Character->IsDead() && !Character->IsGhost()) != bInverse)
		{
			Output.Add(Character);
		}
	}
	return Output;
}

TArray<ALD35Character*> ULD35Helper::InArea(const TArray<ALD35Character*>& Range, const FVector& Location, const float Radius, bool bInverse /*= false*/)
{
	const auto Distance2 = FMath::Square(Radius);
	return Range.FilterByPredicate([&](const AActor* Actor)
	{
		return (FVector::DistSquared(Location, Actor->GetActorLocation()) < Distance2) != bInverse;
	});
}

TArray<ALD35Character*> ULD35Helper::PrioritizeObjectsOfType(const TArray<ALD35Character*>& Range, TSubclassOf<ALD35Character> PriorityClass)
{
	if (!PriorityClass)
		return Range;

	auto Output = Range;
	// The sorting predicate demands references and not pointers.
	Output.StableSort([&](const ALD35Character& Char1, const ALD35Character& Char2)
	{		
		return Char1.IsA(*PriorityClass) && !Char2.IsA(*PriorityClass);
	});
	return Output;
}

TArray<ALD35Character*> ULD35Helper::SortByDistance(const TArray<ALD35Character*>& Range, const FVector& Location, bool bIsAscending /*= true*/)
{
	auto Output = Range;
	// The sorting predicate demands references and not pointers.
	Output.StableSort([&](const ALD35Character& Char1, const ALD35Character& Char2)
	{
		return FVector::DistSquared(Char1.GetActorLocation(), Location) < FVector::DistSquared(Char2.GetActorLocation(), Location) == bIsAscending;
	});
	return Output;
}

ALD35Character* ULD35Helper::SelectFirstWithLineOfSight(const TArray<ALD35Character*>& Range, const ALD35Character* Reference)
{
	if (Reference)
	{
		FVector HitLocation;
		for (const auto Character : Range)
		{
			if (!UNavigationSystem::NavigationRaycast(Reference->GetWorld(), Reference->GetActorLocation(), Character->GetActorLocation(), HitLocation))
			{
				return Character;
			}
		}
	}
	return nullptr;
}

ALD35Character* ULD35Helper::SelectClosest(const TArray<ALD35Character*>& Range, const FVector& Location)
{
	if (Range.Num() <= 0)
	{
		return nullptr;
	}
	else
	{
		auto Output = Range[0];
		auto Distance2 = FVector::DistSquared(Output->GetActorLocation(), Location);
		for (const auto Character : Range)
		{
			const auto Dist2 = FVector::DistSquared(Character->GetActorLocation(), Location);
			if (Dist2 < Distance2)
			{
				Distance2 = Dist2;
				Output = Character;
			}
		}
		return Output;
	}
}

TArray<ACapturePointController*> ULD35Helper::NotCaptured(const TArray<ACapturePointController*>& Range, const ALD35Character* Character)
{
	TArray<ACapturePointController*> Output;
	for (const auto Altar : Range)
	{
		if (Altar->GetOwnerTeam() != Character->GetTeamId())
		{
			Output.Add(Altar);
		}
	}
	return Output;
}

ACapturePointController* ULD35Helper::SelectClosestAltar(const TArray<ACapturePointController*>& Range, const FVector& Location)
{
	if (Range.Num() <= 0)
	{
		return nullptr;
	}
	else
	{
		auto Output = Range[0];
		auto Distance2 = FVector::DistSquared(Output->GetActorLocation(), Location);
		for (const auto Character : Range)
		{
			const auto Dist2 = FVector::DistSquared(Character->GetActorLocation(), Location);
			if (Dist2 < Distance2)
			{
				Distance2 = Dist2;
				Output = Character;
			}
		}
		return Output;
	}
}

ACapturePointController* ULD35Helper::SelectRandomAltar(const TArray<ACapturePointController*>& Range)
{
	if (Range.Num() == 0) return nullptr;
	const auto RandomIndex = FMath::FloorToInt(FMath::Rand() * Range.Num()) % Range.Num();
	return Range[RandomIndex];
}


ALD35GameState& ULD35Helper::GetGameState(UWorld* World)
{
	ALD35GameState* gameState = Cast<ALD35GameState>(World->GetGameState());
	check(gameState);
	return *gameState;
}

ACameraActor* ULD35Helper::GetCameraByName(const FString& CameraName)
{
	for (TObjectIterator<ACameraActor> Itr; Itr; ++Itr)
	{
		if ((*Itr)->GetName() == CameraName)
		{
			return *Itr;
		}
	}

	return nullptr;
}

bool ULD35Helper::IsTeamEmpty(ETeamEnum Team)
{
	for (TObjectIterator<ALD35PlayerState> Itr; Itr; ++Itr)
	//for (TObjectIterator<ALD35Character> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		ALD35PlayerState* playerState = Cast<ALD35PlayerState>(*Itr);
		ALD35Character* character = IsValid(playerState) ? playerState->Character : nullptr;
		if (IsValid(character) && character->GetTeamId() == Team)
		{
			return false;
		}
	}

	return true;
}

bool ULD35Helper::IsTeamReady(ETeamEnum Team)
{
	for (TObjectIterator<ALD35PlayerController> Itr; Itr; ++Itr)
	{
		ALD35PlayerController* player = *Itr;
		ALD35Character* character = IsValid(player) ? Cast<ALD35Character>(player->GetPawn()) : nullptr;
		if (IsValid(character) && character->GetTeamId() == Team && player->MyPlayerState == ELD35PlayerState::Ready)
		{
			return true;
		}
	}

	return false;
}

ETeamEnum ULD35Helper::GetFirstEmptyTeam(UWorld* World)
{
	TArray<ETeamEnum> useTeam;
	for (TActorIterator<ALD35Character> Itr(World); Itr; ++Itr)
	{
		ALD35Character* character = *Itr;
		if (IsValid(character))
		{
			useTeam.AddUnique(character->GetTeamId());
		}
	}

	for (int i = 0; i < static_cast<int>(ETeamEnum::TeamCount); ++i)
	{
		ETeamEnum currentTeam = static_cast<ETeamEnum>(i);
		if (!useTeam.Contains(currentTeam))
		{
			return currentTeam;
		}
	}

	return ETeamEnum::Neutral;
}

ACapturePointController* ULD35Helper::GetFirstCapturePointOfTeam(UWorld* World, ETeamEnum Team)
{
	for (TActorIterator<ACapturePointController> ActorItr(World); ActorItr; ++ActorItr)
	{
		ACapturePointController* capture = *ActorItr;
		if (IsValid(capture) && capture->GetOwnerTeam() == Team)
		{
			return capture;
		}
	}

	return nullptr;
}

ALD35SpawnLocation* ULD35Helper::GetFirstSpawnLocationOfTeam(UWorld* World, ETeamEnum Team)
{
	for (TActorIterator<ALD35SpawnLocation> ActorItr(World); ActorItr; ++ActorItr)
	{
		ALD35SpawnLocation* capture = *ActorItr;
		if (IsValid(capture) && capture->OwnerTeam == Team)
		{
			return capture;
		}
	}

	return nullptr;
}
