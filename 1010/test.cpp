// seat_arrangement.cpp  (UTF-8로 저장)
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <iomanip>
#include <windows.h>
#include <conio.h>
#include <wchar.h>

using namespace std;

void seat_arrangement(int row, int column, int people);
void draw_check02(int column, int row); // 유니코드 박스 그리기(UTF-8)
void gotoxy(int x, int y);
void main_screen();

// ==== 콘솔 핸들 ====
HANDLE HOUT = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE HIN = GetStdHandle(STD_INPUT_HANDLE);

// ==== 유틸 ====
static void get_cursor_1based(short *x, short *y)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(HOUT, &info);
    *x = static_cast<short>(info.dwCursorPosition.X + 1);
    *y = static_cast<short>(info.dwCursorPosition.Y + 1);
}
void gotoxy(int x, int y)
{
    COORD pos{(SHORT)(x - 1), (SHORT)(y - 1)};
    SetConsoleCursorPosition(HOUT, pos);
}
void setAttr(WORD attr) { SetConsoleTextAttribute(HOUT, attr); }

// ==== 좌표/그리드 파라미터 ====
struct Grid
{
    int rows, cols; // 좌석 행/열
    int originX;    // 표 시작 X(1-based)
    int originY;    // 표 시작 Y(1-based)
    int cellW = 4;  // 가로 주기(│ + '───' 3칸) => 4
    int cellH = 2;  // 세로 주기(내용 1줄 + 구분 1줄) => 2
};

// 셀 하나 다시 그리기 (선택 시 배경색 하이라이트)
void renderSeat(const Grid &g, int r, int c, int val, bool selected)
{
    int y = g.originY + 1 + (r - 1) * g.cellH;
    int x = g.originX + 2 + (c - 1) * g.cellW;
    gotoxy(x, y);
    WORD norm = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    WORD sel = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED; // 밝은 회색 배경
    setAttr(selected ? sel : norm);
    if (val > 0)
        cout << setw(3) << val;
    else
        cout << "   ";
    setAttr(norm);
    cout.flush();
}

// 콘솔 입력 모드 on/off
void enableInteractiveInput(DWORD &oldMode)
{
    GetConsoleMode(HIN, &oldMode);
    DWORD m = oldMode;
    m |= ENABLE_EXTENDED_FLAGS;
    m |= ENABLE_WINDOW_INPUT;
    m &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    SetConsoleMode(HIN, m);

    // 커서 숨기기(선택)
    CONSOLE_CURSOR_INFO ci{25, FALSE};
    SetConsoleCursorInfo(HOUT, &ci);
}
void restoreInputMode(DWORD oldMode)
{
    SetConsoleMode(HIN, oldMode);
    CONSOLE_CURSOR_INFO ci{25, TRUE};
    SetConsoleCursorInfo(HOUT, &ci);
}

// 콘솔 한 지점에 공백 채우기(지우기)
void clearAt(int x, int y, int width = 80)
{
    COORD pos{(SHORT)(x - 1), (SHORT)(y - 1)};
    DWORD w;
    FillConsoleOutputCharacterW(HOUT, L' ', width, pos, &w);
    FillConsoleOutputAttribute(HOUT, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, width, pos, &w);
}

// 좌표에 유니코드 문자열을 '정확히' 출력 (자동 줄바꿈/버퍼 영향 없음)
void wprintAt(int x, int y, const wchar_t *text)
{
    COORD pos{(SHORT)(x - 1), (SHORT)(y - 1)};
    DWORD w;
    WriteConsoleOutputCharacterW(HOUT, text, (DWORD)wcslen(text), pos, &w);
}

// 현재 콘솔 버퍼의 가로/세로 크기 얻기
void getConsoleSize(int &cols, int &rows)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(HOUT, &info);
    cols = info.srWindow.Right - info.srWindow.Left + 1;
    rows = info.srWindow.Bottom - info.srWindow.Top + 1;
}

// 유니코드 문자열의 '콘솔 셀 폭' (전각=2, 반각=1)
int consoleCellWidth(const wchar_t *s)
{
    int w = 0;
    for (size_t i = 0; s[i]; ++i)
    {
        WORD type = 0;
        if (GetStringTypeW(CT_CTYPE3, &s[i], 1, &type) && (type & C3_FULLWIDTH))
            w += 2; // 전각
        else
            w += 1; // 반각
    }
    return w;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    system("mode con cols=150 lines=50");
    SetConsoleTitleW(L"자리 배치 프로그램");

    ios::sync_with_stdio(false);
    cin.tie(&cout);

    main_screen();
    return 0;
}

