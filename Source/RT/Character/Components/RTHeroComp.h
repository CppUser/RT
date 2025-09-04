// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "RTHeroComp.generated.h"

class URTInputConfig;
struct FInputActionValue;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class RT_API URTHeroComp : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
public:
	URTHeroComp(const FObjectInitializer& ObjectInitializer);

	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	virtual FName GetFeatureName() const override {return NAME_ActorFeatureName;}

	bool IsReadyToBindInputs() const;
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void InitializePlayerInput(class UInputComponent* PlayerInputComponent);

	void Input_Move(const FInputActionValue& value);
	void Input_Mouse_Look(const FInputActionValue& value);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
public:
	static const FName NAME_ActorFeatureName;
	static const FName NAME_BindInputsNow;
protected:
	//TODO: temp bypass . use either GameFeatures or FMappableConfigPairs
	UPROPERTY(EditAnywhere)
	URTInputConfig* DefaultInputConfig;

	bool bReadyToBindInputs;
};
