// Fill out your copyright notice in the Description page of Project Settings.


#include "RTGameState.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Experience/RTExperienceManagerComp.h"

ARTGameState::ARTGameState(const FObjectInitializer& ObjInit)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ExperienceManagerComponent = CreateDefaultSubobject<URTExperienceManagerComp>(TEXT("ExperienceManagerComponent"));
}

void ARTGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ARTGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ARTGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

void ARTGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
}

void ARTGameState::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
{
	Super::SeamlessTravelTransitionCheckpoint(bToTransitionMap);
}

void ARTGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ARTGameState::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();
}

void ARTGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}
