#include "Creative/FortPlaysetItemDefinition.h"

static inline __int64 (*LoadPlaysetOG)(UPlaysetLevelStreamComponent*) = decltype(LoadPlaysetOG)(__int64(InSDKUtils::GetImageBase() + 0x1779060));
void FortPlaysetItemDefinition::LoadPlayset(UFortPlaysetItemDefinition* Playset, AFortVolume* Volume)
{
	auto LevelStreamComponent = (UPlaysetLevelStreamComponent*)Volume->GetComponentByClass(UPlaysetLevelStreamComponent::StaticClass());
	
	LevelStreamComponent->SetPlayset(Playset);
	LoadPlaysetOG(LevelStreamComponent);
}
