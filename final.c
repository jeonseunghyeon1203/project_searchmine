#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define MAX_WIDTH 30
#define MAX_HEIGHT 60
#define MIN_SIZE 20

// 게임 설정 구조체
typedef struct {
    int width; // 가로 길이
    int height; // 세로 길이
    int mines; // 지뢰 수
} GameSettings;

// 셀 구조체
typedef struct {
    int x; // x 좌표
    int y; // y 좌표
    int isMine; // 지뢰 여부
    int isRevealed; // 드러남 여부
    int isFlagged; // 깃발 표시 여부
    int surroundingMines; // 주변 지뢰 수
} Cell;

// 함수 선언
void printMenu();
void setDifficulty(GameSettings* settings);
void startGame(GameSettings settings, int hardcoreMode);
void resetGame(GameSettings* settings);
void gameOver(int score, int stage, int totalMinesFound, int lives, time_t startTime);
void explainRules();
void displayCredits();
void setColor(int color);
void resetColor();
void clearScreen();
void initializeMap(Cell map[MAX_HEIGHT][MAX_WIDTH], GameSettings settings);
void displayMap(Cell map[MAX_HEIGHT][MAX_WIDTH], GameSettings settings, int stage, int lives, int score, time_t startTime, int remainingMines);
void revealCell(Cell map[MAX_HEIGHT][MAX_WIDTH], int x, int y, GameSettings settings, int* score, int* lives, int* remainingMines, int* totalMinesFound);
void toggleFlag(Cell map[MAX_HEIGHT][MAX_WIDTH], int x, int y, int* score, int* remainingMines, int* totalMinesFound);
int allMinesFound(Cell map[MAX_HEIGHT][MAX_WIDTH], GameSettings settings);
void initializeSettings(GameSettings* settings);

// 색상 열거형
enum colors {
    COLOR_RESET = 0,
    COLOR_HIGHLIGHT = 1,
    COLOR_SKY_BLUE = 36,
    COLOR_GREEN = 32,
    COLOR_ORANGE = 33,
    COLOR_DARK_BLUE = 34,
    COLOR_BROWN = 31,
    COLOR_GRAY = 37,
    COLOR_PURPLE = 35,
    COLOR_PINK = 95,
    COLOR_RED = 41,
    COLOR_WHITE = 97,
    COLOR_YELLOW = 93,
    COLOR_DARK_YELLOW = 33,
};

int main() {
    GameSettings settings;
    initializeSettings(&settings); // 초기 게임 설정
    int choice, hardcoreMode = 0;

    while (1) {
        printMenu();
        printf("메뉴 선택: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                startGame(settings, hardcoreMode);
                break;
            case 2:
                setDifficulty(&settings);
                break;
            case 3:
                hardcoreMode = 1;
                settings.width = 30;
                settings.height = 50;
                settings.mines = 500;
                startGame(settings, hardcoreMode);
                hardcoreMode = 0;
                break;
            case 4:
                explainRules();
                break;
            case 5:
                displayCredits();
                break;
            case 6:
                resetGame(&settings);
                break;
            case 7:
                printf("게임을 종료합니다.\n");
                exit(0);
            default:
                printf("잘못된 선택입니다. 다시 시도하세요.\n");
        }
    }

    return 0;
}

// 게임 설정 초기화 함수
void initializeSettings(GameSettings* settings) {
    settings->width = 20;
    settings->height = 20;
    settings->mines = 1;
}

// 메뉴 출력 함수
void printMenu() {
    clearScreen();
    printf("1. 실행\n");
    printf("2. 난위도 설정\n");
    printf("3. 하드코어 모드\n");
    printf("4. 게임 룰 설명\n");
    printf("5. 개발자 크레딧\n");
    printf("6. 게임 리셋\n");
    printf("7. 게임 종료\n");
}

// 난이도 설정 함수
void setDifficulty(GameSettings* settings) {
    int width, height, mines;
    while (1) {
        printf("맵의 크기를 입력하세요 (최소 %d, 최대 %dx%d)\n", MIN_SIZE, MAX_WIDTH, MAX_HEIGHT);
        printf("가로: ");
        scanf("%d", &width);
        printf("세로: ");
        scanf("%d", &height);

        if (width < MIN_SIZE || width > MAX_WIDTH || height < MIN_SIZE || height > MAX_HEIGHT) {
            printf("잘못된 크기입니다. 다시 입력하세요.\n");
            continue;
        }

        printf("지뢰 수를 입력하세요 (1 ~ %d): ", (width * height) * 50 / 100);
        scanf("%d", &mines);

        if (mines < 1 || mines > (width * height) * 50 / 100) {
            printf("잘못된 지뢰 수입니다. 다시 입력하세요.\n");
            continue;
        }

        settings->width = width;
        settings->height = height;
        settings->mines = mines;
        break;
    }
    printf("난이도가 설정되었습니다: %dx%d 맵에 %d개의 지뢰\n", settings->width, settings->height, settings->mines);
}

