// Fill out your copyright notice in the Description page of Project Settings.


#include "RTLocalPlayer.h"

FDelegateHandle URTLocalPlayer::CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());

	if (PC)
	{
		Delegate.Execute(this,PC);
	}

	return OnPlayerControllerSet.Add(Delegate);
}

FDelegateHandle URTLocalPlayer::CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	if (Pawn)
	{
		Delegate.Execute(this,Pawn);
	}

	return OnPlayerPawnSet.Add(Delegate);
}
