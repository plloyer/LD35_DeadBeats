// Fill out your copyright notice in the Description page of Project Settings.

#include "LD35.h"
#include "GameInputComponent.h"

void UGameInputComponent::Reset()
{
	m_Movement = FVector::ZeroVector;
	m_WantsToShoot = false;
	m_WantsToDeflect = false;
	m_Follow = false;
	m_Return = false;
	m_StartGame = false;
}

bool UGameInputComponent::WantsToShoot(bool consume)
{
	bool value = m_WantsToShoot;
	if (m_ConsumeInputs || consume)
	{
		m_WantsToShoot = false;
	}

	return value;
}

bool UGameInputComponent::WantsToPowerShoot(bool consume)
{
	bool value = m_WantsToPowerShoot;
	if (m_ConsumeInputs || consume)
	{
		m_WantsToPowerShoot = false;
	}

	return value;
}

bool UGameInputComponent::WantsToDeflect()
{
	bool value = m_WantsToDeflect;
	if (m_ConsumeInputs)
	{
		m_WantsToDeflect = false;
	}

	return value;
}

bool UGameInputComponent::WantsToStartGame()
{
	bool value = m_StartGame;
	if (m_ConsumeInputs)
	{
		m_StartGame = false;
	}

	return value;
}