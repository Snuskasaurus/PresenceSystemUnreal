// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

UCLASS()
class PRESENCESYSTEM_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TSubclassOf<class UPantheonGenericDebugMenuSubsystem> PantheonGenericDebugMenuSubsystem;
	
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TSubclassOf<class UWebsocketSubsystem> WebsocketSubsystem;
	
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TSubclassOf<class UPresenceSubsystem> PresenceSubsystem;
	
};
