#include "seating.hpp"

using namespace std;
using namespace Console;

static std::wstring g_exportDir = L".";

SeatAction showBottomMenu(int left, int top) {
    const wchar_t* items[] = {
        L"[1] 다시 배치 (고정 제외)",
        L"[2] 내보내기 TXT",
        L"[3] 내보내기 위치 수정",
        L"[ESC] 메뉴창 끄기"
    };
    const int N = 4;

    int cols, rows; getConsoleSize(cols, rows);

    // 클리어 & 그리기
    for (int i = 0; i < N; ++i) {
        clearAt(1, top + i, cols);
        wprintAt(left, top + i, items[i]);
    }

    // 포인터
    int sel = 0;
    auto drawPtr = [&](int oldSel, int newSel) {
        if (oldSel != -1) clearAt(left - 3, top + oldSel, 2);
        wprintAt(left - 3, top + newSel, L">");
        };
    drawPtr(-1, sel);

    // 조작
    for (;;) {
        int ch = _getch();
        if (ch == 27) return SeatAction::None; // ESC로 닫기

        if (ch == 224 || ch == 0) {
            int a = _getch();
            int old = sel;
            if (a == 72) sel = max(0, sel - 1);     // ↑
            if (a == 80) sel = min(N - 1, sel + 1);   // ↓
            if (old != sel) drawPtr(old, sel);
            continue;
        }
        if (ch == '1') return SeatAction::Reroll;
        if (ch == '2') return SeatAction::Export;
        if (ch == '3') return SeatAction::ChangeDir;

        if (ch == '\r' || ch == ' ') {
            if (sel == 0) return SeatAction::Reroll;
            else if (sel == 1) return SeatAction::Export;
            else if (sel == 2) return SeatAction::ChangeDir;
            else               return SeatAction::Back;
        }
    }
}

// 메뉴가 차지하는 라인 블록 지우기
void clearBottomBlock(int lines) {
    int cols, rows; getConsoleSize(cols, rows);
    int baseY = rows - lines;                 // 메뉴 시작 y
    for (int i = 0; i < lines; ++i)
        clearAt(1, baseY + i, cols);
}

static bool ensureDirectory(const wstring& dir) {
    if (dir.empty()) return false;
    wstring path;
    path.reserve(dir.size());
    for (size_t i = 0; i < dir.size(); ++i) {
        wchar_t ch = dir[i];
        path.push_back(ch);
        // 경로 구분자에서 끊어서 생성 (C:\.. 처럼 드라이브 루트는 건너뜀)
        if (ch == L'\\' || ch == L'/') {
            if (path.size() <= 3) continue; // "C:\" 같은 루트는 skip
            CreateDirectoryW(path.c_str(), nullptr);
        }
    }
    // 마지막 경로까지 한 번 더 시도
    if (!CreateDirectoryW(dir.c_str(), nullptr)) {
        // 이미 있는 것은 OK
        if (GetLastError() != ERROR_ALREADY_EXISTS) return false;
    }
    return true;
}

bool promptExportDir(std::wstring& exportDir) {
    int cols, rows; getConsoleSize(cols, rows);
    int baseY = rows - 4;   // 하단 4줄 사용
    int left = 3;

    clearBottomBlock(4);

    // 현재 위치SS
    std::wstring cur = L"현재 위치 : " + exportDir;
    wprintAt(left, baseY, cur.c_str());

    // 프롬프트 출력
    const wchar_t* prompt = L"변경할 위치 : ";
    wprintAt(left, baseY + 1, prompt);

    // 커서를 프롬프트 끝으로 이동
    int xInput = left + consoleCellWidth(prompt);
    gotoxy(xInput, baseY + 1);

    // 입력 버퍼 정리 후 한 줄 입력 (공백·한글 포함)
    wcin.clear();
    wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');

    std::wstring in;
    std::getline(wcin, in);

    if (in.empty()) {
        clearBottomBlock(4);
        wprintAt(left, baseY, L"변경 취소");
        return false;
    }

    // 디렉터리 보장
    if (!ensureDirectory(in)) {
        clearBottomBlock(4);
        wprintAt(left, baseY, L"폴더 생성/접근 실패. 경로를 다시 확인하세요.");
        return false;
    }

    exportDir = in;

    clearBottomBlock(4);
    std::wstring done = L"변경 완료: " + exportDir;
    wprintAt(left, baseY, done.c_str());
    return true;
}

