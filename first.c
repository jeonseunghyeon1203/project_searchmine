#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define MAX_WIDTH 30
#define MAX_HEIGHT 60
#define MIN_SIZE 20

typedef struct {
    int width;
    int height;
    int mines;
} GameSettings;

typedef struct {
    int x;
    int y;
    int isMine;
    int isRevealed;
    int isFlagged;
    int surroundingMines;
} Cell;

void printMenu();
void setDifficulty(GameSettings* settings);
void startGame(GameSettings settings, int hardcoreMode);
void displayScore();
void saveGame();
void loadGame();
void gameOver(int score);
void explainRules();
void displayCredits();
void setColor(int color);
void resetColor();
void clearScreen();

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
    GameSettings settings = {20, 20, 1}; // 초기 설정값
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
                settings.mines = 800;
                startGame(settings, hardcoreMode);
                hardcoreMode = 0;
                break;
            case 4:
                displayScore();
                break;
            case 5:
                saveGame();
                break;
            case 6:
                explainRules();
                break;
            case 7:
                displayCredits();
                break;
            default:
                printf("잘못된 선택입니다. 다시 시도하세요.\n");
        }
    }

    return 0;
}

void printMenu() {
    clearScreen();
    printf("1. 실행\n");
    printf("2. 난위도 설정\n");
    printf("3. 하드코어 모드\n");
    printf("4. 전적(점수) 불러오기\n");
    printf("5. 저장\n");
    printf("6. 게임 룰 설명\n");
    printf("7. 개발자 크레딧\n");
    printf("8. 종료\n");
}

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

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void setColor(int color) {
    printf("\033[%dm", color);
}

void resetColor() {
    printf("\033[0m");
}

void displayMap(Cell map[MAX_HEIGHT][MAX_WIDTH], GameSettings settings, int stage, int lives, int score, time_t startTime) {
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
                setColor(COLOR_DARK_YELLOW);
                printf("|○| ");
                resetColor();
            } else {
                printf("|□| ");
            }
        }
        printf("\n");
    }
}

void revealCell(Cell map[MAX_HEIGHT][MAX_WIDTH], int x, int y, GameSettings settings, int* score, int* lives) {
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
                    revealCell(map, x + dx, y + dy, settings, score, lives);
                }
            }
        }
    }
}

void toggleFlag(Cell map[MAX_HEIGHT][MAX_WIDTH], int x, int y, int* score) {
    if (x < 0 || x >= MAX_WIDTH || y < 0 || y >= MAX_HEIGHT || map[y][x].isRevealed) {
        return;
    }

    if (map[y][x].isFlagged) {
        map[y][x].isFlagged = 0;
        printf("-깃발을 회수함-\n");
    } else {
        map[y][x].isFlagged = 1;
        if (map[y][x].isMine && !map[y][x].isRevealed) {
            (*score)++;
        }
    }
}

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

void startGame(GameSettings settings, int hardcoreMode) {
    Cell map[MAX_HEIGHT][MAX_WIDTH];
    initializeMap(map, settings);

    int lives = hardcoreMode ? 1 : 5;
    int score = 0;
    int stage = 1;
    int clearedCells = 0;
    int totalCells = settings.width * settings.height - settings.mines;
    time_t startTime = time(NULL);

    while (lives > 0) {
        displayMap(map, settings, stage, lives, score, startTime);
        char action;
        int x, y;
        printf("행동을 선택하세요 (탐색: S, 깃발: F): ");
        while (1) {
            scanf(" %c", &action);
            action = toupper(action);
            if (action == 'S' || action == 'F') break;
            printf("잘못된 입력입니다. 다시 입력하세요 (탐색: S, 깃발: F): ");
        }

        printf("좌표를 입력하세요 (X Y): ");
        scanf("%d %d", &x, &y);

        x--; // 좌표계가 1부터 시작하므로 0으로 변환
        y--; // 좌표계가 1부터 시작하므로 0으로 변환

        if (x < 0 || x >= settings.width || y < 0 || y >= settings.height) {
            printf("잘못된 좌표입니다. 다시 입력하세요.\n");
            continue;
        }

        if (action == 'F') {
            toggleFlag(map, x, y, &score);
        } else if (action == 'S') {
            if (map[y][x].isMine) {
                revealCell(map, x, y, settings, &score, &lives);
                if (lives == 0) {
                    printf("게임 오버!\n");
                    printf("최종 점수: %d\n", score);
                    sleep(3); // 게임 오버 메시지를 3초 동안 표시
                    gameOver(score);
                    return;
                }
            } else {
                revealCell(map, x, y, settings, &score, &lives);
                clearedCells++;
                if (clearedCells == totalCells || allMinesFound(map, settings)) {
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
                    initializeMap(map, settings);
                    clearedCells = 0;
                }
            }
        } else {
            printf("잘못된 선택입니다. 다시 입력하세요.\n");
        }
    }
}

void saveGame() {
    FILE* file = fopen("savegame.dat", "wb");
    if (file == NULL) {
        printf("파일 저장에 실패했습니다.\n");
        return;
    }

    // 필요한 게임 데이터를 저장하는 코드 작성
    // 예: fwrite(...)

    fclose(file);
    printf("게임이 저장되었습니다.\n");
}

void loadGame() {
    FILE* file = fopen("savegame.dat", "rb");
    if (file == NULL) {
        printf("저장된 게임이 없습니다.\n");
        return;
    }

    // 필요한 게임 데이터를 불러오는 코드 작성
    // 예: fread(...)

    fclose(file);
    printf("게임이 불러와졌습니다.\n");
}

void displayScore() {
    // 점수를 표시하는 코드 작성
    printf("점수 불러오기 기능은 아직 구현되지 않았습니다.\n");
}

void gameOver(int score) {
    printf("게임 오버! 최종 점수: %d\n", score);
    sleep(3); // 게임 오버 메시지를 3초 동안 표시
    // 점수를 저장하는 코드 작성
}

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

void displayCredits() {
    clearScreen();
    printf("작성자: 202121018_최재영\n");
    printf("프로그램 이름: 지뢰찾기 개선판 V1.0\n");
    printf("\n메인 메뉴로 돌아가려면 아무 키나 누르세요...");
    getchar();
    getchar(); // 두 번 호출하여 Enter 키도 처리
}
