#include "Data/MonsterData.h"

FPrimaryAssetId UMonsterData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("Monster"), Id);
}

bool UMonsterData::AllowsRole(EMonsterAssignmentRole Role) const
{
	switch (Role)
	{
	case EMonsterAssignmentRole::Companion:
		return (AllowedRoles & static_cast<int32>(EMonsterRoleFlags::Companion)) != 0;
	case EMonsterAssignmentRole::Worker:
		return (AllowedRoles & static_cast<int32>(EMonsterRoleFlags::Worker)) != 0;
	case EMonsterAssignmentRole::Defender:
		return (AllowedRoles & static_cast<int32>(EMonsterRoleFlags::Defender)) != 0;
	default:
		return true;
	}
}
