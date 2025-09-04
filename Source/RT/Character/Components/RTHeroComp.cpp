// Fill out your copyright notice in the Description page of Project Settings.


#include "RTHeroComp.h"

#include "RTPawnExtComp.h"
#include "AbilitySystem/Core/RTAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Input/RTInputComponent.h"
#include "Player/RTPlayerController.h"
#include "Utils/RTGameplayTags.h"

const FName URTHeroComp::NAME_ActorFeatureName("Hero");
const FName URTHeroComp::NAME_BindInputsNow("BindInputsNow");

URTHeroComp::URTHeroComp(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer),DefaultInputConfig(nullptr),bReadyToBindInputs(false)
{
}

bool URTHeroComp::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
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

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

		if (bIsLocallyControlled)
		{
			ARTPlayerController* PC = GetController<ARTPlayerController>();
			if (!Pawn->InputComponent || !PC ||!PC->GetLocalPlayer())
			{
				return false;
			}
		}
		
		return true;
	}
	else if (CurrentState == RTGameplayTags::InitState_DataAvailable && DesiredState == RTGameplayTags::InitState_DataInitialized)
	{
		return Manager->HasFeatureReachedInitState(Pawn,URTPawnExtComp::NAME_ActorFeatureName,RTGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == RTGameplayTags::InitState_DataInitialized && DesiredState == RTGameplayTags::InitState_GameplayReady)
	{
		//TODO: add ability initialization checks ?
		return true;
	}
	
	
	return false;
}

void URTHeroComp::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	if (CurrentState == RTGameplayTags::InitState_DataAvailable && DesiredState == RTGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		ARTPlayerController* PC = GetController<ARTPlayerController>();
		if (!ensure(Pawn && PC))
		{
			return;
		}

		if (URTPawnExtComp* PawnExtComponent = URTPawnExtComp::FindPawnExtComponent(Pawn))
		{
			PawnExtComponent->InitializeAbilitySystem(PC->GetRTAbilitySystemComponent(),Pawn);
		}

		if (Pawn->InputComponent != nullptr)
		{
			InitializePlayerInput(Pawn->InputComponent);
		}

		//TODO: Determine Camera mode
	}
}

void URTHeroComp::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName != URTPawnExtComp::NAME_ActorFeatureName)
	{
		
		if (Params.FeatureState == RTGameplayTags::InitState_DataInitialized)
		{
			CheckDefaultInitialization();
		}
	}
}

void URTHeroComp::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {RTGameplayTags::InitState_Spawned,RTGameplayTags::InitState_DataAvailable,RTGameplayTags::InitState_DataInitialized,RTGameplayTags::InitState_GameplayReady};
	ContinueInitStateChain(StateChain);
}

bool URTHeroComp::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void URTHeroComp::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();
}

void URTHeroComp::BeginPlay()
{
	Super::BeginPlay();
	BindOnActorInitStateChanged(URTPawnExtComp::NAME_ActorFeatureName,FGameplayTag(),false);

	ensure(TryToChangeInitState(RTGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void URTHeroComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	Super::EndPlay(EndPlayReason);
}

void URTHeroComp::InitializePlayerInput(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	//TODO: call to ULocalPlayer should be handled by own UASLocalPlayer 
	//const UASLocalPlayer* LP = Cast<UASLocalPlayer>(PC->GetLocalPlayer());
	const ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const URTPawnExtComp* PawnExtComp = URTPawnExtComp::FindPawnExtComponent(Pawn))
	{
		if (const URTPawnData* PawnData = PawnExtComp->GetPawnData<URTPawnData>())
		{
			if (const URTInputConfig* InputConfig = PawnData->InputConfig)
			{
				if (InputConfig == nullptr)
				{
					if (!DefaultInputConfig)
					{
						//TODO: Need better solution to avoid specifying path 
						DefaultInputConfig = LoadObject<URTInputConfig>(nullptr,TEXT("/Game/RT/Input/DefaultHeroInputConfig.DefaultHeroInputConfig_C"));
					}
					InputConfig = DefaultInputConfig;
					check(InputConfig);
				}
				
				//TODO: Register any default input configs with the settings so that they will be applied to the player during AddInputMappings

				URTInputComponent* IC = CastChecked<URTInputComponent>(PlayerInputComponent);
				IC->AddInputMappings(InputConfig,Subsystem);

				TArray<uint32> BindHandles;
				IC->BindAbilityInputAction(InputConfig,this,&ThisClass::Input_AbilityInputTagPressed,&ThisClass::Input_AbilityInputTagReleased,BindHandles);

				IC->BindNativeInputAction(InputConfig,RTGameplayTags::InputTag_Move,ETriggerEvent::Triggered,this,&ThisClass::Input_Move);
				IC->BindNativeInputAction(InputConfig,RTGameplayTags::InputTag_Mouse_Look,ETriggerEvent::Triggered,this,&ThisClass::Input_Mouse_Look);
			}
		}
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC),NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn),NAME_BindInputsNow);

}

void URTHeroComp::Input_Move(const FInputActionValue& value)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	if (Controller)
	{
		const FVector2D val = value.Get<FVector2D>();
		const FRotator MovementRotation(0.0f,Controller->GetControlRotation().Yaw,0.0f);

		if (val.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection,val.X);
		}

		if (val.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection,val.Y);
		}
	}
}

void URTHeroComp::Input_Mouse_Look(const FInputActionValue& value)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	
	const FVector2D val = value.Get<FVector2D>();

	if (val.X != 0.0f)
	{
		Pawn->AddControllerYawInput(val.X);
	}

	if (val.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(val.Y);
	}
}

void URTHeroComp::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const URTPawnExtComp* PawnExtComp = URTPawnExtComp::FindPawnExtComponent(Pawn))
		{
			if (URTAbilitySystemComponent* ASC = PawnExtComp->GetRTAbilitySystemComponent())
			{
				ASC->AbilityInputTagPressed(InputTag);
			}
		}
	}
}

void URTHeroComp::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const URTPawnExtComp* PawnExtComp = URTPawnExtComp::FindPawnExtComponent(Pawn))
	{
		if (URTAbilitySystemComponent* ASC = PawnExtComp->GetRTAbilitySystemComponent())
		{
			ASC->AbilityInputTagReleased(InputTag);
		}
	}
}
