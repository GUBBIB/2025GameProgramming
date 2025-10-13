#pragma once
#include <windows.h>
#include <wchar.h>

namespace Console {

	// 전역 핸들 (헤더에는 extern만!)
	extern HANDLE HOUT;
	extern HANDLE HIN;

	// 초기화: 코드페이지/타이틀/창 크기 등
	void init(int cols = 150, int lines = 50, const wchar_t* title = L"자리 배치 프로그램");

	// 기본 유틸
	void setAttr(WORD attr);
	void gotoxy(int x, int y);
	void hideCursor(bool hide);
	void get_cursor_1based(short* x, short* y);
	void clearAt(int x, int y, int width = 80);
	void wprintAt(int x, int y, const wchar_t* text);
	void getConsoleSize(int& cols, int& rows);
	int  consoleCellWidth(const wchar_t* s); 
	void setWindowAndBuffer(int winCols, int winLines, int bufCols, int bufLines);
	void restoreDefaults();
	void wprintCentered(int y, const wchar_t* text);      // 한 줄 가운데 출력
	void wprintCenteredLn(int& y, const wchar_t* text);
}