// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "RTPawnData.generated.h"

class URTInputConfig;
class URTAbilitySet;
/**
 * 
 */
UCLASS(BlueprintType,Const)
class RT_API URTPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "RT|Pawn")
	TSubclassOf<APawn> PawnClass = nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "RT|Abilities")
	TArray<TObjectPtr<URTAbilitySet>> AbilitySets;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "RT|Input")
	TObjectPtr<URTInputConfig> InputConfig = nullptr;
};
