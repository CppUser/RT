// Copyright Epic Games, Inc. All Rights Reserved.

#include "RTGameMode.h"

#include "RTGameState.h"
#include "RTWorldSettings.h"
#include "Character/RTCharacter.h"
#include "Character/RTPawnData.h"
#include "Character/Components/RTPawnExtComp.h"
#include "Experience/RTExperience.h"
#include "Experience/RTExperienceManagerComp.h"
#include "Player/RTPlayerController.h"
#include "System/RTAssetManager.h"
#include "UI/RTHUD.h"
#include "UObject/ConstructorHelpers.h"




ARTGameMode::ARTGameMode(const FObjectInitializer& ObjInit) : Super(ObjInit)
{
	DefaultPawnClass = ARTCharacter::StaticClass();
	PlayerControllerClass = ARTPlayerController::StaticClass();
	GameStateClass = ARTGameState::StaticClass();
	HUDClass = ARTHUD::StaticClass();
}

const URTPawnData* ARTGameMode::GetPawnDataForController(const AController* InController) const
{
	if (InController != nullptr)
	{
		if (const ARTPlayerController* PC = Cast<ARTPlayerController>(InController))
		{
			if (const URTPawnData* PawnData = PC->GetPawnData<URTPawnData>())
			{
				return PawnData;
			}
		}
	}

	check(GameState);
	URTExperienceManagerComp* ExperienceManager = GameState->FindComponentByClass<URTExperienceManagerComp>();
	check(ExperienceManager);

	if (ExperienceManager->IsExperienceLoaded())
	{
		const URTExperience* Experience = ExperienceManager->GetCurrentExperienceChecked();
		if (Experience->DefaultPawnData !=nullptr)
		{
			return Experience->DefaultPawnData;
		}

		return URTAssetManager::Get().GetDefaultPawnData();
	}
	
	return nullptr;
}


void ARTGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	GetWorld()->GetTimerManager().SetTimerForNextTick(this,&ThisClass::PrepForExperienceLoading);
}

void ARTGameMode::InitGameState()
{
	Super::InitGameState();

	URTExperienceManagerComp* ExperienceManager = GameState->FindComponentByClass<URTExperienceManagerComp>();
	check(ExperienceManager);
	ExperienceManager->CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate::CreateUObject(this,&ThisClass::OnExperienceLoaded));
}

void ARTGameMode::PrepForExperienceLoading()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceSource;

	//TODO: Currently loading from world settings default experience. impl more options to search experience

	if (!ExperienceId.IsValid())
	{
		if (ARTWorldSettings* WS = Cast<ARTWorldSettings>(GetWorldSettings()))
		{
			ExperienceId = WS->GetDefaultGameplayExperience();
			ExperienceSource = TEXT("WorldSettings");
		}
	}

	URTAssetManager& AssetManager = URTAssetManager::Get();
	FAssetData Dummy;
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId,Dummy))
	{
		UE_LOG(LogTemp,Error,TEXT("EXPERIENCE:Wanted to use %s but couldn't find it, falling back to the default)"), *ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	OnExperienceDataReady(ExperienceId,ExperienceSource);
}

void ARTGameMode::OnExperienceDataReady(FPrimaryAssetId ExperienceId, const FString& ExperienceSource)
{
	if (ExperienceId.IsValid())
	{
		UE_LOG(LogTemp,Log,TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(),*ExperienceSource);

		URTExperienceManagerComp* ExperienceManager = GameState->FindComponentByClass<URTExperienceManagerComp>();
		check(ExperienceManager);
		ExperienceManager->SetCurrentExperience(ExperienceId);
	}
	else
	{
		UE_LOG(LogTemp,Error,TEXT("Failed to identify experience, loading screen will stay up forever"));
	}
}

void ARTGameMode::OnExperienceLoaded(const URTExperience* CurrentExperience)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if ((PC != nullptr) && (PC->GetPawn() == nullptr))
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}
}

