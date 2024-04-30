#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#define MAXLEN 20


struct Node {
    char letter;
    bool show;
    struct Node *next;
};

void getWordFromFile(char* filename, struct Node** head);
struct Node* initNode();
void createListFromString(struct Node** head, char* string);
void guessLetter(struct Node* head, int *livesLeft);
bool isWinner(struct Node* head);
bool modifiedShowStatus(struct Node* head, char c);
void printList(struct Node* head);


int main() {

    struct Node* head = NULL;
    int livesLeft = 5;

    getWordFromFile("levels/level1.txt", &head);

    while ( livesLeft ) {
        printList(head);
        printf(" (%d lives left)", livesLeft);
        guessLetter(head, &livesLeft);

        if ( isWinner(head) ) {
            printList(head);
            printf("\n You win! ");
            return 0;
        }
    }

    printf("\n You lost! ");

    return 0;
}

void getWordFromFile(char* filename, struct Node** head)
{
    FILE *file = fopen(filename, "r");

    int numOfWords = 0;
    char newLine;
    char** words = (char **) malloc(sizeof(char *));
    words[numOfWords] = (char *) malloc(MAXLEN * sizeof(char));

    srand(time(0));

    // read the words from the file using dynamically allocated memory
    while( !feof(file) ) {
        fscanf(file, "%s", words[numOfWords]);
        numOfWords += 1;

        words = (char **) realloc(words, (numOfWords+1) * sizeof(char));
        words[numOfWords] = (char *) malloc(MAXLEN * sizeof(char));
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
    char c, newLine;

    printf("\nGuess a letter: ");
    fscanf(stdin, "%c%c", &c, &newLine);

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

void printList(struct Node* head)
{
    struct Node* node = head;

    printf("\nYour word: ");

    while ( node != NULL ) {
        if( node->show )
            printf("%c", node->letter);
        else
            printf("_");
        node = node->next;
    }
}
