#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>

#define MAX_GUESSES 6
#define MAX_LEN 256

const char *hangmanWord = NULL;
Texture2D backgroundTexture;

struct Node {
    char letter;
    bool show;
    struct Node *next;
};

// Function prototypes
struct Node* selectLevel(int gameLevel);
void getWordFromFile(char* filename, struct Node** head);
void createListFromString(struct Node** head, char* string);
struct Node* initNode();
struct Node* createNode(char letter);
char* createStringFromList(struct Node* head);
const char* chooseRandomWord(const char *wordList[], int listSize);
void drawHangman(int lives);
void drawDeadHangman();
void drawUI(struct Node *wordList, char guessedLettersStr[], int lives);
bool checkWin(struct Node *wordList);
bool checkLoss(int lives);
void resetGame(struct Node **wordList, char guessedLettersStr[], bool *gameOver, int *lives, int gameLevel);
void drawMenu();
void drawGameOver(bool win, const char *word);
void mainGameLoop(struct Node *wordList, char guessedLettersStr[], int *lives, bool *gameOver);

int main() {

    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Hangman");
    SetTargetFPS(60);

    bool gameRunning = true;
    bool gameStarted = false;
    bool gameOver = false;
    bool restartPending = false;
    bool firstGame = true;

    char guessedLettersStr[27] = "";
    struct Node *wordList = NULL;
    int lives = MAX_GUESSES;
    int gameLevel = 0;

    Image backgroundImage = LoadImage("fundal.png");
    backgroundTexture = LoadTextureFromImage(backgroundImage);
    UnloadImage(backgroundImage);

    while (gameRunning) {
        bool guessedLetters[26] = {false};

        if (!gameStarted || restartPending) {
            if (IsKeyPressed(KEY_ENTER)) {
                gameStarted = true;
                restartPending = false;

                while ( gameLevel == 0 )
                {
                    ClearBackground(RAYWHITE);
                    DrawTexture(backgroundTexture, 0, 0, WHITE);
                    drawMenu();
                    if (IsKeyPressed(KEY_ONE)) {
                        gameLevel = 1;
                    } else if (IsKeyPressed(KEY_TWO)) {
                        gameLevel = 2;
                    } else if (IsKeyPressed(KEY_THREE)) {
                        gameLevel = 3;
                    } else if (IsKeyPressed(KEY_FOUR)) {
                        gameLevel = 4;
                    }

                    EndDrawing();
                }
                resetGame(&wordList, guessedLettersStr, &gameOver, &lives, gameLevel);
                gameLevel = 0;
                firstGame = false;
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                gameRunning = false;
                break;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(backgroundTexture, 0, 0, WHITE);
            if (firstGame){
                DrawText("Welcome! Press Enter to start game", 150, screenHeight / 2 - 20, 30, BLACK);
            } else {
                // Display full start options after the first game
                DrawText("Press Enter to start", 170, 250, 30, BLACK);
                DrawText("Press Backspace to quit", 270, 300, 30, BLACK);
            }

            EndDrawing();
        } else if (gameOver) {
            if(checkWin(wordList))
                sleep(3); // pause for 3 seconds to see the word

            for (int i = 0; i < 60 * 2; i++) {
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                if (checkLoss(lives)) {
                    char chosenWord[40] = "You Lose! The word was: ";
                    strcat(chosenWord, createStringFromList(wordList));

                    DrawText(chosenWord, screenWidth / 2 - MeasureText(chosenWord, 40) / 2, screenHeight / 2 - 200, 40, RED);
                    DrawText(hangmanWord, screenWidth / 2 - MeasureText(hangmanWord, 40) / 2, screenHeight / 2 - 160, 40, RED);
                    drawDeadHangman();
                } else {
                    DrawText("You Win!", screenWidth / 2 - MeasureText("You Win!", 40) / 2, screenHeight / 2 - 20, 40, GREEN);
                }
                EndDrawing();
            }

            ClearBackground(RAYWHITE);
            DrawText("Press ENTER to restart...", screenWidth / 2 - MeasureText("Press ENTER to restart...", 20) / 2, screenHeight / 2, 20, DARKGRAY);
            restartPending = true;
        } else {
            if (IsKeyPressed(KEY_BACKSPACE)) {
                gameRunning = false;
                break;
            } else {
                int key = GetKeyPressed();

                if (key >= 0 && key < 256 && isalpha(key)) {
                    int letterIndex = tolower(key) - 'a';
                    char guessedLetter = tolower(key);
                    if (!guessedLetters[letterIndex]) {
                        guessedLetters[letterIndex] = true;
                        bool found = false;
                        bool isModified = false;
                        bool specialSymbols = false;

                        struct Node *current = wordList;
                        while ( current != NULL ) {
                            // if a letter from the string was guessed
                            if ( current->letter == guessedLetter ) {
                                current->show = true; // change its show status
                                found = true; // and know that we found it
                            }
                            current = current->next;
                        }

                        int len = strlen(guessedLettersStr);

                        if (!found) {
                            bool alreadyGuessed = false;
                            for (int i = 0; i < len; i++) {
                                if (guessedLettersStr[i] == key) {
                                    alreadyGuessed = true;
                                    break;
                                }
                            }

                            if (!alreadyGuessed)
                                lives--;
                        }

                        if( !strchr(guessedLettersStr, key)) {
                            guessedLettersStr[len] = key;
                            guessedLettersStr[len + 1] = '\0';
                        }

                        gameOver = checkWin(wordList) || checkLoss(lives);
                    }
                }
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(backgroundTexture, 0, 0, WHITE);
            drawUI(wordList, guessedLettersStr, lives);
            EndDrawing();
        } // End of round loop
    } // End of main game loop

    CloseWindow();

    // Free memory allocated for wordList
    while (wordList != NULL) {
        struct Node *temp = wordList;
        wordList = wordList->next;
        free(temp);
    }

    return 0;
}

void drawMenu() {
    DrawText("Select a Level", 300, 200, 40, BLACK);
    DrawText("1. Easy", 320, 250, 30, BLACK);
    DrawText("2. Medium", 320, 300, 30, BLACK);
    DrawText("3. Hard", 320, 350, 30, BLACK);
    DrawText("4. Very Hard", 320, 400, 30, BLACK);
}

struct Node* selectLevel(int gameLevel) {
    struct Node* head = NULL;
    char level[2];
    char filename[20] = "levels/level";

    level[0] = gameLevel + '0';
    level[1] = '\0';

    strcat(filename, level);
    strcat(filename, ".txt\0");

    getWordFromFile(filename, &head);

    return head;
}

void getWordFromFile(char* filename, struct Node** head)
{
    FILE *file = fopen(filename, "r");

    if(file == NULL)
    {
        printf("Error: file hasn't opened!");
        exit(1);
    }

    int numOfWords = 0;
    char** words = (char **) malloc(sizeof(char *));
    words[numOfWords] = (char *) malloc(MAX_LEN * sizeof(char));

    srand(time(0));

    // read the words from the file using dynamically allocated memory
    while(fgets(words[numOfWords], 100, file) != 0)
    {
        int len = strlen(words[numOfWords]);

        if (len > 0 && words[numOfWords][len - 1] == '\n') {
            words[numOfWords][len - 1] = '\0';
        }   // we want to remove the newline generated by fgets

        numOfWords += 1;

        words = (char **) realloc(words, (numOfWords+1) * sizeof(char *));
        words[numOfWords] = (char *) malloc(MAX_LEN * sizeof(char));
    }

    createListFromString(head, words[rand() % numOfWords]);

    free(words);
    fclose(file);
}

void createListFromString(struct Node** head, char* string)
{
    if (strlen(string) == 0 ) // if the string is empty there is no list to create
        return;

    int i = 0;
    struct Node* newNode = initNode();
    newNode->letter = string[i++];

    *head = newNode; // initiate the head of the list with the first letter of the string

    // iterate through the rest of the string and assign a letter for each node of the list
    while ( string[i] ) {
        struct Node* tempNode = initNode();

        tempNode->letter = string[i];
        if( tempNode->letter == ' ' || tempNode->letter == '-')
            tempNode->show = true;

        newNode->next = tempNode;
        newNode = tempNode;

        i++;
    }
}

struct Node* initNode()
{
    struct Node* newNode = (struct Node *)malloc( sizeof(struct Node) );

    newNode->show = false;
    newNode->next = NULL;

    return newNode;
}

struct Node* createNode(char letter) {
    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->letter = letter;
    newNode->show = false;
    newNode->next = NULL;
    return newNode;
}

char* createStringFromList(struct Node* head)
{
    struct Node* current = head;
    int len = 0;
    while (current != NULL) {
        len++;
        current = current->next;
    }

    char* string = (char *)malloc(len * sizeof(char));
    current = head;
    int i = 0;
    while (current != NULL) {
        string[i++] = current->letter;
        current = current->next;
    }

    string[i] = '\0';

    return string;

}

const char* chooseRandomWord(const char *wordList[], int listSize) {
    srand(time(NULL));
    int randomIndex = rand() % listSize;
    return wordList[randomIndex];
}

void drawHangman(int lives) {
    Vector2 basePos = {100, 500};
    int radius = 30;
    if (lives < 7) {
        DrawLineEx((Vector2){basePos.x, basePos.y + 30}, (Vector2) {basePos.x, basePos.y - 300}, 5, BLACK);
        DrawLineEx((Vector2) {basePos.x, basePos.y - 300}, (Vector2) {basePos.x + 100, basePos.y - 300}, 5, BLACK);
        DrawLineEx((Vector2) {basePos.x + 100, basePos.y - 300}, (Vector2) {basePos.x + 100, basePos.y - 200}, 5,
                   BLACK);
    }
    if (lives < 6) DrawCircle(basePos.x + 100, basePos.y - 200 + radius, radius, BLACK);
    if (lives < 5) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 2 * radius}, (Vector2){basePos.x + 100, basePos.y - 200 + 150}, 5, BLACK);
    if (lives < 4) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 67, basePos.y - 200 + 2 * radius + 20}, 5, BLACK);
    if (lives < 4) DrawLineEx((Vector2){basePos.x + 67, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 55, basePos.y - 200 + 2 * radius + 0}, 5, BLACK);
    if (lives < 3) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 133, basePos.y - 200 + 2 * radius + 20}, 5, BLACK);
    if (lives < 3) DrawLineEx((Vector2){basePos.x + 133, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 145, basePos.y - 200 + 2 * radius + 0}, 5, BLACK);

    // Modified leg drawing
    if (lives < 2) {
        DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 150}, (Vector2){basePos.x + 70, basePos.y - 100 + 20}, 5, BLACK); // Left leg
        DrawLineEx((Vector2){basePos.x + 70, basePos.y - 100 + 20}, (Vector2){basePos.x + 50, basePos.y - 50 + 20}, 5, BLACK);  // Lower left leg
    }
    if (lives < 1) {
        DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 150}, (Vector2){basePos.x + 130, basePos.y - 100 + 20}, 5, BLACK); // Right leg
        DrawLineEx((Vector2){basePos.x + 130, basePos.y - 100 + 20}, (Vector2){basePos.x + 150, basePos.y - 50 + 20}, 5, BLACK); // Lower right leg
    }
}

