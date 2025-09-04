// Fill out your copyright notice in the Description page of Project Settings.


#include "RTCharacter.h"

#include "AbilitySystem/Core/RTAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/RTPawnExtComp.h"


ARTCharacter::ARTCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;

	PawnExtComponent = CreateDefaultSubobject<URTPawnExtComp>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this,&ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this,&ThisClass::OnAbilitySystemUninitialized));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
}

void ARTCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

URTAbilitySystemComponent* ARTCharacter::GetRTAbilitySystemComponent() const
{
	return Cast<URTAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* ARTCharacter::GetAbilitySystemComponent() const
{
	return PawnExtComponent->GetRTAbilitySystemComponent();
}

void ARTCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtComponent->HandleControllerChanged();
}

void ARTCharacter::UnPossessed()
{
	Super::UnPossessed();

	PawnExtComponent->HandleControllerChanged();
}

void ARTCharacter::OnAbilitySystemInitialized()
{
	URTAbilitySystemComponent* ASC = GetRTAbilitySystemComponent();
	check(ASC);

	UE_LOG(LogTemp,Warning,TEXT("Ability System Initialized"));
}

void ARTCharacter::OnAbilitySystemUninitialized()
{
	UE_LOG(LogTemp,Warning,TEXT("Ability System Uninitialized"));
}

void ARTCharacter::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this,UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();
	
}

void ARTCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}

void ARTCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARTCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

