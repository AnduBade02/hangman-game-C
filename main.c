#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
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
void selectLevel(int gameLevel, char **question, char **answer);
void getWordFromFile(char* filename, char** question, char** answer);
struct Node* createListFromString(char* string);
struct Node* initNode();
char* createStringFromList(struct Node* head);
void drawHangman(int lives);
void drawDeadHangman();
void drawUI(struct Node* answerList, char* hangmanQuestion, char guessedLettersStr[], int lives);
bool checkWin(struct Node *answerList);
bool checkLoss(int lives);
void resetGame(struct Node **answerList, char** hangmanQuestion, char guessedLettersStr[], bool *gameOver, int *lives, int gameLevel);
void drawMenu();

int main() {
    // Initialization
    const int screenWidth = 1200;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Hangman");
    SetTargetFPS(60);

    bool gameRunning = true;
    bool gameStarted = false;
    bool gameOver = false;
    bool restartPending = false;
    bool firstGame = true;

    char guessedLettersStr[27] = "";
    char *hangmanQuestion = NULL;
    struct Node *answerList = NULL;
    int lives = MAX_GUESSES;
    int gameLevel = 0;

    // Load the background image
    Image backgroundImage = LoadImage("assets/fundal.png");
    ImageResize(&backgroundImage, screenWidth, screenHeight);
    backgroundTexture = LoadTextureFromImage(backgroundImage);
    UnloadImage(backgroundImage);

    // Main game loop
    while (gameRunning) {
        bool guessedLetters[26] = {false};

        if (!gameStarted || restartPending) {
            if (IsKeyPressed(KEY_ENTER)) {
                gameStarted = true;
                restartPending = false;
                // Select a level
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
                resetGame(&answerList, &hangmanQuestion, guessedLettersStr, &gameOver, &lives, gameLevel);

                gameLevel = 0;
                firstGame = false;
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                gameRunning = false;
                break;
            }
            // Draw the main menu
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(backgroundTexture, 0, 0, WHITE);
            if (firstGame){
                DrawText("Welcome! Press Enter to start game", 350, screenHeight / 2 - 20, 30, BLACK);
            } else {
                // Display full start options after the first game
                DrawText("Press Enter to start", 400, 350, 30, BLACK);
                DrawText("Press Backspace to quit", 450, 400, 30, BLACK);
            }

            EndDrawing();
        } else if (gameOver) {
            if(checkWin(answerList))
                sleep(3); // pause for 3 seconds to see the word

            for (int i = 0; i < 60 * 2; i++) {
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                if (checkLoss(lives)) {
                    char chosenWord[40] = "You Lose! The word was: ";
                    strcat(chosenWord, createStringFromList(answerList));

                    DrawText(chosenWord, screenWidth / 2 - MeasureText(chosenWord, 40) / 2, screenHeight / 2 - 50, 40, RED);
                    DrawText(hangmanWord, screenWidth / 2 - MeasureText(hangmanWord, 40) / 2, screenHeight / 2 - 10, 40, RED);
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
                // Check if the key is a letter
                if (key >= 0 && key < 256 && isalpha(key)) {
                    int letterIndex = tolower(key) - 'a';
                    char guessedLetter = (char)tolower(key);
                    // Check if the letter was already guessed
                    if (!guessedLetters[letterIndex]) {
                        guessedLetters[letterIndex] = true;
                        bool found = false;
                        // Check if the letter is in the answer
                        struct Node *current = answerList;
                        while ( current != NULL ) {
                            // if a letter from the string was guessed
                            if ( current->letter == guessedLetter ) {
                                current->show = true; // change its show status
                                found = true; // and know that we found it
                            }
                            current = current->next;
                        }

                        int len = (int)strlen(guessedLettersStr);

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
                        // Add the guessed letter to the guessed letters string
                        if( !strchr(guessedLettersStr, key)) {
                            guessedLettersStr[len] = (char)key;
                            guessedLettersStr[len + 1] = '\0';
                        }

                        gameOver = checkWin(answerList) || checkLoss(lives);
                    }
                }
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(backgroundTexture, 0, 0, WHITE);
            drawUI(answerList, hangmanQuestion, guessedLettersStr, lives);
            EndDrawing();
        } // End of round loop
    } // End of main game loop

    CloseWindow();

    // Free memory allocated for answerList
    while (answerList != NULL) {
        struct Node *temp = answerList;
        answerList = answerList->next;
        free(temp);
    }

    return 0;
}

void drawMenu() {
    DrawText("Select a Level", 450, 250, 40, BLACK);
    DrawText("1. Easy", 470, 300, 30, BLACK);
    DrawText("2. Medium", 470, 350, 30, BLACK);
    DrawText("3. Hard", 470, 400, 30, BLACK);
    DrawText("4. Very Hard", 470, 450, 30, BLACK);
}

void selectLevel(int gameLevel, char **question, char **answer) {
    char level[2];
    char filename[30] = "assets/levels/level";

    level[0] = (char)(gameLevel + '0');
    level[1] = '\0'; // solution for Linux based systems

    // Concatenate the level number to the filename
    strcat(filename, level);
    strcat(filename, ".txt");

    getWordFromFile(filename, question, answer);
}


void getWordFromFile(char* filename, char** question, char** answer) {
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        printf("Error: file hasn't opened!");
        exit(1);
    }

    int numOfQuestions = 0;
    char** questions = (char **) malloc(sizeof(char *));
    questions[numOfQuestions] = (char *) malloc(MAX_LEN * sizeof(char));

    srand(time(NULL));

    // read the words from the file using dynamically allocated memory
    while(fgets(questions[numOfQuestions], 256, file) != 0) {
        int len = (int)strlen(questions[numOfQuestions]);
        if (len > 0 && questions[numOfQuestions][len - 1] == '\n') {
            questions[numOfQuestions][len - 1] = '\0'; // remove the newline character
        }

        numOfQuestions++;
        questions = (char **) realloc(questions, (numOfQuestions + 1) * sizeof(char *));
        questions[numOfQuestions] = (char *) malloc(MAX_LEN * sizeof(char));
    }

    // choose a random question-answer pair
    char* chosenPair = questions[rand() % numOfQuestions];
    *question = strdup(strtok(chosenPair, "*"));
    *answer = strdup(strtok(NULL, "*"));

    // free allocated memory for questions array
    for (int i = 0; i < numOfQuestions; i++) {
        free(questions[i]);
    }
    free(questions);
    fclose(file);
}


struct Node* createListFromString(char* string)
{
    struct Node* head = NULL;

    if (strlen(string) == 0 ) // if the string is empty there is no list to create
        return NULL;

    int i = 0;
    struct Node* newNode = initNode();
    newNode->letter = string[i++];

    head = newNode; // initiate the head of the list with the first letter of the string

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

    return head;
}

struct Node* initNode()
{
    struct Node* newNode = (struct Node *)malloc( sizeof(struct Node) );

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
    // iterate through the list and assign a letter to the string
    while (current != NULL) {
        string[i++] = current->letter;
        current = current->next;
    }

    string[i] = '\0';

    return string;

}

void drawHangman(int lives) {
    Vector2 basePos = {30, 680};
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
    if (lives < 2) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 50}, (Vector2){basePos.x + 70, basePos.y + 20}, 5, BLACK); // left le
    if (lives < 1) DrawLineEx((Vector2){basePos.x + 100, basePos.y - 50}, (Vector2){basePos.x + 130, basePos.y + 20}, 5, BLACK); // right leg

}

