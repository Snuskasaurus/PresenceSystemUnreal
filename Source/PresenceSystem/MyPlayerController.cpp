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
	}

	// Create debug menu for presence
	{
		UPantheonGenericDebugMenuSubsystem* DebugMenuSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
    
		DebugMenuSubsystem->CreateDebugMenu("NetDebugMenu", "VerticalPreset", FVector2d(10, 400), true);

		{
			const TFunction<void()> Lambda = [this]()->void{ this->ConnectToWebsocketServer(); };
			DebugMenuSubsystem->AddButtonToDebugMenu("NetDebugMenu", "DefaultPreset",
				FPanDebugMenuButtonParameters("Connect", false),Lambda);
		}
		{
			const TFunction<void()> Lambda = [this]()->void{ this->TogglePresence(); };
			DebugMenuSubsystem->AddButtonToDebugMenu("NetDebugMenu", "DefaultPreset",
				FPanDebugMenuButtonParameters("Toggle Presence Connection", false),Lambda);
		}
		{
			const TFunction<void()> Lambda = [this]()->void{ this->ChangePlayerActivity(); };
			DebugMenuSubsystem->AddButtonToDebugMenu("NetDebugMenu", "DefaultPreset",
				FPanDebugMenuButtonParameters("Change Presence Activity", false),Lambda);
		}
		{
			const TFunction<void()> Lambda = [this]()->void{ this->DisconnectFromWebsocketServer(); };
			DebugMenuSubsystem->AddButtonToDebugMenu("NetDebugMenu", "DefaultPreset",
				FPanDebugMenuButtonParameters("Disconnect", false),Lambda);
		}
		{
			const TFunction<void(FString const&)> Lambda = [this](FString const& NewName)->void{ this->ChangeOnlinePlayerName(NewName); };
			DebugMenuSubsystem->AddTextInputToDebugMenu("NetDebugMenu", "DefaultPreset",
				FPanDebugMenuTextInputParameters("OnlineName", false, "Player1"), Lambda);
		}
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

void AMyPlayerController::ConnectToWebsocketServer()
{
	UWebsocketSubsystem* WebsocketSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	WebsocketSubsystem->Connect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::DisconnectFromWebsocketServer()
{
	
	UWebsocketSubsystem* WebsocketSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	WebsocketSubsystem->Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::TogglePresence()
{
	UPresenceSubsystem* PresenceSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPresenceSubsystem>();
	PresenceSubsystem->TogglePresence();
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::ChangePlayerActivity()
{
	UPresenceSubsystem* PresenceSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPresenceSubsystem>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AMyPlayerController::ChangeOnlinePlayerName(FString const& InPlayerName)
{
	UPresenceSubsystem* PresenceSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPresenceSubsystem>();
	PresenceSubsystem->SetLocalPlayerName(InPlayerName);
	DEBUG_LOG("New Player name = %s", *InPlayerName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
