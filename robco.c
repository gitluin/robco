#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


#define ANSI_COLOR_GREEN	"\x1b[32m"
#define ANSI_COLOR_RESET	"\x1b[0m"

#define MIN(A,B)		((A) < (B) ? (A) : (B))
#define MAX(A,B) 		((A) > (B) ? (A) : (B))
#define DICTSIZE		17
#define BOARDROWS		17
#define HEXLEN			6
#define TEXTLEN			12
#define ESC			27
#define BOARDLEN		TEXTLEN * 2 * BOARDROWS
/* words cannot be closer than 2 characters when in game_board */
#define PADDING			2


enum { NO_PAUSE,     YES_PAUSE };


/* ---------------------------------------
 * Util Functions
 * ---------------------------------------
 */

int
minimum(int a, int b, int c){
	return ( (a = a < b ? a : b) < c ? a : c );
}

int
slen(const char* str){
	int i = 0;

	while (*str){ str++; i++; }

	return i;
}

/* thanks to the C# .NET algorithm at programm.top */
int
strsim(const char* str1, const char* str2){
	int i, j, cost;
	int n = slen(str1) + 1, m = slen(str2) + 1;
	int maxlen = MAX(n-1, m-1);
	int dist[n][m];

	for (i=0;i < n;i++)
		dist[i][0] = i;

	for (j=0;j < m;j++)
		dist[0][j] = j;

	for (i=1;i < n;i++){
		for (j=1;j < m;j++){
			cost = str1[i-1] == str2[j-1] ? 0 : 1;
			dist[i][j] = minimum((dist[i-1][j] + 1),
					(dist[i][j-1] + 1),
					(dist[i-1][j-1] + cost));

		if (i > 1 && j > 1 && (str1[i-1] == str2[j-2]) && (str1[i-2] == str2[j-1]))
			dist[i][j] = MIN(dist[i][j], (dist[i-2][j-2] + cost));
		}
	}

	return maxlen - dist[n-1][m-1];
}


/* ---------------------------------------
 * Function Declarations
 * ---------------------------------------
 */

const char* choosepwd(int diff);
void password(int diff);
void printstepwise(const char* str, int do_pause);
int randnum(unsigned int upper);


/* ---------------------------------------
 * Globals
 * ---------------------------------------
 */


const char** pickword[] = { ewords, mwords, hwords };

/* Predefine potential words here. Do this by difficulty:
 * Easy: 4-6 chars
 * Med:  6-8 chars
 * Hard: 8-10 chars
 */

const char* ewords[DICTSIZE] = { "test", "panel", "want", "expire", "bases", "pair", "digest", "clone", "steps", "color", "beta", "water", "from", "break", "shift", "space", "relic" };

const char* mwords[DICTSIZE] = { "incubate", "download", "manual", "reagent", "material", "decimal", "pointer", "androids", "trainer", "special", "elated", "unique", "busted", "include", "satisfy", "mystic", "entered" };

const char* hwords[DICTSIZE] = { "sequence", "transform", "technical", "produced", "terminal", "fantastic", "assembly", "buffered", "terminate", "pressure", "potential", "different", "separate", "relentless", "waiting", "passwords", "emphatic" };

const char* junk_chars = "!@#$%^&*()_+-={}<>,.;:/\\|?[]~`\"";

int num_words[] = { 8, 10, 12 }

struct node {
	char str[TEXTLEN];
	node* next;
};

char game_board[BOARDLEN];

node* lhead = NULL;
node* chead = NULL;
node* rhead = NULL;


/* ---------------------------------------
 * Functions
 * ---------------------------------------
 */

/* Returns the number correct */
int
checkans(char* chosen, char* answer){
	/* number of edits necessary to get to word
	 * 1 edit means the remaining characters were correct
	 * so slen(answer.str) - res
	 */
	if (chosen)
		return (slen(answer) - strsim(chosen, answer));
	else
		return -1;
}

// TODO: new game_board
char*
find_word(int index, char** subset, int* subset_start){
	int i;

	for (i=0;i < TABLENGTH(subset_start);i++){
		subset_end = subset_start[i] + slen(subset[i]);

		if (subset_start <= index || index <= subset_end)
			return subset[i];
	}

	return NULL;
}


