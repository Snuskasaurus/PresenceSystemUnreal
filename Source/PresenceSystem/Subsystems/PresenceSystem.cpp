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
		CurrentLocalActivity = EOnline_PlayerActivity::IN_MENU;
		RequestChangeActivity(EOnline_PlayerActivity::IN_MENU);
		LocalPlayerDebugWidget->SetPlayerActivity(EOnline_PlayerActivity::IN_MENU);
	}
	else
	{
		CurrentLocalActivity = EOnline_PlayerActivity::DISCONNECTED;
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
	TSharedRef<FJsonObject> RequestJsonObject = MakeShared<FJsonObject>();
	{
		FString content = UEnum::GetValueAsString(NewActivity);
		FOnline_Request Request = FOnline_Request(LocalPlayerName, EOnline_RequestType::ChangeActivity, content);

		const bool bSuccessConverting = FJsonObjectConverter::UStructToJsonObject(FOnline_Request::StaticStruct(), &Request, RequestJsonObject);
		//const bool bSuccessConverting = FJsonObjectConverter::UStructToJsonObjectString(Request, RequestString);
		if (bSuccessConverting == false)
		{
			return;
		}
	}

#if 0 // USE_NESTED_JSON
	TSharedRef<FJsonObject> ContentJsonObject = MakeShared<FJsonObject>();
	{
		const FOnline_RequestContent_ChangeActivity RequestContent = FOnline_RequestContent_ChangeActivity(NewActivity);
		const bool bSuccessConverting = FJsonObjectConverter::UStructToJsonObject(FOnline_RequestContent_ChangeActivity::StaticStruct(), &RequestContent, ContentJsonObject);
		if (bSuccessConverting == false)
		{
			return;
		}
		RequestJsonObject->SetObjectField("Content", ContentJsonObject);
	}
#endif
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RequestJsonObject, Writer);
	
	UWebsocketSubsystem* WebsocketSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWebsocketSubsystem>();
	WebsocketSubsystem->TryToSendMessage(OutputString);
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
