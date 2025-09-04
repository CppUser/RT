// Fill out your copyright notice in the Description page of Project Settings.


#include "RTGameInstance.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Utils/RTGameplayTags.h"

URTGameInstance::URTGameInstance(const FObjectInitializer& ObjInit) : Super(ObjInit)
{
}

int32 URTGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId)
{
	int32 Res = Super::AddLocalPlayer(NewPlayer, UserId);
	if (Res != INDEX_NONE)
	{
		if (!PrimaryPlayer.IsValid())
		{
			//TODO: Log to Different Channel
			UE_LOG(LogTemp,Log,TEXT("AddLocalPlayer: Set %s to Primary Player"), *NewPlayer->GetName());
			PrimaryPlayer = NewPlayer;
		}

		//TODO:GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerAdded(Cast<URTLocalPlayer>(NewPlayer));
	}
	
	return Res;
}

bool URTGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	//TODO:GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerDestroyed(Cast<URTLocalPlayer>(ExistingPlayer));
	return Super::RemoveLocalPlayer(ExistingPlayer);
}

void URTGameInstance::Init()
{
	Super::Init();

	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		
		ComponentManager->RegisterInitState(RTGameplayTags::InitState_Spawned,false,FGameplayTag());
		ComponentManager->RegisterInitState(RTGameplayTags::InitState_DataAvailable,false,RTGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(RTGameplayTags::InitState_DataInitialized,false,RTGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(RTGameplayTags::InitState_GameplayReady,false,RTGameplayTags::InitState_DataInitialized);
	}
}

void URTGameInstance::ReturnToMainMenu()
{
	Super::ReturnToMainMenu();
}
