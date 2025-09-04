// Fill out your copyright notice in the Description page of Project Settings.


#include "RTInputConfig.h"

const UInputAction* URTInputConfig::FindNativeInputAction(const FGameplayTag& tag) const
{
	for (const FRTInputAction& IA : NativeActions)
	{
		if (IA.Action && (IA.ActionTag == tag))
		{
			return IA.Action;
		}
	}
	return nullptr;
}

const UInputAction* URTInputConfig::FindAbilityInputAction(const FGameplayTag& tag) const
{
	for (const FRTInputAction& IA : AbilityActions)
	{
		if (IA.Action && (IA.ActionTag == tag))
		{
			return IA.Action;
		}
	}
	return nullptr;
}
