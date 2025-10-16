// bgm.cpp 수정 포인트들만 모았습니다.
#include "bgm.hpp"
#define NOMINMAX
#pragma comment(lib, "winmm.lib")

namespace {
    const wchar_t* kAlias = L"bgm";
    std::wstring g_lastErr;
    bool g_opened = false;

    std::wstring exeDir() {
        wchar_t path[MAX_PATH]{};
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring s(path);
        size_t p = s.find_last_of(L"\\/");
        return (p == std::wstring::npos) ? L"." : s.substr(0, p);
    }
    std::wstring cwd() {
        wchar_t buf[MAX_PATH]{};
        GetCurrentDirectoryW(MAX_PATH, buf);
        return buf;
    }
    std::wstring join(const std::wstring& a, const std::wstring& b) {
        if (a.empty()) return b;
        if (a.back() == L'\\' || a.back() == L'/') return a + b;
        return a + L"\\" + b;
    }
    bool fileExists(const std::wstring& p) {
        DWORD a = GetFileAttributesW(p.c_str());
        return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
    }
    void setErrMM(UINT code) {
        if (code == 0) { g_lastErr.clear(); return; }
        wchar_t buf[256]{};
        mciGetErrorStringW(code, buf, 255);
        g_lastErr = buf;
    }
    UINT mci(const std::wstring& cmd) {
        UINT r = mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
        setErrMM(r);
        return r;
    }
}

namespace Bgm {

    std::wstring defaultPath() {
        // 1) EXE\assets\bgm.mp3 우선
        auto p1 = join(exeDir(), L"assets\\bgm.mp3");
        if (fileExists(p1)) return p1;
        // 2) CWD\assets\bgm.mp3(Visual Studio 디버깅 시 흔함)
        auto p2 = join(cwd(), L"assets\\bgm.mp3");
        return p2;
    }

    bool init(const std::wstring& mp3Path) {
        // 이전 인스턴스 정리
        if (g_opened) { mci(L"stop bgm"); mci(L"close bgm"); g_opened = false; }

        // 파일 존재 확인(경로 문제를 빨리 찾자)
        if (!fileExists(mp3Path)) {
            g_lastErr = L"파일 없음: " + mp3Path;
            return false;
        }

        // 열기 -> 시간 포맷 지정(밀리초) -> 볼륨 기본값
        std::wstring cmd = L"open \"" + mp3Path + L"\" type mpegvideo alias bgm";
        if (mci(cmd) != 0) return false;
        g_opened = true;

        if (mci(L"set bgm time format milliseconds") != 0) return false;
        // 일부 시스템에서 setaudio 전에 on 필요할 때가 있어 같이 호출
        mci(L"setaudio bgm on");
        mci(L"setaudio bgm volume to 700");  // 0~1000

        return true;
    }

    bool play(bool loop) {
        if (!g_opened) return false;
        std::wstring cmd = std::wstring(L"play bgm") + (loop ? L" repeat" : L"");
        return mci(cmd) == 0;
    }

    void pause() { if (g_opened) mci(L"pause bgm"); }
    void resume() { if (g_opened) mci(L"resume bgm"); }

    void stop() {
        if (!g_opened) return;
        mci(L"stop bgm");
        mci(L"close bgm");
        g_opened = false;
    }

    bool setVolume(int v) {
        if (!g_opened) return false;
        if (v < 0) v = 0; if (v > 1000) v = 250;
        wchar_t buf[64];
        swprintf(buf, 64, L"setaudio bgm volume to %d", v);
        return mci(buf) == 0;
    }

    bool setLoopRangeMs(unsigned long fromMs, unsigned long toMs) {
        if (!g_opened) return false;
        mci(L"set bgm time format milliseconds");
        wchar_t buf[128];
        swprintf(buf, 128, L"play bgm from %lu to %lu repeat", fromMs, toMs);
        return mci(buf) == 0;
    }

    void clearLoopRange() {
        if (!g_opened) return;
        mci(L"set bgm time format milliseconds");
        mci(L"play bgm repeat");
    }

    std::wstring lastError() { return g_lastErr; }


    void startBgmTest() {
        auto path = Bgm::defaultPath();
        std::wcout << L"[BGM] 시도 경로: " << path << L"\n";
        if (!Bgm::init(path)) {
            std::wcout << L"[BGM] init 실패: " << Bgm::lastError() << L"\n";
            return;
        }
        if (!Bgm::play(true)) {
            std::wcout << L"[BGM] play 실패: " << Bgm::lastError() << L"\n";
            return;
        }
        std::wcout << L"[BGM] 재생 중 (반복)\n";
    }

} // namespace Bgm
