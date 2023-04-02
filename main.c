#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <json-c/json.h>
#include "words.c"

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

void analyzeFile(char * subredditName, json_object * configFile) {
    FILE * downloadFile;
    int fileSize;
    struct memory * addition;
    struct memory * csvNewLine;
    struct memory * fileName;
    struct memory * fileText;

    CREATEMEMSTRUCT(fileName, char);
    CATSTRTOMEMORYSTRUCT(fileName, json_object_get_string(json_object_object_get(configFile, "downloadsDirectory")));
    CATSTRTOMEMORYSTRUCT(fileName, subredditName);

    printf("%s\n", (char *) fileName->contents);

    fileSize = 0;
    downloadFile = fopen(fileName->contents, "r");
    if (downloadFile != NULL) {
        fseek(downloadFile, 0, SEEK_END);
        fileSize = ftell(downloadFile);
        rewind(downloadFile);
    }

    if ((downloadFile != NULL) & (fileSize != 0)) {
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
        int charPairCounter[27 * 27] = {0};

        char prevC = 0;

        for (int i = 0; i < fileText->size; i++) {
            c = * (char *) (fileText->contents + i);
            if (('a' <= c) && (c <= 'z')) {
                charCounter[c - 'a'] ++;
                if (('a' <= prevC) && (prevC <= 'z')) {
                    charPairCounter[((prevC - 'a') * 27) + (c - 'a')] ++;
                } else if (('A' <= prevC) && (prevC <= 'z')) {
                    charPairCounter[((prevC - 'A') * 27) + ((c - 'a'))] ++;
                } else if (' ' == prevC) {
                    charPairCounter[(27 * 26) + (c - 'a')] ++;
                }
            } else if (('A' <= c) && (c <= 'Z')) {
                charCounter[c - 'A'] ++;
                if (('a' <= prevC) && (prevC <= 'z')) {
                    charPairCounter[((prevC - 'a') * 27) + (c - 'A')] ++;
                } else if (('A' <= prevC) && (prevC <= 'z')) {
                    charPairCounter[((prevC - 'A') * 27) + ((c - 'A'))] ++;
                } else if (' ' == prevC) {
                    charPairCounter[(27 * 26) + (c - 'A')] ++;
                }
            } else if (' ' == c) { 
                if ('a' <= prevC && prevC <= 'z') {
                    charPairCounter[27 * (prevC - 'a') + 26] ++;
                } else if ('A' <= prevC && prevC <= 'Z') {
                    charPairCounter[27 * (prevC - 'A') + 26] ++;
                } else if(c == ' ') {
                    charPairCounter[27 * 27 - 1] ++;
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
        outputFile = fopen(json_object_get_string(json_object_object_get(configFile, "singleLetterOutputsFile")), "a+");
        fputs(subredditName, outputFile);
        fputs(",", outputFile);
        fputs(csvNewLine->contents, outputFile);
    
        fclose(outputFile);
        FREEMEMSTRUCT(csvNewLine);
        FREEMEMSTRUCT(addition);
        FREEMEMSTRUCT(fileText);

        CREATEMEMSTRUCT(addition, char);
        CREATEMEMSTRUCT(csvNewLine, char);

        printf("Number of \"_Z\": %d\n", charPairCounter[27 * 27 - 2]);

        for (int i = 0; i < (27 * 27); i++) {
            if (i == 0) {
                printf("%d\n", charPairCounter[i]);
            }
            addition->size = getNumDigits(charPairCounter[i]) + 1;
            addition->contents = realloc(addition->contents, addition->size);
            snprintf(addition->contents, addition->size, "%d", charPairCounter[i]);

            if (i < ((27 * 27) - 1)) {
                CATSTRTOMEMORYSTRUCT(addition, ",");
            }

            MEMSTRUCTCAT(csvNewLine, addition);
        }

        outputFile = fopen(json_object_get_string(json_object_object_get(configFile, "letterPairOutputsFile")), "a+");
        fputs(subredditName, outputFile);
        fputc(',', outputFile);
        fputs(csvNewLine->contents, outputFile);
        fputc('\n', outputFile);

        fclose(outputFile);
        FREEMEMSTRUCT(csvNewLine);
        FREEMEMSTRUCT(addition);
    }

    FREEMEMSTRUCT(fileName);

    if (downloadFile != NULL) {
        fclose(downloadFile);
    }
}

int main() {
    FILE * subredditsFile;
    int fileSize;
    struct memory * fileText;
    struct memory * subredditName;

    json_object * configFile = json_object_from_file("config.json");

    subredditsFile = fopen(json_object_get_string(json_object_object_get(configFile, "subredditsFile")), "r");
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
    outputFile = fopen(json_object_get_string(json_object_object_get(configFile, "singleLetterOutputsFile")), "w");
    fputs("Subreddit,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z\n", outputFile);
    

    fclose(outputFile);

    outputFile = fopen(json_object_get_string(json_object_object_get(configFile, "letterPairOutputsFile")), "w");
    fputs("Subreddit,", outputFile);
    for (int i = 0; i < (27 * 27); i++) {
        if ((int) floor(i / 27) != 26) {
            fputc((int) floor(i / 27) + 'A', outputFile);
        } else {
            fputc('_', outputFile);
        }
        if (i % 27 != 26) {
            fputc(i % 27 + 'A', outputFile);
        } else {
            fputc('_', outputFile);
        }

        if (i != (27 * 27 - 1)) {
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
            analyzeFile((char *) subredditName->contents, configFile);
            subredditName->size = 1;
            subredditName->contents = realloc(subredditName->contents, subredditName->size);
        } else {
            * (char *) (subredditName->contents + subredditName->size - 1) = * (char *) (fileText->contents + i);
            subredditName->size += 1;
            subredditName->contents = realloc(subredditName->contents, subredditName->size);
            * (char *) (subredditName->contents + subredditName->size - 1) = '\0';
        }
    }

    countWords(configFile);

    FREEMEMSTRUCT(fileText);
    FREEMEMSTRUCT(subredditName);
    json_object_put(configFile);
}