// 경로 결합 (간단히)
wstring joinPath(const wstring& dir, const wstring& file) {
    if (dir.empty()) return file;
    if (dir.back() == L'\\' || dir.back() == L'/') return dir + file;
    return dir + L"\\" + file;
}

void rerollSeats(vector<int>& seats, const vector<char>& locked)  // locked[i] == 1 이면 고정
{
    vector<int> idx;   idx.reserve(seats.size());
    vector<int> vals;  vals.reserve(seats.size());

    for (size_t i = 0; i < seats.size(); ++i) {
        if (!locked[i]) {           // 잠금되지 않은 칸만
            idx.push_back((int)i);
            vals.push_back(seats[i]);
        }
    }

    mt19937 gen(random_device{}());
    shuffle(vals.begin(), vals.end(), gen);  // 값들만 섞는다

    for (size_t k = 0; k < idx.size(); ++k)
        seats[idx[k]] = vals[k];                  // 다시 써 넣기
}

// UTF-16 → UTF-8
string w2u8(const wstring& ws) {
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0,
        ws.c_str(), (int)ws.size(),
        nullptr, 0, nullptr, nullptr);
    string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0,
        ws.c_str(), (int)ws.size(),
        &out[0], len, nullptr, nullptr);
    return out;
}
// 내보내기
bool exportSeatsTxt_UTF8(const vector<int>& seats,
    int rows, int cols,
    const wstring& path)
{
    ofstream ofs(path, ios::binary);
    if (!ofs) return false;

    const unsigned char bom[3] = { 0xEF,0xBB,0xBF };
    ofs.write((const char*)bom, 3);

    ofs << w2u8(L"좌석 배치 (" + to_wstring(rows)
        + L"x" + to_wstring(cols) + L")\r\n");

    for (int r = 0; r < rows; ++r) {
        wstring line;
        for (int c = 0; c < cols; ++c) {
            int v = seats[r * cols + c];
            line += (v > 0 ? to_wstring(v) : L".");
            if (c != cols - 1) line += L"\t";
        }
        line += L"\r\n";
        ofs << w2u8(line);
    }
    return true;
}

// 그리드 좌측(행) / 하단(열) 좌표 라벨 출력
void drawAxesLabels(const Grid& g, int rows, int cols, int step)
{
    int lx = g.originX - 4;
    if (lx < 1) lx = 1;

    for (int r = 1; r <= rows; r += step) {
        int y = g.originY + 1 + (r - 1) * g.cellH; // 내용줄
        gotoxy(lx, y);
        wcout << setw(3) << r;
    }

    int ty = g.originY - 1;                 // 윗면(바닥은 +, 윗면은 -)
    if (ty < 1) ty = 1;                     // 화면 위쪽 경계 보호

    for (int c = 1; c <= cols; c += step) {
        int x = g.originX + 1 + (c - 1) * g.cellW; // 각 셀의 숫자 중앙
        gotoxy(x, ty);
        wcout << setw(3) << c;
    }
    wcout.flush();
}

bool readLineWithEsc(std::wstring& out, int x, int y) {
    out.clear();
    gotoxy(x, y);

    for (;;) {
        int ch = _getwch();

        // 특수키 prefix(0 또는 224) => 다음 코드 먹고 무시
        if (ch == 0 || ch == 224) { _getwch(); continue; }

        if (ch == 27) {                 // ESC
            return false;
        }
        if (ch == L'\r' || ch == L'\n') { // Enter
            return true;
        }
        if (ch == L'\b' || ch == 127) { // Backspace
            if (!out.empty()) {
                out.pop_back();
                // 화면에서 한 글자 지우기
                gotoxy(x + (int)out.size(), y);
                wcout << L' ' << flush;
                gotoxy(x + (int)out.size(), y);
            }
            continue;
        }
        if (iswprint(ch)) {             // 출력 가능한 문자
            out.push_back((wchar_t)ch);
            wcout << (wchar_t)ch << flush;
        }
    }
}

