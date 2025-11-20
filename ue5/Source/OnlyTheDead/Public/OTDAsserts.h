#pragma once

#include "CoreMinimal.h"
#include "OTDLogging.h"

#if DO_CHECK
#define OTD_ASSERT(Condition) \
    do \
    { \
        if (!(Condition)) \
        { \
            OTD_LOG_ERROR("Assertion failed: %s", TEXT(#Condition)); \
            checkf(false, TEXT("OTD_ASSERT failed: %s (%s:%d)"), TEXT(#Condition), TEXT(__FILE__), __LINE__); \
        } \
    } while (0)
#else
#define OTD_ASSERT(Condition) ((void)0)
#endif

#if DO_CHECK
#define OTD_ASSERT_MSG(Condition, Format, ...) \
    do \
    { \
        if (!(Condition)) \
        { \
            OTD_LOG_ERROR("Assertion failed: %s | " Format, TEXT(#Condition), ##__VA_ARGS__); \
            checkf(false, TEXT("OTD_ASSERT failed: %s (%s:%d)"), TEXT(#Condition), TEXT(__FILE__), __LINE__); \
        } \
    } while (0)
#else
#define OTD_ASSERT_MSG(Condition, Format, ...) ((void)0)
#endif

#define OTD_ENSURE(Condition) ensureMsgf((Condition), TEXT("OTD_ENSURE failed: %s (%s:%d)"), TEXT(#Condition), TEXT(__FILE__), __LINE__)
#define OTD_ENSURE_MSG(Condition, Format, ...) ensureMsgf((Condition), TEXT(Format), ##__VA_ARGS__)

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#define OTD_DEBUG_BREAK() \
    do \
    { \
        UE_DEBUG_BREAK(); \
    } while (0)
#else
#define OTD_DEBUG_BREAK() ((void)0)
#endif

#define OTD_GUARD_VALUE(Condition, DefaultValue) ((Condition) ? (Condition) : (DefaultValue))
