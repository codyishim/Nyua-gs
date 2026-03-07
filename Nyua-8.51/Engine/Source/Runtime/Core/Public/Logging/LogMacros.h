#pragma once

enum class ELogLevel { Fatal, Error, Warning, Log, Verbose, VeryVerbose };

inline constexpr ELogLevel Fatal = ELogLevel::Fatal;
inline constexpr ELogLevel Error = ELogLevel::Error;
inline constexpr ELogLevel Warning = ELogLevel::Warning;
inline constexpr ELogLevel Log = ELogLevel::Log;
inline constexpr ELogLevel Verbose = ELogLevel::Verbose;
inline constexpr ELogLevel VeryVerbose = ELogLevel::VeryVerbose;


inline const char *ToString( ELogLevel level ) {
        switch ( level ) {
        case ELogLevel::Verbose:
                return "Verbose";
        case ELogLevel::VeryVerbose:
                return "VeryVerbose";
        case ELogLevel::Log:
                return "Log";
        case ELogLevel::Warning:
                return "Warning";
        case ELogLevel::Error:
                return "Error";
        case ELogLevel::Fatal:
                return "Fatal";
        default:
                return "Unknown";
        }
}

inline const wchar_t *ToWString( ELogLevel level ) {
        switch ( level ) {
        case ELogLevel::Verbose:
                return L"Verbose";
        case ELogLevel::VeryVerbose:
                return L"VeryVerbose";
        case ELogLevel::Log:
                return L"Log";
        case ELogLevel::Warning:
                return L"Warning";
        case ELogLevel::Error:
                return L"Error";
        case ELogLevel::Fatal:
                return L"Fatal";
        default:
                return L"Unknown";
        }
}

inline void SetConsoleColorByLogLevel( ELogLevel Level ) {
        HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );

        WORD color = FOREGROUND_RED | FOREGROUND_GREEN |
                     FOREGROUND_BLUE; // default white

        switch ( Level ) {
        case ELogLevel::Warning:
                color = FOREGROUND_RED | FOREGROUND_GREEN; // yellow
                break;
        case ELogLevel::Error:
                color = FOREGROUND_RED; // red
                break;
        case ELogLevel::Fatal:
                color = FOREGROUND_RED | FOREGROUND_INTENSITY; // bright red
                break;
        case ELogLevel::Verbose:
                color = FOREGROUND_GREEN | FOREGROUND_BLUE; // cyan
                break;
        case ELogLevel::VeryVerbose:
                color = FOREGROUND_BLUE | FOREGROUND_INTENSITY; // bright blue
                break;
        case ELogLevel::Log:
        default:
                color = FOREGROUND_RED | FOREGROUND_GREEN |
                        FOREGROUND_BLUE; // white
                break;
        }

        SetConsoleTextAttribute( hConsole, color );
}

inline void ResetConsoleColor() {
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ),
                                 FOREGROUND_RED | FOREGROUND_GREEN |
                                     FOREGROUND_BLUE );
}


struct FLogCategory {
        const char *Name;
        ELogLevel RuntimeVerbosity;
};


#if _DEBUG
#define DEFINE_LOG_CATEGORY( CategoryName )                                    \
        inline FLogCategory CategoryName = { #CategoryName, ELogLevel::VeryVerbose }
#else
#define DEFINE_LOG_CATEGORY( CategoryName )                                    \
        inline FLogCategory CategoryName = { #CategoryName, ELogLevel::Log }
#endif

inline void UELogImpl(FLogCategory& Category, ELogLevel Level, const char* msg) {
        if ((int)Level > (int)Category.RuntimeVerbosity)
                return;

        SetConsoleColorByLogLevel(Level);
        std::cout << Category.Name << ": "
                  << ToString(Level) << ": " << msg << std::endl;
        ResetConsoleColor();
}

inline std::string WStringToUTF8(const std::wstring& wstr) {
        return std::string(wstr.begin(), wstr.end());
}

inline void UELogWImpl(FLogCategory& Category, ELogLevel Level, const wchar_t* msg) {
        if ((int)Level > (int)Category.RuntimeVerbosity)
                return;

        std::wstring wfinal(msg);
        std::string finalbuf = WStringToUTF8(wfinal);

        SetConsoleColorByLogLevel(Level);
        std::cout << Category.Name << ": "
                  << ToString(Level) << ": " << finalbuf << std::endl;
        ResetConsoleColor();
}

inline void UE_LOG_DISPATCH(FLogCategory& Category, ELogLevel Level, const char* Format, ...) {
        va_list args;
        va_start(args, Format);

        char buffer[2048];
        vsnprintf(buffer, sizeof(buffer), Format, args);
        va_end(args);

        UELogImpl(Category, Level, buffer);
}

inline void UE_LOG_DISPATCH(FLogCategory& Category, ELogLevel Level, const wchar_t* Format, ...) {
        va_list args;
        va_start(args, Format);

        wchar_t buffer[2048];
        vswprintf(buffer, sizeof(buffer) / sizeof(wchar_t), Format, args);
        va_end(args);

        UELogWImpl(Category, Level, buffer);
}

#define UE_LOG(Category, Verbosity, Format, ...) \
UE_LOG_DISPATCH(Category, Verbosity, Format, ##__VA_ARGS__)

#define UE_LOG_W(Category, Verbosity, Format, ...) \
UE_LOG_DISPATCH(Category, Verbosity, Format, ##__VA_ARGS__)

DEFINE_LOG_CATEGORY(LogServer);
DEFINE_LOG_CATEGORY(LogWorld);
DEFINE_LOG_CATEGORY(LogNet);
DEFINE_LOG_CATEGORY(LogFort);
DEFINE_LOG_CATEGORY(LogAbilitySystem);