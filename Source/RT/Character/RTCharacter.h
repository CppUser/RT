// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RTCharacter.generated.h"

UCLASS()
class RT_API ARTCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARTCharacter();

	virtual void PreInitializeComponents() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


};
