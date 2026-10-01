

#pragma once

#include "Engine/GameInstance.h"
#include "LD35GameInstance.generated.h"

UENUM()
enum class EApplicationState : uint8
{
	MainMenu,
	Game
};

/**
 * 
 */
UCLASS()
class LD35_API ULD35GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = LD35GameInstance)
	static ULD35GameInstance* GetGameInstance()	{ return Cast<ULD35GameInstance>(GWorld->GetGameInstance()); }

	UFUNCTION(BlueprintPure, Category=LD35GameInstance)
	EApplicationState GetApplicationState() const { return m_State; }
	
	UFUNCTION()
	void SetApplicationState(EApplicationState State) { m_State = State; }
	
private:
	EApplicationState m_State;
};

inline ULD35GameInstance* GetLD35GameInstance()
{
	return ULD35GameInstance::GetGameInstance();
}