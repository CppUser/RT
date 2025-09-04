// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RTExperience.generated.h"

class URTPawnData;
/**
 * 
 */
UCLASS()
class RT_API URTExperience : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif

public:
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const URTPawnData> DefaultPawnData;
};
