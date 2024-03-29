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
enum EPlayerActivity
{
	DISCONNECTED,
	IN_MENU,
	PLAYING,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FOnline_Request_FriendList
{
	FString LocalPlayerName;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FOnline_Request_ChangeActivity
{
	FString LocalPlayerName;
	EPlayerActivity PlayerActivity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FOnline_Response_FriendActivityChanged
{
	FString PlayerName;
	EPlayerActivity PlayerActivity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Blueprintable)
class UDebugMenu_PresenceFriendWidget : public UDebugMenu_CustomWidget
{
	GENERATED_BODY()

public:
	void SetPlayerName(FString InPlayerName) { PlayerName = InPlayerName; }
	void SetPlayerActivity(EPlayerActivity InPlayerActivity) { PlayerActivity = InPlayerActivity; }
	
protected:
	UPROPERTY(BlueprintReadOnly) FString PlayerName;
	UPROPERTY(BlueprintReadOnly) TEnumAsByte<EPlayerActivity> PlayerActivity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Blueprintable, Abstract)
class PRESENCESYSTEM_API UPresenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	void RequestFriendList();
	void RequestChangeActivity();
	
	void OnFriendActivityChanged();

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly) TSubclassOf<UDebugMenu_PresenceFriendWidget> PresenceWidgetClassPreset;
	
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////