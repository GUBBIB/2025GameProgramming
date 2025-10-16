#pragma once
#include <windows.h>
#include <wchar.h>
#include <io.h>
#include <fcntl.h>
#include <algorithm>
#include <iostream>

namespace Console {

	// 전역 핸들
	extern HANDLE HOUT;
	extern HANDLE HIN;

	// 초기화: 코드페이지/타이틀/창 크기 등
	void init(int cols = 150, int lines = 50, const wchar_t* title = L"자리 배치 프로그램");

	// 기본 유틸
	void setAttr(WORD attr);
	// 포인터 위치 이동
	void gotoxy(int x, int y);
	// 포인터 숨김
	void hideCursor(bool hide);
	// 현재 포인터 위치 얻기
	void get_cursor_1based(short* x, short* y);
	// x * y 위치의 width 칸 지우기
	void clearAt(int x, int y, int width = 80);
	// x * y 위치에 문자열 출력
	void wprintAt(int x, int y, const wchar_t* text);
	// 현재 콘솔 창 크기 얻기
	void getConsoleSize(int& cols, int& rows);
	// 문자열의 콘솔 셀 너비 계산
	int  consoleCellWidth(const wchar_t* s);
	// 창 크기와 버퍼 크기 설정
	void setWindowAndBuffer(int winCols, int winLines, int bufCols, int bufLines);
	// 초기 설정 복원
	void restoreDefaults();
	// 한 줄 가운데 출력
	void wprintCentered(int y, const wchar_t* text);      

}

