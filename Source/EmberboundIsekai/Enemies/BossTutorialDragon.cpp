#include "Enemies/BossTutorialDragon.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/EmberProjectile.h"
#include "Combat/HealthComponent.h"
#include "Core/EmberAudioSubsystem.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Enemies/BossTelegraphActor.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

ABossTutorialDragon::ABossTutorialDragon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABossTutorialDragon::BeginPlay()
{
	Super::BeginPlay();
	ConfigureRuntimeStats(450.0f, 20.0f, 165.0f, FVector(1.8f, 1.8f, 1.2f), FLinearColor(0.95f, 0.18f, 0.08f));
	if (UEmberAudioSubsystem* Audio = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEmberAudioSubsystem>() : nullptr)
	{
		Audio->PlaySfx(TEXT("boss-roar"), this);
	}
	if (UHealthComponent* Health = GetHealthComponent())
	{
		Health->OnDamaged.AddDynamic(this, &ABossTutorialDragon::HandleBossDamaged);
		Health->OnDied.AddDynamic(this, &ABossTutorialDragon::HandleBossDied);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PatternTimerHandle, this, &ABossTutorialDragon::CastNextPattern, 1.2f, false, 1.2f);
	}
}

void ABossTutorialDragon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (DashTimeRemaining > 0.0f && IsAlive())
	{
		const float DashSpeed = bEnraged ? 1350.0f : 1000.0f;
		AddActorWorldOffset(DashDirection * DashSpeed * DeltaSeconds, true);
		DashTimeRemaining -= DeltaSeconds;
		DamagePlayerDuringDash();
	}
}

void ABossTutorialDragon::DamageBoss(float Amount)
{
	if (UHealthComponent* Health = GetHealthComponent())
	{
		Health->ApplyDamage(Amount);
	}
}

float ABossTutorialDragon::GetHealthPercent() const
{
	if (const UHealthComponent* Health = GetHealthComponent())
	{
		return Health->GetMaxHP() > 0.0f ? Health->GetCurrentHP() / Health->GetMaxHP() : 0.0f;
	}
	return 0.0f;
}

void ABossTutorialDragon::CastNextPattern()
{
	if (!IsAlive() || !GetWorld())
	{
		return;
	}
	switch (NextPattern)
	{
	case EBossPattern::FireballVolley:
		CastFireballVolley();
		NextPattern = EBossPattern::DashCharge;
		break;
	case EBossPattern::DashCharge:
		CastDashCharge();
		NextPattern = EBossPattern::FlameRing;
		return;
	case EBossPattern::FlameRing:
	default:
		CastFlameRing();
		NextPattern = EBossPattern::FireballVolley;
		break;
	}
	GetWorld()->GetTimerManager().SetTimer(PatternTimerHandle, this, &ABossTutorialDragon::CastNextPattern, 1.2f, false);
}

void ABossTutorialDragon::CastFireballVolley()
{
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: BossPattern FireballVolley"));
	AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!PlayerActor)
	{
		return;
	}
	const FVector BaseDirection = (PlayerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	const int32 Count = bEnraged ? 7 : 5;
	const float StepDegrees = 10.0f;
	const float FirstOffset = -StepDegrees * static_cast<float>(Count - 1) * 0.5f;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		FireProjectile(BaseDirection.RotateAngleAxis(FirstOffset + StepDegrees * Index, FVector::UpVector), 12.0f, 850.0f);
	}
}

void ABossTutorialDragon::CastDashCharge()
{
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: BossPattern DashCharge"));
	AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!PlayerActor || !GetWorld())
	{
		return;
	}
	DashDirection = (PlayerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (DashDirection.IsNearlyZero())
	{
		DashDirection = GetActorForwardVector();
	}
	const float Length = 900.0f;
	const FVector Center = GetActorLocation() + DashDirection * (Length * 0.5f);
	if (ABossTelegraphActor* Telegraph = GetWorld()->SpawnActor<ABossTelegraphActor>(ABossTelegraphActor::StaticClass(), Center, DashDirection.Rotation()))
	{
		Telegraph->Configure(Center, DashDirection.Rotation(), Length, 150.0f, 1.0f);
	}
	GetWorld()->GetTimerManager().SetTimer(DashTelegraphTimerHandle, this, &ABossTutorialDragon::BeginDash, 1.0f, false);
}

void ABossTutorialDragon::BeginDash()
{
	DashTimeRemaining = 0.45f;
	DashDamageCooldownUntil = 0.0f;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PatternTimerHandle, this, &ABossTutorialDragon::CastNextPattern, 1.2f, false);
	}
}

void ABossTutorialDragon::CastFlameRing()
{
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: BossPattern FlameRing"));
	const int32 Count = bEnraged ? 16 : 12;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float Angle = 360.0f * static_cast<float>(Index) / static_cast<float>(Count);
		FireProjectile(FVector::ForwardVector.RotateAngleAxis(Angle, FVector::UpVector), 12.0f, 760.0f);
	}
}

void ABossTutorialDragon::FireProjectile(const FVector& Direction, float Damage, float Speed)
{
	if (!GetWorld())
	{
		return;
	}
	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		const FVector SpawnLocation = GetActorLocation() + Direction.GetSafeNormal2D() * 85.0f + FVector(0.0f, 0.0f, 55.0f);
		if (AEmberProjectile* Projectile = Pool->Acquire<AEmberProjectile>(AEmberProjectile::StaticClass(), FTransform(Direction.Rotation(), SpawnLocation)))
		{
			Projectile->Fire(Direction, Damage, Speed, 0, 0, this, true);
		}
	}
}

void ABossTutorialDragon::CheckEnrage()
{
	if (bEnraged || GetHealthPercent() >= 0.3f)
	{
		return;
	}
	bEnraged = true;
	RuntimeMoveSpeed *= 1.35f;
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: BossEnraged"));
}

void ABossTutorialDragon::DamagePlayerDuringDash()
{
	AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!PlayerActor || !GetWorld())
	{
		return;
	}
	if (FVector::DistSquared2D(PlayerActor->GetActorLocation(), GetActorLocation()) > FMath::Square(125.0f))
	{
		return;
	}
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now < DashDamageCooldownUntil)
	{
		return;
	}
	DashDamageCooldownUntil = Now + 0.5f;
	if (UHealthComponent* Health = PlayerActor->FindComponentByClass<UHealthComponent>())
	{
		Health->ApplyDamage(18.0f);
	}
}

void ABossTutorialDragon::HandleBossDamaged(float, float)
{
	CheckEnrage();
}

void ABossTutorialDragon::HandleBossDied(AActor*)
{
	if (bDefeatBroadcast)
	{
		return;
	}
	bDefeatBroadcast = true;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PatternTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(DashTelegraphTimerHandle);
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: BossDefeated"));
	OnBossDefeated.Broadcast(this);
}
