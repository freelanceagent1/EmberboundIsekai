#include "Run/RunUpgradeManager.h"
#include "Combat/AutoAttackComponent.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/HealthComponent.h"
#include "Combat/XPComponent.h"
#include "Core/EconomySubsystem.h"
#include "Run/EmberBoneShieldOrbiter.h"
#include "Run/EmberCompanionSlime.h"
#include "Run/EmberSoulFlamePatch.h"
#include "UI/UpgradeChoiceWidget.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

static TAutoConsoleVariable<int32> CVarEmberAutoPickUpgrades(
	TEXT("Ember.AutoPickUpgrades"),
	0,
	TEXT("Auto-select a random visible upgrade card after 0.5 seconds."));

static TAutoConsoleVariable<int32> CVarEmberDebugAutoGrantXP(
	TEXT("Ember.DebugAutoGrantXP"),
	0,
	TEXT("Development smoke hook: grant XP periodically during a run."));

void URunUpgradeManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadCatalog();
	RegisterConsoleHooks();
}

void URunUpgradeManager::Deinitialize()
{
	UnregisterConsoleHooks();
	ResetRun();
	Super::Deinitialize();
}

void URunUpgradeManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	TryBindPlayer();
	InWorld.GetTimerManager().SetTimer(PlayerBindTimerHandle, this, &URunUpgradeManager::TryBindPlayer, 0.25f, true);
	StartDebugXPGrant();
}

void URunUpgradeManager::LoadCatalog()
{
	UpgradeCatalog.Reset();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().ScanPathsSynchronous({ TEXT("/Game/Emberbound/Data/Upgrades") }, true);
	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssetsByPath(TEXT("/Game/Emberbound/Data/Upgrades"), Assets, true);
	for (const FAssetData& Asset : Assets)
	{
		if (Asset.GetClass()->IsChildOf(URunUpgradeData::StaticClass()))
		{
			if (URunUpgradeData* Data = Cast<URunUpgradeData>(Asset.GetAsset()))
			{
				UpgradeCatalog.Add(Data);
			}
		}
	}
	if (UpgradeCatalog.Num() == 0)
	{
		UE_LOG(LogEmberbound, Warning, TEXT("EMBER: UpgradeCatalogEmpty"));
	}
	else
	{
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: UpgradeCatalogLoaded count=%d"), UpgradeCatalog.Num());
	}
}

void URunUpgradeManager::TryBindPlayer()
{
	if (!GetWorld())
	{
		return;
	}
	if (CVarEmberDebugAutoGrantXP.GetValueOnGameThread() != 0 && !GetWorld()->GetTimerManager().IsTimerActive(DebugXPGrantTimerHandle))
	{
		StartDebugXPGrant();
	}
	AEmberPlayerCharacter* FoundPlayer = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!FoundPlayer || FoundPlayer == Player)
	{
		return;
	}
	Player = FoundPlayer;
	if (UXPComponent* XP = Player->GetXPComponent())
	{
		XP->OnLevelUp.AddDynamic(this, &URunUpgradeManager::HandleLevelUp);
	}
}

void URunUpgradeManager::HandleLevelUp(int32)
{
	++PendingLevelUps;
	ShowNextOffer();
}

TArray<URunUpgradeData*> URunUpgradeManager::RollOffer(int32 Count) const
{
	TArray<URunUpgradeData*> Result;
	TArray<URunUpgradeData*> Candidates;
	for (URunUpgradeData* Upgrade : UpgradeCatalog)
	{
		if (CanOffer(Upgrade))
		{
			Candidates.Add(Upgrade);
		}
	}
	while (Result.Num() < Count && Candidates.Num() > 0)
	{
		float TotalWeight = 0.0f;
		for (URunUpgradeData* Candidate : Candidates)
		{
			TotalWeight += GetRarityWeight(Candidate->Rarity);
		}
		float Roll = FMath::FRandRange(0.0f, TotalWeight);
		int32 PickedIndex = 0;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			Roll -= GetRarityWeight(Candidates[Index]->Rarity);
			if (Roll <= 0.0f)
			{
				PickedIndex = Index;
				break;
			}
		}
		Result.Add(Candidates[PickedIndex]);
		Candidates.RemoveAtSwap(PickedIndex);
	}
	return Result;
}

void URunUpgradeManager::ShowNextOffer()
{
	if (ActiveChoiceWidget || PendingLevelUps <= 0 || !GetWorld())
	{
		return;
	}
	TryBindPlayer();
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}
	TArray<URunUpgradeData*> Offer = RollOffer(3);
	if (Offer.Num() == 0)
	{
		--PendingLevelUps;
		return;
	}
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	PC->SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	PC->SetInputMode(InputMode);
	ActiveChoiceWidget = CreateWidget<UUpgradeChoiceWidget>(PC, UUpgradeChoiceWidget::StaticClass());
	if (ActiveChoiceWidget)
	{
		ActiveChoiceWidget->SetManager(this);
		ActiveChoiceWidget->SetOffer(Offer);
		ActiveChoiceWidget->AddToViewport(100);
	}
	if (CVarEmberAutoPickUpgrades.GetValueOnGameThread() != 0)
	{
		AutoPickActiveOffer();
	}
}

