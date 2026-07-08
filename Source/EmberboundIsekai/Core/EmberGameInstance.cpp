#include "Core/EmberGameInstance.h"
#include "Core/SaveManagerSubsystem.h"
#include "EmberboundIsekai.h"

void UEmberGameInstance::Init()
{
	Super::Init();
	GetSubsystem<USaveManagerSubsystem>();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: GameInstanceInit"));
}
