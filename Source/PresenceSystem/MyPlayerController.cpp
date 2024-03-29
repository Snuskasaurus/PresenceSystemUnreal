// Fill out your copyright notice in the Description page of Project Settings.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MyPlayerController.h"

#include "Subsystems/WebsocketSubsystem.h"
#include "DebugMenu/DebugMenu.h"
#include "GameFramework/GameUserSettings.h"
#include "Subsystems/PresenceSystem.h"

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
	}

	// Create debug menu for presence
	{
		UPantheonGenericDebugMenuSubsystem* DebugMenuSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
    
		DebugMenuSubsystem->CreateDebugMenu("NetDebugMenu", "VerticalPreset", FVector2d(10, 400), true);
	
		const TFunction<void()> LambdaConnectButton = [this]()->void{ this->ConnectToServer(); };
		DebugMenuSubsystem->AddButtonToDebugMenu("NetDebugMenu", "Default",
			FPanDebugMenuButtonParameters("Connect", false),LambdaConnectButton);
	
		const TFunction<void()> LambdaActivityButton = [this]()->void{ this->ChangePlayerActivity(); };
		DebugMenuSubsystem->AddButtonToDebugMenu("NetDebugMenu", "Default",
			FPanDebugMenuButtonParameters("Change activity", false),LambdaActivityButton);
	
		const TFunction<void()> LambdaDisconnectButton = [this]()->void{ this->DisconnectFromServer(); };
		DebugMenuSubsystem->AddButtonToDebugMenu("NetDebugMenu", "Default",
			FPanDebugMenuButtonParameters("Disconnect", false),LambdaDisconnectButton);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UPantheonGenericDebugMenuSubsystem* DebugMenuSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	DebugMenuSubsystem->DestroyDebugMenu("NetDebugMenu");
	
	UPantheonGenericDebugMenuSubsystem* PantheonGenericDebugMenuSubsystem = GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	if (PantheonGenericDebugMenuSubsystem)
	{
		PantheonGenericDebugMenuSubsystem->InitializeGenericDebugMenuSubsystem(this);
	}
	
	DebugMenuSubsystem->UninitializeGenericDebugMenuSubsystem(this);
	
	Super::EndPlay(EndPlayReason);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void AMyPlayerController::ConnectToServer()
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::DisconnectFromServer()
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::ChangePlayerActivity()
{
	UPantheonGenericDebugMenuSubsystem* DebugMenuSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	UPresenceSubsystem* PresenceSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPresenceSubsystem>();
    
	DebugMenuSubsystem->AddCustomWidgetToDebugMenu("NetDebugMenu", "LocalPlayer", PresenceSubsystem->PresenceWidgetClassPreset);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
