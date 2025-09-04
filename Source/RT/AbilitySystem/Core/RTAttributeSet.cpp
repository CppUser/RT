// Fill out your copyright notice in the Description page of Project Settings.


#include "RTAttributeSet.h"
#include "RTAbilitySystemComponent.h"

URTAttributeSet::URTAttributeSet()
{
}

UWorld* URTAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

URTAbilitySystemComponent* URTAttributeSet::GetASAbilitySystemComponent() const
{
	return Cast<URTAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
