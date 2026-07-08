#include "Core/EmberCaptureDebug.h"

#include "EmberboundIsekai.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace
{
	bool bCommandLineCaptureScheduled = false;

	FString LatestScreenshotName()
	{
		TArray<FString> Files;
		IFileManager::Get().FindFilesRecursive(Files, *FPaths::ScreenShotDir(), TEXT("HighresScreenshot*.png"), true, false, false);
		Files.Sort([](const FString& Left, const FString& Right)
		{
			return IFileManager::Get().GetTimeStamp(*Left) > IFileManager::Get().GetTimeStamp(*Right);
		});

		return Files.Num() > 0 ? FPaths::GetCleanFilename(Files[0]) : TEXT("pending");
	}

	bool TryParseCommandLineCapture(float& OutDelaySeconds, bool& bOutShowUI)
	{
		const FString CommandLine(FCommandLine::Get());
		const int32 CommandIndex = CommandLine.Find(TEXT("Ember.CaptureAfter"), ESearchCase::IgnoreCase);
		if (CommandIndex == INDEX_NONE)
		{
			return false;
		}

		FString CaptureArgs = CommandLine.Mid(CommandIndex + FCString::Strlen(TEXT("Ember.CaptureAfter")));
		int32 EndIndex = INDEX_NONE;
		if (CaptureArgs.FindChar(TEXT(','), EndIndex) || CaptureArgs.FindChar(TEXT('"'), EndIndex))
		{
			CaptureArgs = CaptureArgs.Left(EndIndex);
		}

		TArray<FString> Tokens;
		CaptureArgs.ParseIntoArrayWS(Tokens);
		OutDelaySeconds = 6.0f;
		if (Tokens.Num() > 0)
		{
			OutDelaySeconds = FCString::Atof(*Tokens[0]);
		}

		bOutShowUI = false;
		for (const FString& Token : Tokens)
		{
			if (Token.Equals(TEXT("showui"), ESearchCase::IgnoreCase))
			{
				bOutShowUI = true;
				break;
			}
		}

		return true;
	}
}

void FEmberCaptureDebug::ScheduleCapture(UWorld* World, float DelaySeconds, bool bShowUI)
{
	if (!World)
	{
		UE_LOG(LogEmberbound, Warning, TEXT("EMBER: CaptureAfter failed no active world"));
		return;
	}

	TWeakObjectPtr<UWorld> WeakWorld(World);
	FTimerDelegate CaptureDelegate = FTimerDelegate::CreateLambda([WeakWorld, bShowUI]()
	{
		UWorld* TimerWorld = WeakWorld.Get();
		if (!TimerWorld || !GEngine)
		{
			UE_LOG(LogEmberbound, Warning, TEXT("EMBER: CaptureAfter skipped world invalid"));
			return;
		}

		FScreenshotRequest::RequestScreenshot(TEXT("HighresScreenshot"), bShowUI, true, false);
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: CaptureFired file=%s"), *FPaths::GetCleanFilename(FScreenshotRequest::GetFilename()));
	});

	FTimerHandle CaptureTimer;
	World->GetTimerManager().SetTimer(CaptureTimer, CaptureDelegate, FMath::Max(0.0f, DelaySeconds), false);
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: CaptureScheduled seconds=%.2f showui=%d"), DelaySeconds, bShowUI ? 1 : 0);
}

void FEmberCaptureDebug::ScheduleFromCommandLine(UWorld* World)
{
	if (bCommandLineCaptureScheduled)
	{
		return;
	}

	float DelaySeconds = 6.0f;
	bool bShowUI = false;
	if (TryParseCommandLineCapture(DelaySeconds, bShowUI))
	{
		bCommandLineCaptureScheduled = true;
		ScheduleCapture(World, DelaySeconds, bShowUI);
	}
}
