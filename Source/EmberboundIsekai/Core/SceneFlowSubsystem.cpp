#include "Core/SceneFlowSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "EmberboundIsekai.h"

void USceneFlowSubsystem::TravelToExpedition()
{
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: TravelToExpedition"));
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, TEXT("/Game/Emberbound/Maps/Expedition/L_Expedition"), true, TEXT("game=/Script/EmberboundIsekai.ExpeditionGameMode"));
	}
}

void USceneFlowSubsystem::TravelToCamp()
{
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: TravelToCamp"));
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, TEXT("/Game/Emberbound/Maps/Camp/L_Camp"), true, TEXT("game=/Script/EmberboundIsekai.CampGameMode"));
	}
}
