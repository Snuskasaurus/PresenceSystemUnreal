// Copyright Epic Games, Inc. All Rights Reserved.

#include "PresenceSystem.h"

#include "JsonObjectConverter.h"
#include "WebsocketSubsystem.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, PresenceSystem, "PresenceSystem" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UPresenceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (this == nullptr)
	{
		check(false);
		return false;
	}
	if(GetClass()->IsInBlueprint() == true)
	{
		return true;
	}
	return false;
}

void UPresenceSubsystem::InitializePresenceSubsystem()
{
	UPantheonGenericDebugMenuSubsystem* DebugMenuSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	DebugMenuSubsystem->CreateDebugMenu("NetDebugMenu", "VerticalPreset", FVector2d(10, 400), true);
	
	{
		FPanDebugMenuCustomWidgetInfo* LocalPlayerWidgetInfo = DebugMenuSubsystem->AddCustomWidgetToDebugMenu("NetDebugMenu", "LocalPlayer", PresenceWidgetClassPreset);
		LocalPlayerDebugWidget = Cast<UDebugMenu_PresenceFriendWidget>(LocalPlayerWidgetInfo->WidgetPtr);
		LocalPlayerDebugWidget->SetPlayerName(LocalPlayerName);
	}

	{
		const TFunction<void(FString const&)> Lambda = [this](FString const& NewName)->void{ this->SetLocalPlayerName(NewName); };
		DebugMenuSubsystem->AddTextInputToDebugMenu("NetDebugMenu", "DefaultPreset",
			FPanDebugMenuTextInputParameters("OnlineName", false, "Player1"), Lambda);
	}
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
}

void UPresenceSubsystem::TogglePresence()
{
	UWebsocketSubsystem* WebsocketSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	UPantheonGenericDebugMenuSubsystem* DebugMenuSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	if (WebsocketSubsystem->CheckIfConnected() == false)
		return;

	if (CurrentLocalActivity == EOnline_PlayerActivity::DISCONNECTED)
	{
		RequestChangeActivity(EOnline_PlayerActivity::IN_MENU);
		LocalPlayerDebugWidget->SetPlayerActivity(EOnline_PlayerActivity::IN_MENU);
	}
	else
	{
		RequestChangeActivity(EOnline_PlayerActivity::DISCONNECTED);
		LocalPlayerDebugWidget->SetPlayerActivity(EOnline_PlayerActivity::DISCONNECTED);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::SetLocalPlayerName(FString const& InLocalPlayerName)
{
	if (CurrentLocalActivity == EOnline_PlayerActivity::DISCONNECTED)
	{
		LocalPlayerName = InLocalPlayerName;
		LocalPlayerDebugWidget->SetPlayerName(LocalPlayerName);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::RequestChangeActivity(EOnline_PlayerActivity NewActivity)
{
	FString RequestString = "";
	
	{
		FString StringFromStruct = "";
		const FOnline_RequestHeader RequestHeader = FOnline_RequestHeader(LocalPlayerName, EOnline_RequestType::ChangeActivity);
		const bool bSuccessConverting = FJsonObjectConverter::UStructToJsonObjectString(RequestHeader, StringFromStruct);
		if (bSuccessConverting == false)
		{
			return;
		}
		RequestString += StringFromStruct;
	}
	{
		FString StringFromStruct = "";
		const FOnline_Request_ChangeActivity RequestStruct = FOnline_Request_ChangeActivity(NewActivity);
		const bool bSuccessConverting = FJsonObjectConverter::UStructToJsonObjectString(RequestStruct, StringFromStruct);
		if (bSuccessConverting == false)
		{
			return;
		}
		RequestString += StringFromStruct;
	}

	CurrentLocalActivity = NewActivity;
	UWebsocketSubsystem* WebsocketSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	WebsocketSubsystem->TryToSendMessage(RequestString);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::OnFriendActivityChanged()
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::ConnectToWebsocketServer()
{
	UWebsocketSubsystem* WebsocketSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	WebsocketSubsystem->Connect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::DisconnectFromWebsocketServer()
{
	
	UWebsocketSubsystem* WebsocketSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	WebsocketSubsystem->Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::ChangePlayerActivity()
{
	UPresenceSubsystem* PresenceSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPresenceSubsystem>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
