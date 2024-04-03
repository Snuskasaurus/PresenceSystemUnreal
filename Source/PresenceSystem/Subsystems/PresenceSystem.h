// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PresenceSystem/DebugMenu/DebugMenu.h"

#include "PresenceSystem.generated.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Client connect to server
	   //  - Server subscribe this player to all friends events
	   //  - Server send presence data of every friends to the connected client

// Client Disconnect from server
	   //  - Server disconnect and notify every friends
	   //  - Server un-subscribe
	   //  - Stop heartbeat

// Client Change his activity
	   //  - Server notify every friends

// Server send heartbeat
	   //  - Client respond to it
	   //      - if not, server disconnect and notify every friends

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum EOnline_PlayerActivity
{
	DISCONNECTED,
	IN_MENU,
	PLAYING,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum EOnline_RequestType
{
	FriendList,
	ChangeActivity,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FOnline_Request
{
	GENERATED_BODY()
	FOnline_Request() = default;
	FOnline_Request(FString const& InPlayerName, EOnline_RequestType InRequestType, FString const& InContent)
		: PlayerName(InPlayerName), RequestType(InRequestType), Content(InContent) {};
	
	UPROPERTY() FString PlayerName;
	UPROPERTY() TEnumAsByte<EOnline_RequestType> RequestType;
	UPROPERTY() FString Content;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FOnline_RequestContent_ChangeActivity
{
	GENERATED_BODY()
	
	FOnline_RequestContent_ChangeActivity() = default;
	FOnline_RequestContent_ChangeActivity(EOnline_PlayerActivity InPlayerActivity)
		: PlayerActivity(InPlayerActivity) {};
	
	UPROPERTY() TEnumAsByte<EOnline_PlayerActivity> PlayerActivity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FOnline_Response_FriendActivityChanged
{
	FString PlayerName;
	EOnline_PlayerActivity PlayerActivity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Blueprintable)
class UDebugMenu_PresenceFriendWidget : public UDebugMenu_CustomWidget
{
	GENERATED_BODY()

public:
	
	void SetPlayerName(FString InPlayerName) { PlayerName = InPlayerName; }
	void SetPlayerActivity(EOnline_PlayerActivity InPlayerActivity) { PlayerActivity = InPlayerActivity; }
	
protected:
	UPROPERTY(BlueprintReadOnly) FString PlayerName;
	UPROPERTY(BlueprintReadOnly) TEnumAsByte<EOnline_PlayerActivity> PlayerActivity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Blueprintable, Abstract)
class PRESENCESYSTEM_API UPresenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	void InitializePresenceSubsystem();
	void RequestChangeActivity(EOnline_PlayerActivity NewActivity);
	void OnFriendActivityChanged();

private:
	
	void TogglePresence();
	void ConnectToWebsocketServer();
	void DisconnectFromWebsocketServer();
	void ChangePlayerActivity();
	void SetLocalPlayerName(FString const& InLocalPlayerName);

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly) TSubclassOf<UDebugMenu_PresenceFriendWidget> PresenceWidgetClassPreset;

private:

	FString LocalPlayerName = "Player1";
	EOnline_PlayerActivity CurrentLocalActivity;

	UDebugMenu_PresenceFriendWidget* LocalPlayerDebugWidget;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////