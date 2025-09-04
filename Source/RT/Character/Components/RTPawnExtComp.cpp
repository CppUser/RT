// Fill out your copyright notice in the Description page of Project Settings.


#include "RTPawnExtComp.h"

#include "AbilitySystem/Core/RTAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Utils/RTGameplayTags.h"

const FName URTPawnExtComp::NAME_ActorFeatureName("PawnExtension");

URTPawnExtComp::URTPawnExtComp(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	PawnData = nullptr;
	ASC = nullptr;
}

void URTPawnExtComp::SetPawnData(const URTPawnData* InPawnData)
{
	check(InPawnData);
	
	if (PawnData)
	{
		APawn* Pawn = GetPawnChecked<APawn>();
		UE_LOG(LogTemp,Error,TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]"),
			*GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData))
		return;
	}
	PawnData = InPawnData;

	CheckDefaultInitialization();
}

bool URTPawnExtComp::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == RTGameplayTags::InitState_Spawned)
	{
		if (Pawn)
		{
			return true;
		}
	}

	if (CurrentState == RTGameplayTags::InitState_Spawned && DesiredState == RTGameplayTags::InitState_DataAvailable)
	{
		if (!PawnData && !GetController<AController>())
		{
			return false;
		}
		return true;
	}
	else if (CurrentState == RTGameplayTags::InitState_DataAvailable && DesiredState == RTGameplayTags::InitState_DataInitialized)
	{
		return Manager->HaveAllFeaturesReachedInitState(Pawn,RTGameplayTags::InitState_DataAvailable);
	}
	else if (CurrentState == RTGameplayTags::InitState_DataInitialized && DesiredState == RTGameplayTags::InitState_GameplayReady)
	{
		return true;
	}
	
	return false;
}

void URTPawnExtComp::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
}

void URTPawnExtComp::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == RTGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void URTPawnExtComp::CheckDefaultInitialization()
{
	CheckDefaultInitializationForImplementers();
	static const TArray<FGameplayTag> StateChain = {RTGameplayTags::InitState_Spawned,RTGameplayTags::InitState_DataAvailable,RTGameplayTags::InitState_DataInitialized,RTGameplayTags::InitState_GameplayReady};
	ContinueInitStateChain(StateChain);
}

void URTPawnExtComp::InitializeAbilitySystem(URTAbilitySystemComponent* InASC, AActor* OwnerActor)
{
	check(InASC);
	check(OwnerActor);

	if (ASC == InASC)
	{
		return;
	}

	if (ASC)
	{
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogTemp,Warning,TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(OwnerActor), *GetNameSafe(ExistingAvatar))

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		ensure(!ExistingAvatar);
		//TODO: Uninitialize other components
	}

	ASC = InASC;
	ASC->InitAbilityActorInfo(OwnerActor,Pawn);

	OnAbilitySystemInitialized.Broadcast();
}

void URTPawnExtComp::UninitializeAbilitySystem()
{
	if (!ASC)
	{
		return;
	}

	if (ASC->GetAvatarActor() == GetOwner())
	{
		ASC->CancelAbilities(nullptr); //TODO: add ability types to ignore if they in mid process
		//TODO: Clear Input ?
		ASC->RemoveAllGameplayCues();


		if (ASC->GetOwnerActor() != nullptr)
		{
			ASC->SetAvatarActor(nullptr);
		}
		else
		{
			ASC->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	ASC = nullptr;
}

void URTPawnExtComp::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (ASC)
	{
		Delegate.Execute();
	}
}

void URTPawnExtComp::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

void URTPawnExtComp::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void URTPawnExtComp::HandleControllerChanged()
{
	if (ASC && (ASC->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(ASC->AbilityActorInfo->OwnerActor == ASC->GetOwnerActor());
		if (ASC->GetOwnerActor() == nullptr)
		{
			UninitializeAbilitySystem();
		}
		else
		{
			ASC->RefreshAbilityActorInfo();
		}
	}

	CheckDefaultInitialization();
}

void URTPawnExtComp::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("URTPawnExtComp: on [%s] con only be added to Pawn actors"),*GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnComponents;
	Pawn->GetComponents(URTPawnExtComp::StaticClass(),PawnComponents);
	ensureAlwaysMsgf((PawnComponents.Num() == 1), TEXT("Only one URTPawnExtComp should exist on [%s]."),*GetNameSafe(GetOwner()));

	RegisterInitStateFeature();
}

void URTPawnExtComp::BeginPlay()
{
	Super::BeginPlay();
	BindOnActorInitStateChanged(NAME_None,FGameplayTag(),false);

	ensure(TryToChangeInitState(RTGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void URTPawnExtComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	Super::EndPlay(EndPlayReason);
}
