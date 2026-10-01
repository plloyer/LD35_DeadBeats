// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#ifndef __LD35_H__
#define __LD35_H__

// This file is the private precompiled header for your game.
// You must include it first in each .cpp file.
//
// Place any includes here that are needed by the majority of your .cpp files

#include "EngineMinimal.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "VisualLogger/VisualLogger.h"

#include "LD35Helper.h"
#include "LD35GameState.h"
#include "LD35Types.h"

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLD35, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCapturePoint, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogLD35PlayerController, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogLD35Character, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogLD35GameMode, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogLD35Projectile, Log, All);

// from https://wiki.unrealengine.com/Log_Macro_with_Netmode_and_Colour
#define NETMODE_WORLD (((GEngine == NULL) || (GWorld == NULL))  ? TEXT("") \
		: (GEngine->GetNetMode(GWorld) == NM_Client)            ? TEXT("[         Client] ") \
		: (GEngine->GetNetMode(GWorld) == NM_ListenServer)      ? TEXT("[   ListenServer] ") \
		: (GEngine->GetNetMode(GWorld) == NM_DedicatedServer)   ? TEXT("[DedicatedServer] ") \
		:                                                         TEXT("[     Standalone] "))

#if _MSC_VER
#define FUNC_NAME    TEXT(__FUNCTION__)
#elif defined(__clang__)
#define FUNC_NAME    ANSI_TO_TCHAR(__FUNCTION__)
#else
#define FUNC_NAME    TEXT("unsupported")
#endif

#define LD35_LOG(CategoryName, Verbosity, Format, ...) \
{ \
	SET_WARN_COLOR( COLOR_CYAN );\
	FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__ ); \
	UE_LOG(CategoryName, Verbosity, TEXT("%s%s() : %s"), NETMODE_WORLD, FUNC_NAME, *Msg);\
	CLEAR_WARN_COLOR();\
}

#define LD35_VLOG(Owner, CategoryName, Verbosity, Format, ...) \
{ \
	SET_WARN_COLOR( COLOR_CYAN );\
	FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__ ); \
	UE_LOG(CategoryName, Verbosity, TEXT("%s%s() : %s"), NETMODE_WORLD, FUNC_NAME, *Msg);\
	UE_VLOG(Owner, CategoryName, Verbosity, TEXT("%s%s() : %s"), NETMODE_WORLD, FUNC_NAME, *Msg);\
	CLEAR_WARN_COLOR();\
}

#endif