void draw_check01() {
    hideCursor(false);
    system("cls");

    int cols, rows;
    getConsoleSize(cols, rows);

    const int xPrompt = max(1, (int)(cols * 0.05));
    int y = 3;

    auto askInt = [&](const wchar_t* label) -> int {
        int v;
        for (;;) {
            clearAt(1, y, cols);
            gotoxy(xPrompt, y);
            wcout << label << flush;

            short cx, cy;
            get_cursor_1based(&cx, &cy);
            gotoxy(cx, cy);

            if (!(wcin >> v)) { // 숫자 아님
                wcin.clear();
                wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
                clearAt(1, y + 1, cols);
                wprintAt(xPrompt, y + 1, L"숫자를 입력하세요.");
                Sleep(1000);
                clearAt(1, y + 1, cols);
                continue;
            }
            if (v <= 0) { // 0 이하일 때 재입력
                clearAt(1, y + 1, cols);
                wprintAt(xPrompt, y + 1, L"1 이상의 값을 입력하세요.");
                Sleep(1000);
                clearAt(1, y + 1, cols);
                continue;
            }

            if (v >= 500) { // 0 이하일 때 재입력
                clearAt(1, y + 1, cols);
                wprintAt(xPrompt, y + 1, L"500 이하의 값을 입력하세요.");
                Sleep(1000);
                clearAt(1, y + 1, cols);
                continue;
            }
            wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
            break;
        }
        ++y;
        return v;
        };

    auto askInt_People = [&](const wchar_t* label, int size) -> int {
        int v;
        for (;;) {
            clearAt(1, y, cols);
            gotoxy(xPrompt, y);
            wcout << label << flush;

            short cx, cy;
            get_cursor_1based(&cx, &cy);
            gotoxy(cx, cy);

            if (!(wcin >> v)) { // 숫자 아님
                wcin.clear();
                wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
                clearAt(1, y + 1, cols);
                wprintAt(xPrompt, y + 1, L"숫자를 입력하세요.");
                Sleep(1000);
                clearAt(1, y + 1, cols);
                continue;
            }
            if (v <= 0) { // 0 이하일 때 재입력
                clearAt(1, y + 1, cols);
                wprintAt(xPrompt, y + 1, L"1 이상의 값을 입력하세요.");
                Sleep(1000);
                clearAt(1, y + 1, cols);
                continue;
            }

            if (v > size) { // 0 이하일 때 재입력
                clearAt(1, y + 1, cols);
                wchar_t msg[100];
                swprintf(msg, 100, L"자리 배치 범위를 초과했습니다. (최대 %d명)", size);
                wprintAt(xPrompt, y + 1, msg);                Sleep(1000);
                clearAt(1, y + 1, cols);
                continue;
            }
            wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
            break;
        }
        ++y;
        return v;
        };


    int row = askInt(L"행의 수를 입력하고 Enter> ");
    int col = askInt(L"열의 수를 입력하고 Enter> ");
    int people = askInt_People(L"배치 인원수를 입력하고 Enter> ", row * col);
    gotoxy(xPrompt, y);
    wcout << L"[←→↑↓] 이동  [Space / Enter] 잠금/해제  [M / Tab] 메뉴 창  [Esc] 종료" << flush;


    seat_arrangement(row, col, people);

    clearAt(1, y + 1, cols);
    gotoxy(xPrompt, y + 1);
    wcout << L"아무 키나 누르면 메인으로..." << flush;
    _getch();
}


