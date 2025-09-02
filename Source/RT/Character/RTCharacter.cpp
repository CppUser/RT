// Fill out your copyright notice in the Description page of Project Settings.


#include "RTCharacter.h"
#include "Components/GameFrameworkComponentManager.h"

// Sets default values
ARTCharacter::ARTCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ARTCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ARTCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called when the game starts or when spawned
void ARTCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARTCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ARTCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