char*
choosepwd(int diff){
	time_t t;
	srand((unsigned) time(&t));

	return pickword[diff][rand() % DICTSIZE];
}

/* Build a random string
 * Sample from a uniform distribution to decide what entry gets the real password
 * Use a mixture of junk and other words
 */

/* Run the password minigame */
void
printpassword(int diff){
	char buf;
	char* chosen;
	int i, rn, res;
	char* pwd = choosepwd(diff);
	// TODO:
	char** subset = ecalloc(subset_len, sizeof(MEEP));
	int attempts = 3, ptr = 0;
	int init_hex = randnum(0xFFFF);
	int maxjunk = slen(junk_chars);
	int subset_len = num_words[diff];
	int subset_start[subset_len];

	// TODO: alternative to game_board:
	// 	a single string that is TEXTLEN * 2 * BOARDROWS long
	// 	the printf loop iterates character-by-character
	// 	game_board is separate from the hex-based row indices
	// 	track absolute position in string
	// 		y position is which BOARDROWS bin absolute is in
	// 		x position is which TEXTLEN bin absolute is in
	// 			(adjusted for the two columns)
	// 		word choice, cursor position can be done based on
	// 			absolute position
	// 		(x,y) is just for moving the cursor
	//
	// TODO:
	// 	2. select a subset of the diff words
	// 		a. more words with higher difficulty: 8/10/12 for E/M/H
	// 	3. randomly choose one as the real password
	// 	4. shuffle subset
	
	for (i=0;i < BOARDLEN;i++){
		rn = randnum(maxjunk);
		game_board[i] = junk_chars[rn];
	}

	/* select a subset of words */

	/* select a password from subset */

	/* shuffle subset */

	for (i=0;i < subset_len;i++){
		// TODO: what if you can't find a spot?
		rn = randnum(BOARDLEN);
		while (overlaps(rn, subset[i], subset, subset_start)){
			rn = randnum(BOARDLEN);
		}
		
		/* copy char-by-char */
		for (j=0;j < slen(subset[i]);j++,rn++){
			game_board[rn] = subset[i][j];
		}
	}

	printheader("ENTER PASSWORD NOW\n\n", attempts);

	// TODO: left, center, and right sidebars

	/* populate hex nodes */
	for (i=0;i < BOARDROWS;i++){
		snprintf(tmpstr, HEXLEN, "0x%X", init_hex + i*10);
		push_sidebar(tmpstr, lhead);

		snprintf(tmpstr, HEXLEN, "0x%X", init_hex + (i+BOARDROWS)*10);
		push_sidebar(tmpstr, chead);
	}

	// TODO:
	// event loop
	// 	read user input, moving "cursor" around
	// 		draw cursor
	//
	// TODO: "Up,Down,Left,Right is known as extended key and to detect
	// 	them you have to read Two Char first one is Null and the second
	// 	is the ASCII code but ESC is not extended key so you can detect
	// 	it with only one char."

	//while (fgets(buf, sizeof(buf), stdin) && attempts > 0){
	while ( (buf = getchar()) && attempts > 0){
		switch(buf){
		case 'h':
			move_ptr(&ptr, 'x', -1);
			break;
		case 'j':
			move_ptr(&ptr, 'y', +1);
			break;
		case 'k':
			move_ptr(&ptr, 'y', -1);
			break;
		case 'l':
			move_ptr(&ptr, 'x', +1);
			break;
		case '\n':
			chosen = find_word(ptr, subset, subset_start);

			res = checkans(chosen, pwd);
			if (res >= 0){
				snprintf(tmpstr, TEXTLEN, "%s", chosen.str);
				push_sidebar(tmpstr);

				/* not quite! */
				if (res != slen(pwd.str)){
					attempts--;

					push_sidebar("Entry denied");

					snprintf(tmpstr, TEXTLEN, "%d/%d correct.", res, slen(pwd.str));
					push_sidebar(tmpstr);
				
				/* correct! */
				} else {
					push_sidebar("Access granted!");
				}

			} else {
				push_sidebar("Dud removed.");
				remove_dud(*x_pos, *y_pos, &game_board);
			}

			break;
		case ESC:
			// TODO: allow "cheating" by escaping out like in the game
			break;
		}

		// TODO: redraw entire screen?
		system("clear");

		if (attempts == 1)
			printheader("!!! WARNING: LOCKOUT IMMINENT !!!\n\n", attempts);

		print_board(game_board, lhead, chead, rhead);
	}

}


