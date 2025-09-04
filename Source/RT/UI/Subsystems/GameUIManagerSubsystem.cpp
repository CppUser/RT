// Fill out your copyright notice in the Description page of Project Settings.


#include "GameUIManagerSubsystem.h"

void UGameUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
}

void UGameUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UGameUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(),ChildClasses,false);

		return ChildClasses.Num() == 0;
	}
	return false;
}

void UGameUIManagerSubsystem::NotifyPlayerAdded(URTLocalPlayer* LocalPlayer)
{
}

void UGameUIManagerSubsystem::NotifyPlayerRemoved(URTLocalPlayer* LocalPlayer)
{
}

void UGameUIManagerSubsystem::NotifyPlayerDestroyed(URTLocalPlayer* LocalPlayer)
{
}
