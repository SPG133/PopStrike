#include "PSAvatar.h"

#include "HAL/FileManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageCore.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <commdlg.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

namespace
{
    TSharedPtr<IImageWrapper> ReadImage(const TArray<uint8>& Bytes, int32 MaxSide)
    {
        if (Bytes.IsEmpty()) return nullptr;

        auto& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
        const EImageFormat Format = Module.DetectImageFormat(Bytes.GetData(), Bytes.Num());
        if (Format != EImageFormat::PNG && Format != EImageFormat::JPEG) return nullptr;

        auto Image = Module.CreateImageWrapper(Format);
        if (!Image || !Image->SetCompressed(Bytes.GetData(), Bytes.Num())) return nullptr;
        if (Image->GetWidth() <= 0 || Image->GetHeight() <= 0 ||
            Image->GetWidth() > MaxSide || Image->GetHeight() > MaxSide) return nullptr;
        return Image;
    }
}

bool PSAvatar::PickImageFile(FString& OutFilename)
{
#if PLATFORM_WINDOWS
    WCHAR Buffer[32768] = {};
    OPENFILENAMEW Dialog{};
    Dialog.lStructSize = sizeof(Dialog);
    Dialog.lpstrFile = Buffer;
    Dialog.nMaxFile = UE_ARRAY_COUNT(Buffer);
    Dialog.lpstrFilter = L"PNG/JPEG\0*.png;*.jpg;*.jpeg\0\0";
    Dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameW(&Dialog))
    {
        OutFilename = Buffer;
        return true;
    }
#endif
    return false;
}

bool PSAvatar::EncodeFile(const FString& Filename, TArray<uint8>& OutBytes)
{
    OutBytes.Reset();
    const int64 Size = IFileManager::Get().FileSize(*Filename);
    if (Size <= 0 || Size > 8 * 1024 * 1024) return false;

    TArray<uint8> Source;
    FImage Image, Small;
    if (!FFileHelper::LoadFileToArray(Source, *Filename) || !ReadImage(Source, 4096) ||
        !FImageUtils::DecompressImage(Source.GetData(), Source.Num(), Image)) return false;

    Image.ResizeTo(Small, 128, 128, ERawImageFormat::BGRA8, EGammaSpace::sRGB);
    TArray64<uint8> Compressed;
    if (!FImageUtils::CompressImage(Compressed, TEXT("jpg"), Small, 75) ||
        Compressed.Num() > MaxBytes) return false;

    OutBytes.Append(Compressed.GetData(), Compressed.Num());
    return true;
}

bool PSAvatar::IsValid(const TArray<uint8>& Bytes)
{
    if (Bytes.IsEmpty()) return true;
    if (Bytes.Num() > MaxBytes || !ReadImage(Bytes, 128)) return false;
    FImage Image;
    return FImageUtils::DecompressImage(Bytes.GetData(), Bytes.Num(), Image);
}
