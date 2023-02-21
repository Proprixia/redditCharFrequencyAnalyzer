#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "macros.c"

struct memory {
    void * contents;
    size_t size;
};

int getNumDigits (int numIn) {
    int numDigits = 1;
    int num = numIn;

    while (abs(num) >= 10) {
        num = (int) floor(abs(num) / 10);
        numDigits ++;
    }
    if (numIn < 0) {
        numDigits ++;
    }

    return numDigits;
}

void analyzeFile(char * subredditName) {
    FILE * downloadFile;
    int fileSize;
    struct memory * addition;
    struct memory * csvNewLine;
    struct memory * fileName;
    struct memory * fileText;

    CREATEMEMSTRUCT(fileName, char);
    CATSTRTOMEMORYSTRUCT(fileName, "./Downloads/");
    CATSTRTOMEMORYSTRUCT(fileName, subredditName);


    printf("%s\n", (char *) fileName->contents);

    downloadFile = fopen(fileName->contents, "r");
    fseek(downloadFile, 0, SEEK_END);
    fileSize = ftell(downloadFile);
    rewind(downloadFile);

    CREATEMEMSTRUCT(fileText, char);
    fileText->size = fileSize;
    fileText->contents = realloc(fileText->contents, fileText->size);

    printf("File size: %d\n", fileSize);

    char c;
    for (int i = 0; i < fileText->size; i++) {
        c = fgetc(downloadFile);
        * (char *) (fileText->contents + i) = c;
    }
    * (char *) (fileText->contents + fileText->size - 1) = '\0';
    
    int charCounter[26] = {0};
    int charPairCounter[26 * 26] = {0};

    char prevC = 0;

    for (int i = 0; i < fileText->size; i++) {
        c = * (char *) (fileText->contents + i);
        if (('a' <= c) && (c <= 'z')) {
            charCounter[c - 'a'] ++;
            if (('a' <= prevC) && (prevC <= 'z')) {
                charPairCounter[((prevC - 'a') * 26) + (c - 'a')] ++;
                if ((((prevC - 'a') * 26) + (c - 'a')) == 0) {
                    printf("%c%c\n", prevC, c);
                }
            } else if (('A' <= prevC) && (prevC <= 'z')) {
                charPairCounter[((prevC - 'A') * 26) + ((c - 'a'))] ++;
                if ((((prevC - 'A') * 26) + (c - 'a')) == 0) {
                    printf("%c%c\n", prevC, c);
                }
            }
        } else if (('A' <= c) && (c <= 'Z')) {
            charCounter[c - 'A'] ++;
            if (('a' <= prevC) && (prevC <= 'z')) {
                charPairCounter[((prevC - 'a') * 26) + (c - 'A')] ++;
                if ((((prevC - 'a') * 26) + (c - 'A')) == 0) {
                    printf("%c%c\n", prevC, c);
                }
            } else if (('A' <= prevC) && (prevC <= 'z')) {
                charPairCounter[((prevC - 'A') * 26) + ((c - 'A'))] ++;
                if ((((prevC - 'A') * 26) + (c - 'A')) == 0) {
                    printf("%c%c\n", prevC, c);
                }
            }
        } else {
            c = 0;
        }

        prevC = c;
    }

    CREATEMEMSTRUCT(csvNewLine, char);
    CREATEMEMSTRUCT(addition, char);

    for (int i = 0; i < 26; i++) {
        if (i < 25) {
            addition->size = (getNumDigits(charCounter[i]) + 2);
            addition->contents = realloc(addition->contents, addition->size);
            snprintf(addition->contents, addition->size, "%d,", charCounter[i]);
        } else {
            addition->size = (getNumDigits(charCounter[i]) + 2);
            addition->contents = realloc(addition->contents, addition->size);
            snprintf(addition->contents, addition->size, "%d\n", charCounter[i]);
        }
        MEMSTRUCTCAT(csvNewLine, addition);
    }

    printf("%s", (char *) csvNewLine->contents);

    FILE * outputFile;
    outputFile = fopen("singleLetterOutputs.csv", "a+");
    fputs(subredditName, outputFile);
    fputs(",", outputFile);
    fputs(csvNewLine->contents, outputFile);
    
    fclose(outputFile);
    fclose(downloadFile);
    FREEMEMSTRUCT(csvNewLine);
    FREEMEMSTRUCT(addition);
    FREEMEMSTRUCT(fileName);
    FREEMEMSTRUCT(fileText);

    CREATEMEMSTRUCT(addition, char);
    CREATEMEMSTRUCT(csvNewLine, char);

    for (int i = 0; i < (26 * 26); i++) {
        if (i == 0) {
            printf("%d\n", charPairCounter[i]);
        }
        addition->size = getNumDigits(charPairCounter[i]) + 1;
        addition->contents = realloc(addition->contents, addition->size);
        snprintf(addition->contents, addition->size, "%d", charPairCounter[i]);

        if (i < ((26 * 25) + 25)) {
            CATSTRTOMEMORYSTRUCT(addition, ",");
        }

        MEMSTRUCTCAT(csvNewLine, addition);
    }

    outputFile = fopen("letterPairOutputs.csv", "a+");
    fputs(subredditName, outputFile);
    fputc(',', outputFile);
    fputs(csvNewLine->contents, outputFile);
    fputc('\n', outputFile);

    fclose(outputFile);
    FREEMEMSTRUCT(csvNewLine);
    FREEMEMSTRUCT(addition);
}