void URunUpgradeManager::AutoPickActiveOffer()
{
	if (ActiveChoiceWidget)
	{
		ActiveChoiceWidget->ChooseIndex(FMath::RandRange(0, ActiveChoiceWidget->GetOfferCount() - 1));
	}
}

void URunUpgradeManager::ApplyUpgrade(FName UpgradeId)
{
	for (URunUpgradeData* Upgrade : UpgradeCatalog)
	{
		if (Upgrade && Upgrade->Id == UpgradeId)
		{
			ApplyUpgrade(Upgrade);
			return;
		}
	}
}

void URunUpgradeManager::ApplyUpgrade(URunUpgradeData* Upgrade)
{
	if (!Upgrade || !CanOffer(Upgrade))
	{
		return;
	}
	TryBindPlayer();
	const int32 NewStack = GetStack(Upgrade->Id) + 1;
	Stacks.Add(Upgrade->Id, NewStack);
	UAutoAttackComponent* AutoAttack = Player ? Player->FindComponentByClass<UAutoAttackComponent>() : nullptr;
	switch (Upgrade->EffectType)
	{
	case ERunUpgradeEffectType::SplitShot:
		if (AutoAttack) { AutoAttack->SplitCount += FMath::RoundToInt(Upgrade->FloatParams.FindRef(TEXT("count"))); }
		break;
	case ERunUpgradeEffectType::PiercingShot:
		if (AutoAttack) { AutoAttack->PierceCount += FMath::RoundToInt(Upgrade->FloatParams.FindRef(TEXT("pierce"))); }
		break;
	case ERunUpgradeEffectType::RicochetShot:
		if (AutoAttack) { AutoAttack->RicochetCount += FMath::RoundToInt(Upgrade->FloatParams.FindRef(TEXT("ricochet"))); }
		break;
	case ERunUpgradeEffectType::RapidFire:
		if (AutoAttack) { AutoAttack->ApplyAttackSpeedMultiplier(Upgrade->FloatParams.FindRef(TEXT("multiplier"))); }
		break;
	case ERunUpgradeEffectType::HeavyArrow:
		if (AutoAttack) { AutoAttack->ApplyDamageMultiplier(Upgrade->FloatParams.FindRef(TEXT("multiplier"))); }
		break;
	case ERunUpgradeEffectType::FireArrow:
		ProjectileStatusPayload.BurnDps = FMath::Max(ProjectileStatusPayload.BurnDps, Upgrade->FloatParams.FindRef(TEXT("dps")));
		ProjectileStatusPayload.BurnDuration = FMath::Max(ProjectileStatusPayload.BurnDuration, Upgrade->FloatParams.FindRef(TEXT("duration")));
		break;
	case ERunUpgradeEffectType::PoisonArrow:
		ProjectileStatusPayload.PoisonDps += Upgrade->FloatParams.FindRef(TEXT("dps"));
		ProjectileStatusPayload.PoisonDuration = FMath::Max(ProjectileStatusPayload.PoisonDuration, Upgrade->FloatParams.FindRef(TEXT("duration")));
		break;
	case ERunUpgradeEffectType::FrostArrow:
		ProjectileStatusPayload.FrostSlowFraction = FMath::Max(ProjectileStatusPayload.FrostSlowFraction, Upgrade->FloatParams.FindRef(TEXT("slow")));
		ProjectileStatusPayload.FrostDuration = FMath::Max(ProjectileStatusPayload.FrostDuration, Upgrade->FloatParams.FindRef(TEXT("duration")));
		break;
	case ERunUpgradeEffectType::BoneShield:
		SpawnBoneShield();
		break;
	case ERunUpgradeEffectType::HealingWisp:
		if (GetWorld()) { GetWorld()->GetTimerManager().SetTimer(HealingWispTimerHandle, this, &URunUpgradeManager::HealFromWisp, 3.0f, true, 3.0f); }
		break;
	case ERunUpgradeEffectType::PickupMagnet:
		if (Player) { Player->ApplyPickupRadiusMultiplier(Upgrade->FloatParams.FindRef(TEXT("multiplier"))); }
		break;
	case ERunUpgradeEffectType::CritFocus:
		if (AutoAttack) { AutoAttack->CritChance += Upgrade->FloatParams.FindRef(TEXT("crit")); }
		break;
	case ERunUpgradeEffectType::CompanionSlime:
		SpawnCompanionSlime();
		break;
	case ERunUpgradeEffectType::SoulFlameTrail:
		if (GetWorld()) { GetWorld()->GetTimerManager().SetTimer(SoulTrailTimerHandle, this, &URunUpgradeManager::DropSoulFlamePatch, 0.3f, true, 0.3f); }
		break;
	case ERunUpgradeEffectType::GoldRush:
		GoldMultiplier += Upgrade->FloatParams.FindRef(TEXT("bonus"));
		break;
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: UpgradeApplied %s stack=%d"), *Upgrade->Id.ToString(), NewStack);
}

void URunUpgradeManager::NotifyChoiceClosed()
{
	ActiveChoiceWidget = nullptr;
	PendingLevelUps = FMath::Max(0, PendingLevelUps - 1);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoPickTimerHandle);
		UGameplayStatics::SetGamePaused(World, false);
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
	ShowNextOffer();
}

