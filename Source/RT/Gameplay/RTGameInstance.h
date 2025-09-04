// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RTGameInstance.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class RT_API URTGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	URTGameInstance(const FObjectInitializer& ObjInit);

	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
	virtual void Init() override;
	virtual void ReturnToMainMenu() override;

private:
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;
};
