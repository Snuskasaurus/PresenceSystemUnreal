// Fill out your copyright notice in the Description page of Project Settings.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MyPlayerController.h"

#include "WebsocketSubsystem.h"
#include "DebugMenu/DebugMenu.h"
#include "GameFramework/GameUserSettings.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(true);
	// FInputModeGameAndUI InputMode;
	// SetInputMode(InputMode);
	GEngine->GameUserSettings->SetScreenResolution(FIntPoint(720, 600));
	GEngine->GameUserSettings->ApplySettings(false);

	SetActorTickInterval(0.5f);
	
	UWebsocketSubsystem* presenceSubsystem = GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	presenceSubsystem->CreateDebugMenu();
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWebsocketSubsystem* presenceSubsystem = GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	presenceSubsystem->DestroyDebugMenu();
	
	UPantheonGenericDebugMenuSubsystem* PantheonGenericDebugMenuSubsystem = GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	if (PantheonGenericDebugMenuSubsystem)
	{
		PantheonGenericDebugMenuSubsystem->InitializeGenericDebugMenuSubsystem(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
		
	UPantheonGenericDebugMenuSubsystem* PantheonGenericDebugMenuSubsystem = GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	if (PantheonGenericDebugMenuSubsystem)
	{
		PantheonGenericDebugMenuSubsystem->UninitializeGenericDebugMenuSubsystem(this);
	}
	
	UWebsocketSubsystem* presenceSubsystem = GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	presenceSubsystem->TickWebsocketSubsystem();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
