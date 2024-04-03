// Fill out your copyright notice in the Description page of Project Settings.

#include "WebsocketSubsystem.h"

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "WebSocketsModule.h"
#include "PresenceSystem/DebugMenu/DebugMenu.h"

#define DEBUG_LOG(Text, ...)			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT(Text), ##__VA_ARGS__));
#define DEBUG_LOG_BLUE(Text, ...)		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT(Text), ##__VA_ARGS__));
#define DEBUG_LOG_GREEN(Text, ...)		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT(Text), ##__VA_ARGS__));
#define DEBUG_LOG_WARNING(Text, ...)	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT(Text), ##__VA_ARGS__));
#define DEBUG_LOG_ERROR(Text, ...)		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT(Text), ##__VA_ARGS__));

DEFINE_LOG_CATEGORY(PanLogWebSocket);

UE_DISABLE_OPTIMIZATION

//----------------------------------------------------------------------------------------------------------------------
void UWebsocketSubsystem::Connect()
{
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(WebSocket_Url, WebSocket_Protocol);
	
	WebSocket->OnConnected().AddLambda([this]() -> void
		{ UWebsocketSubsystem::SocketOnConnected(); });
	
	WebSocket->OnConnectionError().AddLambda([this](const FString & Error) -> void
		{ UWebsocketSubsystem::SocketOnConnectionError(Error); });
	
	WebSocket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean) -> void
		{ UWebsocketSubsystem::SocketOnClosed(StatusCode, Reason, bWasClean); });
	
	WebSocket->OnMessage().AddLambda([this](const FString & Message) -> void
		{ UWebsocketSubsystem::SocketOnMessage(Message); });
	
	WebSocket->OnRawMessage().AddLambda([this](const void* Data, SIZE_T Size, SIZE_T BytesRemaining) -> void
		{ UWebsocketSubsystem::SocketOnRawMessage(Data, Size, BytesRemaining); });
	
	WebSocket->OnMessageSent().AddLambda([this](const FString& MessageString) -> void
		{ UWebsocketSubsystem::SocketOnMessageSent(MessageString); });
	
	DEBUG_LOG("Trying to establish WebSocket connection...");
	WebSocket->Connect();
}
//----------------------------------------------------------------------------------------------------------------------
void UWebsocketSubsystem::Disconnect()
{
	if (WebSocket.IsValid() == false)
		return;

	if (WebSocket->IsConnected() == false)
		return;
	
	DEBUG_LOG("Closing WebSocket connection");
	WebSocket->Close();
}
//----------------------------------------------------------------------------------------------------------------------
void UWebsocketSubsystem::TickWebsocketSubsystem()
{
	
}
//----------------------------------------------------------------------------------------------------------------------
bool UWebsocketSubsystem::TryToSendMessage(FString const& Message)
{
	if (WebSocket.IsValid() == false)
		return false;

	if (Message.IsEmpty() == true)
		return false;
	
	if (WebSocket->IsConnected() == false)
		return false;
	
	WebSocket->Send(Message);

	return true;
}
//----------------------------------------------------------------------------------------------------------------------
void UWebsocketSubsystem::SocketOnConnected()
{
	DEBUG_LOG_GREEN("WebSocket - Connection established");
	
	IsConnected = true;
}
//----------------------------------------------------------------------------------------------------------------------
void UWebsocketSubsystem::SocketOnConnectionError(const FString& Error)
{
	const FString ErrorMessage = Error.IsEmpty() ? TEXT("Unknow") : Error;
	DEBUG_LOG_ERROR("WebSocket - Connection failed, Error=%s", *ErrorMessage);
}
//----------------------------------------------------------------------------------------------------------------------
void UWebsocketSubsystem::SocketOnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	const FString ReasonMessage = Reason.IsEmpty() ? TEXT("Unknow") : Reason;
	if (bWasClean)
	{
		DEBUG_LOG("WebSocket - Connection closed, Reason=%s", *ReasonMessage);
	}
	DEBUG_LOG_WARNING("WebSocket - Connection closed, Reason=%s", *ReasonMessage);
	
	IsConnected = false;
}
//----------------------------------------------------------------------------------------------------------------------
void UWebsocketSubsystem::SocketOnMessage(const FString& Message)
{
	DEBUG_LOG_GREEN("WebSocket - message received: %s", *Message);
}
//----------------------------------------------------------------------------------------------------------------------
void UWebsocketSubsystem::SocketOnRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining)
{
	//DEBUG_LOG_GREEN("SocketOnRawMessage");
}
//----------------------------------------------------------------------------------------------------------------------
void UWebsocketSubsystem::SocketOnMessageSent(const FString& Message)
{
	DEBUG_LOG_BLUE("WebSocket - message sent: %s", *Message);
}
//----------------------------------------------------------------------------------------------------------------------

UE_ENABLE_OPTIMIZATION