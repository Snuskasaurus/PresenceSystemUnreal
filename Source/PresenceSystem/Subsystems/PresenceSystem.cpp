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

void UPresenceSubsystem::TogglePresence()
{
	UWebsocketSubsystem* WebsocketSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	UPantheonGenericDebugMenuSubsystem* DebugMenuSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	if (WebsocketSubsystem->CheckIfConnected() == false)
		return;
		
	if (CurrentLocalActivity == EOnline_PlayerActivity::DISCONNECTED)
	{
		RequestChangeActivity(EOnline_PlayerActivity::IN_MENU);
		LocalPlayerWidgetInfo = DebugMenuSubsystem->AddCustomWidgetToDebugMenu("NetDebugMenu", "LocalPlayer", PresenceWidgetClassPreset);
	}
	else
	{
		RequestChangeActivity(EOnline_PlayerActivity::DISCONNECTED);
		
		DebugMenuSubsystem->RemoveCustomWidgetFromDebugMenu("NetDebugMenu", "LocalPlayer");
		LocalPlayerWidgetInfo = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::SetLocalPlayerName(FString const& InLocalPlayerName)
{
	if (CurrentLocalActivity == EOnline_PlayerActivity::DISCONNECTED)
	{
		LocalPlayerName = InLocalPlayerName;
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
