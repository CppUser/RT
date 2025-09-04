// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "RTPlayerController.generated.h"

struct FAbilitySet_GrantedHandles;
class URTExperience;
class URTPawnData;
class URTAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class RT_API ARTPlayerController : public APlayerController,public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	ARTPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	URTAbilitySystemComponent* GetRTAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	
protected:
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void ReceivedPlayer() override;
	virtual void SetPawn(APawn* InPawn) override;
	
	virtual void AcknowledgePossession(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PlayerTick(float DeltaTime) override;
	
	void SetPawnData(const URTPawnData* InPawnData);
private:
	void OnExperienceLoaded(const URTExperience* CurrentExperience);
public:
	static const FName NAME_RTAbilityReady;
protected:
	UPROPERTY(VisibleAnywhere,Category="RT|PlayerController")
	TObjectPtr<const URTPawnData> PawnData;
private:
	UPROPERTY(VisibleAnywhere,Category="RT|PlayerController")
	TObjectPtr<URTAbilitySystemComponent> ASC;

	TArray<FAbilitySet_GrantedHandles*> GrantedAbilitySetHandles;
};
