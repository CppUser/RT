// Fill out your copyright notice in the Description page of Project Settings.


#include "RTExperienceManagerComp.h"

#include "RTExperience.h"
#include "RTExperienceManager.h"
#include "System/RTAssetManager.h"

//@TODO: Async load the experience definition itself
//@TODO: Handle failures explicitly (go into a 'completed but failed' state rather than check()-ing)
//@TODO: Do the action phases at the appropriate times instead of all at once
//@TODO: Support deactivating an experience and do the unloading actions
//@TODO: Think about what deactivation/cleanup means for preloaded assets
//@TODO: Handle deactivating game features, right now we 'leak' them enabled
// (for a client moving from experience to experience we actually want to diff the requirements and only unload some, not unload everything for them to just be immediately reloaded)
//@TODO: Handle both built-in and URL-based plugins (search for colon?)

URTExperienceManagerComp::URTExperienceManagerComp(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void URTExperienceManagerComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);


	//@TODO: Ensure proper handling of a partially-loaded state too
	if (LoadState == ERTExperienceLoadState::Loaded)
	{
		LoadState = ERTExperienceLoadState::Deactivating;

		NumExpectedPausers = INDEX_NONE;
		NumObservedPausers = 0;
	}
}

void URTExperienceManagerComp::SetCurrentExperience(FPrimaryAssetId ExperienceID)
{
	URTAssetManager& AssetManager = URTAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceID);
	TSubclassOf<URTExperience> AssetClass = Cast<UClass>(AssetPath.TryLoad());
	check(AssetClass);
	const URTExperience* Experience = GetDefault<URTExperience>(AssetClass);

	check(Experience != nullptr);
	check(CurrentExperience == nullptr);
	CurrentExperience = Experience;
	StartExperienceLoad();
}

const URTExperience* URTExperienceManagerComp::GetCurrentExperienceChecked() const
{
	check(LoadState == ERTExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}

bool URTExperienceManagerComp::IsExperienceLoaded() const
{
	return (LoadState == ERTExperienceLoadState::Loaded) && (CurrentExperience != nullptr);
}

void URTExperienceManagerComp::CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(Delegate));
	}
}

void URTExperienceManagerComp::CallOrRegister_OnExperienceLoaded_LowPriority(FOnExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_LowPriority.Add(MoveTemp(Delegate));
	}
}

void URTExperienceManagerComp::CallOrRegister_OnExperienceLoaded_HighPriority(FOnExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_HighPriority.Add(MoveTemp(Delegate));
	}
}

void URTExperienceManagerComp::StartExperienceLoad()
{
	check(CurrentExperience != nullptr);
	check(LoadState == ERTExperienceLoadState::Unloaded);

	UE_LOG(LogTemp, Log, TEXT("EXPERIENCE: StartExperienceLoad(CurrentExperience = %s)"),
		*CurrentExperience->GetPrimaryAssetId().ToString());

	LoadState = ERTExperienceLoadState::Loading;

	URTAssetManager& AssetManager = URTAssetManager::Get();

	TSet<FPrimaryAssetId> BundleAssetList;
	TSet<FSoftObjectPath> RawAssetList;

	TArray<FName> BundlesToLoad;
	BundlesToLoad.Add(FRTBundles::Equipped);

	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList.Array(), BundlesToLoad, {}, false, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);
	}

	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;
	if (RawAssetList.Num() > 0)
	{
		RawLoadHandle = AssetManager.LoadAssetList(RawAssetList.Array(), FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, TEXT("StartExperienceLoad()"));
	}

	TSharedPtr<FStreamableHandle> Handle = nullptr;
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		Handle = AssetManager.GetStreamableManager().CreateCombinedHandle({ BundleLoadHandle, RawLoadHandle });
	}
	else
	{
		Handle = BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
	}

	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnExperienceLoadComplete);
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);

		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
			{
				OnAssetsLoadedDelegate.ExecuteIfBound();
			}));
	}

	TSet<FPrimaryAssetId> PreloadAssetList;
	//@TODO: Determine assets to preload (but not blocking-ly)
	if (PreloadAssetList.Num() > 0)
	{
		AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(), BundlesToLoad, {});
	}
}

void URTExperienceManagerComp::OnExperienceLoadComplete()
{
	check(LoadState == ERTExperienceLoadState::Loading);
	check(CurrentExperience != nullptr);

	UE_LOG(LogTemp, Log, TEXT("EXPERIENCE: OnExperienceLoadComplete(CurrentExperience = %s)"),
		*CurrentExperience->GetPrimaryAssetId().ToString());
	
	OnExperienceFullLoadCompleted();
}

void URTExperienceManagerComp::OnExperienceFullLoadCompleted()
{
	check(LoadState != ERTExperienceLoadState::Loaded);
	
	LoadState = ERTExperienceLoadState::Loaded;

	OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_HighPriority.Clear();

	OnExperienceLoaded.Broadcast(CurrentExperience);
	OnExperienceLoaded.Clear();

	OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_LowPriority.Clear();
}
