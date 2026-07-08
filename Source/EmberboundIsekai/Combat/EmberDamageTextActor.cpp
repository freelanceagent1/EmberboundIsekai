#include "Combat/EmberDamageTextActor.h"
#include "Components/TextRenderComponent.h"

AEmberDamageTextActor::AEmberDamageTextActor()
{
	PrimaryActorTick.bCanEverTick = true;

	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DamageText"));
	SetRootComponent(TextComponent);
	TextComponent->SetHorizontalAlignment(EHTA_Center);
	TextComponent->SetVerticalAlignment(EVRTA_TextCenter);
	TextComponent->SetWorldSize(32.0f);
	TextComponent->SetTextRenderColor(FColor::White);
	TextComponent->SetCastShadow(false);
	SetActorEnableCollision(false);
}

void AEmberDamageTextActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Lifetime += DeltaSeconds;
	const float Alpha = FMath::Clamp(Lifetime / MaxLifetime, 0.0f, 1.0f);
	SetActorLocation(BaseLocation + FVector(0.0f, 0.0f, 72.0f * Alpha));

	const FLinearColor Faded = FLinearColor(BaseColor.R, BaseColor.G, BaseColor.B, 1.0f - Alpha);
	TextComponent->SetTextRenderColor(Faded.ToFColor(true));
}

void AEmberDamageTextActor::OnAcquired_Implementation()
{
	Lifetime = 0.0f;
	SetActorTickEnabled(true);
}

void AEmberDamageTextActor::OnReleased_Implementation()
{
	Lifetime = 0.0f;
	SetActorTickEnabled(false);
}

void AEmberDamageTextActor::ShowDamage(float Damage, bool bCritical)
{
	BaseLocation = GetActorLocation();
	BaseColor = bCritical ? FLinearColor(1.0f, 0.26f, 0.08f, 1.0f) : FLinearColor(1.0f, 0.92f, 0.45f, 1.0f);
	TextComponent->SetWorldSize(bCritical ? 44.0f : 32.0f);
	TextComponent->SetText(FText::AsNumber(FMath::Max(1, FMath::RoundToInt(Damage))));
	TextComponent->SetTextRenderColor(BaseColor.ToFColor(true));
}
