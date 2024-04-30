#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_LEN 20
#define MAX_LIVES 5

struct Node {
    char letter;
    bool show;
    struct Node *next;
};

struct Node* selectLevel();
void getWordFromFile(char* filename, struct Node** head);
struct Node* initNode();
void createListFromString(struct Node** head, char* string);
void guessLetter(struct Node* head, int *livesLeft);
bool isWinner(struct Node* head);
bool modifiedShowStatus(struct Node* head, char c);
void revealWord(struct Node* head);
void printList(struct Node* head);

int main() {

    struct Node* head = NULL;
    int livesLeft = MAX_LIVES;

    head = selectLevel();

    while ( livesLeft ) {
        printList(head);
        printf(" (\033[0;31m%d\033[0m lives left)", livesLeft);
        guessLetter(head, &livesLeft);

        if ( isWinner(head) ) {
            printList(head);
            printf("\n\033[0;32mYou win!\033[0m ");
            return 0;
        }
    }

    printf("\n\033[0;31mYou lost!\033[0m");
    revealWord(head);

    return 0;
}

struct Node* selectLevel()
{
    struct Node* head = NULL;
    char level[1];
    char filename[20] = "levels/level";

    printf("Select a level (1, 2, 3): ");
    fscanf(stdin, "%s", level);
    fgetc(stdin);

    strcat(filename, level);
    strcat(filename, ".txt\0");

    getWordFromFile(filename, &head);

    return head;
}

void getWordFromFile(char* filename, struct Node** head)
{
    FILE *file = fopen(filename, "r");

    int numOfWords = 0;
    char newLine;
    char** words = (char **) malloc(sizeof(char *));
    words[numOfWords] = (char *) malloc(MAX_LEN * sizeof(char));

    srand(time(0));

    // read the words from the file using dynamically allocated memory
    while( !feof(file) ) {
        fscanf(file, "%s", words[numOfWords]);
        numOfWords += 1;

        words = (char **) realloc(words, (numOfWords+1) * sizeof(char *));
        words[numOfWords] = (char *) malloc(MAX_LEN * sizeof(char));
    }

    createListFromString(head, words[rand() % numOfWords]);

    free(words);
    fclose(file);
}


struct Node* initNode()
{
    struct Node* newNode = (struct Node *)malloc( sizeof(struct Node) );

    newNode->show = false;
    newNode->next = NULL;

    return newNode;
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

        newNode->next = tempNode;
        newNode = tempNode;

        i++;
    }

}

void guessLetter(struct Node* head, int *livesLeft)
{
    char c;

    printf("\nGuess a letter: ");
    c = fgetc(stdin);
    fgetc(stdin);

    if ( !modifiedShowStatus(head, c) ) // if no letter was guessed
        *livesLeft -= 1;                // decrement the no of guesses left
}

bool isWinner(struct Node* head)
{
    struct Node* node = head;
    bool condition = true; // we assume that every letter was guessed

    while ( node != NULL ) {
        if( node->show == false ) // if a not guessed letter is found
            condition = false;    // condition is now false
        node = node->next;
    }

    return condition;
}

bool modifiedShowStatus(struct Node* head, char c)
{
    struct Node* node = head;
    bool isModified = false;

    while ( node != NULL ) {
        // if a letter from the string was guessed
        if ( node->letter == c ) {
            node->show = true; // change its show status
            isModified = true; // and know not to decrement no of lives left
        }
        node = node->next;
    }

    return isModified;
}

void revealWord(struct Node* head)
{
    struct Node* node = head;
    // make the entire word visible
    while ( node != NULL ) {
        node->show = true;
        node = node->next;
    }

    printList(head);
}


void printList(struct Node* head)
{
    struct Node* node = head;

    printf("\nYour word: ");

    while ( node != NULL ) {
        if( node->show )
            printf("\033[0;32m%c\033[0m", node->letter);
        else
            printf("_");
        node = node->next;
    }
}
