// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

UCLASS()
class PRESENCESYSTEM_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	void ConnectToServer();
	void DisconnectFromServer();
	void ChangePlayerActivity();
	void ChangeOnlinePlayerName(FString const& InPlayerName);
};
