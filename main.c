#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>

const char *words[] = {"program", "coding", "computer", "algorithm", "variable", "function"};
int numWords = sizeof(words) / sizeof(words[0]);

const char* chooseRandomWord(const char *wordList[], int listSize) {
    srand(time(NULL));
    int randomIndex = rand() % listSize;
    return wordList[randomIndex];
}

void drawHangman(int lives) {
    // Implement your hangman drawing logic here (e.g., using DrawLine, DrawCircle, etc.)
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Hangman");
    SetTargetFPS(60);

    const char *chosenWord = chooseRandomWord(words, numWords);
    int wordLength = strlen(chosenWord);
    char hiddenWord[wordLength + 1];
    memset(hiddenWord, '_', wordLength);
    hiddenWord[wordLength] = '\0';

    int lives = 6;
    bool gameOver = false;
    bool guessedLetters[26] = {false};

    while (!WindowShouldClose()) {
        if (!gameOver) {
            int key = GetKeyPressed();
            if (key >= 0 && key < 256 && isalpha(key)) {
                int letterIndex = tolower(key) - 'a';
                if (!guessedLetters[letterIndex]) {
                    guessedLetters[letterIndex] = true;

                    bool found = false;
                    for (int i = 0; i < wordLength; ++i) {
                        if (tolower(chosenWord[i]) == tolower(key)) {
                            hiddenWord[i] = chosenWord[i];
                            found = true;
                        }
                    }

                    if (!found) {
                        lives--;
                        if (lives == 0) {
                            gameOver = true;
                        }
                    }

                    if (strcmp(hiddenWord, chosenWord) == 0) {
                        gameOver = true;
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw UI (e.g., hiddenWord, guessedLetters, lives, win/loss messages, hangman figure)
        DrawText(hiddenWord, 20, 20, 20, BLACK);
        drawHangman(lives);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}