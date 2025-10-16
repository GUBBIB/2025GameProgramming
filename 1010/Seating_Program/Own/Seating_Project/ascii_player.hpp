#pragma once
#define NOMINMAX
#include "console.hpp"      
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <conio.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std;
using namespace Gdiplus;
using namespace Console;

// RGB 색 구조체
struct RGB { BYTE r, g, b; };
// 아스키 아트 프레임 구조체
struct AsciiFrame {
    vector<wstring> lines;  // 한 프레임의 콘솔 라인들
    int delay_ms = 33;      // 프레임 딜레이

    vector<vector<RGB>> colors; // lines와 동일 shape, [row][col] 색

};

// 콘솔에서 GIF를 아스키 아트로 재생
// - gifPath : 재생할 GIF 경로
// - targetCols : 가로 최대 칸(문자) 수 (예: 120)
// - fps       : 초당 프레임 (0이면 GIF 디폴트 딜레이 사용)
// - header    : 맨 위 가운데 정렬 텍스트
// - footer    : 맨 아래 가운데 정렬 텍스트
// - topMargin, midGap, botMargin : 헤더/영상/풋터 사이 여백(줄)
bool play_gif_ascii(const std::wstring& gifPath,
    int targetCols,
    int fps,
    const std::wstring& header,
    const std::wstring& footer,
    int topMargin = 2,
    int midGap = 2,
    int botMargin = 1);

// 비디오 다시 재생/일시 중지
bool ascii_player_should_stop();
void ascii_player_request_stop();
// 비디오 재생
void play_ascii_video(const std::wstring& gifPath,
    const std::wstring& header,
    const std::wstring& footer,
    int preferCols = -1,   // -1이면 콘솔 폭 기준으로 자동
    int fps = 0,           // 0이면 GIF 기본 딜레이 사용
    int topMargin = 1,
    int midGap = 2,
    int botMargin = 1);

// 내부 구현용
wchar_t pix2ch(unsigned char y);
// VT 모드 활성화
void enableVtColors();
// 콘솔 문자 크기에 맞춰 목표 크기 계산
void calcTargetSize(int srcW, int srcH, int maxCols, int maxRows, int& outCols, int& outRows);
// GIF 파일을 아스키 프레임 벡터로 변환
bool loadGifToAsciiFrames(const std::wstring& path, int targetCols, int maxRows, vector<AsciiFrame>& out);   