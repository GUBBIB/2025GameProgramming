// menu.cpp
#include "menu.hpp"

using namespace std;
using namespace Console;

void main_screen() {
    restoreDefaults();
    system("cls");

    wcout << L"\n\n\n\n";
    wcout << L"                    #####   #######    ##     ######    ####    ##   ##    ####            ######   ####       ##     ##   ## \n";
    wcout << L"                   ##   ##   ##   #   ####    # ## #     ##     ###  ##   ##  ##            ##  ##   ##       ####    ###  ## \n";
    wcout << L"                   #         ## #    ##  ##     ##       ##     #### ##  ##                 ##  ##   ##      ##  ##   #### ## \n";
    wcout << L"                    #####    ####    ##  ##     ##       ##     ## ####  ##                 #####    ##      ##  ##   ## #### \n";
    wcout << L"                        ##   ## #    ######     ##       ##     ##  ###  ##  ###            ##       ##   #  ######   ##  ### \n";
    wcout << L"                   ##   ##   ##   #  ##  ##     ##       ##     ##   ##   ##  ##            ##       ##  ##  ##  ##   ##   ## \n";
    wcout << L"                    #####   #######  ##  ##    ####     ####    ##   ##    #####           ####     #######  ##  ##   ##   ## \n\n";
    wcout << L"\n\n\n\n";

    const wchar_t* item0 = L"[1] 자리 배치 프로그램 실행";
    const wchar_t* item1 = L"[2] 심심해서 만든 영상";
    const wchar_t* item2 = L"[ESC] 종료";

    int cols, rows; getConsoleSize(cols, rows);
    const int yItem0 = 21;
    const int yItem1 = 23;
    const int yItem2 = 25;

    int x0 = (cols - consoleCellWidth(item0)) / 2 + 1;
    int x1 = (cols - consoleCellWidth(item1)) / 2 + 1;
    int x2 = (cols - consoleCellWidth(item2)) / 2 + 1;

    clearAt(1, yItem0, cols);
    clearAt(1, yItem1, cols);
    clearAt(1, yItem2, cols);
    wprintAt(x0, yItem0, item0);
    wprintAt(x1, yItem1, item1);
    wprintAt(x2, yItem2, item2);

    const wchar_t* POINTER = L">";
    int selX[3] = { x0, x1, x2 };
    int selY[3] = { yItem0, yItem1, yItem2 };

    int selected = 0;
    hideCursor(true);

    // 포인터 그리기 함수: 선택 인덱스로 그려주기
    auto drawPointer = [&](int oldSel, int newSel) {
        // 이전 포인터 지우기
        if (oldSel >= 0) {
            clearAt(selX[oldSel] - 3, selY[oldSel], 2);
        }
        // 새 포인터 그리기
        wprintAt(selX[newSel] - 3, selY[newSel], POINTER);
        };

    drawPointer(-1, selected);
    while (true) {
        int ch = _getch();

        if (ch == 27) { // ESC
            system("cls");
            wcout << L"프로그램을 종료합니다...\n";
            hideCursor(false);
            return;
        }

        if (ch == 224 || ch == 0) {
            int a = _getch();
            int before = selected;
            if (a == 72) selected = max(0, selected - 1); // ↑
            if (a == 80) selected = min(2, selected + 1); // ↓  
            if (selected != before) drawPointer(before, selected);
            continue;
        }

        // 숫자키 바로가기
        if (ch == '1') { selected = 0; }
        else if (ch == '2') { selected = 1; }
        else if (ch == '3') { selected = 2; }

        if (ch == '\r' || ch == ' ' || ch == '1' || ch == '2' || ch == '3') {
            if (selected == 0) {
                draw_check01();
                return main_screen();
            }
            else if (selected == 1) {
                play_ascii_video(L"assets\\mario_star.gif",
                    L"SEATING PROGRAM",
                    L"made by 장문용 & GPT",
                    -1,   // 콘솔 폭 기준 자동
                    0,    // GIF 딜레이 사용(고정 FPS 쓰려면 12~15 추천)
                    1, 2, 1);
                return main_screen();
            }
            else if (selected == 2) {
                system("cls");
                wcout << L"프로그램을 종료합니다...\n";
                hideCursor(false);
                return;
            }
        }
    }
}