#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <json-c/json.h>
#include "macros.c"

struct memory {
    void * contents;
    size_t size;
};

void countWordsInSubreddit(struct memory * text, struct memory * table, int columnNo) {
    // Declares variables for the word counting function.
    char c;
    int matchFound = 0;
    struct memory * currentWord;
    struct memory * freqsList;
    struct memory * wordsList;

    // Initializes the memory structs used in the word counting function.
    CREATEMEMSTRUCT(currentWord, char);
    CREATEMEMSTRUCT(freqsList, int);

    // Assigns the first column of the table to a wordsList placeholder, primarily to make code neater (clearly not a priority).
    wordsList = (struct memory *) * (struct memory **) table->contents;

    // Preparing a new integer list freqsList, which will contain the frequencies of each word and become the next column of the table, by setting its size in rows as equal to the wordsList.
    freqsList->size = (int) ceil((wordsList->size / sizeof(char *))) * (sizeof(int));
    freqsList->contents = realloc(freqsList->contents, freqsList->size);
    
    // Initializes the frequency of each word in the file to 0.
    for (int i = 0; i < freqsList->size / sizeof(int); i ++) {
        * (int *) (freqsList->contents + (sizeof(int) * i)) = 0;
    }

    // Important piece of code which iterates through the text buffer, identifies each word, and increments the appropriate position in the freqsList or creates a new row appropriately.
    for (int i = 0; i < text->size; i++) {
        c = (char) * (char *) (text->contents + (i * sizeof(char))); // Assigns char c to the next character in the text buffer.

        if (c >= 'a' && c <= 'z') { // If char c is a lowercase letter, append it to the currentWord memory struct.
            currentWord->size += sizeof(char);
            currentWord->contents = realloc(currentWord->contents, currentWord->size);
            * (char *) (currentWord->contents + currentWord->size - 2) = c;    
        } else if (c >= 'A' && c <= 'Z') { // Same for if char c is an uppercase letter, but appends the lowercase of that letter.
            currentWord->size += sizeof(char);
            currentWord->contents = realloc(currentWord->contents, currentWord->size);
            * (char *) (currentWord->contents + currentWord->size - 2) = c - 'A' + 'a';
        } else if (currentWord->size > 1) { // If char c is not a letter at all, we assume it is the end of a word and begin determining to increment a frequency value or add a new word.
            * (char *) (currentWord->contents + currentWord->size - 1) = '\0'; // Null-terminates the currentWord memory struct so that it can be effectively compared.

            matchFound = 0; // Boolean used to determine if the currentWord was already used or new. Assumed false at first for each iteration.

            // Scours the wordsList for a matching word. If such a word is found, increments the appropriate row in the freqsList by 1, resets currentWord, sets matchFound to true, and breaks the loop.
            for (int i = 0; i < wordsList->size / sizeof(char *); i++) {
                if (strncmp((char *) * (char **) (wordsList->contents + i * sizeof(char *)), currentWord->contents, currentWord->size) == 0) {
                    matchFound = 1;
                    currentWord->size = 1;
                    currentWord->contents = realloc(currentWord->contents, currentWord->size);
                    
                    * (int *) (freqsList->contents + (i * sizeof(int))) += 1;

                    break;
                }
            }

            // If the searching phase failed to find a resulting match for the current word, this appends new rows to the lists and sets their values appropriately.
            if (matchFound == 0) {
                // Increases the size of wordsList by one row (char *) and inserts a pointer to the currentWord string.
                wordsList->size += sizeof(char *);
                wordsList->contents = realloc(wordsList->contents, wordsList->size);
                memcpy(wordsList->contents + wordsList->size - sizeof(char *), &currentWord->contents, sizeof(char *));
                
                // Increases the size of freqsList by one row (int) and inserts a starting value of 1, since that word has appeared one time.
                freqsList->size += (1 * sizeof(int));
                freqsList->contents = realloc(freqsList->contents, freqsList->size);
                * (int *) (freqsList->contents + freqsList->size - sizeof(int)) = 1;

                // Reset currentWord. Since the contents have been included in the words list, we use malloc rather than realloc.
                currentWord->size = 1;
                currentWord->contents = malloc(currentWord->size);
            }
        }
    }

    * (struct memory **) (table->contents) = (struct memory *) wordsList; // Places the updated words list, originally taken from the table, back in the table.
    
    // Increases the size of the table to accomadate an extra column and adds the freqsList column as that new column.
    table->size += sizeof(struct memory *);
    table->contents = realloc(table->contents, table->size);
    * (struct memory **) (table->contents + (columnNo * sizeof(struct memory *))) = freqsList;

    FREEMEMSTRUCT(currentWord);
}

