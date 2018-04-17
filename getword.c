/*
Logan Lasiter - cssc0049
Professor: Carroll
Class: CS570
Due Date: 9/6/17

File Usage: Takes input from stdin and separates it into words with
Tabs/Newlines/EOF as the delimeters placing each word where the
pointer passed in points to
Return: Size of word that is currently pointed to
*/

#include "getword.h"

int getword(char *w) {
	// variable initialization
	int c;
	int counter = 0;
	int bslash = 0;
	int apostrophe = 0;

	// take in characters until EOF
	while( (c = getchar()) != EOF) {
			if (counter == 254) {
					ungetc(c,stdin);
					w[254] = '\0';
					if(apostrophe == 1) {
						return -3;
					}
					return counter;
			}
			if(c == '\t' || c == ' ') {
					/* spaces and tabs in quotes are included in the word */
					if(apostrophe == 1) {
							w[counter] = c;
							counter++;
							continue;
					}
					/* backslash in front of space or tab is included in the word */
					if(bslash == 1) {
							w[counter] = c;
							counter++;
							bslash = 0;
							continue;
					}
					/* tab or space ends word */
					if (counter > 0) {
							w[counter] = '\0';
							if(apostrophe == 1) {
								return -3;
							}
							return counter;
					} else {
							continue;
					}
			} else if(c == '\n' || c == ';') {
					/* this is the end of line */
					if (counter == 0) {
							w[0] = '\0';
							return 0;
					/* need to put newline or semicolon back so that it can be read again so a 0 can be returned signalling the end of the line */
					} else {
							ungetc(c, stdin);
							w[counter] = '\0';
							if(apostrophe == 1) {
								return -3;
							}
							return counter;
					}
			} else if(c == '\'') {
					/* escaping a quote */ 
					if(bslash == 1) {
						w[counter - 1] = c;
						bslash = 0;
						continue;
					} else if(apostrophe == 1) {
						/* a second apostrophe has been seen ending a word */
						apostrophe = 0;
						continue;
					} else {
						/* a single apostrophe has been seen creating special rules for the following characters until another apostrophe, a newline/semi colon, or EOF is seen*/
						apostrophe = 1;
						continue;
					}
			} else if(c == '\\') {
					/* a blackslash is escaping another backslash */
					if(bslash == 1 || apostrophe == 1) {
							w[counter] = c;
							counter++;
							if(bslash==1){
								bslash = 0;
								continue;
							}
					}
					/* single backslash seen outside of quotes so set the flag */
					bslash = 1;
					continue;
			} else if(c == '<' || c == '>' || c == '|' || c == '&') {
					/* escaping a meta character or inside quote*/
					if(bslash == 1 || apostrophe == 1) {
						w[counter] = c;
						counter++;
						if(bslash==1)bslash = 0;
						continue;
					/* escaping a meta character */
					} else if (counter > 0) {
						ungetc(c, stdin);
						w[counter] = '\0';
						if(apostrophe == 1) {
							return -3;
						}
						return counter;
					/* put the meta character in the char array */
					} else {
						w[0] = c;
						w[1] = '\0';
						counter = 1;
						if(c == '>') {
							if ((c = getchar()) == '!' ) {
								w[1] = c;
								w[2] = '\0';
								counter = 2;
							} else {
							ungetc(c, stdin);
							}
						}
						if(apostrophe == 1) {
							return -3;
						}
						return counter;
					}
			} else {
				/* If new line/tab/EOF/metacharacter is not encountered put
				the character into the array and increment
				pointer */
				if(bslash == 1) {
					bslash = 0;
				}
				w[counter] = c;
				counter++;
			}
	}

	/* If EOF is encountered but there is a word
	before it return the count and put the last
	thing on the line so EOF will be read
	*/
	if(counter > 0) {
			ungetc(c, stdin);
			w[counter] = '\0';
			return counter;
	} else {
			w[0] = '\0';
			return -1;
	}
}