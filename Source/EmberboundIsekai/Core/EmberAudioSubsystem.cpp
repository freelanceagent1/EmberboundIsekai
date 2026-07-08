#include "Core/EmberAudioSubsystem.h"
#include "Core/EmberSaveGame.h"
#include "Core/SaveManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "Subsystems/SubsystemCollection.h"
#include "EmberboundIsekai.h"

void UEmberAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<USaveManagerSubsystem>();
	RegisterDebugHooks();
}

void UEmberAudioSubsystem::Deinitialize()
{
	UnregisterDebugHooks();
	Super::Deinitialize();
}

void UEmberAudioSubsystem::PlaySfx(FName SfxId, UObject* WorldContextObject)
{
	if (SfxId.IsNone() || GetMasterVolume() <= 0.0f)
	{
		return;
	}
	const FString AssetName = FString::Printf(TEXT("SFX_%s"), *SfxId.ToString().Replace(TEXT("-"), TEXT("_")));
	const FString Path = FString::Printf(TEXT("/Game/Emberbound/Generated/SFX/%s.%s"), *AssetName, *AssetName);
	if (USoundWave* Sound = LoadObject<USoundWave>(nullptr, *Path))
	{
		UGameplayStatics::PlaySound2D(WorldContextObject, Sound, GetMasterVolume());
	}
}

void UEmberAudioSubsystem::SetMasterVolume(float Volume)
{
	USaveManagerSubsystem* SaveManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	if (!SaveManager || !SaveManager->GetCurrentSave())
	{
		return;
	}
	SaveManager->GetCurrentSave()->MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	SaveManager->SaveNow();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: MasterVolume %.2f"), SaveManager->GetCurrentSave()->MasterVolume);
}

float UEmberAudioSubsystem::GetMasterVolume() const
{
	const USaveManagerSubsystem* SaveManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	return SaveManager && SaveManager->GetCurrentSave() ? SaveManager->GetCurrentSave()->MasterVolume : 1.0f;
}

void UEmberAudioSubsystem::RegisterDebugHooks()
{
	IConsoleManager::Get().RegisterConsoleCommand(TEXT("Ember.SetMasterVolume"), TEXT("Set master SFX volume 0..1."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEmberAudioSubsystem::SetVolumeConsole), ECVF_Default);
}

void UEmberAudioSubsystem::UnregisterDebugHooks()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Ember.SetMasterVolume"), false);
}

void UEmberAudioSubsystem::SetVolumeConsole(const TArray<FString>& Args)
{
	SetMasterVolume(Args.IsValidIndex(0) ? FCString::Atof(*Args[0]) : 1.0f);
}