void drawDeadHangman(void) {
    Vector2 basePos = {100, 500};
    int radius = 30;
    DrawLineEx((Vector2){basePos.x, basePos.y + 30}, (Vector2) {basePos.x, basePos.y - 300}, 5, BLACK);
    DrawLineEx((Vector2){basePos.x, basePos.y - 300}, (Vector2){basePos.x + 100, basePos.y - 300}, 5, BLACK);
    DrawLineEx((Vector2){basePos.x + 100, basePos.y - 300}, (Vector2){basePos.x + 100, basePos.y - 200}, 5, BLACK);
    DrawCircle(basePos.x + 100, basePos.y - 200 + radius, radius, BLACK);
    float scurtareX = 4.0f; // Ajustați valoarea pentru a controla scurtarea pe orizontală
    float scurtareY = 4.0f; // Ajustați valoarea pentru a controla scurtarea pe verticală
    DrawLineEx((Vector2){basePos.x + 80 + scurtareX, basePos.y - 185 + scurtareY},
               (Vector2){basePos.x + 100 - scurtareX, basePos.y - 165 - scurtareY}, 3, WHITE);
    DrawLineEx((Vector2){basePos.x + 80 + scurtareX, basePos.y - 165 - scurtareY},
               (Vector2){basePos.x + 100 - scurtareX, basePos.y - 185 + scurtareY}, 3, WHITE);
    DrawLineEx((Vector2){basePos.x + 100 + scurtareX, basePos.y - 185 + scurtareY},
               (Vector2){basePos.x + 120 - scurtareX, basePos.y - 165 - scurtareY}, 3, WHITE);
    DrawLineEx((Vector2){basePos.x + 100 + scurtareX, basePos.y - 165 - scurtareY},
               (Vector2){basePos.x + 120 - scurtareX, basePos.y - 185 + scurtareY}, 3, WHITE);
    DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 2 * radius}, (Vector2){basePos.x + 100, basePos.y - 200 + 150}, 5, BLACK);
    DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 67, basePos.y - 200 + 2 * radius + 20}, 5, BLACK);
    DrawLineEx((Vector2){basePos.x + 67, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 55, basePos.y - 200 + 2 * radius + 0}, 5, BLACK);
    DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 133, basePos.y - 200 + 2 * radius + 20}, 5, BLACK);
    DrawLineEx((Vector2){basePos.x + 133, basePos.y - 200 + 2 * radius + 20}, (Vector2){basePos.x + 145, basePos.y - 200 + 2 * radius + 0}, 5, BLACK);
    DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 150}, (Vector2){basePos.x + 70, basePos.y - 100 + 20}, 5, BLACK); // Left leg
    DrawLineEx((Vector2){basePos.x + 70, basePos.y - 100 + 20}, (Vector2){basePos.x + 50, basePos.y - 50 + 20}, 5, BLACK);  // Lower left leg
    DrawLineEx((Vector2){basePos.x + 100, basePos.y - 200 + 150}, (Vector2){basePos.x + 130, basePos.y - 100 + 20}, 5, BLACK); // Right leg
    DrawLineEx((Vector2){basePos.x + 130, basePos.y - 100 + 20}, (Vector2){basePos.x + 150, basePos.y - 50 + 20}, 5, BLACK); // Lower right leg
}


