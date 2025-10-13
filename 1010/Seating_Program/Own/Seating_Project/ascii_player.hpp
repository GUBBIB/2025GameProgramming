#pragma once
#include <string>

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

// 재생을 사용자 입력으로 끊고 싶으면 이 함수를 주기적으로 체크
bool ascii_player_should_stop();
void ascii_player_request_stop();

void play_ascii_video(const std::wstring& gifPath,
    const std::wstring& header,
    const std::wstring& footer,
    int preferCols = -1,   // -1이면 콘솔 폭 기준으로 자동
    int fps = 0,           // 0이면 GIF 기본 딜레이 사용
    int topMargin = 1,
    int midGap = 2,
    int botMargin = 1);