void draw_check02(int L, int T, int c, int r) {
    auto GOTO = [&](int x, int y) { gotoxy(L + x - 1, T + y - 1); };

    GOTO(1, 1); wcout << L"┌";
    for (int i = 0; i < c; ++i) wcout << L"───" << (i == c - 1 ? L"┐" : L"┬");

    for (int i = 0; i < r; ++i) {
        GOTO(1, 2 + i * 2); wcout << L"│";
        for (int j = 0; j < c; ++j) wcout << L"   │";

        if (i != r - 1) {
            GOTO(1, 3 + i * 2); wcout << L"├";
            for (int j = 0; j < c; ++j) wcout << L"───" << (j == c - 1 ? L"┤" : L"┼");
        }
    }
    GOTO(1, 2 * r + 1); wcout << L"└";
    for (int i = 0; i < c; ++i) wcout << L"───" << (i == c - 1 ? L"┘" : L"┴");
    wcout << flush;
    wcout << L"\n\n\n";
}

void renderSeat(const Grid& g, int r, int c, int val, bool selected) {
    int y = g.originY + 1 + (r - 1) * g.cellH;
    int x = g.originX + 1 + (c - 1) * g.cellW;
    gotoxy(x, y);
    WORD norm = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    WORD sel = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
    setAttr(selected ? sel : norm);
    if (val > 0) wcout << setw(3) << val; else wcout << "   ";
    setAttr(norm);
    wcout.flush();
}

