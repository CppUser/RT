// Copyright Epic Games, Inc. All Rights Reserved.

#include "RTGameMode.h"

#include "EngineUtils.h"
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

void ARTGameMode::BeginPlay()
{
	Super::BeginPlay();

#if !UE_BUILD_SHIPPING && WITH_EDITOR
	GetWorld()->GetTimerManager().SetTimer(
		DebugTimerHandle,
		[this]()
		{
			UWorld* World = GetWorld();
			if (!World) return;
            
			int32 PlayerCount = World->GetNumPlayerControllers();
            
			int32 PawnCount = 0;
			for (APawn* Pawn : TActorRange<APawn>(World))
			{
				if (Pawn && Pawn->IsA<ARTCharacter>())
				{
					PawnCount++;
				}
			}
            
			UE_LOG(LogTemp, Log, TEXT("DEBUG: Players: %d, RT Characters: %d"), PlayerCount, PawnCount);
		},
		5.0f, 
		true
	);
#endif
}

void ARTGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DebugTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DebugTimerHandle);
	}
    
	ProcessedControllers.Empty();
	FailedRestartAttempts.Empty();
	InitializingPlayers.Empty();
	
	Super::EndPlay(EndPlayReason);
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
	UE_LOG(LogTemp, Log, TEXT("=== GAME INITIALIZATION START ==="));
	Super::InitGame(MapName, Options, ErrorMessage);
    
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::PrepForExperienceLoading);
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
	UE_LOG(LogTemp, Log, TEXT("Experience loaded, checking players for respawn..."));

	bExperienceLoaded = true;
	InitializingPlayers.Empty();
    
	ProcessedControllers.Empty();
    
	TArray<APlayerController*> ControllersToRestart;
    
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if (!PC || !IsValid(PC))
		{
			continue;
		}
        
		if (ProcessedControllers.Contains(PC))
		{
			continue;
		}
        
		if (!PC->GetPawn() && PlayerCanRestart(PC))
		{
			ControllersToRestart.Add(PC);
			ProcessedControllers.Add(PC);
			UE_LOG(LogTemp, Log, TEXT("Marking player for restart: %s"), *GetNameSafe(PC));
		}
		else if (PC->GetPawn())
		{
			ProcessedControllers.Add(PC);
			UE_LOG(LogTemp, Log, TEXT("Player already has pawn: %s -> %s"), 
				   *GetNameSafe(PC), *GetNameSafe(PC->GetPawn()));
		}
	}
    
	UE_LOG(LogTemp, Log, TEXT("Total players to restart: %d"), ControllersToRestart.Num());
    
	for (APlayerController* PC : ControllersToRestart)
	{
		if (IsValid(PC) && !PC->GetPawn())
		{
			UE_LOG(LogTemp, Log, TEXT("Restarting player: %s"), *GetNameSafe(PC));
			RestartPlayer(PC);
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
	if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
	{
		if (PC->GetPawn())
		{
			UE_LOG(LogTemp, Warning, TEXT("Player %s already has pawn %s, not spawning another"), 
				   *GetNameSafe(PC), *GetNameSafe(PC->GetPawn()));
			return PC->GetPawn();
		}
	}
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = true;

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		UE_LOG(LogTemp, Log, TEXT("Spawning pawn of class %s for controller %s"), 
			   *GetNameSafe(PawnClass), *GetNameSafe(NewPlayer));
               
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
					UE_LOG(LogTemp, Error, TEXT("No PawnData available for controller %s"), *GetNameSafe(NewPlayer));
				}
			}

			SpawnedPawn->FinishSpawning(SpawnTransform);
			UE_LOG(LogTemp, Log, TEXT("Successfully spawned pawn %s"), *GetNameSafe(SpawnedPawn));
			return SpawnedPawn;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to spawn pawn for controller %s"), *GetNameSafe(NewPlayer));
	return nullptr;
}

bool ARTGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

void ARTGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (IsExperienceLoaded() && NewPlayer && !NewPlayer->GetPawn())
	{
		UE_LOG(LogTemp, Log, TEXT("HandleStartingNewPlayer: Starting new player %s"), *GetNameSafe(NewPlayer));
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
	else if (NewPlayer && NewPlayer->GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleStartingNewPlayer: Player %s already has pawn %s, skipping"), 
			*GetNameSafe(NewPlayer), *GetNameSafe(NewPlayer->GetPawn()));
	}
	else if (!IsExperienceLoaded())
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleStartingNewPlayer: Experience not loaded yet for player %s"), *GetNameSafe(NewPlayer));
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
	if (!Player || Player->IsPendingKillPending())
	{
		return false;
	}
	
	if (Player->GetPawn() && !Player->GetPawn()->IsPendingKillPending())
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerCanRestart: Player %s already has valid pawn %s"), 
			*GetNameSafe(Player), *GetNameSafe(Player->GetPawn()));
		return false;
	}
	
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
	int32& Attempts = FailedRestartAttempts.FindOrAdd(NewPlayer);
	Attempts++;
	
	UE_LOG(LogTemp, Error, TEXT("FailedToRestartPlayer: %s (Attempt %d)"), *GetNameSafe(NewPlayer), Attempts);
	
	if (Attempts >= 3)
	{
		UE_LOG(LogTemp, Error, TEXT("FailedToRestartPlayer: Giving up on %s after %d attempts"), *GetNameSafe(NewPlayer), Attempts);
		return;
	}
	
	Super::FailedToRestartPlayer(NewPlayer);

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
		{
			if (PlayerCanRestart(NewPC))
			{
				RequestPlayerRestartNextFrame(NewPlayer, false);
			}
		}
		
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
