// Fill out your copyright notice in the Description page of Project Settings.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MyPlayerController.h"

#include "Subsystems/WebsocketSubsystem.h"
#include "DebugMenu/DebugMenu.h"
#include "GameFramework/GameUserSettings.h"
#include "Subsystems/PresenceSystem.h"

#define DEBUG_LOG(Text, ...)			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT(Text), ##__VA_ARGS__));

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Init everything
	{
		SetShowMouseCursor(true);
		// FInputModeGameAndUI InputMode;
		// SetInputMode(InputMode);
		GEngine->GameUserSettings->SetScreenResolution(FIntPoint(720, 600));
		GEngine->GameUserSettings->ApplySettings(false);
		SetActorTickInterval(0.5f);

		UPantheonGenericDebugMenuSubsystem* DebugMenuSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
		DebugMenuSubsystem->InitializeGenericDebugMenuSubsystem(this);
	
		UPresenceSubsystem* PresenceSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPresenceSubsystem>();
		PresenceSubsystem->InitializePresenceSubsystem();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UPantheonGenericDebugMenuSubsystem* DebugMenuSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	DebugMenuSubsystem->DestroyDebugMenu("NetDebugMenu");
	
	DebugMenuSubsystem->UninitializeGenericDebugMenuSubsystem(this);
	
	Super::EndPlay(EndPlayReason);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
