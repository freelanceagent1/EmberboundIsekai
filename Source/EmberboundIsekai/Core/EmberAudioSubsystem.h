#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EmberAudioSubsystem.generated.h"

UCLASS()
class EMBERBOUNDISEKAI_API UEmberAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void PlaySfx(FName SfxId, UObject* WorldContextObject);
	void SetMasterVolume(float Volume);
	float GetMasterVolume() const;

private:
	FDelegateHandle SetVolumeCommandHandle;

	void RegisterDebugHooks();
	void UnregisterDebugHooks();
	void SetVolumeConsole(const TArray<FString>& Args);
};