void main_screen()
{
    system("cls");

    cout << "                                                                                                                               \n";
    cout << "                                                                                                                               \n";
    cout << "                                                                                                                               \n";
    cout << "                                                                                                                               \n";
    cout << "                    #####   #######    ##     ######    ####    ##   ##    ####            ######   ####       ##     ##   ## \n";
    cout << "                   ##   ##   ##   #   ####    # ## #     ##     ###  ##   ##  ##            ##  ##   ##       ####    ###  ## \n";
    cout << "                   #         ## #    ##  ##     ##       ##     #### ##  ##                 ##  ##   ##      ##  ##   #### ## \n";
    cout << "                    #####    ####    ##  ##     ##       ##     ## ####  ##                 #####    ##      ##  ##   ## #### \n";
    cout << "                        ##   ## #    ######     ##       ##     ##  ###  ##  ###            ##       ##   #  ######   ##  ### \n";
    cout << "                   ##   ##   ##   #  ##  ##     ##       ##     ##   ##   ##  ##            ##       ##  ##  ##  ##   ##   ## \n";
    cout << "                    #####   #######  ##  ##    ####     ####    ##   ##    #####           ####     #######  ##  ##   ##   ## \n\n";
    cout << "                                                                                                                               \n";
    cout << "                                                                                                                               \n";
    cout << "                                                                                                                               \n";
    cout << "                                                                                                                               \n";

    const int xText = 58;

    const int yItem0 = 21;
    const int yItem1 = 23;

    const wchar_t *item0 = L"[1] 자리 배치 프로그램 실행";
    const wchar_t *item1 = L"[ESC] 종료";

    int cols, rows;
    getConsoleSize(cols, rows);

    // 가로 가운데 X좌표 (1-based)
    int w0 = consoleCellWidth(item0);    int x0 = (cols - w0) / 2 - 1;
    int w1 = consoleCellWidth(item1);    int x1 = (cols - w1) / 2 + 1;

    // 줄 정리 후 텍스트 '좌표 고정 출력'
    clearAt(1, yItem0, cols);
    clearAt(1, yItem1, cols);
    wprintAt(x0, yItem0, item0);
    wprintAt(x1, yItem1, item1);

    // 포인터(>도 유니코드로 그리자)
    const wchar_t *POINTER = L">";
    const int pX = x0 - 3;
    int pY = yItem0;

    auto drawPointer = [&](int newY)
    {
        if (pY != -1)
            clearAt(pX, pY, 2);      // 이전 위치 지우기(2칸 안전)
        wprintAt(pX, newY, POINTER); // 새 위치 그리기
        pY = newY;
    };

    // 선택 상태 & 최초 포인터(요구사항 3)
    int selected = 0; // 0: 실행, 1: 종료
    int selY[2] = {yItem0, yItem1};
    drawPointer(selY[selected]); // ← 시작하자마자 "자리 배치…" 가리킴
    
    const int item0L = xText, item0T = yItem0, item0R = xText + (int)wcslen(item0) - 1, item0B = yItem0;
    const int item1L = xText, item1T = yItem1, item1R = xText + (int)wcslen(item1) - 1, item1B = yItem1;

    // 입력 모드
    DWORD oldMode;
    GetConsoleMode(HIN, &oldMode);
    DWORD newMode = oldMode | ENABLE_EXTENDED_FLAGS;
    newMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(HIN, newMode);

    // 커서 숨김
    CONSOLE_CURSOR_INFO ci{25, FALSE};
    SetConsoleCursorInfo(HOUT, &ci);

    // 이벤트 루프
    INPUT_RECORD ir;
    DWORD nRead;
    bool running = true;
    while (true)
    {
        int ch = _getch();

        if (ch == 27) { // ESC
            system("cls");
            wcout << L"프로그램을 종료합니다...\n";
            return;
        }

        if (ch == 224 || ch == 0) {
            // 방향키: 두 번째 코드로 판단
            int a = _getch();
            int before = selected;
            if (a == 72) selected = max(0, selected - 1);  // ↑
            if (a == 80) selected = min(1, selected + 1);  // ↓
            if (selected != before) drawPointer(selY[selected]);
            continue;
        }

        if (ch == '\r' || ch == ' ' || ch == '1') {
            if (selected == 0 || ch == '1') {
                // 실행
                CONSOLE_CURSOR_INFO ci{25, TRUE};
                SetConsoleCursorInfo(HOUT, &ci);
                system("cls");
                int row, col, people;
                cout << "행의 수를 입력하고 Enter>";  cin >> row;
                cout << "열의 수를 입력하고 Enter>";  cin >> col;
                cout << "자리에 앉힐 인원수를 입력하고 Enter>";  cin >> people;
                seat_arrangement(row, col, people);
                cout << "\n\n아무 키나 누르면 메인으로..."; _getch();
                return main_screen(); // 메뉴 복귀
            } else {
                system("cls");
                wcout << L"프로그램을 종료합니다...\n";
                return;
            }
        }
    }

    // 원복
    SetConsoleMode(HIN, oldMode);
    ci.bVisible = TRUE;
    SetConsoleCursorInfo(HOUT, &ci);
    system("cls");
    cout << "프로그램을 종료합니다...\n";
}