void drawDeadHangman(void) {
    Vector2 basePos = {30, 680};
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
    DrawLineEx((Vector2){basePos.x + 100, basePos.y - 50}, (Vector2){basePos.x + 70, basePos.y + 20}, 5, BLACK); // Shorter left leg
    DrawLineEx((Vector2){basePos.x + 100, basePos.y - 50}, (Vector2){basePos.x + 130, basePos.y + 20}, 5, BLACK);
}


void drawUI(struct Node* answerList, char* hangmanQuestion, char guessedLettersStr[], int lives) {

    int screenWidth = 1200;

    DrawText("Hangman", screenWidth / 2 - MeasureText("Hangman", 80) / 2, 10, 80, BLACK);
    drawHangman(lives);

    struct Node* current = answerList;
    int x = 100;
    int y = 250;
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
    DrawText(TextFormat("Guessed: %s", guessedLettersStr), 100, 150, 40, DARKGRAY);
    DrawText(TextFormat("Lives: %d", lives), 100, 200, 40, DARKGRAY);
    DrawText(TextFormat("Question:"), 550, 300, 40, DARKGRAY);
    DrawText(TextFormat("\n%s", hangmanQuestion), 180, 350, 18, DARKGRAY);
}


bool checkWin(struct Node *answerList) {
    struct Node *current = answerList;
    // Check if all the letters have been guessed
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

void resetGame(struct Node **answerList, char** hangmanQuestion, char guessedLettersStr[], bool *gameOver, int *lives, int gameLevel) {
    // Free memory allocated for answerList
    while (*answerList != NULL) {
        struct Node *temp = *answerList;
        *answerList = (*answerList)->next;
        free(temp);
    }
    // Free memory allocated for hangmanQuestion
    if (hangmanWord != NULL) {
        free((void *)hangmanWord);
    }

    char *hangmanAnswer = NULL;
    // Select a new level
    selectLevel(gameLevel, hangmanQuestion, &hangmanAnswer);
    // Save the answer to a global variable
    *answerList = createListFromString(hangmanAnswer);
    *gameOver = false;
    *lives = MAX_GUESSES;
    memset(guessedLettersStr, 0, sizeof(guessedLettersStr));

}
