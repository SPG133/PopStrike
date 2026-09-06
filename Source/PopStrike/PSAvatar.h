#pragma once

#include "CoreMinimal.h"

class UTexture2D;

namespace PSAvatar
{
    bool ValidateNetworkAvatar(const TArray<uint8>& Bytes);
    bool PickImageFile(FString& OutFilename);
    bool BuildNetworkAvatar(const FString& Filename, TArray<uint8>& OutBytes, UTexture2D*& OutTexture);
    UTexture2D* TextureFromBytes(const TArray<uint8>& Bytes);
}
