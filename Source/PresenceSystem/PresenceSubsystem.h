// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "PresenceSubsystem.generated.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IWebSocket;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(PanLogWebSocket, All, All);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class PRESENCESYSTEM_API UPresenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	void ConnectToServer();
	void DisconnectFromServer();

	void SendMessage();
	
public:
	
	// This code will run once connected.
	static void SocketOnConnected();
	
	// This code will run if the connection failed. Check Error to see what happened.
	static void SocketOnConnectionError(const FString & Error);
	
	// This code will run when the connection to the server has been terminated.
	// Because of an error or a call to Socket->Close().
	static void SocketOnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
    
	// This code will run when we receive a string message from the server.
	static void SocketOnMessage(const FString & Message);
    
	// This code will run when we receive a raw (binary) message from the server.
	static void SocketOnRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining);
    
	// This code is called after we sent a message to the server.
	static void SocketOnMessageSent(const FString& MessageString);
	
private:
	
	const FString WebSocket_Url = TEXT("ws://127.0.0.1:6666/");
	const FString WebSocket_Protocol = TEXT("ws");
	TSharedPtr<IWebSocket> WebSocket;
	
};
