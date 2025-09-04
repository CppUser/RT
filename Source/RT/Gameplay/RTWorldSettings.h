// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "RTWorldSettings.generated.h"

class URTExperience;
/**
 * 
 */
UCLASS()
class RT_API ARTWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
public:
	ARTWorldSettings(const FObjectInitializer& ObjInit);

	FPrimaryAssetId GetDefaultGameplayExperience() const;
	
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif

public:

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category=PIE)
	bool ForceStandaloneNetMode = false;
#endif

protected:
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<URTExperience> DefaultGameplayExperience;
};
