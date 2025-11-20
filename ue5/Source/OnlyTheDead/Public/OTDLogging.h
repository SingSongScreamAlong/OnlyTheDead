#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOTD, Log, All);

#define OTD_LOG(Verbosity, Format, ...) UE_LOG(LogOTD, Verbosity, TEXT("%s:%d | " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)
#define OTD_LOG_DISPLAY(Format, ...) OTD_LOG(Display, Format, ##__VA_ARGS__)
#define OTD_LOG_INFO(Format, ...) OTD_LOG(Log, Format, ##__VA_ARGS__)
#define OTD_LOG_WARNING(Format, ...) OTD_LOG(Warning, Format, ##__VA_ARGS__)
#define OTD_LOG_ERROR(Format, ...) OTD_LOG(Error, Format, ##__VA_ARGS__)
#define OTD_LOG_VERBOSE(Format, ...) OTD_LOG(Verbose, Format, ##__VA_ARGS__)
#define OTD_LOG_VERY_VERBOSE(Format, ...) OTD_LOG(VeryVerbose, Format, ##__VA_ARGS__)

struct FOTDScopeLog
{
public:
    explicit FOTDScopeLog(const TCHAR* InScopeLabel)
        : ScopeLabel(InScopeLabel)
        , StartSeconds(FPlatformTime::Seconds())
    {
        OTD_LOG_VERBOSE("Scope Enter: %s", ScopeLabel);
    }

    ~FOTDScopeLog()
    {
        const double Duration = FPlatformTime::Seconds() - StartSeconds;
        OTD_LOG_VERBOSE("Scope Exit: %s (%.4f s)", ScopeLabel, Duration);
    }

private:
    const TCHAR* ScopeLabel;
    double StartSeconds;
};

#define OTD_SCOPE_LOG(ScopeLabel) FOTDScopeLog PREPROCESSOR_JOIN(FOTDScopeLog_, __LINE__)(ScopeLabel)
