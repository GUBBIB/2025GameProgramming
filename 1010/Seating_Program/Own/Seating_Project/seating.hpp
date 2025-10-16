#pragma once
#define NOMINMAX
#include "console.hpp"
#include <vector>
#include <windows.h>
#include <conio.h>
#include <random>
#include <iomanip>
#include <iostream>
#include <fstream>   
#include <string>
#include <ctime>     

using namespace std;
using namespace Console;

// 그리드 정보 구조체
struct Grid {
    int rows, cols;   // 좌석 행/열
    int originX;      // 표 시작 X(1-based)
    int originY;      // 표 시작 Y(1-based)
    int cellW = 4;    // │ + '───'
    int cellH = 2;    // 내용 1줄 + 구분 1줄
};

// 하단 메뉴 액션
enum class SeatAction { None, Reroll, Export, ChangeDir, Back };

// 좌석 배치 프로그램 메인 출력
void draw_check01();
// 그리드 그리기
void draw_check02(int L, int T, int c, int r);
// 한 좌석 렌더링
void renderSeat(const Grid& g, int r, int c, int val, bool selected);
// 좌석 배치 프로그램 메인
void seat_arrangement(int row, int column, int people);
// 하단 메뉴 표시 및 선택
SeatAction showBottomMenu(int left, int top);
// 하단 메뉴 블록 지우기
void clearBottomBlock(int lines = 4);
// 내보내기 위치 프롬프트
bool promptExportDir(std::wstring& exportDir);
// 경로 합치기
wstring joinPath(const wstring& dir, const wstring& file);
// 좌석 재배치 (locked[i] == 1 이면 고정)
void rerollSeats(vector<int>& seats, const vector<char>& locked);
// wstring을 UTF-8 string으로 변환
string w2u8(const wstring& ws);
// UTF-8 텍스트 파일로 내보내기
bool exportSeatsTxt_UTF8(const vector<int>& seats, int rows, int cols, const wstring& path);
// 축 레이블 그리기
void drawAxesLabels(const Grid& g, int rows, int cols, int step = 1);
// ESC로 취소 가능, 한 줄 입력 (공백·한글 포함)
bool readLineWithEsc(std::wstring& out, int x, int y);
