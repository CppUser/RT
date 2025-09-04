// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameUIManagerSubsystem.generated.h"

class URTLocalPlayer;
/**
 * 
 */
UCLASS()
class RT_API UGameUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UGameUIManagerSubsystem() {}

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void NotifyPlayerAdded(URTLocalPlayer* LocalPlayer);
	virtual void NotifyPlayerRemoved(URTLocalPlayer* LocalPlayer);
	virtual void NotifyPlayerDestroyed(URTLocalPlayer* LocalPlayer);
};
