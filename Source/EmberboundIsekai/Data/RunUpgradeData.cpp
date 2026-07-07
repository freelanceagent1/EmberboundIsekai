#include "Data/RunUpgradeData.h"

FPrimaryAssetId URunUpgradeData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("RunUpgrade"), Id.IsNone() ? GetFName() : Id);
}
