#include "Core/EconomySubsystem.h"

void UEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	for (EEmberResource Resource : { EEmberResource::Gold, EEmberResource::Wood, EEmberResource::SoulAsh, EEmberResource::MonsterEssence, EEmberResource::Embers })
	{
		Wallet.Add(Resource, 0);
	}
}

void UEconomySubsystem::Add(EEmberResource Resource, int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	const int32 NewAmount = Get(Resource) + Amount;
	Wallet.Add(Resource, NewAmount);
	OnResourceChanged.Broadcast(Resource, NewAmount);
}

bool UEconomySubsystem::Spend(EEmberResource Resource, int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}
	if (Get(Resource) < Amount)
	{
		return false;
	}
	const int32 NewAmount = Get(Resource) - Amount;
	Wallet.Add(Resource, NewAmount);
	OnResourceChanged.Broadcast(Resource, NewAmount);
	return true;
}

int32 UEconomySubsystem::Get(EEmberResource Resource) const
{
	if (const int32* Amount = Wallet.Find(Resource))
	{
		return *Amount;
	}
	return 0;
}

void UEconomySubsystem::Set(EEmberResource Resource, int32 Amount)
{
	const int32 NewAmount = FMath::Max(0, Amount);
	Wallet.Add(Resource, NewAmount);
	OnResourceChanged.Broadcast(Resource, NewAmount);
}
