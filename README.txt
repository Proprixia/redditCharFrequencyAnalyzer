This program is a letter counter that analyzes how many times each letter is used in a set of input files listed to it by a "subreddits" file.
The analyzer was built as part of an inquiry into whether and why different online communities exhibit different patterns in letter frequency.
This program's sister project is a program which downloads the top 100 posts from each Subreddit in a text file, "subreddits", alongside all of the comments. The sister project can be found at https://github.com/Proprixia/redditCharFreqDownloader.
The program is licensed under the GNU Public License version 3, or a later version at your option.

To operate the program, first compile it using gcc with the command "gcc -Wall -Werror main.c -l json-c -o Analyzer".
You will need to set several options in the "config.json" file in the same directory as the executable. These are:
+ "subredditsFile": An input text file containing the newline-delineated names of every subreddit you wish to analyze
+ "downloadsDirectory": The input directory containing each subreddit you wish to analyze, with each subreddit text download in its own file, a file with which it shares a name
+ "singleLetterOutputsFile": A csv output file to be created containing data about the frequency of individual letters in each subreddit
+ "letterPairOutputsFile": A csv output file to be created containing data about the frequency of pairs of letters in each subreddit
Once all of these values are set, with matching files existing for "subredditsFile" and "downloadsDirectory", you may run the executable. As of February 22, 2023, there are no options at execution time. Enjoy your data!

If you have any questions, comments, or concerns, please contact the developer at Proprixia on GitHub.

In the future, I may add advanced configuration options, better feedback, more advanced statistics, and immunity to certain errors.
