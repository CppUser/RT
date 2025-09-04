// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "RTAssetManager.generated.h"

class URTPawnData;

struct FRTBundles
{
	static const FName Equipped;
};
/**
 * 
 */
UCLASS()
class RT_API URTAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:

	static URTAssetManager& Get();

	const URTPawnData* GetDefaultPawnData() const;
protected:
	virtual void StartInitialLoading() override;
};
