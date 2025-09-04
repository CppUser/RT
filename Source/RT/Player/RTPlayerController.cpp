// Fill out your copyright notice in the Description page of Project Settings.


#include "RTPlayerController.h"

#include "RTLocalPlayer.h"
#include "AbilitySystem/RTAbilitySet.h"
#include "AbilitySystem/Core/RTAbilitySystemComponent.h"
#include "Character/RTPawnData.h"
#include "Character/Components/RTPawnExtComp.h"
#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Gameplay/RTGameMode.h"
#include "Gameplay/Experience/RTExperienceManagerComp.h"

const FName ARTPlayerController::NAME_RTAbilityReady("ASAbilityReady");

ARTPlayerController::ARTPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ASC = ObjectInitializer.CreateDefaultSubobject<URTAbilitySystemComponent>(this,TEXT("AbilitySystemComponent"));
	ASC->SetIsReplicated(false);

}

void ARTPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ARTPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//TODO: Check if it needs to be done in preinit comp
	// if (URTPawnExtComp* PawnExtComp = URTPawnExtComp::FindPawnExtComponent(GetPawn()))
	// {
	// 	PawnExtComp->CheckDefaultInitialization();		
	// }
	
	check(ASC);
	ASC->InitAbilityActorInfo(this,GetPawn());

	UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		URTExperienceManagerComp* ExperienceManager = GameState->FindComponentByClass<URTExperienceManagerComp>();
		check(ExperienceManager);
		ExperienceManager->CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate::CreateUObject(this,&ThisClass::OnExperienceLoaded));
	}
}

void ARTPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ARTPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (URTAbilitySystemComponent* AbilSysComp = GetRTAbilitySystemComponent())
	{
		AbilSysComp->ProcessAbilityInput(DeltaTime,bGamePaused);
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ARTPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}

void ARTPlayerController::ReceivedPlayer()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::ReceivedPlayer();

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->ReceivedPlayer();
	}

	if (URTLocalPlayer* LocalPlayer = Cast<URTLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerControllerSet.Broadcast(LocalPlayer,this);
	}
}

void ARTPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (URTLocalPlayer* LocalPlayer = Cast<URTLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer,InPawn);
	}
}

void ARTPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);

	if (URTLocalPlayer* LocalPlayer = Cast<URTLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer,InPawn);
	}
}

void ARTPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	if (URTLocalPlayer* LocalPlayer = Cast<URTLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer,nullptr);
	}
}

void ARTPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->PlayerTick(DeltaTime);
	}
}

void ARTPlayerController::SetPawnData(const URTPawnData* InPawnData)
{
	check(InPawnData);

	if (PawnData)
	{
		UE_LOG(LogTemp,Error,TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]"), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData))
		return;
	}

	PawnData = InPawnData;

	for (const URTAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(ASC,nullptr);
		}
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this,NAME_RTAbilityReady);
}

void ARTPlayerController::OnExperienceLoaded(const URTExperience* CurrentExperience)
{
	if (ARTGameMode* AsGameMode = GetWorld()->GetAuthGameMode<ARTGameMode>())
	{
		if (const URTPawnData* NewPawnData = AsGameMode->GetPawnDataForController(this))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogTemp,Error,TEXT("ASPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"),*GetNameSafe(this));
		}
	}
}

URTAbilitySystemComponent* ARTPlayerController::GetRTAbilitySystemComponent() const
{
	return ASC;
}

UAbilitySystemComponent* ARTPlayerController::GetAbilitySystemComponent() const
{
	return GetRTAbilitySystemComponent();
}
