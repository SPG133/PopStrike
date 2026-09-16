#pragma once

#include "CoreMinimal.h"

class UTexture2D;

namespace PSAvatar
{
    // 选择图片并生成网络头像
    UTexture2D* LoadPreviewTexture(
        const FString& Filename);

    
    bool PickImageFile(FString& OutFilename);
}
