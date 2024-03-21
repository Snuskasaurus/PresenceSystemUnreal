// Fill out your copyright notice in the Description page of Project Settings.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MyPlayerController.h"

#include "PresenceSubsystem.h"
#include "GameFramework/GameUserSettings.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);
	GEngine->GameUserSettings->SetScreenResolution(FIntPoint(720, 600));
	GEngine->GameUserSettings->ApplySettings(false);

	SetActorTickInterval(0.5f);
	
	UPresenceSubsystem* presenceSubsystem = GetGameInstance()->GetSubsystem<UPresenceSubsystem>();
	presenceSubsystem->ConnectToServer();
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UPresenceSubsystem* presenceSubsystem = GetGameInstance()->GetSubsystem<UPresenceSubsystem>();
	presenceSubsystem->DisconnectFromServer();
	
	Super::EndPlay(EndPlayReason);
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	
	UPresenceSubsystem* presenceSubsystem = GetGameInstance()->GetSubsystem<UPresenceSubsystem>();
	presenceSubsystem->SendMessage();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
