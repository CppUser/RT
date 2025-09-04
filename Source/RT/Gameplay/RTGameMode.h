// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RTGameMode.generated.h"

class URTPawnData;
class URTExperience;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);


UCLASS(minimalapi)
class ARTGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARTGameMode(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual void InitGameState() override;
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	virtual void GenericPlayerInitialization(AController* NewPlayer) override;
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;

	virtual bool ControllerCanRestart(AController* Controller);

	UFUNCTION(BlueprintCallable)
	void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);


	// Delegate called on player initialization, described above 
	FOnGameModePlayerInitialized OnGameModePlayerInitialized;

	UFUNCTION(BlueprintCallable,Category = "RT|Pawn")
	const URTPawnData* GetPawnDataForController(const AController* InController) const;
	
private:
	void PrepForExperienceLoading();
	void OnExperienceDataReady(FPrimaryAssetId ExperienceId,const FString& ExperienceSource);

	void OnExperienceLoaded(const URTExperience* CurrentExperience);
	bool IsExperienceLoaded() const;

private:
	TSet<TWeakObjectPtr<APlayerController>> InitializingPlayers;
	bool bExperienceLoaded = false;

	FTimerHandle DebugTimerHandle;
	FTimerHandle ProcessedControllersTimerHandle;
	FTimerHandle FailedRestartTimerHandle;
};



