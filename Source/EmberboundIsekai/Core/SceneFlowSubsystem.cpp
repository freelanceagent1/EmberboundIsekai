#include "Core/SceneFlowSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "EmberboundIsekai.h"

void USceneFlowSubsystem::TravelToExpedition()
{
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: TravelToExpedition"));
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, TEXT("/Game/Emberbound/Maps/Expedition/L_Expedition"));
	}
}

void USceneFlowSubsystem::TravelToCamp()
{
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: TravelToCampStub"));
}
