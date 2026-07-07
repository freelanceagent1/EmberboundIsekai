#include "Core/EmberGameInstance.h"
#include "EmberboundIsekai.h"

void UEmberGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: GameInstanceInit"));
}
