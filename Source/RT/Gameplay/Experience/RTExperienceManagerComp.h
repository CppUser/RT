// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "RTExperienceManagerComp.generated.h"

class URTExperience;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnExperienceLoaded, const URTExperience*);

enum class ERTExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};


/**
 * 
 */
UCLASS()
class RT_API URTExperienceManagerComp : public UGameStateComponent
{
	GENERATED_BODY()
public:
	URTExperienceManagerComp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void SetCurrentExperience(FPrimaryAssetId ExperienceID);
	const URTExperience* GetCurrentExperienceChecked() const;
	bool IsExperienceLoaded() const;

	void CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate&& Delegate);
	void CallOrRegister_OnExperienceLoaded_LowPriority(FOnExperienceLoaded::FDelegate&& Delegate);
	void CallOrRegister_OnExperienceLoaded_HighPriority(FOnExperienceLoaded::FDelegate&& Delegate);

private:
	void StartExperienceLoad();
	void OnExperienceLoadComplete();
	void OnExperienceFullLoadCompleted();
private:
	UPROPERTY()
	TObjectPtr<const URTExperience> CurrentExperience;

	ERTExperienceLoadState LoadState = ERTExperienceLoadState::Unloaded;

	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;

	FOnExperienceLoaded OnExperienceLoaded;
	FOnExperienceLoaded OnExperienceLoaded_LowPriority;
	FOnExperienceLoaded OnExperienceLoaded_HighPriority;
};
