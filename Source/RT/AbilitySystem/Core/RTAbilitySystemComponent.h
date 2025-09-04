// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "RTGameplayAbility.h"
#include "RTAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class RT_API URTAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	URTAbilitySystemComponent(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	void ProcessAbilityInput(float dt, bool bGamePaused);
	void ClearAbilityInput();

	typedef TFunctionRef<bool(const URTGameplayAbility* ASAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc);
	void CancelInputActivatedAbilities();
	
	bool IsActivationGroupBlocked(EAbilityActivationGroup Group) const;
	void AddAbilityToActivationGroup(EAbilityActivationGroup Group, URTGameplayAbility* InAbility);
	void RemoveAbilityFromActivationGroup(EAbilityActivationGroup Group, URTGameplayAbility* InAbility);
	void CancelActivationGroupAbilities(EAbilityActivationGroup Group, URTGameplayAbility* IgnoreAbility);

	void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;
protected:
	void TryActivateAbilityOnSpawn();

	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
	virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);
protected:
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	int32 ActivationGroupCounts[static_cast<uint8>(EAbilityActivationGroup::MAX)];
};
