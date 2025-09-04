// Fill out your copyright notice in the Description page of Project Settings.


#include "RTAssetManager.h"
#include "AbilitySystemGlobals.h"

const FName FRTBundles::Equipped("Equipped");

URTAssetManager& URTAssetManager::Get()
{
	check(GEngine);

	if (URTAssetManager* S = Cast<URTAssetManager>(GEngine->AssetManager))
	{
		return *S;
	}

	UE_LOG(LogTemp,Fatal,TEXT("Invalid AssetManagerClass in DefaultEngine.ini. It must be set to ARTssetManager"));

	return *NewObject<URTAssetManager>();
}

const URTPawnData* URTAssetManager::GetDefaultPawnData() const
{
	//TODO: Implement
	return nullptr;
}

void URTAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	UAbilitySystemGlobals::Get().InitGlobalData();
}
