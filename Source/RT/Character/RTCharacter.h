// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Character.h"
#include "RTCharacter.generated.h"

class URTPawnExtComp;
class URTAbilitySystemComponent;

UCLASS(Blueprintable,Config=Game)
class RT_API ARTCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ARTCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PreInitializeComponents() override;
	
	
	UFUNCTION(BlueprintCallable)
	URTAbilitySystemComponent* GetRTAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnAbilitySystemInitialized();
	virtual void OnAbilitySystemUninitialized();
private:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="RT|Character",meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URTPawnExtComp> PawnExtComponent;

};
