#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define MAX_WIDTH 30
#define MAX_HEIGHT 60
#define MIN_SIZE 20

// ���� ���� ����ü
typedef struct {
    int width; // ���� ����
    int height; // ���� ����
    int mines; // ���� ��
} GameSettings;

// �� ����ü
typedef struct {
    int x; // x ��ǥ
    int y; // y ��ǥ
    int isMine; // ���� ����
    int isRevealed; // �巯�� ����
    int isFlagged; // ��� ǥ�� ����
    int surroundingMines; // �ֺ� ���� ��
} Cell;

// �Լ� ����
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

// ���� ������
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
    initializeSettings(&settings); // �ʱ� ���� ����
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
                printf("������ �����մϴ�.\n");
                exit(0);
            default:
                printf("�߸��� �����Դϴ�. �ٽ� �õ��ϼ���.\n");
        }
    }

    return 0;
}

// ���� ���� �ʱ�ȭ �Լ�
void initializeSettings(GameSettings* settings) {
    settings->width = 20;
    settings->height = 20;
    settings->mines = 1;
}

// �޴� ��� �Լ�
void printMenu() {
    clearScreen();
    printf("1. ����\n");
    printf("2. ������ ����\n");
    printf("3. �ϵ��ھ� ���\n");
    printf("4. ���� �� ����\n");
    printf("5. ������ ũ����\n");
    printf("6. ���� ����\n");
    printf("7. ���� ����\n");
}

// ���̵� ���� �Լ�
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

// �� �ʱ�ȭ �Լ�
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

// ȭ�� ����� �Լ�
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// ���� ���� �Լ�
void setColor(int color) {
    printf("\033[%dm", color);
}

// ���� �ʱ�ȭ �Լ�
void resetColor() {
    printf("\033[0m");
}

// �� ǥ�� �Լ�
void displayMap(Cell map[MAX_HEIGHT][MAX_WIDTH], GameSettings settings, int stage, int lives, int score, time_t startTime, int remainingMines) {
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
    printf("���� ����: %d\n", remainingMines);
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
                setColor(COLOR_YELLOW);
                printf("|��| ");
                resetColor();
            } else {
                printf("|��| ");
            }
        }
        printf("\n");
    }
}

// �� �巯���� �Լ�
void revealCell(Cell map[MAX_HEIGHT][MAX_WIDTH], int x, int y, GameSettings settings, int* score, int* lives, int* remainingMines, int* totalMinesFound) {
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
                    revealCell(map, x + dx, y + dy, settings, score, lives, remainingMines, totalMinesFound);
                }
            }
        }
    }
}

// ��� ��� �Լ�
void toggleFlag(Cell map[MAX_HEIGHT][MAX_WIDTH], int x, int y, int* score, int* remainingMines, int* totalMinesFound) {
    if (x < 0 || x >= MAX_WIDTH || y < 0 || y >= MAX_HEIGHT || map[y][x].isRevealed) {
        return;
    }

    if (map[y][x].isFlagged) {
        map[y][x].isFlagged = 0;
        printf("-����� ȸ����-\n");
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

// ��� ���ڰ� ã�Ҵ��� Ȯ���ϴ� �Լ�
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

// ���� ���� �Լ�
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
        printf("�ൿ�� �����ϼ��� (Ž��: S, ���: F, ����: out): ");
        while (1) {
            scanf("%s", action);
            if (strcmp(action, "S") == 0 || strcmp(action, "s") == 0 || strcmp(action, "F") == 0 || strcmp(action, "f") == 0 || strcmp(action, "out") == 0) break;
            printf("�߸��� �Է��Դϴ�. �ٽ� �Է��ϼ��� (Ž��: S, ���: F, ����: out): ");
        }

        if (strcmp(action, "out") == 0) {
            printf("������ �����մϴ�. ���� �޴��� ���ư��ϴ�.\n");
            gameOver(score, stage, totalMinesFound, lives, startTime);
            return;
        }

        printf("��ǥ�� �Է��ϼ��� (X Y): ");
        scanf("%d %d", &x, &y);

        x--; // ��ǥ�谡 1���� �����ϹǷ� 0���� ��ȯ
        y--; // ��ǥ�谡 1���� �����ϹǷ� 0���� ��ȯ

        if (x < 0 || x >= settings.width || y < 0 || y >= settings.height) {
            printf("�߸��� ��ǥ�Դϴ�. �ٽ� �Է��ϼ���.\n");
            continue;
        }

        if (strcmp(action, "F") == 0 || strcmp(action, "f") == 0) {
            toggleFlag(map, x, y, &score, &remainingMines, &totalMinesFound);
        } else if (strcmp(action, "S") == 0 || strcmp(action, "s") == 0) {
            revealCell(map, x, y, settings, &score, &lives, &remainingMines, &totalMinesFound);
            if (lives == 0) {
                printf("���� ����!\n");
                printf("���� ����: %d\n", score);
                Sleep(3000); // ���� ���� �޽����� 3�� ���� ǥ��
                gameOver(score, stage, totalMinesFound, lives, startTime);
                return;
            }
        }

        if (clearedCells == totalCells || remainingMines == 0 || allMinesFound(map, settings)) {
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
            remainingMines = settings.mines;
            initializeMap(map, settings);
            clearedCells = 0;
        }
    }
}

// ���� ��� �ʱ�ȭ �Լ�
void resetGame(GameSettings* settings) {
    initializeSettings(settings);
    printf("���� ����� �ʱ�ȭ�Ǿ����ϴ�.\n");
}

// ���� ���� �Լ�
void gameOver(int score, int stage, int totalMinesFound, int lives, time_t startTime) {
    clearScreen();
    printf("���� ����! ���� ����: %d\n", score);
    printf("Ŭ������ ��������: %d\n", stage);
    printf("ã�� �� ������ ��: %d\n", totalMinesFound);
    printf("���� ���: %d\n", lives);
    printf("�ɸ� �ð�: %ld��\n", time(NULL) - startTime);
    printf("\n�ƹ� Ű�� �Է��Ͽ� �������� ���ư��ϴ�...\n");
    getchar();
    getchar(); // �� �� ȣ���Ͽ� Enter Ű�� ó��
}

// ���� �� ���� �Լ�
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

// ������ ũ���� ǥ�� �Լ�
void displayCredits() {
    clearScreen();
    printf("�ۼ���: 202121018_���翵\n");
    printf("���α׷� �̸�: ����ã�� ������ V1.4\n");
    printf("\n\n- ������Ʈ �α� -");
    printf("\n_V1.0 : ���� ���������� ����");
    printf("\n_V1.1 : �� ������ ����");
    printf("\n_V1.2 : ���� ��� ��� �߰�");
    printf("\n_V1.3 : ���� �÷� �߰�");
    printf("\n_V1.4 : �����ڵ� ���� �� �ּ��߰�");

    printf("\n���� �޴��� ���ư����� �ƹ� Ű�� ��������...");
    getchar();
    getchar(); // �� �� ȣ���Ͽ� Enter Ű�� ó��
}