void seat_arrangement(int row, int column, int people) {
    const int total = row * column;
    if (total <= 0) return;
    people = max(0, min(people, total));

    // 필요한 콘솔 버퍼(스크롤 가능) 확보
    int needCols = 4 * column + 8;
    int needLines = 2 * row + 12;
    setWindowAndBuffer(150, 50, max(150, needCols), max(50, needLines));

    // 좌석 숫자 준비
    vector<int> seats(total, 0);
    for (int i = 0; i < people; ++i) seats[i] = i + 1;
    shuffle(seats.begin(), seats.end(), mt19937(random_device{}()));

    int cols, rows; getConsoleSize(cols, rows);
    const int gridW = 1 + 4 * column;   // 위에서 계산한 폭
    const int gridH = 2 * row + 1;      // 위에서 계산한 높이

    const int footer = 8;               // 아래 안내/메뉴 여유


    short curX, curY; get_cursor_1based(&curX, &curY);
    const int minTop = curY + 2;

    const int minLeft = max(5, (int)(cols * 0.05));


    const int idealLeft = (cols > gridW) ? ((cols - gridW) / 2 + 1) : 5;
    const int idealTop = (rows > gridH + footer) ? ((rows - (gridH + footer)) / 2 + 1) : 2;

    int left = max(minLeft, idealLeft);
    int top = max(minTop, idealTop);

    const int gapBelowGrid = 2;                   // 그리드와 메뉴 사이 여백
    const int menuTopY = top + gridH + gapBelowGrid;

    const int menuBlockH = 4 /*N*/ + 2;

    const int bufW = max(cols, gridW + 10);      // 좌우 여백 조금
    const int bufH = max(rows, gridH + footer + 14); // 아래 여백
    setWindowAndBuffer(cols, rows, bufW, bufH);

    //short ox, oy; get_cursor_1based(&ox, &oy);
    draw_check02(left, top + 2, column, row);
    Grid g{ row, column, left, top + 2 };

    drawAxesLabels(g, row, column);

    for (int r = 1; r <= row; ++r)
        for (int c = 1; c <= column; ++c)
            renderSeat(g, r, c, seats[(r - 1) * column + (c - 1)], false);

    vector<char> selected(total, 0);
    int curR = 1, curC = 1;
    renderSeat(g, curR, curC, seats[(curR - 1) * column + (curC - 1)], true);

    bool running = true;
    while (running) {
        int ch = _getch();
        if (ch == 27) break;                 // ESC
        if (ch == 224 || ch == 0) {
            int a = _getch();
            int prevR = curR, prevC = curC;
            if (a == 75 && curC > 1)          --curC;       // ←
            else if (a == 77 && curC < column) ++curC;      // →
            else if (a == 72 && curR > 1)      --curR;       // ↑
            else if (a == 80 && curR < row)    ++curR;       // ↓
            if (prevR != curR || prevC != curC) {
                renderSeat(g, prevR, prevC, seats[(prevR - 1) * column + (prevC - 1)],
                    selected[(prevR - 1) * column + (prevC - 1)]);
                renderSeat(g, curR, curC, seats[(curR - 1) * column + (curC - 1)], true);
            }
            continue;
        }
        if (ch == ' ' || ch == '\r') {
            int idx = (curR - 1) * column + (curC - 1);
            selected[idx] ^= 1;
            renderSeat(g, curR, curC, seats[idx], true);
        }
        if (ch == 'm' || ch == 'M' || ch == 9) {

            int cols, rows;
            getConsoleSize(cols, rows);

            CONSOLE_SCREEN_BUFFER_INFO info;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
            int bufferHeight = info.dwSize.Y;

            gotoxy(1, bufferHeight - rows + 1);

            const int menuLeft = 5;

            SeatAction act = showBottomMenu(menuLeft, menuTopY + 2);
            if (act == SeatAction::None) continue;
            if (act == SeatAction::Reroll) {
                rerollSeats(seats, selected);
                // 전체 리프레시
                for (int r = 1; r <= row; ++r)
                    for (int c = 1; c <= column; ++c) {
                        int idx = (r - 1) * column + (c - 1);
                        bool isCur = (r == curR && c == curC);
                        renderSeat(g, r, c, seats[idx], isCur ? true : (bool)selected[idx]);
                    }
                continue;
            }
            if (act == SeatAction::Export) {
                // 파일명 자동
                time_t t = time(nullptr); tm tm{}; localtime_s(&tm, &t);
                wchar_t name[128];
                swprintf(name, 128, L"seating_%04d%02d%02d_%02d%02d%02d.txt",
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec);

                wstring fullpath = joinPath(g_exportDir, name);
                bool ok = exportSeatsTxt_UTF8(seats, row, column, fullpath);

                // 상태표시 (가운데)
                int cols, rows; getConsoleSize(cols, rows);
                int ymsg = g.originY + row * 2 + 5;
                clearAt(1, ymsg, cols);
                wprintCentered(ymsg, ok ? (wstring(L"저장 완료: ") + name + fullpath).c_str()
                    : L"저장 실패");
                continue;
            }

            if (act == SeatAction::ChangeDir) {

                if (ch == 27) break;                 // ESC

                // 안내/입력 UI (아래쪽 두 줄 사용)
                int cols, rows; getConsoleSize(cols, rows);
                int yask = rows - 6;

                clearAt(1, menuTopY + 6, cols);
                clearAt(1, menuTopY + 7, cols);
                wprintAt(5, menuTopY + 7, L"내보내기 폴더 경로를 입력하세요. 예) C:\\Users\\me\\Desktop");
                wprintAt(5, menuTopY + 8, L"> ");
                gotoxy(7, menuTopY + 8);


                // 버퍼에 남은 개행 제거 후 한 줄 입력
                wstring path;

                bool okLine = readLineWithEsc(path, 7, menuTopY + 8);

                if (!okLine) {
                    wprintAt(5, menuTopY + 9, L"입력을 취소했습니다.");
                    // 취소 처리 후 계속
                }
                else if (path.empty()) {
                    wprintAt(5, menuTopY + 9, L"빈 경로입니다.");
                }
                else if (ensureDirectory(path)) {
                    g_exportDir = path;
                    wprintAt(5, menuTopY + 9, (L"설정 완료: " + g_exportDir).c_str());
                }
                else {
                    wprintAt(5, menuTopY + 9, L"폴더 생성/접근 실패. 경로를 다시 확인하세요.");
                }

                continue;
            }

            if (act == SeatAction::Back) break;
        }
    }

}
