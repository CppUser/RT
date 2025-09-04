// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "RTLocalPlayer.generated.h"

/**
 * 
 */
UCLASS()
class RT_API URTLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()
public:

	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerControllerSetDelegate,URTLocalPlayer* LocalPlayer,APlayerController* PlayerController)
	FPlayerControllerSetDelegate OnPlayerControllerSet;
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerPawnSetDelegate, URTLocalPlayer* LocalPlayer, APawn* Pawn);
	FPlayerPawnSetDelegate OnPlayerPawnSet;

	FDelegateHandle CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate);
	FDelegateHandle CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate);

};