void seat_arrangement(int row, int column, int people)
{
    const int total = row * column;
    if (total <= 0)
        return;

    if (people < 0)
        people = 0;
    if (people > total)
        people = total;

    // 좌석값: 1..people + 0(공석)
    vector<int> seats(total, 0);
    for (int i = 0; i < people; ++i)
        seats[i] = i + 1;
    mt19937 gen(random_device{}());
    shuffle(seats.begin(), seats.end(), gen);

    // 표 시작 좌표(현재 커서)
    short ox, oy;
    get_cursor_1based(&ox, &oy);

    // 표 그리기
    draw_check02(column, row);

    // 그리드 정보
    Grid g{row, column, ox, oy};

    // 초기 렌더
    for (int r = 1; r <= row; ++r)
        for (int c = 1; c <= column; ++c)
            renderSeat(g, r, c, seats[(r - 1) * column + (c - 1)], false);

    // 선택 상태와 현재 커서 위치
    vector<char> selected(total, 0);
    int curR = 1, curC = 1;
    renderSeat(g, curR, curC, seats[(curR - 1) * column + (curC - 1)], true);

    // 콘솔 입력 모드 활성화
    DWORD oldMode;
    enableInteractiveInput(oldMode);

    cout << "\n[마우스] 클릭=토글  [←→↑↓] 이동  [Space/Enter] 토글  [Esc] 종료\n"
         << flush;

    // 이벤트 루프
    INPUT_RECORD rec;
    DWORD nRead;
    bool running = true;
    while (running)
    {
        int ch = _getch();
        if (ch == 27) { // ESC
            break;
        }
        if (ch == 224 || ch == 0) {
            int a = _getch();
            int prevR = curR, prevC = curC;
            if (a == 75 && curC > 1)        --curC;     // ←
            else if (a == 77 && curC < column) ++curC;  // →
            else if (a == 72 && curR > 1)     --curR;    // ↑
            else if (a == 80 && curR < row)   ++curR;    // ↓

            if (prevR != curR || prevC != curC) {
                renderSeat(g, prevR, prevC, seats[(prevR-1)*column+(prevC-1)],
                           selected[(prevR-1)*column+(prevC-1)]);
                renderSeat(g, curR, curC, seats[(curR-1)*column+(curC-1)], true);
            }
            continue;
        }
        if (ch == ' ' || ch == '\r') {
            int idx = (curR - 1) * column + (curC - 1);
            selected[idx] ^= 1;
            renderSeat(g, curR, curC, seats[idx], true); // 강조 유지
        }
    }

    // 입력 모드 원복
    restoreInputMode(oldMode);
}

// ───────────────────────────────────────────────────────────

void draw_check02(int c, int r)
{
    // UTF-8 유니코드 박스: ┌─┬─┐ / │ │ │ / ├─┼─┤ / └─┴─┘
    cout << "┌";
    for (int i = 0; i < c; ++i)
        cout << "───" << (i == c - 1 ? "┐" : "┬");
    cout << "\n";

    for (int i = 0; i < r; ++i)
    {
        cout << "│";
        for (int j = 0; j < c; ++j)
            cout << "   │";
        cout << "\n";

        if (i != r - 1)
        {
            cout << "├";
            for (int j = 0; j < c; ++j)
                cout << "───" << (j == c - 1 ? "┤" : "┼");
            cout << "\n";
        }
    }

    cout << "└";
    for (int i = 0; i < c; ++i)
        cout << "───" << (i == c - 1 ? "┘" : "┴");
    cout << "\n";
}
