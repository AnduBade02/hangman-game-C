#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define MAX_GUESSES 6
#define NUM_LETTERS 26

typedef struct LetterNode {
    char letter;
    bool show;
    struct LetterNode *next;
} LetterNode;

LetterNode* createLetterNode(char letter) {
    LetterNode *newNode = (LetterNode*)malloc(sizeof(LetterNode));
    newNode->letter = letter;
    newNode->show = false;
    newNode->next = NULL;
    return newNode;
}

LetterNode* createWordList(const char *word) {
    LetterNode *head = NULL;
    LetterNode *current = NULL;
    for (int i = 0; word[i] != '\0'; ++i) {
        LetterNode *newNode = createLetterNode(word[i]);
        if (head == NULL) {
            head = newNode;
            current = newNode;
        } else {
            current->next = newNode;
            current = newNode;
        }
    }
    return head;
}

const char* chooseRandomWord(const char *wordList[], int listSize) {
    srand(time(NULL));
    int randomIndex = rand() % listSize;
    return wordList[randomIndex];
}

void drawHangman(int lives) {
    Vector2 basePos = {100, 500};
    int radius = 30;
    DrawLineEx(basePos, (Vector2){basePos.x, basePos.y - 250}, 5, BLACK);
    DrawLineEx((Vector2){basePos.x, basePos.y - 250}, (Vector2){basePos.x + 100, basePos.y - 250}, 5, BLACK);
    DrawLineEx((Vector2){basePos.x + 100, basePos.y - 250}, (Vector2){basePos.x + 100, basePos.y - 200}, 5, BLACK);

    if (lives < 6) DrawCircle(basePos.x + 100, basePos.y - 200 + radius, radius, BLACK);
    if (lives < 5) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 2 * radius}, (Vector2){basePos.x + 100, basePos.y - 200 + 150}, 5, BLACK);
    if (lives < 4) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 50, basePos.y - 200 + 2 * radius + 20}, 5, BLACK);
    if (lives < 3) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 150, basePos.y - 200 + 2 * radius + 20}, 5, BLACK);
    if (lives < 2) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 150}, (Vector2){basePos.x + 50, basePos.y - 50 + 20}, 5, BLACK);
    if (lives < 1) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 150}, (Vector2){basePos.x + 150, basePos.y - 50 + 20}, 5, BLACK);
}

void drawUI(LetterNode *wordList, char guessedLettersStr[], int lives) {
    int screenWidth = 800;
    int screenHeight = 600;

    DrawText("Hangman", screenWidth / 2 - MeasureText("Hangman", 40) / 2, 10, 40, BLACK);
    drawHangman(lives);

    LetterNode *current = wordList;
    int x = 20;
    while (current != NULL) {
        if (current->show) {
            DrawText(TextFormat("%c", current->letter), x, 20, 20, BLACK);
        } else {
            DrawText("_", x, 20, 20, BLACK);
        }
        x += 25;
        current = current->next;
    }
    DrawText(TextFormat("Guessed: %s", guessedLettersStr), 20, 50, 20, DARKGRAY);
    DrawText(TextFormat("Lives: %d", lives), 20, 80, 20, DARKGRAY);
}

bool checkWin(LetterNode *wordList) {
    LetterNode *current = wordList;
    while (current != NULL) {
        if (!current->show) {
            return false;
        }
        current = current->next;
    }
    return true;
}

bool checkLoss(int lives) {
    return lives == 0;
}

void resetGame(LetterNode **wordList, char guessedLettersStr[], bool *gameOver, int *lives) {
    while (*wordList != NULL) {
        LetterNode *temp = *wordList;
        *wordList = (*wordList)->next;
        free(temp);
    }

    const char *words[] = {"program", "coding", "computer", "algorithm", "variable", "function"};
    int numWords = sizeof(words) / sizeof(words[0]);
    const char *chosenWord = chooseRandomWord(words, numWords);
    *wordList = createWordList(chosenWord);
    *gameOver = false;
    *lives = MAX_GUESSES;
    memset(guessedLettersStr, 0, sizeof(guessedLettersStr));
}

int main() {
    const char *words[] = {"program", "coding", "computer", "algorithm", "variable", "function"};
    int numWords = sizeof(words) / sizeof(words[0]);

    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Hangman");
    SetTargetFPS(60);

    bool gameRunning = true;
    char guessedLettersStr[27] = "";
    LetterNode *wordList = NULL;
    bool gameOver = false;
    int lives = MAX_GUESSES;
    bool gameStarted = false;

    while (!WindowShouldClose() && gameRunning) {
        if (!gameStarted || (gameOver && IsKeyPressed(KEY_ENTER))) {
            gameStarted = true;
            resetGame(&wordList, guessedLettersStr, &gameOver, &lives);
        }

        if(gameStarted)
        {
            const char *chosenWord = chooseRandomWord(words, numWords);
            wordList = createWordList(chosenWord);
            bool guessedLetters[26] = {false};

            while (!WindowShouldClose() && !gameOver) {
                int key = GetKeyPressed();
                if (key >= 0 && key < 256 && isalpha(key)) {
                    int letterIndex = tolower(key) - 'a';
                    if (!guessedLetters[letterIndex]) {
                        guessedLetters[letterIndex] = true;
                        bool found = false;

                        LetterNode *current = wordList;
                        while (current != NULL) {
                            if (tolower(current->letter) == tolower(key)) {
                                current->show = true;
                                found = true;
                            }
                            current = current->next;
                        }

                        if (!found) {
                            lives--;
                        }

                        gameOver = checkWin(wordList) || checkLoss(lives);

                        int len = strlen(guessedLettersStr);
                        guessedLettersStr[len] = key;
                        guessedLettersStr[len + 1] = '\0';
                    }
                }

                BeginDrawing();
                ClearBackground(RAYWHITE);
                drawUI(wordList, guessedLettersStr, lives);

                if (gameOver) {
                    for (int i = 0; i < 60 * 2; i++) {
                        BeginDrawing();
                        ClearBackground(RAYWHITE);
                        if (checkLoss(lives)) {
                            DrawText("You Lose! The word was:", screenWidth / 2 - MeasureText("You Lose! The word was:", 40) / 2, screenHeight / 2 - 40, 40, RED);
                            DrawText(chosenWord, screenWidth / 2 - MeasureText(chosenWord, 40) / 2, screenHeight / 2, 40, RED);
                        } else {
                            DrawText("You Win!", screenWidth / 2 - MeasureText("You Win!", 40) / 2, screenHeight / 2 - 20, 40, GREEN);
                        }
                        EndDrawing();
                    }

                    ClearBackground(RAYWHITE);
                    DrawText("Press ENTER to restart...", screenWidth / 2 - MeasureText("Press ENTER to restart...", 20) / 2, screenHeight / 2, 20, DARKGRAY);
                    if (IsKeyPressed(KEY_ENTER)) {
                        resetGame(&wordList, guessedLettersStr, &gameOver, &lives);
                        gameStarted = false;
                    }
                }

                EndDrawing();
            } // End of round loop (inner while)
        }
    } // End of main game loop (outer while)

    while (wordList != NULL) {
        LetterNode *temp = wordList;
        wordList = wordList->next;
        free(temp);
    }

    CloseWindow();
    return 0;
}
