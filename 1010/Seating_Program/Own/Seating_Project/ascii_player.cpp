#define NOMINMAX
#include "ascii_player.hpp"
#include "console.hpp"      // 네가 쓰는 Console 유틸 (gotoxy, wprintCentered 등)
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <conio.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iostream>

#pragma comment(lib, "gdiplus.lib")

using namespace std;
using namespace Gdiplus;
using namespace Console;

struct RGB { BYTE r, g, b; };

static volatile bool g_stop = false;
bool ascii_player_should_stop() { return g_stop; }
void ascii_player_request_stop() { g_stop = true; }

// 밝기→문자 매핑(어두움→밝음)
static const wchar_t* LUT = L" ░▒▓█"; // 길이 10

struct AsciiFrame {
    vector<wstring> lines;  // 한 프레임의 콘솔 라인들
    int delay_ms = 33;      // 프레임 딜레이

    vector<vector<RGB>> colors; // lines와 동일 shape, [row][col] 색

};

// 픽셀(0~255)→문자
static wchar_t pix2ch(unsigned char y) {
    size_t idx = (size_t)((y / 255.0) * (wcslen(LUT) - 1));
    return LUT[idx];
}

static void enableVtColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) {
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
        SetConsoleMode(hOut, mode);
    }
}

// 콘솔 셀 가로:세로 비율 보정(폰트에 따라 다르지만 대충 문자 높이가 더 큼)
// 문자 한 칸이 가로 1, 세로 2 정도라고 보고 원본 높이를 0.5배로 맞춤.
static void calcTargetSize(int srcW, int srcH, int maxCols, int maxRows,
    int& outCols, int& outRows)
{
    if (srcW <= 0 || srcH <= 0) { outCols = outRows = 0; return; }

    // 문자 종횡비 보정: 세로는 더 “비싸다”
    double aspect = (double)srcW / (double)srcH;          // 원본 가로/세로
    double charAR = 0.5;                                   // 한 글자당 (가로/세로) 보정

    // 목표는 outCols <= maxCols, outRows <= maxRows를 만족하면서 최대 크기
    int bestC = max(1, min(maxCols, srcW));
    int bestR = max(1, min(maxRows, (int)((double)bestC / aspect / charAR)));

    // 세로 제한 넘으면 다시 맞추기
    if (bestR > maxRows) {
        bestR = maxRows;
        bestC = (int)(bestR * aspect * charAR);
        bestC = max(1, min(bestC, maxCols));
    }
    outCols = bestC;
    outRows = bestR;
}

static bool loadGifToAsciiFrames(const std::wstring& path,
    int targetCols, int maxRows,
    vector<AsciiFrame>& out)
{
    out.clear();

    // GDI+ 시작
    GdiplusStartupInput si{};
    ULONG_PTR token = 0;
    if (GdiplusStartup(&token, &si, nullptr) != Ok) return false;

    Bitmap* bmp = Bitmap::FromFile(path.c_str(), FALSE);
    if (!bmp || bmp->GetLastStatus() != Ok) { delete bmp; GdiplusShutdown(token); return false; }

    // 프레임 차원(애니 GIF)
    UINT count = bmp->GetFrameDimensionsCount();
    vector<GUID> ids(count);
    bmp->GetFrameDimensionsList(ids.data(), count);
    UINT frameCount = bmp->GetFrameCount(&ids[0]);

    // 프레임 딜레이(1/100초 단위, PropertyTagFrameDelay)
    UINT size = bmp->GetPropertyItemSize(PropertyTagFrameDelay);
    vector<BYTE> buf(size);
    PropertyItem* pDel = nullptr;
    if (size && bmp->GetPropertyItem(PropertyTagFrameDelay, size, (PropertyItem*)buf.data()) == Ok) {
        pDel = (PropertyItem*)buf.data();
    }

    // 콘솔에서 사용할 가장 큰 캔버스 크기 계산(원본 기준)
    int srcW = bmp->GetWidth();
    int srcH = bmp->GetHeight();

    int cols, rows;
    calcTargetSize(srcW, srcH, targetCols, maxRows, cols, rows);
    if (cols <= 0 || rows <= 0) { delete bmp; GdiplusShutdown(token); return false; }

    // 프레임별로 ASCII 생성
    out.reserve(frameCount);
    for (UINT i = 0; i < frameCount; ++i) {
        bmp->SelectActiveFrame(&ids[0], i);

        // 원본을 rows x cols 로 다운샘플링하며 아스키 변환
        AsciiFrame af;
        af.lines.resize(rows);
        af.colors.assign(rows, vector<RGB>(cols)); // ← 색 버퍼 준비

        // 딜레이
        if (pDel && pDel->length >= 4 * frameCount) {
            UINT* d = (UINT*)pDel->value;
            af.delay_ms = (int)d[i] * 10; // 1/100초 → ms
            if (af.delay_ms <= 0) af.delay_ms = 33;
        }

        // 간단한 최근접 샘플링으로 픽셀 접근
        for (int r = 0; r < rows; ++r) {
            wstring line; line.reserve(cols);
            double srcY = (r + 0.5) * srcH / rows;
            int y = (int)min<double>(max<double>(srcY, 0), srcH - 1);

            for (int c = 0; c < cols; ++c) {
                double srcX = (c + 0.5) * srcW / cols;
                int x = (int)min<double>(max<double>(srcX, 0), srcW - 1);

                Color col; bmp->GetPixel(x, y, &col);

                // 밝기 Y = 0.299R + 0.587G + 0.114B
                unsigned char Y = (unsigned char)min<int>(255,
                    (int)(0.299 * col.GetR() + 0.587 * col.GetG() + 0.114 * col.GetB()));
                line.push_back(pix2ch(Y));

                af.colors[r][c] = RGB{ (BYTE)col.GetR(), (BYTE)col.GetG(), (BYTE)col.GetB() };
            }
            af.lines[r] = std::move(line);
        }

        out.push_back(std::move(af));
    }

    delete bmp;
    GdiplusShutdown(token);
    return !out.empty();
}


