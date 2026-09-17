#pragma once

#include "CoreMinimal.h"

namespace PSAvatar
{
    constexpr int32 MaxBytes = 16 * 1024;

    bool PickImageFile(FString& OutFilename);
    bool EncodeFile(const FString& Filename, TArray<uint8>& OutBytes);
    bool IsValid(const TArray<uint8>& Bytes);
}
