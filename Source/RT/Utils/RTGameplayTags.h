#pragma once
#include "NativeGameplayTags.h"

namespace RTGameplayTags
{
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);
	
	
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Mouse_Look);
	
	RT_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
	
}