void play_ascii_video(const std::wstring& gifPath,
    const std::wstring& header,
    const std::wstring& footer,
    int preferCols,
    int fps,
    int topMargin,
    int midGap,
    int botMargin)
{
    // 화면 상태 초기화
    Console::restoreDefaults();
    system("cls");
    Console::hideCursor(true);

    enableVtColors();

    // 콘솔 폭에 맞춰 가로폭 자동 결정
    int cols, rows;
    Console::getConsoleSize(cols, rows);

    // preferCols가 유효하면 그 값, 아니면 콘솔 폭에서 좌우 여백 조금 빼서 사용
    int targetCols = (preferCols > 0 ? preferCols
        : std::max(40, cols - 10)); // 최소 40칸

    // 재생 (Esc/Q 로 종료)
    (void)play_gif_ascii(gifPath,
        targetCols,
        fps,
        header,
        footer,
        topMargin,
        midGap,
        botMargin);

    // 끝난 뒤 화면 정리
    Console::hideCursor(false);
}


bool play_gif_ascii(const std::wstring& gifPath,
    int targetCols,
    int fps,
    const std::wstring& header,
    const std::wstring& footer,
    int topMargin,
    int midGap,
    int botMargin)
{
    g_stop = false;

    int cols, rows;
    getConsoleSize(cols, rows);

    // 화면에 맞게 프레임 변환
    vector<AsciiFrame> frames;
    if (!loadGifToAsciiFrames(gifPath, targetCols, max(1, rows - (topMargin + midGap + botMargin + 2)), frames))
        return false;

    // 레이아웃 계산
    int videoH = (int)frames[0].lines.size();
    int videoW = (int)frames[0].lines[0].size();

    // 헤더/풋터 한 줄씩
    int totalH = topMargin + 1 + midGap + videoH + botMargin + 1;
    // 수직 배치 시작 y(중앙에 가깝게)
    int yStart = max(1, (rows - totalH) / 2 + 1);

    // 헤더
    int yHeader = yStart + topMargin;
    // 비디오
    int yVideo = yHeader + 1 + midGap;
    // 풋터
    int yFooter = yVideo + videoH + botMargin;

    // 수평 가운데(x는 각 줄마다 가운데로)
    auto drawHeaderFooter = [&]() {
        clearAt(1, yHeader, cols);
        wprintCentered(yHeader, header.c_str());
        clearAt(1, yFooter, cols);
        wprintCentered(yFooter, footer.c_str());
        };

    drawHeaderFooter();

    // 비디오 좌측 시작 x
    int xVideo = (cols - videoW) / 2 + 1;

    // 재생 루프
    size_t idx = 0;
    auto last = chrono::steady_clock::now();

    // 프레임 고정 fps(옵션)
    int fixedDelay = (fps > 0) ? (1000 / fps) : -1;

    while (!g_stop) {
        const AsciiFrame& f = frames[idx];

        // 프레임 그리기(색 포함)
        for (int r = 0; r < videoH; ++r) {
            gotoxy(xVideo, yVideo + r);     // 커서 이동 (wprintAt 말고 커서이동 + 직접 출력)
            int pr = -1, pg = -1, pb = -1;  // 현재 라인에 설정된 색(변할 때만 ANSI 전송)

            for (int c = 0; c < videoW; ++c) {
                const RGB& rgb = f.colors[r][c];

                if (rgb.r != pr || rgb.g != pg || rgb.b != pb) {
                    // 24-bit 전경색 설정: ESC[38;2;R;G;Bm
                    wcout << L"\x1b[38;2;"
                        << (int)rgb.r << L";"
                        << (int)rgb.g << L";"
                        << (int)rgb.b << L"m";
                    pr = rgb.r; pg = rgb.g; pb = rgb.b;
                }

                wcout << f.lines[r][c];
            }
            // 라인 끝나면 색 초기화
            wcout << L"\x1b[0m";
        }

        // 입력 종료
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 27 || ch == 'q' || ch == 'Q') break;
        }

        // 프레임 동기
        int waitMs = (fixedDelay > 0 ? fixedDelay : f.delay_ms);
        auto now = chrono::steady_clock::now();
        auto spent = chrono::duration_cast<chrono::milliseconds>(now - last).count();
        if (spent < waitMs) this_thread::sleep_for(chrono::milliseconds(waitMs - spent));
        last = chrono::steady_clock::now();

        idx = (idx + 1) % frames.size();
    }

    return true;
}
