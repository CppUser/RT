// Fill out your copyright notice in the Description page of Project Settings.


#include "RTAbilitySystemComponent.h"

URTAbilitySystemComponent::URTAbilitySystemComponent(const FObjectInitializer& ObjInit) : Super(ObjInit)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void URTAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);
	
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	
	if (bHasNewPawnAvatar)
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			URTGameplayAbility* AbilityCDO = CastChecked<URTGameplayAbility>(AbilitySpec.Ability);

			if (AbilityCDO->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerActor)
			{
				TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
				for (UGameplayAbility* AbilityInstance : Instances)
				{
					URTGameplayAbility* ASAbilityInstance = CastChecked<URTGameplayAbility>(AbilityInstance);
					ASAbilityInstance->OnPawnAvatarSet();
				}
			}
			else
			{
				AbilityCDO->OnPawnAvatarSet();
			}
		}

		//TODO: Register global ASC
		//TODO: Initialize AnimInstance (for use of gameplay tags)

		TryActivateAbilityOnSpawn();
	}
}

void URTAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//TODO: Unregister Global ACS once it implemented
	Super::EndPlay(EndPlayReason);
}

void URTAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && ( AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void URTAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && ( AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void URTAbilitySystemComponent::ProcessAbilityInput(float dt, bool bGamePaused)
{
	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();
	
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const URTGameplayAbility* ASAbilityCDO = CastChecked<URTGameplayAbility>(AbilitySpec->Ability);

				if (ASAbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const URTGameplayAbility* ASAbilityCDO = CastChecked<URTGameplayAbility>(AbilitySpec->Ability);

					if (ASAbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void URTAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void URTAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		URTGameplayAbility* AbilityCDO = CastChecked<URTGameplayAbility>(AbilitySpec.Ability);

		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::InstancedPerActor)
		{
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				URTGameplayAbility* ASAbilityInstance = CastChecked<URTGameplayAbility>(AbilityInstance);

				if (ShouldCancelFunc(ASAbilityInstance, AbilitySpec.Handle))
				{
					if (ASAbilityInstance->CanBeCanceled())
					{
						ASAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), ASAbilityInstance->GetCurrentActivationInfo(),false);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *ASAbilityInstance->GetName());
					}
				}
			}
		}
		else
		{
			if (ShouldCancelFunc(AbilityCDO, AbilitySpec.Handle))
			{
				check(AbilityCDO->CanBeCanceled());
				AbilityCDO->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(),false);
			}
		}
	}
}

void URTAbilitySystemComponent::CancelInputActivatedAbilities()
{
	auto ShouldCancelFunc = [this](const URTGameplayAbility* ASAbility, FGameplayAbilitySpecHandle Handle)
	{
		const EAbilityActivationPolicy ActivationPolicy = ASAbility->GetActivationPolicy();
		return ((ActivationPolicy == EAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == EAbilityActivationPolicy::WhileInputActive));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc);
}

bool URTAbilitySystemComponent::IsActivationGroupBlocked(EAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case EAbilityActivationGroup::Independent:
		bBlocked = false;
		break;
	case EAbilityActivationGroup::Exclusive_Replaceable:
	case EAbilityActivationGroup::Exclusive_Blocking:
		bBlocked = (ActivationGroupCounts[static_cast<uint8>(EAbilityActivationGroup::Exclusive_Blocking)] > 0);
		break;
	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), static_cast<uint8>(Group));
		break;
	}

	return bBlocked;
}

void URTAbilitySystemComponent::AddAbilityToActivationGroup(EAbilityActivationGroup Group,
	URTGameplayAbility* InAbility)
{
	check(InAbility);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] < INT32_MAX);
	ActivationGroupCounts[static_cast<uint8>(Group)]++;

	switch (Group)
	{
	case EAbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
		break;

	case EAbilityActivationGroup::Exclusive_Replaceable:
	case EAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(EAbilityActivationGroup::Exclusive_Replaceable, InAbility);
		break;
	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), static_cast<uint8>(Group));
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)EAbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[static_cast<uint8>(EAbilityActivationGroup::Exclusive_Blocking)];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogTemp, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void URTAbilitySystemComponent::RemoveAbilityFromActivationGroup(EAbilityActivationGroup Group,
	URTGameplayAbility* InAbility)
{
	check(InAbility);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] > 0);

	ActivationGroupCounts[static_cast<uint8>(Group)]--;
}

void URTAbilitySystemComponent::CancelActivationGroupAbilities(EAbilityActivationGroup Group,
	URTGameplayAbility* IgnoreAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreAbility](const URTGameplayAbility* ASAbility, FGameplayAbilitySpecHandle Handle)
	{
		return ((ASAbility->GetActivationGroup() == Group) && (ASAbility != IgnoreAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc);
}

void URTAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	//TODO:TagRelationshipMapping
	// if (TagRelationshipMapping)
	// {
	// 	TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	// }
}

void URTAbilitySystemComponent::TryActivateAbilityOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec&  AbilitySpec : ActivatableAbilities.Items)
	{
		const URTGameplayAbility* AbilityCDO = CastChecked<URTGameplayAbility>(AbilitySpec.Ability);
		AbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(),AbilitySpec);
	}
}

void URTAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	if (Spec.IsActive())
	{
		FGameplayAbilityActivationInfo ActivationInfo;
        
		if (Spec.GetAbilityInstances().Num() > 0)
		{
			if (UGameplayAbility* AbilityInstance = Spec.GetAbilityInstances()[0])
			{
				ActivationInfo = AbilityInstance->GetCurrentActivationInfo();
			}
		}
        
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, ActivationInfo.GetActivationPredictionKey());
	}
}

void URTAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		FGameplayAbilityActivationInfo ActivationInfo;
        
		if (Spec.GetAbilityInstances().Num() > 0)
		{
			if (UGameplayAbility* AbilityInstance = Spec.GetAbilityInstances()[0])
			{
				ActivationInfo = AbilityInstance->GetCurrentActivationInfo();
			}
		}
        
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, ActivationInfo.GetActivationPredictionKey());
	}
}

void URTAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	URTGameplayAbility* ASAbility = CastChecked<URTGameplayAbility>(Ability);
	AddAbilityToActivationGroup(ASAbility->GetActivationGroup(), ASAbility);
}

void URTAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void URTAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
	bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	URTGameplayAbility* ASAbility = CastChecked<URTGameplayAbility>(Ability);

	RemoveAbilityFromActivationGroup(ASAbility->GetActivationGroup(), ASAbility);
}

void URTAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags,
	UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags,
	bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	//TODO: TagRelationshipMapping
	// if (TagRelationshipMapping)
	// {
	// 	// Use the mapping to expand the ability tags into block and cancel tag
	// 	TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	// }
	
	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, BlockTags,
	                                      bExecuteCancelTags, CancelTags);
}

void URTAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags,
	UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	//TODO: Implement me
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);
}

void URTAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureReason)
{
	UE_LOG(LogTemp, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetPathNameSafe(Ability), *FailureReason.ToString());

	if (const URTGameplayAbility* ASAbility = Cast<const URTGameplayAbility>(Ability))
	{
		ASAbility->OnAbilityFailedToActivate(FailureReason);
	}	
}
