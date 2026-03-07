#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <iostream>
#include <cstdint>

#include "SDK/SDK/Basic.hpp"
#include "SDK/SDK/CoreUObject_structs.hpp"
#include "SDK/SDK/CoreUObject_classes.hpp"
#include "SDK/SDK/Engine_structs.hpp"
#include "SDK/SDK/Engine_classes.hpp"
#include "SDK/SDK/FortniteGame_structs.hpp"
#include "SDK/SDK/FortniteGame_classes.hpp"
#include "SDK/SDK/GameplayAbilities_structs.hpp"
#include "SDK/SDK/GameplayAbilities_classes.hpp"
#include "SDK/SDK/TrapTool_classes.hpp"
#include "SDK/SDK/TrapTool_ContextTrap_Athena_classes.hpp"
#include "SDK/SDK/Tiered_Chest_Athena_classes.hpp"
#include "SDK/SDK/GAB_Spray_Generic_classes.hpp"
#include "SDK/SDK/GAB_Emote_Generic_classes.hpp"
#include "SDK/SDK/B_Prj_Athena_PlaysetGrenade_classes.hpp"
using namespace SDK;
#include <winhttp.h>

#pragma comment(lib, "Vendor/MinHook/minhook.lib")
#include "Vendor/MinHook/MinHook.h"

#include "Logging/LogMacros.h"
#include <shellapi.h>
#pragma comment(lib, "winhttp.lib")

inline bool bLategame = false;
constexpr bool bGameSessions = false;
constexpr bool bProd = false;
constexpr bool bCreative = false;
inline int Port = 7777;

// TODO: Move this shit
static inline UObject* (*StaticFindObjectOG)(UClass*, UObject* Package, const wchar_t* OrigInName, bool ExactClass) = decltype(StaticFindObjectOG)(__int64(GetModuleHandleW(0)) + 0x1e825f0);
template <typename T>
static inline T* StaticFindObject(std::string ObjectName, UClass* ObjectClass = T::StaticClass())
{
    auto OrigInName = std::wstring(ObjectName.begin(), ObjectName.end()).c_str();

    return (T*)StaticFindObjectOG(ObjectClass, nullptr, OrigInName, false);
}

static inline UObject* (*StaticLoadObjectOG)(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename, uint32_t LoadFlags, UObject* Sandbox, bool bAllowObjectReconciliation, void*) = decltype(StaticLoadObjectOG)(__int64(GetModuleHandleW(0)) + 0x1e838d0);
template<typename T>
static inline T* StaticLoadObject(std::string name)
{
    T* Object = StaticFindObject<T>(name);

    if (!Object)
    {
        auto Name = std::wstring(name.begin(), name.end()).c_str();
        Object = (T*)StaticLoadObjectOG(T::StaticClass(), nullptr, Name, nullptr, 0, nullptr, false, nullptr);
    }

    return Object;
}

static inline std::vector<UObject*> GetObjectsOfClass(UClass* Class = UObject::StaticClass())
{
    std::vector<UObject*> ArrayOfObjects;
    for (int i = 0; i < UObject::GObjects->Num(); i++)
    {
        UObject* Object = reinterpret_cast<UObject*>(UObject::GObjects->GetByIndex(i));
        if (!Object) continue;
        if (Object->IsA(Class))
        {
            ArrayOfObjects.push_back(Object);
        }
    }

    return ArrayOfObjects;
}

static std::vector<std::wstring> args = []() {
    std::vector<std::wstring> result;
    LPWSTR* argv;
    int argc;

    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        for (int i = 0; i < argc; ++i) {
            result.emplace_back(argv[i]);
        }
        LocalFree(argv);
    }
    return result;
    }();

static void PostRequest(const std::wstring& host, const std::wstring& path, const std::string& body = "{}")
{
    HINTERNET s = WinHttpOpen(L"Nyua/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!s) return;

    HINTERNET c = WinHttpConnect(s, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!c) { WinHttpCloseHandle(s); return; }

    HINTERNET r = WinHttpOpenRequest(c, L"POST", path.c_str(), NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (r)
    {
        WinHttpSendRequest(r, L"Content-Type: application/json\r\n", 0,
            (LPVOID)body.data(), (DWORD)body.size(), (DWORD)body.size(), 0);
        WinHttpReceiveResponse(r, NULL);
        WinHttpCloseHandle(r);
    }
    WinHttpCloseHandle(c);
    WinHttpCloseHandle(s);
}
static std::string GetRequest(
    const std::wstring& host, 
    const std::wstring& path,
    const std::unordered_map<std::wstring, std::wstring>& headers = {},
    INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT,
    bool useHttps = true)
{
    std::string response;
    
    HINTERNET s = WinHttpOpen(L"Nyua/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!s) return response;

    HINTERNET c = WinHttpConnect(s, host.c_str(), port, 0);
    if (!c) { WinHttpCloseHandle(s); return response; }

    DWORD flags = useHttps ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET r = WinHttpOpenRequest(c, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (r)
    {
        if (!headers.empty())
        {
            std::wstring headerString;
            for (const auto& [key, value] : headers)
            {
                headerString += key + L": " + value + L"\r\n";
            }
            WinHttpAddRequestHeaders(r, headerString.c_str(), 
                static_cast<DWORD>(headerString.length()), 
                WINHTTP_ADDREQ_FLAG_ADD);
        }

        if (WinHttpSendRequest(r, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
            WinHttpReceiveResponse(r, NULL))
        {
            DWORD bytesAvailable = 0;
            while (WinHttpQueryDataAvailable(r, &bytesAvailable) && bytesAvailable > 0)
            {
                std::vector<char> buffer(bytesAvailable + 1, 0);
                DWORD bytesRead = 0;
                if (WinHttpReadData(r, buffer.data(), bytesAvailable, &bytesRead))
                {
                    response.append(buffer.data(), bytesRead);
                }
                else break;
            }
        }
        WinHttpCloseHandle(r);
    }
    WinHttpCloseHandle(c);
    WinHttpCloseHandle(s);
    
    return response;
}