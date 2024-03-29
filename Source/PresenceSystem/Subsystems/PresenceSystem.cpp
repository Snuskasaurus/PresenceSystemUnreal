// Copyright Epic Games, Inc. All Rights Reserved.

#include "PresenceSystem.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, PresenceSystem, "PresenceSystem" );

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

void UPresenceSubsystem::RequestFriendList()
{
	
}

void UPresenceSubsystem::RequestChangeActivity()
{
	
}

void UPresenceSubsystem::OnFriendActivityChanged()
{
	
}
