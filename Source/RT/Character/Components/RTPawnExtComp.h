// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Character/RTPawnData.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "RTPawnExtComp.generated.h"

class URTAbilitySystemComponent;


UCLASS()
class RT_API URTPawnExtComp : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
public:
	URTPawnExtComp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static URTPawnExtComp* FindPawnExtComponent(const AActor* InActor) {return (InActor ? InActor->FindComponentByClass<URTPawnExtComp>() : nullptr);}

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	void SetPawnData(const URTPawnData* InPawnData);

	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	virtual FName GetFeatureName() const override {return NAME_ActorFeatureName;}

	void InitializeAbilitySystem(URTAbilitySystemComponent* InASC, AActor* OwnerActor);
	void UninitializeAbilitySystem();
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);
	UFUNCTION(BlueprintPure)
	URTAbilitySystemComponent* GetRTAbilitySystemComponent() const {return ASC;}

	void SetupPlayerInputComponent();
	void HandleControllerChanged();
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	static const FName NAME_ActorFeatureName;
protected:
	UPROPERTY(EditInstanceOnly, Category = "RT|Pawn")
	TObjectPtr<const URTPawnData> PawnData;

	UPROPERTY()
	TObjectPtr<URTAbilitySystemComponent> ASC;

	FSimpleMulticastDelegate OnAbilitySystemInitialized;
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;
};
