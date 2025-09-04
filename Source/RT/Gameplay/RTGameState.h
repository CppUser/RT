// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RTGameState.generated.h"

class URTExperienceManagerComp;
/**
 * 
 */
UCLASS()
class RT_API ARTGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	ARTGameState(const FObjectInitializer& ObjInit = FObjectInitializer::Get());
	
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	//~AGameStateBase interface
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;
	//~End of AGameStateBase interface
	
private:
	UPROPERTY()
	TObjectPtr<URTExperienceManagerComp> ExperienceManagerComponent;
};