// TODO: 
int
overlaps(int index, const char* to_add, char** subset, int* subset_start){
	for (j=0;j < subset_len;j++){
		subset_end = subset_start[j] + slen(subset[j]);

		// if start or end of string is in someone else's space, continue
		// if subset_start[j] - PADDING < rn < subset_end + PADDING
		// 	or
		// if subset_start[j] - PADDING < rn + slen(subset[i]) < subset_end + PADDING
			return 1;
	}

	return 0;
}


void
printheader(const char* header, int attempts){
	printstepwise(header, YES_PAUSE);
	printf("%d", attempts);

	printstepwise(" ATTEMPTS LEFT:", NO_PAUSE);
	for (i=0;i < attempts;i++)
		printstepwise(" @", NO_PAUSE);

	printstepwise("\n\n", YES_PAUSE);
}


void
remove_dud(int index, char** game_board){
	*game_board[index] = '.'
}


// TODO: if out of bounds, don't move
void
move_x(int* x, int dir){

}


// TODO: if out of bounds, don't move
void
move_y(int* y, int dir){

}


void
push_sidebar(const char* str){
	node* cnode = ecalloc(1, sizeof(node));
	cnode->next = NULL;
	snprintf(cnode.str, TEXTLEN, "%s", str);
	attach(cnode);
}


void
attach(node* c){
	int i;
	node* current;

	if (!head){
		head = c;
		return;
	}

	for (i=0,current=head;current->next;current=current->next,i++);

	while (i >= BOARDROWS - 1){
		current = head;
		head = head->next;

		free(current);
		i--;
	}

	for (current=head;current->next;current=current->next);

	current->next = c;
}


void
print_board(char* game_board, node* lhead, node* chead, node* rhead){
	int i, col1_idx, col2_idx;
	node* lcurrent = lhead;
	node* ccurrent = chead;
	node* rcurrent = rhead;

	for (i=0;i < BOARDROWS;i++){
		col1_idx = i * TEXTLEN;
		col2_idx = i * TEXTLEN * 2;

		if (lcurrent){
			prinf("%s ", lcurrent.str);
			lcurrent = lcurrent->next;
		}

		for (j=col1_idx;j < col1_idx + TEXTLEN;j++)
			printf("%c", game_board[j]);

		printf("  ");

		if (ccurrent){
			prinf("%s ", ccurrent.str);
			ccurrent = ccurrent->next;
		}

		for (j=col2_idx;j < col2_idx + TEXTLEN;j++)
			printf("%c", game_board[j]);

		printf(" ");

		if (current){
			printf(">%s\n", current.str);
			current = current->next;

		} else {
			printf(">\n");
		}
	}
}


/* Print string one char at a time */
void
printstepwise(const char* str, int do_pause){
	while (*str){
		printf("%c", *str);
		fflush(stdout);
		/* Subtle pause */
		usleep(20000);
		str++;
	}

	/* Dramatic pause */
	if (do_pause)
		usleep(200000);
}

int
randnum(unsigned int upper){
	time_t t;
	srand((unsigned) time(&t));
	/* random number between 0 and 0xFFFF */
	return rand() % upper;
}

void
usage(){
	printf("Usage: robco [difficulty]\n");
	printf("Password-guessing minigame from the Fallout series.\n");
	printf("\n");
	printf("Difficulties:\n");
	printf("  1 - Easy\n");
	printf("  2 - Medium\n");
	printf("  3 - Hard\n");
}

int
main(int argc, char* argv[]){
	int diff;

	if (argc != 2){
		usage();
		return 0;
	}

	diff = atoi(argv[1]);

	printf(ANSI_COLOR_GREEN);

	printstepwise("Welcome to ROBCO Industries (TM) TermLink\n", YES_PAUSE);
	printstepwise("ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\n", YES_PAUSE);
	printpassword(diff);

	printf(ANSI_COLOR_RESET);

	return 0;
}

