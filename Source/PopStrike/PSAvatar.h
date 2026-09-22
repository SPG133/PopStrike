#pragma once

#include "CoreMinimal.h"

class UTexture2D;

namespace PSAvatar
{
    constexpr int32 MaxBytes = 16 * 1024;

    bool PickImageFile(FString& OutFilename);
    bool EncodeFile(const FString& Filename, TArray<uint8>& OutBytes);
    bool IsValid(const TArray<uint8>& Bytes);
    UTexture2D* CreateTexture(const TArray<uint8>& Bytes, int32 ColorSeed);
}