int main() {
    FILE * subredditsFile;
    int fileSize;
    struct memory * fileText;
    struct memory * subredditName;

    subredditsFile = fopen("subreddits", "r");
    fseek(subredditsFile, 0, SEEK_END);
    fileSize = ftell(subredditsFile);
    rewind(subredditsFile);

    CREATEMEMSTRUCT(fileText, char);
    fileText->size = fileSize + 1;
    fileText->contents = realloc(fileText->contents, fileText->size);
    
    char c;
    for (int i = 0; i < fileText->size; i++) {
        c = fgetc(subredditsFile);
        * (char *) (fileText->contents + i) = c;
    }
    * (char *) (fileText->contents + fileText->size - 1) = '\0';

    CREATEMEMSTRUCT(subredditName, char);

    fclose(subredditsFile);

    FILE * outputFile;
    outputFile = fopen("singleLetterOutputs.csv", "w");
    fputs("Subreddit,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z\n", outputFile);

    fclose(outputFile);

    outputFile = fopen("letterPairOutputs.csv", "w");
    fputs("Subreddit,", outputFile);
    for (int i = 0; i < (26 * 26); i++) {
        fputc('A' + (char) (floor(i / 26)), outputFile);
        fputc('A' + (i % 26), outputFile);
        if (i != ((26 * 25) + 25)) {
            fputc(',', outputFile);
        }
    }
    fputc('\n', outputFile);
    fclose(outputFile);

    FREEMEMSTRUCT(subredditName);
    CREATEMEMSTRUCT(subredditName, char);

    for (int i = 0; i < fileText->size; i++) {
        if (* (char *) (fileText->contents + i) == '\n') {
            printf("%s\n", (char *) subredditName->contents);
            analyzeFile((char *) subredditName->contents);
            subredditName->size = 1;
            subredditName->contents = realloc(subredditName->contents, subredditName->size);
        } else {
            * (char *) (subredditName->contents + subredditName->size - 1) = * (char *) (fileText->contents + i);
            subredditName->size += 1;
            subredditName->contents = realloc(subredditName->contents, subredditName->size);
            * (char *) (subredditName->contents + subredditName->size - 1) = '\0';
        }
    }

    FREEMEMSTRUCT(fileText);
    FREEMEMSTRUCT(subredditName);
}