void countWords(struct json_object * configFile) {
    // Declare variables for main words analyzer function.
    char c;
    FILE * downloadFile;
    FILE * subredditsFile;
    FILE * wordsOutputFile;
    int columnNo;
    struct memory * currentFileName;
    struct memory * currentSubreddit;
    struct memory * firstColumn;
    struct memory * table;
    struct memory * text;
    
    // Initialize firstColumn memory struct. This structure is the first column in the table, representing the words list. Field "size" must be 0 or else segfault, I don't make the rules.
    CREATEMEMSTRUCT(firstColumn, char *);
    firstColumn->size = 0;
    
    // Initializes the table memory struct, which will itself contain other memory structs (columns). The first column (firstColumn) contains words, with further columns containing the frequency of each word in each subreddit. firstColumn is inserted to initialize the table.
    CREATEMEMSTRUCT(table, struct memory *);
    * (struct memory **) table->contents = (struct memory *) firstColumn;

    // Opens the important I/O files, will be soon be replaced with more sophisticated options based on config files.
    wordsOutputFile = fopen(json_object_get_string(json_object_object_get(configFile, "wordsOutputFile")), "w");
    subredditsFile = fopen(json_object_get_string(json_object_object_get(configFile, "subredditsFile")), "r");

    // Initializes variables to be used for accessing files (currentFileName) and for outputting subreddit names to the output file.
    CREATEMEMSTRUCT(currentFileName, char);
    CREATEMEMSTRUCT(currentSubreddit, char);
    CATSTRTOMEMORYSTRUCT(currentFileName, json_object_get_string(json_object_object_get(configFile, "downloadsDirectory")));
    columnNo = 1; // Used with the countWordsInSubreddit() function to tell the function to which column it should output.

    // Loop in which we iterate through the subredditsFile, open the text of each file, and send the appropriate data to the countWordsInSubreddit() function.
    while ((c = fgetc(subredditsFile)) != EOF) {
        if (c != '\n') {
            // Add letters to the currentSubreddit memory struct until a new line is found.
            currentSubreddit->size += 1;
            currentSubreddit->contents = realloc(currentSubreddit->contents, currentSubreddit->size);
            * (char *) (currentSubreddit->contents + currentSubreddit->size - 2) = c;
        } else {
            // Once a new line is found, open the file corresponding to the currentSubreddit value using the currentFileName memory struct.
            * (char *) (currentSubreddit->contents + currentSubreddit->size - 1) = '\0'; // Null-terminate to prevent memory errors.
            MEMSTRUCTCAT(currentFileName, currentSubreddit);

            printf("Opening file: %s\n", (char *) currentFileName->contents); // Print useful feedbakck for the user.

            fprintf(wordsOutputFile, ", %s", (char *) currentSubreddit->contents); // Output to the output CSV the title of each subreddit, just for CSV clearness.
            
            downloadFile = fopen((char *) currentFileName->contents, "r"); // Opens the appropriate file to scan through

            CREATEMEMSTRUCT(text, char);

            if (downloadFile != NULL) {
                // Initialize the text buffer, determine the appropriate size for that buffer, and allocate that size.
                fseek(downloadFile, 0, SEEK_END);
                text->size = ftell(downloadFile);
                rewind(downloadFile);
            }
            
            if ((text->size != 0) && (downloadFile != NULL)) {
                text->contents = realloc(text->contents, text->size);

                // Iterate through the text file to properly create the buffer, null-terminate the buffer.
                for (int i = 0; i < text->size; i++) {
                    c = fgetc(downloadFile);
                    * (char *) (text->contents + i) = c;
                }
                * (char *) (text->contents + text->size - 1) = '\0';

                fclose(downloadFile);

                // Send the important information to the countWords function, which will do the heavy lifting.
                countWordsInSubreddit(text, table, columnNo);

                columnNo += 1; // Increment columnNo since the next run of countWordsInSubreddit() should output to the next column of the table.
            }

            // Cleanup

            if (text->size == 0) { // If text-<size was not 0, then downloadFile was already closed. This simply cleans up memory.
                fclose(downloadFile);
            }
        
            FREEMEMSTRUCT(currentSubreddit);
            FREEMEMSTRUCT(currentFileName);
            FREEMEMSTRUCT(text);
            CREATEMEMSTRUCT(currentFileName, char);
            CREATEMEMSTRUCT(currentSubreddit, char);
            CATSTRTOMEMORYSTRUCT(currentFileName, json_object_get_string(json_object_object_get(configFile, "downloadsDirectory")));
        }
    }
    // Now that the table has been completed with all of the statistics we need, we will clean that stage of the process, then output the table to the output CSV.
    
    // Free unneeded memory.
    FREEMEMSTRUCT(currentSubreddit);
    FREEMEMSTRUCT(currentFileName);
    fclose(subredditsFile);

    fputc('\n', wordsOutputFile); // In the loop, the program fills the first row of the CSV with subreddit names. Now, as we output the table, we need a new line.

    // Worst code I've ever written. Outputs the table to the CSV using a hellish number of memor struct pointer dereferences.

    for (int i = 0; i < ((struct memory *) * (struct memory **) table->contents)->size / sizeof(char *); i ++) { // For each row in the table (since text output to CSV demands we go row-by-row)
        for (int j = 0; j < (table->size / sizeof(struct memory *)); j++) { // For each column in the table
            
            // This is the portion that actually outputs data in the table to the CSV using text output
            if (j == 0) { // If the loop is in the first column, we are in the column of words and must use string outputs
                fputs((char *) * (char **) (((struct memory *) * (struct memory **) table->contents)->contents + i * sizeof(char *)), wordsOutputFile); // Print the string at that point in the table
                free((char *) * (char **) (((struct memory *) * (struct memory **) table->contents)->contents + i * sizeof(char *))); // After printing that string, free that string in memory
            } else { // If the loop is not in the first column, that column contains integers referring to the frequency of that word in each subreddit, demanding integer output 
                if ((((struct memory *) * (struct memory **) (table->contents + j * sizeof(struct memory *)))->size) > i * sizeof(int)) { // If this column has rows defined to this point
                    fprintf(wordsOutputFile, "%d", (int) * (int *) (((struct memory *) * (struct memory **) (table->contents + j * sizeof(struct memory *)))->contents + i * sizeof(int))); // Print the integer at this point in the table
                } else { // If the column is too short (this word was never in the file and was first found afterward)
                    fputc('0', wordsOutputFile); // Output a 0 to the CSV
                }
            }

            // If the current row is not the last row in the column, output a comma to begin the next cell in the CSV
            if (j != (table->size / sizeof(struct memory *)) - 1) {
                fputc(',', wordsOutputFile);
            }
        }

        fputc('\n', wordsOutputFile); // Once each column of that row has been output, we move on to the next row using a newline in the CSV
    }

    // Cleanup

    fclose(wordsOutputFile); // Once the entire table has been output to the CSV, we close the output file.

    // Iterates through each column in the table, frees that column's contents, and frees the column itself
    for (int i = 0; i < (int) ceil(table->size / sizeof(struct memory *)); i++) {
        free(((struct memory *) * (struct memory **) (table->contents + i * sizeof(struct memory *)))->contents);
        free((struct memory *) * (struct memory **) (table->contents + i * sizeof(struct memory *)));
    }

    FREEMEMSTRUCT(table); // Finally, frees the root table
}