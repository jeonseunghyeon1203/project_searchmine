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
    GameSettings settings = {20, 20, 1}; // �ʱ� ������
    int choice, hardcoreMode = 0;

    while (1) {
        printMenu();
        printf("�޴� ����: ");
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
                printf("�߸��� �����Դϴ�. �ٽ� �õ��ϼ���.\n");
        }
    }

    return 0;
}

void printMenu() {
    clearScreen();
    printf("1. ����\n");
    printf("2. ������ ����\n");
    printf("3. �ϵ��ھ� ���\n");
    printf("4. ����(����) �ҷ�����\n");
    printf("5. ����\n");
    printf("6. ���� �� ����\n");
    printf("7. ������ ũ����\n");
    printf("8. ����\n");
}

void setDifficulty(GameSettings* settings) {
    int width, height, mines;
    while (1) {
        printf("���� ũ�⸦ �Է��ϼ��� (�ּ� %d, �ִ� %dx%d)\n", MIN_SIZE, MAX_WIDTH, MAX_HEIGHT);
        printf("����: ");
        scanf("%d", &width);
        printf("����: ");
        scanf("%d", &height);

        if (width < MIN_SIZE || width > MAX_WIDTH || height < MIN_SIZE || height > MAX_HEIGHT) {
            printf("�߸��� ũ���Դϴ�. �ٽ� �Է��ϼ���.\n");
            continue;
        }

        printf("���� ���� �Է��ϼ��� (1 ~ %d): ", (width * height) * 50 / 100);
        scanf("%d", &mines);

        if (mines < 1 || mines > (width * height) * 50 / 100) {
            printf("�߸��� ���� ���Դϴ�. �ٽ� �Է��ϼ���.\n");
            continue;
        }

        settings->width = width;
        settings->height = height;
        settings->mines = mines;
        break;
    }
    printf("���̵��� �����Ǿ����ϴ�: %dx%d �ʿ� %d���� ����\n", settings->width, settings->height, settings->mines);
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
    printf("���� ��������: %d\n", stage);
    printf("�� ũ��: %dx%d\n", settings.width, settings.height);
    printf("���� ��: %d\n", settings.mines);
    printf("���� ���: ");
    for (int i = 0; i < lives; i++) {
        printf("$ ");
    }
    printf("(%d ����)\n", lives);
    printf("����: %d\n", score);
    printf("��� �ð�: %ld��\n", time(NULL) - startTime);
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
                    printf("|��| ");
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
                    printf("|��| ");
                    resetColor();
                }
            } else if (map[i][j].isFlagged) {
                setColor(COLOR_DARK_YELLOW);
                printf("|��| ");
                resetColor();
            } else {
                printf("|��| ");
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
        printf("���ڸ� ��Ʈ�Ƚ��ϴ�! ���� ���: %d\n", *lives - 1);
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
        printf("-����� ȸ����-\n");
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
        printf("�ൿ�� �����ϼ��� (Ž��: S, ���: F): ");
        while (1) {
            scanf(" %c", &action);
            action = toupper(action);
            if (action == 'S' || action == 'F') break;
            printf("�߸��� �Է��Դϴ�. �ٽ� �Է��ϼ��� (Ž��: S, ���: F): ");
        }

        printf("��ǥ�� �Է��ϼ��� (X Y): ");
        scanf("%d %d", &x, &y);

        x--; // ��ǥ�谡 1���� �����ϹǷ� 0���� ��ȯ
        y--; // ��ǥ�谡 1���� �����ϹǷ� 0���� ��ȯ

        if (x < 0 || x >= settings.width || y < 0 || y >= settings.height) {
            printf("�߸��� ��ǥ�Դϴ�. �ٽ� �Է��ϼ���.\n");
            continue;
        }

        if (action == 'F') {
            toggleFlag(map, x, y, &score);
        } else if (action == 'S') {
            if (map[y][x].isMine) {
                revealCell(map, x, y, settings, &score, &lives);
                if (lives == 0) {
                    printf("���� ����!\n");
                    printf("���� ����: %d\n", score);
                    sleep(3); // ���� ���� �޽����� 3�� ���� ǥ��
                    gameOver(score);
                    return;
                }
            } else {
                revealCell(map, x, y, settings, &score, &lives);
                clearedCells++;
                if (clearedCells == totalCells || allMinesFound(map, settings)) {
                    score += settings.mines;
                    printf("-�������� Ŭ����-\n");
                    lives = lives < 255 ? lives + 1 : lives; // �ִ� ����� 255��
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
            printf("�߸��� �����Դϴ�. �ٽ� �Է��ϼ���.\n");
        }
    }
}

void saveGame() {
    FILE* file = fopen("savegame.dat", "wb");
    if (file == NULL) {
        printf("���� ���忡 �����߽��ϴ�.\n");
        return;
    }

    // �ʿ��� ���� �����͸� �����ϴ� �ڵ� �ۼ�
    // ��: fwrite(...)

    fclose(file);
    printf("������ ����Ǿ����ϴ�.\n");
}

void loadGame() {
    FILE* file = fopen("savegame.dat", "rb");
    if (file == NULL) {
        printf("����� ������ �����ϴ�.\n");
        return;
    }

    // �ʿ��� ���� �����͸� �ҷ����� �ڵ� �ۼ�
    // ��: fread(...)

    fclose(file);
    printf("������ �ҷ��������ϴ�.\n");
}

void displayScore() {
    // ������ ǥ���ϴ� �ڵ� �ۼ�
    printf("���� �ҷ����� ����� ���� �������� �ʾҽ��ϴ�.\n");
}

void gameOver(int score) {
    printf("���� ����! ���� ����: %d\n", score);
    sleep(3); // ���� ���� �޽����� 3�� ���� ǥ��
    // ������ �����ϴ� �ڵ� �ۼ�
}

void explainRules() {
    clearScreen();
    printf("���� �� ����:\n");
    printf("Ž���ϴ� ĭ �ݰ� 1ĭ �� ���ڰ� �� �� �ִ��� �˷��ִ� ���� �ε�������:\n");
    printf("1: ");
    setColor(COLOR_SKY_BLUE);
    printf("�ϴû�\n");
    resetColor();
    printf("2: ");
    setColor(COLOR_GREEN);
    printf("�ʷϻ�\n");
    resetColor();
    printf("3: ");
    setColor(COLOR_ORANGE);
    printf("��Ȳ��\n");
    resetColor();
    printf("4: ");
    setColor(COLOR_DARK_BLUE);
    printf("���� ����\n");
    resetColor();
    printf("5: ");
    setColor(COLOR_BROWN);
    printf("����\n");
    resetColor();
    printf("6: ");
    setColor(COLOR_GRAY);
    printf("ȸ��\n");
    resetColor();
    printf("7: ");
    setColor(COLOR_PURPLE);
    printf("�����\n");
    resetColor();
    printf("8: ");
    setColor(COLOR_PINK);
    printf("��ȫ��\n");
    resetColor();

    printf("���: ");
    setColor(COLOR_DARK_YELLOW);
    printf("����� |��|\n");
    resetColor();
    printf("����: ");
    setColor(COLOR_RED);
    printf("������ |��|\n");
    resetColor();
    printf("��������: ");
    setColor(COLOR_WHITE);
    printf("�Ͼ�� |��|\n");
    resetColor();

    printf("\n�� �ɺ��� �ǹ�:\n");
    printf("|��| : Ž������ ���� ����\n");
    printf("|��| : ����� ������ ����, ���ڸ� ã�Ҵٰ� ǥ��\n");
    printf("|��| : ���ڰ� �ִ� ����\n");
    printf("|��| : ������ ����\n");
    printf("�� ���ڴ� �ش� ���� �ֺ� 1ĭ ���� �����ϴ� ������ ���� ��Ÿ���ϴ�.\n");
    printf("\n���� �޴��� ���ư����� �ƹ� Ű�� ��������...");
    getchar();
    getchar(); // �� �� ȣ���Ͽ� Enter Ű�� ó��
}

void displayCredits() {
    clearScreen();
    printf("�ۼ���: 202121018_���翵\n");
    printf("���α׷� �̸�: ����ã�� ������ V1.0\n");
    printf("\n���� �޴��� ���ư����� �ƹ� Ű�� ��������...");
    getchar();
    getchar(); // �� �� ȣ���Ͽ� Enter Ű�� ó��
}