// 맵 초기화 함수
void initializeMap(Cell map[MAX_HEIGHT][MAX_WIDTH], GameSettings settings) {
    int i, j;
    for (i = 0; i < settings.height; i++) {
        for (j = 0; j < settings.width; j++) {
            map[i][j].x = j;
            map[i][j].y = i;
            map[i][j].isMine = 0;
            map[i][j].isRevealed = 0;
            map[i][j].isFlagged = 0;
            map[i][j].surroundingMines = 0;
        }
    }

    srand(time(NULL));
    int minesPlaced = 0;
    while (minesPlaced < settings.mines) {
        int x = rand() % settings.width;
        int y = rand() % settings.height;
        if (!map[y][x].isMine) {
            map[y][x].isMine = 1;
            minesPlaced++;
        }
    }

    for (i = 0; i < settings.height; i++) {
        for (j = 0; j < settings.width; j++) {
            if (map[i][j].isMine) continue;
            int mineCount = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nx = j + dx;
                    int ny = i + dy;
                    if (nx >= 0 && nx < settings.width && ny >= 0 && ny < settings.height && map[ny][nx].isMine) {
                        mineCount++;
                    }
                }
            }
            map[i][j].surroundingMines = mineCount;
        }
    }
}

// 화면 지우기 함수
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// 색상 설정 함수
void setColor(int color) {
    printf("\033[%dm", color);
}

// 색상 초기화 함수
void resetColor() {
    printf("\033[0m");
}

// 맵 표시 함수
void displayMap(Cell map[MAX_HEIGHT][MAX_WIDTH], GameSettings settings, int stage, int lives, int score, time_t startTime, int remainingMines) {
    clearScreen();
    printf("현재 스테이지: %d\n", stage);
    printf("맵 크기: %dx%d\n", settings.width, settings.height);
    printf("지뢰 수: %d\n", settings.mines);
    printf("남은 목숨: ");
    for (int i = 0; i < lives; i++) {
        printf("$ ");
    }
    printf("(%d 남음)\n", lives);
    printf("점수: %d\n", score);
    printf("경과 시간: %ld초\n", time(NULL) - startTime);
    printf("남은 지뢰: %d\n", remainingMines);
    printf("\n");

    printf("     ");
    for (int i = 0; i < settings.width; i++) {
        printf("%03d ", i + 1);
    }
    printf("\n");

    for (int i = 0; i < settings.height; i++) {
        printf("%03d ", i + 1);
        for (int j = 0; j < settings.width; j++) {
            if (map[i][j].isRevealed) {
                if (map[i][j].isMine) {
                    setColor(COLOR_RED);
                    printf("|●| ");
                    resetColor();
                } else if (map[i][j].surroundingMines > 0) {
                    switch (map[i][j].surroundingMines) {
                        case 1:
                            setColor(COLOR_SKY_BLUE);
                            break;
                        case 2:
                            setColor(COLOR_GREEN);
                            break;
                        case 3:
                            setColor(COLOR_ORANGE);
                            break;
                        case 4:
                            setColor(COLOR_DARK_BLUE);
                            break;
                        case 5:
                            setColor(COLOR_BROWN);
                            break;
                        case 6:
                            setColor(COLOR_GRAY);
                            break;
                        case 7:
                            setColor(COLOR_PURPLE);
                            break;
                        case 8:
                            setColor(COLOR_PINK);
                            break;
                    }
                    printf("|%d| ", map[i][j].surroundingMines);
                    resetColor();
                } else {
                    setColor(COLOR_WHITE);
                    printf("|■| ");
                    resetColor();
                }
            } else if (map[i][j].isFlagged) {
                setColor(COLOR_YELLOW);
                printf("|○| ");
                resetColor();
            } else {
                printf("|□| ");
            }
        }
        printf("\n");
    }
}

