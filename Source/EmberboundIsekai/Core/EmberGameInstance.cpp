#include "Core/EmberGameInstance.h"
#include "Core/EmberAutopilotSubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "UI/MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "EmberboundIsekai.h"

void UEmberGameInstance::Init()
{
	Super::Init();
	GetSubsystem<USaveManagerSubsystem>();
	GetSubsystem<UEmberAutopilotSubsystem>();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: GameInstanceInit"));
}

void UEmberGameInstance::OnStart()
{
	Super::OnStart();
	if (bSplashShown)
	{
		return;
	}
	if (FParse::Param(FCommandLine::Get(), TEXT("EmberSkipMainMenu")))
	{
		bSplashShown = true;
		bMainMenuActive = false;
		return;
	}
	bSplashShown = true;
	bMainMenuActive = true;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (UMainMenuWidget* Menu = CreateWidget<UMainMenuWidget>(PC, UMainMenuWidget::StaticClass()))
		{
			Menu->AddToViewport(1000);
			PC->SetShowMouseCursor(true);
			PC->SetInputMode(FInputModeGameAndUI());
		}
	}
}

bool UEmberGameInstance::WillShowInitialMainMenu() const
{
	return !bSplashShown && !FParse::Param(FCommandLine::Get(), TEXT("EmberSkipMainMenu"));
}
