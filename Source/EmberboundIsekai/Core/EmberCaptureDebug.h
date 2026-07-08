#pragma once

#include "CoreMinimal.h"

class EMBERBOUNDISEKAI_API FEmberCaptureDebug
{
public:
	static void ScheduleCapture(UWorld* World, float DelaySeconds, bool bShowUI);
	static void ScheduleFromCommandLine(UWorld* World);
};