// 셀 드러내기 함수
void revealCell(Cell map[MAX_HEIGHT][MAX_WIDTH], int x, int y, GameSettings settings, int* score, int* lives, int* remainingMines, int* totalMinesFound) {
    if (x < 0 || x >= settings.width || y < 0 || y >= settings.height || map[y][x].isRevealed) {
        return;
    }

    map[y][x].isRevealed = 1;
    if (map[y][x].isMine) {
        printf("지뢰를 터트렸습니다! 남은 목숨: %d\n", *lives - 1);
        (*lives)--;
    } else {
        if (map[y][x].surroundingMines == 0) {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    revealCell(map, x + dx, y + dy, settings, score, lives, remainingMines, totalMinesFound);
                }
            }
        }
    }
}

// 깃발 토글 함수
void toggleFlag(Cell map[MAX_HEIGHT][MAX_WIDTH], int x, int y, int* score, int* remainingMines, int* totalMinesFound) {
    if (x < 0 || x >= MAX_WIDTH || y < 0 || y >= MAX_HEIGHT || map[y][x].isRevealed) {
        return;
    }

    if (map[y][x].isFlagged) {
        map[y][x].isFlagged = 0;
        printf("-깃발을 회수함-\n");
        if (map[y][x].isMine) {
            (*remainingMines)++;
        }
    } else {
        map[y][x].isFlagged = 1;
        if (map[y][x].isMine && !map[y][x].isRevealed) {
            (*score)++;
            (*remainingMines)--;
            (*totalMinesFound)++;
        }
    }
}

// 모든 지뢰가 찾았는지 확인하는 함수
int allMinesFound(Cell map[MAX_HEIGHT][MAX_WIDTH], GameSettings settings) {
    for (int i = 0; i < settings.height; i++) {
        for (int j = 0; j < settings.width; j++) {
            if (map[i][j].isMine && !map[i][j].isFlagged) {
                return 0;
            }
        }
    }
    return 1;
}

// 게임 시작 함수
void startGame(GameSettings settings, int hardcoreMode) {
    Cell map[MAX_HEIGHT][MAX_WIDTH];
    initializeMap(map, settings);

    int lives = hardcoreMode ? 3 : 5;
    int score = 0;
    int stage = 1;
    int clearedCells = 0;
    int totalCells = settings.width * settings.height - settings.mines;
    int remainingMines = settings.mines;
    int totalMinesFound = 0;
    time_t startTime = time(NULL);

    while (lives > 0) {
        displayMap(map, settings, stage, lives, score, startTime, remainingMines);
        char action[4];
        int x, y;
        printf("행동을 선택하세요 (탐색: S, 깃발: F, 종료: out): ");
        while (1) {
            scanf("%s", action);
            if (strcmp(action, "S") == 0 || strcmp(action, "s") == 0 || strcmp(action, "F") == 0 || strcmp(action, "f") == 0 || strcmp(action, "out") == 0) break;
            printf("잘못된 입력입니다. 다시 입력하세요 (탐색: S, 깃발: F, 종료: out): ");
        }

        if (strcmp(action, "out") == 0) {
            printf("게임을 종료합니다. 메인 메뉴로 돌아갑니다.\n");
            gameOver(score, stage, totalMinesFound, lives, startTime);
            return;
        }

        printf("좌표를 입력하세요 (X Y): ");
        scanf("%d %d", &x, &y);

        x--; // 좌표계가 1부터 시작하므로 0으로 변환
        y--; // 좌표계가 1부터 시작하므로 0으로 변환

        if (x < 0 || x >= settings.width || y < 0 || y >= settings.height) {
            printf("잘못된 좌표입니다. 다시 입력하세요.\n");
            continue;
        }

        if (strcmp(action, "F") == 0 || strcmp(action, "f") == 0) {
            toggleFlag(map, x, y, &score, &remainingMines, &totalMinesFound);
        } else if (strcmp(action, "S") == 0 || strcmp(action, "s") == 0) {
            revealCell(map, x, y, settings, &score, &lives, &remainingMines, &totalMinesFound);
            if (lives == 0) {
                printf("게임 오버!\n");
                printf("최종 점수: %d\n", score);
                Sleep(3000); // 게임 오버 메시지를 3초 동안 표시
                gameOver(score, stage, totalMinesFound, lives, startTime);
                return;
            }
        }

        if (clearedCells == totalCells || remainingMines == 0 || allMinesFound(map, settings)) {
            score += settings.mines;
            printf("-스테이지 클리어-\n");
            lives = lives < 255 ? lives + 1 : lives; // 최대 목숨은 255개
            stage++;
            int newMines = settings.mines + (settings.mines * 0.05);
            if (newMines <= (settings.width * settings.height) * 0.7) {
                settings.mines = newMines > settings.mines ? newMines : settings.mines + 1;
            }
            settings.width += 2;
            settings.height += 2;
            totalCells = settings.width * settings.height - settings.mines;
            remainingMines = settings.mines;
            initializeMap(map, settings);
            clearedCells = 0;
        }
    }
}

