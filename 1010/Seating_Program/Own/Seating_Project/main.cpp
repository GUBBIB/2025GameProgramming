#include "console.hpp"
#include "menu.hpp"
#include "bgm.hpp"

void SetWorkingDirToExeFolder() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring exePath(path);
    size_t pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        std::wstring dir = exePath.substr(0, pos);
        SetCurrentDirectoryW(dir.c_str());
    }
}

int main() {
    SetWorkingDirToExeFolder();

    auto mp3 = Bgm::defaultPath()#include "console.hpp"
#include "menu.hpp"
#include "bgm.hpp"
using namespace Console;

void SetWorkingDirToExeFolder() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring exePath(path);
    size_t pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        std::wstring dir = exePath.substr(0, pos);
        SetCurrentDirectoryW(dir.c_str());
    }
}

int main() {
    SetWorkingDirToExeFolder();

    auto mp3 = Bgm::defaultPath();      // .\assets\bgm.mp3
    if (!Bgm::init(mp3)) {
        gotoxy(40, 40);
        wprintf(L"[BGM 실패] %s\n", Bgm::lastError().c_str());
    }
    else {
        Bgm::setVolume(700);            // 0~1000
        Bgm::play(true);                // 무한 반복
    }

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Console::init(150, 40);
    main_screen();

    Bgm::stop(); // 종료 시 정리
    return 0;
}
;      // .\assets\bgm.mp3
    if (!Bgm::init(mp3)) {
        wprintf(L"[BGM 실패] %s\n", Bgm::lastError().c_str());
    }
    else {
        Bgm::setVolume(700);            // 0~1000
        Bgm::play(true);                // 무한 반복
        // 필요하면 특정 구간만:
        // Bgm::setLoopRangeMs(0, 10000); // 처음 10초 반복
    }
    
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Console::init(150, 40);
    main_screen();

    Bgm::stop(); // 종료 시 정리
    return 0;
}
