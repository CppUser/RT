// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "Engine/DataAsset.h"
#include "RTInputConfig.generated.h"

USTRUCT(BlueprintType)
struct FRTInputMapping
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	UInputMappingContext* Context = nullptr;

	UPROPERTY(EditDefaultsOnly)
	int8 Priority = 0;
};

USTRUCT(BlueprintType)
struct FRTInputAction
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	const UInputAction* Action = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ActionTag;
};


UCLASS()
class RT_API URTInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	const UInputAction* FindNativeInputAction(const FGameplayTag& tag) const;
	const UInputAction* FindAbilityInputAction(const FGameplayTag& tag) const;

public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, meta = (TitleProperty = Context))
	FRTInputMapping DefaultMapping;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, meta = (TitleProperty = InputAction))
	TArray<FRTInputAction> NativeActions;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, meta = (TitleProperty = InputAction))
	TArray<FRTInputAction> AbilityActions;
	
};