bool ARTGameMode::IsExperienceLoaded() const
{
	check(GameState);
	URTExperienceManagerComp* ExperienceManager = GameState->FindComponentByClass<URTExperienceManagerComp>();
	check(ExperienceManager);
	return ExperienceManager->IsExperienceLoaded();
}

UClass* ARTGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const URTPawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}
	
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ARTGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// Never save the default player pawns into a map.
	SpawnInfo.bDeferConstruction = true;

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
		{
			if (URTPawnExtComp* PawnExtComp = URTPawnExtComp::FindPawnExtComponent(SpawnedPawn))
			{
				if (const URTPawnData* PawnData = GetPawnDataForController(NewPlayer))
				{
					PawnExtComp->SetPawnData(PawnData);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(SpawnedPawn));
				}
			}

			SpawnedPawn->FinishSpawning(SpawnTransform);

			return SpawnedPawn;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
	}

	return nullptr;
}

bool ARTGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

void ARTGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

AActor* ARTGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	//TODO: implement UASPlayerSpawningManagerComponent
	// if (URTPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<URTPlayerSpawningManagerComponent>())
	// {
	// 	return PlayerSpawningComponent->ChoosePlayerStart(Player);
	// }
	
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ARTGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	//TODO: implement UASPlayerSpawningManagerComponent
	// if (URTPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<URTPlayerSpawningManagerComponent>())
	// {
	// PlayerSpawningComponent->FinishRestartPlayer(NewPlayer, StartRotation);
	// }
	
	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

bool ARTGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return ControllerCanRestart(Player);
}

bool ARTGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	// Do nothing, we'll wait until PostLogin when we try to spawn the player for real.
	// Doing anything right now is no good, systems like team assignment haven't even occurred yet.
	return true;
}

void ARTGameMode::GenericPlayerInitialization(AController* NewPlayer)
{
	Super::GenericPlayerInitialization(NewPlayer);

	OnGameModePlayerInitialized.Broadcast(this, NewPlayer);
}

void ARTGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);

	// If we tried to spawn a pawn and it failed, lets try again *note* check if there's actually a pawn class
	// before we try this forever.
	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
		{
			// If it's a player don't loop forever, maybe something changed and they can no longer restart if so stop trying.
			if (PlayerCanRestart(NewPC))
			{
				RequestPlayerRestartNextFrame(NewPlayer, false);			
			}
			else
			{
				UE_LOG(LogTemp, Verbose, TEXT("FailedToRestartPlayer(%s) and PlayerCanRestart returned false, so we're not going to try again."), *GetPathNameSafe(NewPlayer));
			}
		}
		else
		{
			RequestPlayerRestartNextFrame(NewPlayer, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("FailedToRestartPlayer(%s) but there's no pawn class so giving up."), *GetPathNameSafe(NewPlayer));
	}
}

bool ARTGameMode::ControllerCanRestart(AController* Controller)
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{	
		if (!Super::PlayerCanRestart_Implementation(PC))
		{
			return false;
		}
	}
	else
	{
		// Bot version of Super::PlayerCanRestart_Implementation
		if ((Controller == nullptr) || Controller->IsPendingKillPending())
		{
			return false;
		}
	}

	//TODO: implement UASPlayerSpawningManagerComponent
	// if (URTPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<URTPlayerSpawningManagerComponent>())
	// {
	// 	return PlayerSpawningComponent->ControllerCanRestart(Controller);
	// }

	return true;
}

void ARTGameMode::RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset)
{
	if (bForceReset && (Controller != nullptr))
	{
		Controller->Reset();
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(PC, &APlayerController::ServerRestartPlayer_Implementation);
	}
	//TODO: implement AASAIController
	// else if (ARTAIController* BotController = Cast<ARTAIController>(Controller))
	// {
	// 	GetWorldTimerManager().SetTimerForNextTick(BotController, &ARTAIController::ServerRestartController);
	// }
}
