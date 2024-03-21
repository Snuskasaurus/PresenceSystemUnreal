// Fill out your copyright notice in the Description page of Project Settings.

#include "PresenceSubsystem.h"

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "WebSocketsModule.h"

#define DEBUG_LOG(Text, ...) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT(Text), ##__VA_ARGS__));
#define DEBUG_LOG_SUCCESS(Text, ...) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT(Text), ##__VA_ARGS__));
#define DEBUG_LOG_WARNING(Text, ...) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT(Text), ##__VA_ARGS__));
#define DEBUG_LOG_ERROR(Text, ...) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT(Text), ##__VA_ARGS__));

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(PanLogWebSocket);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UE_DISABLE_OPTIMIZATION

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::ConnectToServer()
{
	ensureAlways(this);
	
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(WebSocket_Url, WebSocket_Protocol);
	
	WebSocket->OnConnected().AddLambda([]() -> void
		{ UPresenceSubsystem::SocketOnConnected(); });
	
	WebSocket->OnConnectionError().AddLambda([](const FString & Error) -> void
		{ UPresenceSubsystem::SocketOnConnectionError(Error); });
	
	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) -> void
		{ UPresenceSubsystem::SocketOnClosed(StatusCode, Reason, bWasClean); });
	
	WebSocket->OnMessage().AddLambda([](const FString & Message) -> void
		{ UPresenceSubsystem::SocketOnMessage(Message); });
	
	WebSocket->OnRawMessage().AddLambda([](const void* Data, SIZE_T Size, SIZE_T BytesRemaining) -> void
		{ UPresenceSubsystem::SocketOnRawMessage(Data, Size, BytesRemaining); });
	
	WebSocket->OnMessageSent().AddLambda([](const FString& MessageString) -> void
		{ UPresenceSubsystem::SocketOnMessageSent(MessageString); });
	
	DEBUG_LOG("Trying to establish WebSocket connection...");
	WebSocket->Connect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::DisconnectFromServer()
{
	ensureAlways(this);
	
	DEBUG_LOG("Closing WebSocket connection");
	WebSocket->Close();
}

void UPresenceSubsystem::SendMessage()
{
	if (WebSocket->IsConnected())
	{
		WebSocket->Send("Hello");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::SocketOnConnected()
{
	DEBUG_LOG_SUCCESS("Websocket connection established");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::SocketOnConnectionError(const FString& Error)
{
	const FString ErrorMessage = Error.IsEmpty() ? TEXT("Unknow") : Error;
	DEBUG_LOG_ERROR("Websocket connection failed, Error=%s", *ErrorMessage);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::SocketOnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	const FString ReasonMessage = Reason.IsEmpty() ? TEXT("Unknow") : Reason;
	if (bWasClean)
	{
		DEBUG_LOG("WebSocket connection closed, Reason=%s", *ReasonMessage);
	}
	DEBUG_LOG_WARNING("WebSocket connection closed, Reason=%s", *ReasonMessage);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::SocketOnMessage(const FString& Message)
{
	DEBUG_LOG("WebSocket message=%s", *Message);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::SocketOnRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining)
{
	DEBUG_LOG("SocketOnRawMessage");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPresenceSubsystem::SocketOnMessageSent(const FString& MessageString)
{
	DEBUG_LOG("SocketOnMessageSent");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UE_ENABLE_OPTIMIZATION