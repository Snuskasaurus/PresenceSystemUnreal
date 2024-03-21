// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "PresenceSubsystem.generated.h"

UCLASS()
class PRESENCESYSTEM_API UPresenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	void ConnectPresence();
	void DisconnectPresence();
	
	const FString WebSocket_Url;
	const FString WebSocket_Protocol;
	TSharedPtr<IWebSocket> WebSocket;
	
};
