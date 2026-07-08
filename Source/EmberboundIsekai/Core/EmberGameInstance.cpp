#include "Core/EmberGameInstance.h"
#include "Core/SaveManagerSubsystem.h"
#include "UI/EmberSplashWidget.h"
#include "Kismet/GameplayStatics.h"
#include "EmberboundIsekai.h"

void UEmberGameInstance::Init()
{
	Super::Init();
	GetSubsystem<USaveManagerSubsystem>();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: GameInstanceInit"));
}

void UEmberGameInstance::OnStart()
{
	Super::OnStart();
	if (bSplashShown)
	{
		return;
	}
	bSplashShown = true;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (UEmberSplashWidget* Splash = CreateWidget<UEmberSplashWidget>(PC, UEmberSplashWidget::StaticClass()))
		{
			Splash->AddToViewport(1000);
		}
	}
}