void drawUI(struct Node* wordList, char guessedLettersStr[], int lives) {

    int screenHeight = 600;
    int screenWidth = 800;

    DrawText("Hangman", screenWidth / 2 - MeasureText("Hangman", 80) / 2, 10, 80, BLACK);
    drawHangman(lives);

    struct Node* current = wordList;
    int x = 300;
    int y = 300;
    int space = 25; // Adjust the space between characters
    while (current != NULL) {
        if (current->show) {
            DrawText(TextFormat("%c", current->letter), x, y, 40, BLACK);
        } else {
            DrawText("_", x, y, 40, BLACK);
        }

        x += space; // Increment x position
        current = current->next;
    }
    DrawText(TextFormat("Guessed: %s", guessedLettersStr), 300, 200, 40, DARKGRAY);
    DrawText(TextFormat("Lives: %d", lives), 300, 250, 40, DARKGRAY);
}


bool checkWin(struct Node *wordList) {
    struct Node *current = wordList;
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

void resetGame(struct Node **wordList, char guessedLettersStr[], bool *gameOver, int *lives, int gameLevel)
{
    while (*wordList != NULL) {
        struct Node *temp = *wordList;
        *wordList = (*wordList)->next;
        free(temp);
    }

    if (hangmanWord != NULL) {
        free(hangmanWord);
    }

    *wordList = selectLevel(gameLevel);
    *gameOver = false;
    *lives = MAX_GUESSES;
    memset(guessedLettersStr, 0, sizeof(guessedLettersStr));
}
