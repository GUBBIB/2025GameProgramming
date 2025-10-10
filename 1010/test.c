#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

void seat_arrangement(int row, int column, int people);
void shuffle_number(int sit_number[], int total);
void draw_check02(int column, int row); // 유니코드 박스 그리기(UTF-8)
void gotoxy(int x, int y);

// 현재 커서 위치(1-based) 얻기
static void get_cursor_1based(short *x, short *y)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    *x = (short)(info.dwCursorPosition.X + 1);
    *y = (short)(info.dwCursorPosition.Y + 1);
}

int main(void)
{
    // 콘솔 출력 코드 페이지를 UTF-8로 설정 (유니코드 박스 문자 폭=1 보장)
    srand((unsigned)time(NULL));

    int row, column, people;
    printf("자리배치 프로그램\n\n");
    printf("좌석배치에 대한 행과 열을 입력해야 합니다.\n");
    printf("학생수는 최대 100명이라는 문구는 예시였고, 이제 행*열만큼 동적 할당합니다.\n");
    printf("행의 수를 입력하고 Enter>");
    scanf("%d", &row);
    printf("열의 수를 입력하고 Enter>");
    scanf("%d", &column);
    printf("자리에 앉힐 인원수를 입력하고 Enter>");
    scanf("%d", &people);

    if (row <= 0 || column <= 0)
    {
        printf("행과 열은 1 이상이어야 합니다.\n");
        return 0;
    }

    seat_arrangement(row, column, people);
    printf("\n\n");
    return 0;
}

void seat_arrangement(int row, int column, int people)
{
    int total = row * column;
    if (total <= 0)
        return;

    // people이 total을 넘으면 total로 맞춤
    if (people > total)
        people = total;
    if (people < 0)
        people = 0;

    // 동적 배열: 총 좌석 수만큼
    int *sit_number = (int *)malloc(sizeof(int) * total);
    if (!sit_number)
    {
        printf("메모리 할당 실패\n");
        return;
    }

    // 1..people, 그 뒤는 0(공석)으로 채움
    for (int i = 0; i < people; ++i)
        sit_number[i] = i + 1;
    for (int i = people; i < total; ++i)
        sit_number[i] = 0;

    // 전체 좌석을 섞어서 무작위 위치에 배치되도록 함
    shuffle_number(sit_number, total);

    // 현재 커서 위치를 원점으로 저장
    short originX, originY;
    get_cursor_1based(&originX, &originY);

    // 원점에 그리드 출력
    draw_check02(column, row);

    // 숫자 출력: 각 셀 내부 중앙-좌측 정렬(2자리 기준)로 배치
    // 셀 폭은 "───" 3칸, 경계는 '│' 1칸 → 가로 주기는 4칸
    // 세로는 내용행과 경계행이 번갈아 1줄씩 → 내용행 y = originY + 1 + i*2
    // 첫 내용 행의 왼쪽 여백 다음 칸 = originX + 2
    int count = 0;
    // 좌표 계산부 (핵심)
    for (int i = 1, k = 0; i <= row; ++i)
    {
        for (int j = 1; j <= column; ++j, ++k)
        {
            int val = sit_number[k];

            int y = originY + 1 + (i - 1) * 2; // ✅ 내용줄
            int x = originX + 2 + (j - 1) * 4; // 2,6,10,...

            gotoxy(x, y);
            if (val > 0)
                printf("%3d", val);
            else
                printf("   "); // 공석은 공백
        }
    }

    free(sit_number);
}

void shuffle_number(int sit_number[], int total)
{
    // Fisher?Yates
    for (int i = total - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        int tmp = sit_number[i];
        sit_number[i] = sit_number[j];
        sit_number[j] = tmp;
    }
}

void draw_check02(int c, int r)
{
    // UTF-8 유니코드 박스: ┌─┬─┐ / │ │ │ / ├─┼─┤ / └─┴─┘
    // 각 셀은 폭 3('───'), 경계 포함 주기 4
    // 상단
    printf("┌");
    for (int i = 0; i < c; ++i)
    {
        printf("───");
        printf(i == c - 1 ? "┐" : "┬");
    }
    printf("\n");

    for (int i = 0; i < r; ++i)
    {
        // 내용 행
        printf("│");
        for (int j = 0; j < c; ++j)
        {
            printf("   "); // 숫자는 gotoxy로 따로 채움
            printf("│");
        }
        printf("\n");

        // 행 구분선 (마지막 행 뒤에는 하단선)
        if (i != r - 1)
        {
            printf("├");
            for (int j = 0; j < c; ++j)
            {
                printf("───");
                printf(j == c - 1 ? "┤" : "┼");
            }
            printf("\n");
        }
    }

    // 하단
    printf("└");
    for (int i = 0; i < c; ++i)
    {
        printf("───");
        printf(i == c - 1 ? "┘" : "┴");
    }
    printf("\n");
}

void gotoxy(int x, int y)
{
    COORD Pos = {(SHORT)(x - 1), (SHORT)(y - 1)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}
