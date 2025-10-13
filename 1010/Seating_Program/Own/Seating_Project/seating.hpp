#pragma once
#include <vector>

struct Grid {
    int rows, cols;   // 좌석 행/열
    int originX;      // 표 시작 X(1-based)
    int originY;      // 표 시작 Y(1-based)
    int cellW = 4;    // │ + '───'
    int cellH = 2;    // 내용 1줄 + 구분 1줄
};

void draw_check01();
void draw_check02(int c, int r);
void renderSeat(const Grid& g, int r, int c, int val, bool selected);
void seat_arrangement(int row, int column, int people);
