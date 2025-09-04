// Fill out your copyright notice in the Description page of Project Settings.


#include "RTInputComponent.h"

void URTInputComponent::AddInputMappings(const URTInputConfig* InputConfig,
	UEnhancedInputLocalPlayerSubsystem* InInputSubsystem) const
{
	check(InputConfig);
	check(InInputSubsystem);

	InInputSubsystem->AddMappingContext(InputConfig->DefaultMapping.Context,InputConfig->DefaultMapping.Priority);
}

void URTInputComponent::RemoveInputMappings(const URTInputConfig* InputConfig,
	UEnhancedInputLocalPlayerSubsystem* InInputSubsystem)
{
	check(InputConfig);
	check(InInputSubsystem);

	InInputSubsystem->RemoveMappingContext(InputConfig->DefaultMapping.Context);
}

void URTInputComponent::RemoveBinds(TArray<uint32_t>& Handles)
{
	//TODO: Implement me
}
