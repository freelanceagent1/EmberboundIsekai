#include "EmberboundIsekai.h"
#include "Core/EmberCaptureDebug.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogEmberbound);

namespace
{
#if !UE_BUILD_SHIPPING
	UWorld* FindEmberCaptureWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (World && (World->IsGameWorld() || World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
			{
				return World;
			}
		}

		return GEngine->GetWorldContexts().Num() > 0 ? GEngine->GetWorldContexts()[0].World() : nullptr;
	}

#endif
}

class FEmberboundIsekaiModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();

#if !UE_BUILD_SHIPPING
		CaptureAfterCommand = IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("Ember.CaptureAfter"),
			TEXT("Capture a HighResShot after N seconds. Usage: Ember.CaptureAfter <seconds> [showui]"),
			FConsoleCommandWithArgsDelegate::CreateRaw(this, &FEmberboundIsekaiModule::CaptureAfterConsole),
			ECVF_Default);
#endif
	}

	virtual void ShutdownModule() override
	{
#if !UE_BUILD_SHIPPING
		if (CaptureAfterCommand)
		{
			IConsoleManager::Get().UnregisterConsoleObject(CaptureAfterCommand);
			CaptureAfterCommand = nullptr;
		}
#endif

		FDefaultGameModuleImpl::ShutdownModule();
	}

private:
#if !UE_BUILD_SHIPPING
	void CaptureAfterConsole(const TArray<FString>& Args)
	{
		const float DelaySeconds = Args.IsValidIndex(0) ? FMath::Max(0.0f, FCString::Atof(*Args[0])) : 6.0f;
		bool bShowUI = false;
		for (const FString& Arg : Args)
		{
			if (Arg.Equals(TEXT("showui"), ESearchCase::IgnoreCase))
			{
				bShowUI = true;
				break;
			}
		}

		UWorld* World = FindEmberCaptureWorld();
		FEmberCaptureDebug::ScheduleCapture(World, DelaySeconds, bShowUI);
	}

	IConsoleObject* CaptureAfterCommand = nullptr;
#endif
};

IMPLEMENT_PRIMARY_GAME_MODULE(FEmberboundIsekaiModule, EmberboundIsekai, "EmberboundIsekai");