void URunUpgradeManager::ResetRun()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PlayerBindTimerHandle);
		World->GetTimerManager().ClearTimer(HealingWispTimerHandle);
		World->GetTimerManager().ClearTimer(SoulTrailTimerHandle);
		World->GetTimerManager().ClearTimer(AutoPickTimerHandle);
		World->GetTimerManager().ClearTimer(DebugXPGrantTimerHandle);
	}
	for (AEmberBoneShieldOrbiter* Shield : BoneShields)
	{
		if (Shield) { Shield->Destroy(); }
	}
	for (AEmberSoulFlamePatch* Patch : SoulFlamePatches)
	{
		if (Patch) { Patch->Destroy(); }
	}
	if (CompanionSlime) { CompanionSlime->Destroy(); }
	if (ActiveChoiceWidget) { ActiveChoiceWidget->RemoveFromParent(); }
	BoneShields.Reset();
	SoulFlamePatches.Reset();
	CompanionSlime = nullptr;
	ActiveChoiceWidget = nullptr;
	Stacks.Reset();
	ProjectileStatusPayload = FEmberProjectileStatusPayload();
	PendingLevelUps = 0;
	GoldMultiplier = 1.0f;
}

void URunUpgradeManager::RegisterConsoleHooks()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Ember.GrantXP"),
		TEXT("Grant XP to the player. Usage: Ember.GrantXP 25"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &URunUpgradeManager::GrantXPConsole),
		ECVF_Default);
}

void URunUpgradeManager::UnregisterConsoleHooks()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Ember.GrantXP"), false);
}

void URunUpgradeManager::GrantXPConsole(const TArray<FString>& Args)
{
	const int32 Amount = Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 10;
	GrantXPToPlayer(Amount);
}

void URunUpgradeManager::GrantXPToPlayer(int32 Amount)
{
	TryBindPlayer();
	if (Player && Player->GetXPComponent())
	{
		Player->GetXPComponent()->AddXP(Amount);
	}
}

void URunUpgradeManager::StartDebugXPGrant()
{
	if (!GetWorld() || CVarEmberDebugAutoGrantXP.GetValueOnGameThread() == 0)
	{
		return;
	}
	if (GetWorld()->GetTimerManager().IsTimerActive(DebugXPGrantTimerHandle))
	{
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(DebugXPGrantTimerHandle, FTimerDelegate::CreateUObject(this, &URunUpgradeManager::GrantXPToPlayer, 20), 1.25f, true, 1.0f);
}

void URunUpgradeManager::HealFromWisp()
{
	TryBindPlayer();
	if (Player && Player->GetHealthComponent())
	{
		Player->GetHealthComponent()->Heal(2.0f);
	}
}

void URunUpgradeManager::DropSoulFlamePatch()
{
	TryBindPlayer();
	if (!Player || !GetWorld())
	{
		return;
	}
	AEmberSoulFlamePatch* Patch = GetWorld()->SpawnActor<AEmberSoulFlamePatch>(AEmberSoulFlamePatch::StaticClass(), Player->GetActorLocation(), FRotator::ZeroRotator);
	if (Patch)
	{
		SoulFlamePatches.Add(Patch);
	}
}

int32 URunUpgradeManager::GetStack(FName UpgradeId) const
{
	if (const int32* Found = Stacks.Find(UpgradeId))
	{
		return *Found;
	}
	return 0;
}

bool URunUpgradeManager::CanOffer(const URunUpgradeData* Upgrade) const
{
	return Upgrade && GetStack(Upgrade->Id) < Upgrade->MaxStacks;
}

float URunUpgradeManager::GetRarityWeight(ERunUpgradeRarity Rarity) const
{
	switch (Rarity)
	{
	case ERunUpgradeRarity::Rare:
		return 30.0f;
	case ERunUpgradeRarity::Epic:
		return 10.0f;
	default:
		return 60.0f;
	}
}

void URunUpgradeManager::SpawnBoneShield()
{
	TryBindPlayer();
	if (!Player || !GetWorld())
	{
		return;
	}
	AEmberBoneShieldOrbiter* Shield = GetWorld()->SpawnActor<AEmberBoneShieldOrbiter>(AEmberBoneShieldOrbiter::StaticClass(), Player->GetActorLocation(), FRotator::ZeroRotator);
	if (Shield)
	{
		Shield->AttachToPlayer(Player, BoneShields.Num());
		BoneShields.Add(Shield);
	}
}

void URunUpgradeManager::SpawnCompanionSlime()
{
	TryBindPlayer();
	if (!Player || !GetWorld() || CompanionSlime)
	{
		return;
	}
	CompanionSlime = GetWorld()->SpawnActor<AEmberCompanionSlime>(AEmberCompanionSlime::StaticClass(), Player->GetActorLocation() + FVector(120.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	CompanionSlime->SetPlayer(Player);
}
