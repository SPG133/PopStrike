#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "PSConnectionSubsystem.h"
#include "PSAvatar.h"
#include "PSLobbyTypes.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPSNetworkInputTest, "PopStrike.Network.InputBoundaries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPSNetworkInputTest::RunTest(const FString& Parameters)
{
    for (const TCHAR* Address : {TEXT("127.0.0.1:7777"), TEXT("localhost"), TEXT("game.example.com:65535"), TEXT("192.168.1.2:1")})
        TestTrue(Address, UPSConnectionSubsystem::IsValidAddress(Address));
    for (const TCHAR* Address : {TEXT(""), TEXT("127.0.0.1:0"), TEXT("host:65536"), TEXT("host:"), TEXT("host:12.5"), TEXT("host?listen"), TEXT("/Game/UI/Maps/L_Lobby"), TEXT("http://host"), TEXT("a b"), TEXT("a..b"), TEXT("-host"), TEXT("999.0.0.1"), TEXT("127.1"), TEXT("host:1:2")})
        TestFalse(Address, UPSConnectionSubsystem::IsValidAddress(Address));
    TestTrue(TEXT("Empty avatar is optional"), PSAvatar::ValidateNetworkAvatar({}));
    TestFalse(TEXT("Malformed image rejected"), PSAvatar::ValidateNetworkAvatar({1,2,3,4}));
    TArray<uint8> Oversize; Oversize.SetNumZeroed(12289);
    TestFalse(TEXT("Oversize avatar rejected"), PSAvatar::ValidateNetworkAvatar(Oversize));
    TestEqual(TEXT("Negative map wrap"), PSLobby::WrapMapIndex(-1), PSLobby::GetMaps().Num()-1);
    TestEqual(TEXT("Positive map wrap"), PSLobby::WrapMapIndex(PSLobby::GetMaps().Num()), 0);
    return true;
}
#endif
