#include "console.hpp"
#include <io.h>
#include <fcntl.h>
#include <algorithm>
#include <iostream>

namespace Console {
    static int  g_defWinCols = 150;
    static int  g_defWinLines = 50;
    static int  g_defBufCols = 150;
    static int  g_defBufLines = 50;
    static wchar_t g_title[128] = L"자리 배치 프로그램";

    HANDLE HOUT = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE HIN = GetStdHandle(STD_INPUT_HANDLE);

    void init(int cols, int lines, const wchar_t* title)
    {
        _setmode(_fileno(stdout), _O_U16TEXT);
        _setmode(_fileno(stdin), _O_U16TEXT);
        _setmode(_fileno(stderr), _O_U16TEXT);

        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        SetConsoleTitleW(title);
        g_defWinCols = cols;
        g_defWinLines = lines;
        g_defBufCols = cols;
        g_defBufLines = lines;
        wcsncpy_s(g_title, title, _TRUNCATE);

        wchar_t cmd[64];
        swprintf(cmd, 64, L"mode con cols=%d lines=%d", cols, lines);
        _wsystem(cmd);

        CONSOLE_CURSOR_INFO ci{ 25, TRUE };
        SetConsoleCursorInfo(HOUT, &ci);
    }

    // ─────────── 기본 유틸 ───────────
    void setAttr(WORD attr) { SetConsoleTextAttribute(HOUT, attr); }

    void gotoxy(int x, int y)
    {
        COORD pos{ (SHORT)(x - 1), (SHORT)(y - 1) };
        SetConsoleCursorPosition(HOUT, pos);
    }

    void hideCursor(bool hide)
    {
        CONSOLE_CURSOR_INFO ci{};
        GetConsoleCursorInfo(HOUT, &ci);
        ci.bVisible = hide ? FALSE : TRUE;
        SetConsoleCursorInfo(HOUT, &ci);
    }

    void get_cursor_1based(short* x, short* y)
    {
        CONSOLE_SCREEN_BUFFER_INFO info{};
        GetConsoleScreenBufferInfo(HOUT, &info);
        *x = (short)(info.dwCursorPosition.X + 1);
        *y = (short)(info.dwCursorPosition.Y + 1);
    }

    void clearAt(int x, int y, int width)
    {
        COORD pos{ (SHORT)(x - 1), (SHORT)(y - 1) };
        DWORD w;
        FillConsoleOutputCharacterW(HOUT, L' ', width, pos, &w);
        FillConsoleOutputAttribute(HOUT,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
            width, pos, &w);
    }

    void wprintAt(int x, int y, const wchar_t* text)
    {
        COORD pos{ (SHORT)(x - 1), (SHORT)(y - 1) };
        DWORD w;
        WriteConsoleOutputCharacterW(HOUT, text, (DWORD)wcslen(text), pos, &w);
    }

    void getConsoleSize(int& cols, int& rows)
    {
        CONSOLE_SCREEN_BUFFER_INFO info{};
        GetConsoleScreenBufferInfo(HOUT, &info);
        cols = info.srWindow.Right - info.srWindow.Left + 1;
        rows = info.srWindow.Bottom - info.srWindow.Top + 1;
    }

    int consoleCellWidth(const wchar_t* s)
    {
        int w = 0;
        for (size_t i = 0; s[i]; ++i)
        {
            WORD type = 0;
            if (GetStringTypeW(CT_CTYPE3, &s[i], 1, &type) && (type & C3_FULLWIDTH))
                w += 2;
            else
                w += 1;
        }
        return w;
    }

    void setWindowAndBuffer(int winCols, int winLines, int bufCols, int bufLines)
    {
        COORD largest = GetLargestConsoleWindowSize(HOUT);
        winCols = min(winCols, (int)largest.X);
        winLines = min(winLines, (int)largest.Y);
        bufCols = max(bufCols, winCols);
        bufLines = max(bufLines, winLines);

        SMALL_RECT tiny = { 0, 0, 1, 1 };
        SMALL_RECT rect = { 0, 0, (SHORT)(winCols - 1), (SHORT)(winLines - 1) };
        COORD      buf = { (SHORT)bufCols, (SHORT)bufLines };

        // 안전 순서: 창을 최소화 → 버퍼 확대/축소 → 창 원하는 크기로
        SetConsoleWindowInfo(HOUT, TRUE, &tiny);
        SetConsoleScreenBufferSize(HOUT, buf);
        SetConsoleWindowInfo(HOUT, TRUE, &rect);
    }

    void restoreDefaults() {
        setWindowAndBuffer(g_defWinCols, g_defWinLines,
            g_defBufCols, g_defBufLines);

        // 색/커서/화면 초기화
        setAttr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        hideCursor(false);
        // 화면 클리어
        CONSOLE_SCREEN_BUFFER_INFO info{};
        GetConsoleScreenBufferInfo(HOUT, &info);
        COORD origin{ 0,0 };
        DWORD n, total = info.dwSize.X * info.dwSize.Y;
        FillConsoleOutputCharacterW(HOUT, L' ', total, origin, &n);
        FillConsoleOutputAttribute(HOUT,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
            total, origin, &n);
        SetConsoleCursorPosition(HOUT, origin);
    }

    void wprintCentered(int y, const wchar_t* text) {
        int cols, rows; getConsoleSize(cols, rows);
        int w = consoleCellWidth(text);
        int x = (cols - w) / 2 + 1;
        wprintAt(x, y, text);
    }
    void wprintCenteredLn(int& y, const wchar_t* text) {
        wprintCentered(y, text);
        ++y;
    }

}
