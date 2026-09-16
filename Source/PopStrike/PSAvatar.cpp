#include "PSAvatar.h"

#include "ImageUtils.h"
#include "Misc/Paths.h"

#if PLATFORM_WINDOWS

#include "Windows/AllowWindowsPlatformTypes.h"
#include <commdlg.h>
#include "Windows/HideWindowsPlatformTypes.h"

#endif


bool PSAvatar::PickImageFile(FString& OutFilename)
{
#if PLATFORM_WINDOWS

    // 用大 Buffer，避免普通 MAX_PATH 太短
    WCHAR FileBuffer[32768] = {};

    OPENFILENAMEW Dialog;
    FMemory::Memzero(Dialog);

    Dialog.lStructSize = sizeof(OPENFILENAMEW);

    // 暂时不指定父窗口，简单可靠
    Dialog.hwndOwner = nullptr;

    Dialog.lpstrFile = FileBuffer;
    Dialog.nMaxFile = UE_ARRAY_COUNT(FileBuffer);

    // Windows 文件选择器里的类型过滤
    Dialog.lpstrFilter =
        L"Image Files (*.png;*.jpg;*.jpeg)\0"
        L"*.png;*.jpg;*.jpeg\0"
        L"PNG Files (*.png)\0"
        L"*.png\0"
        L"JPEG Files (*.jpg;*.jpeg)\0"
        L"*.jpg;*.jpeg\0"
        L"\0";

    Dialog.nFilterIndex = 1;

    Dialog.Flags =
        OFN_FILEMUSTEXIST |
        OFN_PATHMUSTEXIST |
        OFN_NOCHANGEDIR |
        OFN_EXPLORER;


    // 真正打开 Windows 文件选择窗口
    if (GetOpenFileNameW(&Dialog))
    {
        OutFilename = FString(FileBuffer);

        UE_LOG(
            LogTemp,
            Log,
            TEXT("Avatar selected: %s"),
            *OutFilename);

        return true;
    }

#endif

    return false;
}


UTexture2D* PSAvatar::LoadPreviewTexture(
    const FString& Filename)
{
    // 文件路径为空
    if (Filename.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("LoadPreviewTexture: Filename is empty."));

        return nullptr;
    }


    // 文件不存在
    if (!FPaths::FileExists(Filename))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("LoadPreviewTexture: File does not exist: %s"),
            *Filename);

        return nullptr;
    }


    // UE 直接从硬盘图片创建一个临时 Texture2D
    UTexture2D* Texture =
        FImageUtils::ImportFileAsTexture2D(Filename);


    if (!Texture)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("LoadPreviewTexture: Failed to load image: %s"),
            *Filename);

        return nullptr;
    }


    UE_LOG(
        LogTemp,
        Log,
        TEXT("Avatar preview loaded successfully: %s"),
        *Filename);


    return Texture;
}
