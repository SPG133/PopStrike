#include "PSAvatar.h"

#include "PSLobbyTypes.h"
#include "HAL/FileManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageCore.h"
#include "ImageUtils.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/FileHelper.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <commdlg.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

namespace
{
    struct FColorBucket
    {
        int32 Count = 0;
        FIntVector Sum = FIntVector::ZeroValue;

        FColor Average() const
        {
            return Count > 0
                ? FColor(Sum.X / Count, Sum.Y / Count, Sum.Z / Count)
                : FColor::Black;
        }
    };

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

    FPSPlayerAppearance MakeFallbackAppearance(int32 ColorSeed)
    {
        FRandomStream Random(ColorSeed);
        const uint8 Hue = static_cast<uint8>(Random.RandRange(0, 255));
        FPSPlayerAppearance Result;
        Result.PrimaryColor = FLinearColor::MakeFromHSV8(Hue, 180, 220).ToFColor(true);
        Result.SecondaryColor =
            FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue + 96), 170, 235).ToFColor(true);
        return Result;
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
    if (FSlateApplication::IsInitialized())
    {
        Dialog.hwndOwner = static_cast<HWND>(const_cast<void*>(
            FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr)));
    }
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

bool PSAvatar::ExtractAppearance(const TArray<uint8>& Bytes, int32 ColorSeed,
    FPSPlayerAppearance& OutAppearance)
{
    OutAppearance = MakeFallbackAppearance(ColorSeed);
    if (Bytes.IsEmpty()) return true;

    FImage Source, Image;
    if (!FImageUtils::DecompressImage(Bytes.GetData(), Bytes.Num(), Source)) return false;
    Source.CopyTo(Image, ERawImageFormat::BGRA8, EGammaSpace::sRGB);
    if (Image.RawData.IsEmpty()) return false;

    TMap<uint16, FColorBucket> Buckets;
    const FColor* Pixels = reinterpret_cast<const FColor*>(Image.RawData.GetData());
    const int64 PixelCount = Image.RawData.Num() / sizeof(FColor);
    for (int64 Index = 0; Index < PixelCount; ++Index)
    {
        const FColor Pixel = Pixels[Index];
        if (Pixel.A < 32) continue;
        const uint16 Key = static_cast<uint16>(
            ((Pixel.R >> 4) << 8) | ((Pixel.G >> 4) << 4) | (Pixel.B >> 4));
        FColorBucket& Bucket = Buckets.FindOrAdd(Key);
        ++Bucket.Count;
        Bucket.Sum += FIntVector(Pixel.R, Pixel.G, Pixel.B);
    }
    if (Buckets.IsEmpty()) return false;

    TArray<FColorBucket> Ranked;
    Buckets.GenerateValueArray(Ranked);
    Ranked.Sort([](const FColorBucket& A, const FColorBucket& B)
    {
        return A.Count > B.Count;
    });

    OutAppearance.PrimaryColor = Ranked[0].Average();
    OutAppearance.SecondaryColor = Ranked.Num() > 1
        ? Ranked[1].Average() : OutAppearance.PrimaryColor;
    return true;
}

UTexture2D* PSAvatar::CreateTexture(const TArray<uint8>& Bytes, int32 ColorSeed)
{
    if (!Bytes.IsEmpty()) return FImageUtils::ImportBufferAsTexture2D(Bytes);

    FPSPlayerAppearance Appearance;
    ExtractAppearance(Bytes, ColorSeed, Appearance);
    UTexture2D* Texture = UTexture2D::CreateTransient(1, 1, PF_B8G8R8A8);
    if (Texture)
    {
        uint32* Pixel = static_cast<uint32*>(
            Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
        *Pixel = Appearance.PrimaryColor.DWColor();
        Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
        Texture->UpdateResource();
    }
    return Texture;
}
