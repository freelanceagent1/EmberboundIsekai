#include "Data/BuildingData.h"

FPrimaryAssetId UBuildingData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("Building"), Id);
}