// 게임 기록 초기화 함수
void resetGame(GameSettings* settings) {
    initializeSettings(settings);
    printf("게임 기록이 초기화되었습니다.\n");
}

// 게임 오버 함수
void gameOver(int score, int stage, int totalMinesFound, int lives, time_t startTime) {
    clearScreen();
    printf("게임 오버! 최종 점수: %d\n", score);
    printf("클리어한 스테이지: %d\n", stage);
    printf("찾은 총 지뢰의 수: %d\n", totalMinesFound);
    printf("남은 목숨: %d\n", lives);
    printf("걸린 시간: %ld초\n", time(NULL) - startTime);
    printf("\n아무 키나 입력하여 메인으로 돌아갑니다...\n");
    getchar();
    getchar(); // 두 번 호출하여 Enter 키도 처리
}

// 게임 룰 설명 함수
void explainRules() {
    clearScreen();
    printf("게임 룰 설명:\n");
    printf("탐색하는 칸 반경 1칸 내 지뢰가 몇 개 있는지 알려주는 숫자 인디케이터:\n");
    printf("1: ");
    setColor(COLOR_SKY_BLUE);
    printf("하늘색\n");
    resetColor();
    printf("2: ");
    setColor(COLOR_GREEN);
    printf("초록색\n");
    resetColor();
    printf("3: ");
    setColor(COLOR_ORANGE);
    printf("주황색\n");
    resetColor();
    printf("4: ");
    setColor(COLOR_DARK_BLUE);
    printf("진한 남색\n");
    resetColor();
    printf("5: ");
    setColor(COLOR_BROWN);
    printf("갈색\n");
    resetColor();
    printf("6: ");
    setColor(COLOR_GRAY);
    printf("회색\n");
    resetColor();
    printf("7: ");
    setColor(COLOR_PURPLE);
    printf("보라색\n");
    resetColor();
    printf("8: ");
    setColor(COLOR_PINK);
    printf("분홍색\n");
    resetColor();

    printf("깃발: ");
    setColor(COLOR_DARK_YELLOW);
    printf("노란색 |○|\n");
    resetColor();
    printf("지뢰: ");
    setColor(COLOR_RED);
    printf("빨간색 |●|\n");
    resetColor();
    printf("안전지대: ");
    setColor(COLOR_WHITE);
    printf("하얀색 |■|\n");
    resetColor();

    printf("\n각 심볼의 의미:\n");
    printf("|□| : 탐색되지 않은 구역\n");
    printf("|○| : 깃발이 세워진 구역, 지뢰를 찾았다고 표시\n");
    printf("|●| : 지뢰가 있는 구역\n");
    printf("|■| : 안전한 구역\n");
    printf("각 숫자는 해당 구역 주변 1칸 내에 존재하는 지뢰의 수를 나타냅니다.\n");
    printf("\n메인 메뉴로 돌아가려면 아무 키나 누르세요...");
    getchar();
    getchar(); // 두 번 호출하여 Enter 키도 처리
}

// 개발자 크레딧 표시 함수
void displayCredits() {
    clearScreen();
    printf("작성자: 202121018_최재영\n");
    printf("프로그램 이름: 지뢰찾기 개선판 V1.4\n");
    printf("\n\n- 업데이트 로그 -");
    printf("\n_V1.0 : 게임 메인프레임 구현");
    printf("\n_V1.1 : 맵 가독성 개선");
    printf("\n_V1.2 : 게임 결과 출력 추가");
    printf("\n_V1.3 : 게임 컬러 추가");
    printf("\n_V1.4 : 더미코드 제거 및 주석추가");

    printf("\n메인 메뉴로 돌아가려면 아무 키나 누르세요...");
    getchar();
    getchar(); // 두 번 호출하여 Enter 키도 처리
}
