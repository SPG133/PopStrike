#include "PSAvatar.h"

#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "ImageCore.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "PSGameConfig.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <commdlg.h>
#include "Windows/HideWindowsPlatformTypes.h"
#undef LoadImage
#endif

namespace PSAvatar
{
    bool ValidateNetworkAvatar(const TArray<uint8>& Bytes)
    {
        if (Bytes.IsEmpty()) return true;
        if (Bytes.Num() > PSConfig::MaxAvatarBytes) return false;
        auto& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
        auto Wrapper = Module.CreateImageWrapper(EImageFormat::JPEG);
        if (!Wrapper || !Wrapper->SetCompressed(Bytes.GetData(), Bytes.Num()) ||
            Wrapper->GetWidth() != PSConfig::AvatarSide || Wrapper->GetHeight() != PSConfig::AvatarSide) return false;
        TArray64<uint8> Raw;
        return Wrapper->GetRaw(ERGBFormat::BGRA, 8, Raw) && Raw.Num() == PSConfig::AvatarSide * PSConfig::AvatarSide * 4;
    }
    bool PickImageFile(FString& OutFilename)
    {
#if PLATFORM_WINDOWS
        wchar_t File[32768] = {};
        const wchar_t Filter[] = L"Images\0*.png;*.jpg;*.jpeg\0PNG\0*.png\0JPEG\0*.jpg;*.jpeg\0\0";

        OPENFILENAMEW Dialog{};
        Dialog.lStructSize = sizeof(Dialog);
        Dialog.lpstrFile = File;
        Dialog.nMaxFile = UE_ARRAY_COUNT(File);
        Dialog.lpstrFilter = Filter;
        Dialog.nFilterIndex = 1;
        Dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

        if (::GetOpenFileNameW(&Dialog))
        {
            OutFilename = FString(File);
            return true;
        }
#endif
        return false;
    }

    bool BuildNetworkAvatar(const FString& Filename, TArray<uint8>& OutBytes, UTexture2D*& OutTexture)
    {
        OutBytes.Reset();
        OutTexture = nullptr;

        const FString Ext = FPaths::GetExtension(Filename).ToLower();
        const int64 FileSize = IFileManager::Get().FileSize(*Filename);
        if ((Ext != TEXT("png") && Ext != TEXT("jpg") && Ext != TEXT("jpeg")) ||
            FileSize <= 0 || FileSize > PSConfig::MaxAvatarFileBytes)
        {
            return false;
        }

        FImage Source;
        if (!FImageUtils::LoadImage(*Filename, Source) || Source.SizeX <= 0 || Source.SizeY <= 0 ||
            Source.SizeX > 8192 || Source.SizeY > 8192)
        {
            return false;
        }

        FImage Small;
        FImageCore::ResizeImageAllocDest(
            Source,
            Small,
            PSConfig::AvatarSide,
            PSConfig::AvatarSide,
            FImageCore::EResizeImageFilter::AdaptiveSharp);

        for (const int32 Quality : {70, 50, 30})
        {
            TArray64<uint8> Encoded;
            if (FImageUtils::CompressImage(Encoded, TEXT("jpg"), Small, Quality) &&
                Encoded.Num() <= PSConfig::MaxAvatarBytes)
            {
                OutBytes.Append(Encoded.GetData(), static_cast<int32>(Encoded.Num()));
                OutTexture = TextureFromBytes(OutBytes);
                return OutTexture != nullptr;
            }
        }

        return false;
    }

    UTexture2D* TextureFromBytes(const TArray<uint8>& Bytes)
    {
        return Bytes.IsEmpty() || !ValidateNetworkAvatar(Bytes) ? nullptr : FImageUtils::ImportBufferAsTexture2D(Bytes);
    }
}
