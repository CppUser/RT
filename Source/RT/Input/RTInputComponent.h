// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "RTInputConfig.h"
#include "RTInputComponent.generated.h"

/**
 * 
 */
UCLASS(Config = Input)
class RT_API URTInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:

	void AddInputMappings(const URTInputConfig* InputConfig,UEnhancedInputLocalPlayerSubsystem* InInputSubsystem) const;
	void RemoveInputMappings(const URTInputConfig* InputConfig,UEnhancedInputLocalPlayerSubsystem* InInputSubsystem);

	template<class TClass, typename Func>
	void BindNativeInputAction(const URTInputConfig* InputConfig,const FGameplayTag& InputTag, ETriggerEvent TriggerEvent,TClass* obj,Func func);

	template<class TClass, typename PressedFunc,typename ReleasedFunc>
	void BindAbilityInputAction(const URTInputConfig* InputConfig,TClass* obj,PressedFunc pressed,ReleasedFunc released,TArray<uint32_t> handles);

	void RemoveBinds(TArray<uint32_t>& Handles);
};

template <class TClass, typename Func>
void URTInputComponent::BindNativeInputAction(const URTInputConfig* InputConfig, const FGameplayTag& InputTag,
	ETriggerEvent TriggerEvent, TClass* obj, Func func)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputAction(InputTag))
	{
		BindAction(IA, TriggerEvent, obj, func);
	}
}

template <class TClass, typename PressedFunc, typename ReleasedFunc>
void URTInputComponent::BindAbilityInputAction(const URTInputConfig* InputConfig, TClass* obj, PressedFunc pressed,
	ReleasedFunc released, TArray<uint32_t> handles)
{
	check(InputConfig);
	for (const FRTInputAction& IA : InputConfig->AbilityActions)
	{
		if (IA.Action && IA.ActionTag.IsValid())
		{
			if (pressed)
			{
				BindAction(IA.Action,ETriggerEvent::Triggered,obj,pressed,IA.ActionTag);
			}
			if (released)
			{
				BindAction(IA.Action,ETriggerEvent::Completed,obj,released,IA.ActionTag);
			}
		}
	}
}
