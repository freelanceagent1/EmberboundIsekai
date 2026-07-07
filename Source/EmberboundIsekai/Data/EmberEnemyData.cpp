#include "Data/EmberEnemyData.h"

FPrimaryAssetId UEmberEnemyData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("Enemy"), GetFName());
}
