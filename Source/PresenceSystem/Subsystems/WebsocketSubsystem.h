// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "WebsocketSubsystem.generated.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IWebSocket;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(PanLogWebSocket, All, All);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class PRESENCESYSTEM_API UWebsocketSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	void Connect();
	void Disconnect();
	void TickWebsocketSubsystem();

	bool CheckIfConnected() { return IsConnected; }
	
	bool TryToSendMessage(FString const& Message);
	
public:
	
	// This code will run once connected.
	void SocketOnConnected();
	
	// This code will run if the connection failed. Check Error to see what happened.
	void SocketOnConnectionError(const FString & Error);
	
	// This code will run when the connection to the server has been terminated.
	// Because of an error or a call to Socket->Close().
	void SocketOnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
    
	// This code will run when we receive a string message from the server.
	void SocketOnMessage(const FString & Message);
    
	// This code will run when we receive a raw (binary) message from the server.
	void SocketOnRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining);
    
	// This code is called after we sent a message to the server.
	void SocketOnMessageSent(const FString& Message);
	
private:
	
	const FString WebSocket_Url = TEXT("ws://127.0.0.1:6666/");
	const FString WebSocket_Protocol = TEXT("wolcen");
	TSharedPtr<IWebSocket> WebSocket;

	bool IsConnected = false;
	
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////