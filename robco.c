#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>


#define ANSI_COLOR_GREEN		"\x1b[32m"
#define ANSI_COLOR_RESET		"\x1b[0m"

#define MIN(A,B)			((A) < (B) ? (A) : (B))
#define MAX(A,B) 			((A) > (B) ? (A) : (B))
#define TABLENGTH(X)    		(sizeof(X)/sizeof(*X))
#define DICTSIZE			17
#define BOARDROWS			17
#define HEXLEN				6
#define TEXTLEN				12
#define ESC				27
#define BOARDLEN			TEXTLEN * 2 * BOARDROWS
/* words cannot be closer than 2 characters when in game_board */
#define PADDING				2


/* Structure:
 * 	game_board is a single string that is TEXTLEN * 2 * BOARDROWS long
 * 	the printf loop iterates character-by-character
 * 	game_board is separate from the hex-based row indices
 * 	track absolute position in string
 * 		y position is which BOARDROWS bin absolute is in
 * 		x position is which TEXTLEN bin absolute is in
 * 			(adjusted for the two columns)
 * 		word choice, cursor position can be done based on
 * 			absolute position
 * 		(x,y) is just for moving the cursor
 */


enum { NO_PAUSE,     YES_PAUSE };


typedef struct node node;
typedef struct gameword gameword;

struct node {
	char str[TEXTLEN];
	node* next;
};

struct gameword {
	char str[TEXTLEN];
	int start;
	gameword* next;
};



/* ---------------------------------------
 * Util Functions
 * ---------------------------------------
 */

void
die(const char* e, ...){
	fprintf(stdout, "sara: %s\n", e);
	exit(1);
}

void*
ecalloc(size_t nmemb, size_t size){
	void* p;

	if ( !(p = calloc(nmemb, size)) )
		die("ecalloc failed");

	return p;
}

int
minimum(int a, int b, int c){
	return ( (a = a < b ? a : b) < c ? a : c );
}

int
randnum(unsigned int upper){
	/* random number between 0 and 0xFFFF */
	return rand() % upper;
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

void attach(node* c, node** head);
void attachg(gameword* c, gameword** head);
int checkans(char* chosen, char* answer);
char* choosepwd(int diff);
void cleanup();
gameword* find_word(int index, char** subset, int* subset_start, int subset_len);
void move_ptr(int* ptr, char coord, int dir);
int overlaps(int index, const char* to_add, gameword* subset);
void popg(gameword* g, gameword** head);
void print_board(int ptr, char* game_board, node* lhead, node* chead, node* rhead);
void printheader(const char* header, int attempts);
void printpassword(int diff);
void printstepwise(const char* str, int do_pause);
int ptr_col(int ptr);
int ptr_row(int ptr);
void push_sidebar(const char* str, node** head);
void remove_dud(int index, int delim_index, char** subset, int subset_start, int subset_len, char** game_board);
void usage();


/* ---------------------------------------
 * Globals
 * ---------------------------------------
 */

/* Predefine potential words here. Do this by difficulty:
 * Easy: 4-6 chars
 * Med:  6-8 chars
 * Hard: 8-10 chars
 */

const char* ewords[DICTSIZE] = {
	"test", "panel", "want", "expire", "bases", "pair", "digest", "clone",
	"steps", "color", "beta", "water", "from", "break", "shift", "space",
	"relic"
};

const char* mwords[DICTSIZE] = {
	"incubate", "download", "manual", "reagent", "material", "decimal",
	"pointer", "androids", "trainer", "special", "elated", "unique",
	"busted", "include", "satisfy", "mystic", "entered"
};

const char* hwords[DICTSIZE] = {
	"sequence", "transform", "technical", "produced", "terminal",
	"fantastic", "assembly", "buffered", "terminate", "pressure",
	"potential", "different", "separate", "relentless", "waiting",
	"passwords", "emphatic"
};

char game_board[BOARDLEN];
const char* junk_chars = "!@#$%^&*()_+-={}<>,.;:/\\|?[]~`\"";
const char* delim_chars = "(){}[]<>";
const char** pickword[] = { ewords, mwords, hwords };
int num_words[] = { 8, 10, 12 };
node* lhead = NULL;
node* chead = NULL;
node* rhead = NULL;


/* ---------------------------------------
 * Functions
 * ---------------------------------------
 */


void
attach(node* c, node** head){
	int i;
	node* current;

	if (!*head){
		*head = c;
		return;
	}

	for (i=0,current=*head;current->next;current=current->next,i++);

	while (i >= BOARDROWS - 1){
		current = *head;
		*head = (*head)->next;

		free(current);
		i--;
	}

	for (current=*head;current->next;current=current->next);

	current->next = c;
}


void
attachg(gameword* c, gameword** head){
	int i;
	node* current;

	if (!*head){
		*head = c;
		return;
	}

	for (current=*head;current->next;current=current->next);

	current->next = c;
}


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


char*
choosepwd(int diff){
	time_t t;
	srand((unsigned) time(&t));

	return pickword[diff][rand() % DICTSIZE];
}


void
cleanup(){
	node* current, * tmp;

	node* heads[3] = { lhead, chead, rhead };

	for (i=0;i < 3;i ++){
		current = heads[i];

		while (current){
			tmp = current;
			current = current->next;

			free(tmp);
		}
	}
}


int
find_delim(int index, char* game_board){
	int i;
	char tmp;
	char sdelim = game_board[index];
	char edelim = '0';
	int switched = 0;
	char* sdelims = "({[<";
	char* edelims = ")}]>";

	for (i=0;i < slen(sdelims);i++){
		if (sdelim == sdelims[i]){
			edelim = edelims[i];

			break;
		}
	}

	for (i=0;i < slen(edelims);i++){
		if (sdelim == edelims[i]){
			edelim = edelims[i];

			/* Proper alignment */
			tmp = sdelim;
			sdelim = edelim;
			edelim = tmp;
			switched = 1;

			break;
		}
	}

	if (edelim == '0')
		return -1;

	if (!switched){
		for (i=index;i < BOARDLEN;i++)
			if (game_board[i] == edelim)
				return i;

	} else {
		for (i=index;i > 0;i--)
			if (game_board[i] == sdelim)
				return i;

	}
}


gameword*
find_word(int index, gameword* subset){
	int i, subset_end;
	gameword* current = subset;

	while (current){
		subset_end = current->start + slen(current->str);

		if (current->start <= index && index <= subset_end)
			return current;

		current = current->next;
	}

	return NULL;
}


void
move_ptr(int* ptr, char coord, int dir){
	switch(coord){
	case 'x':
		if ((*ptr % (dir > 0 ? TEXTLEN - 1 : TEXTLEN)) == 0)
			return;

		*ptr += dir;
		break;
	case 'y':
		if ( (ptr_row(*ptr) == 0 && dir < 0) || (ptr_row(*ptr) == BOARDROWS && dir > 0) )
			return;

		*ptr += dir * TEXTLEN;
		break;
	}
}


int
overlaps(int index, const char* to_add, gameword* subset){
	int subset_end, end_point;
	gameword* current = subset;

	while (current){
		// TODO: what if current->start has not been set yet?
		subset_end = current->start + slen(current->str);
		end_point = index + slen(to_add);

		/* if endpoint is not smaller than the starting point,
		 * or if the starting point is not bigger than the ending
		 * point, we overlap with this string
		 */
		if ( !(end_point == MIN(end_point, current->start - PADDING)) ||
		(index == MAX(index, subset_end + PADDING)) )
			return 1;
	}

	return 0;
}


// TODO: Linus pointers
void
popg(gameword* g, gameword** head){
	gameword* tmp, * current = *head;

	while (current){
		if (current == g){

			return;
		}

		current = current->next;
	}
}


void
print_board(int ptr, char* game_board, node* lhead, node* chead, node* rhead){
	int i, j, col1_idx, col2_idx;
	node* lcurrent = lhead;
	node* ccurrent = chead;
	node* rcurrent = rhead;

	for (i=0;i < BOARDROWS;i++){
		col1_idx = i * TEXTLEN;
		col2_idx = i * TEXTLEN * 2;

		if (lcurrent){
			printf("%s ", lcurrent->str);
			lcurrent = lcurrent->next;
		}

		for (j=col1_idx;j < col1_idx + TEXTLEN;j++)
			printf("%c", game_board[j]);

		printf("  ");

		if (ccurrent){
			printf("%s ", ccurrent->str);
			ccurrent = ccurrent->next;
		}

		for (j=col2_idx;j < col2_idx + TEXTLEN;j++){
			// TODO: extended ASCII is non-standard
			if (ptr_row(ptr) == i && ptr_col(ptr) == j)
				printf("%c", 219);
			else
				printf("%c", game_board[j]);

		}

		printf(" ");

		if (rcurrent){
			printf(">%s\n", rcurrent->str);
			rcurrent = rcurrent->next;

		} else {
			printf(">\n");
		}
	}
}


void
printheader(const char* header, int attempts){
	int i;

	printstepwise(header, YES_PAUSE);
	printf("%d", attempts);

	printstepwise(" ATTEMPTS LEFT:", NO_PAUSE);
	for (i=0;i < attempts;i++)
		printstepwise(" @", NO_PAUSE);

	printstepwise("\n\n", YES_PAUSE);
}


/* Build a random string
 * Sample from a uniform distribution to decide what entry gets the real password
 * Use a mixture of junk and other words
 */

/* Run the password minigame */
void
printpassword(int diff){
	char buf;
	int i, j, rn, res;
	gameword* cursub;
	char tmpstr[TEXTLEN];
	char* pwd = choosepwd(diff);
	int attempts = 3, ptr = 0, duds = 0, delim_index = -1, allow_replenish = 1;
	int init_hex = randnum(0xFFFF);
	int maxjunk = slen(junk_chars);
	gameword* subset, * chosen = NULL;
	
	for (i=0;i < BOARDLEN;i++){
		rn = randnum(maxjunk);
		game_board[i] = junk_chars[rn];
	}

	// TODO:
	/* select a subset of words */
	for (i=0;i < num_words[diff];i++){
		// TODO: don't pick duplicates, so rn is always increasing
		rn = ;

		gword = ecalloc(1, sizeof(gameword));
		gword->str = pickword[diff][rn];

		attachg(gword, &subset);
	}

	// TODO: get pwd_idx-rd gameword->str
	/* select a password from subset */
	pwd_idx = randnum(num_words[diff]);

	// TODO:
	/* shuffle subset */

	/* insert words into game board */
	for (cursub=subset;cursub && cursub->next;cursub=cursub->next){
		rn = randnum(BOARDLEN);

		// TODO: what if you can't find a spot?
		while (overlaps(rn, cursub))
			rn = randnum(BOARDLEN);

		for (j=0;j < slen(cursub->str);j++,rn++)
			game_board[rn] = cursub->str[j];
	}

	printheader("ENTER PASSWORD NOW\n\n", attempts);

	/* populate hex nodes */
	for (i=0;i < BOARDROWS;i++){
		snprintf(tmpstr, HEXLEN, "0x%X", init_hex + i*10);
		push_sidebar(tmpstr, &lhead);

		snprintf(tmpstr, HEXLEN, "0x%X", init_hex + (i+BOARDROWS)*10);
		push_sidebar(tmpstr, &chead);
	}

	// TODO:
	// 	highlight word/delim range under cursor
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
			chosen = find_word(ptr, subset);
			
			res = checkans(chosen->str, pwd);
			if (res >= 0){
				snprintf(tmpstr, TEXTLEN, "%s", chosen->str);
				push_sidebar(tmpstr, &rhead);

				/* not quite! */
				if (res != slen(pwd)){
					attempts--;

					push_sidebar("Entry denied", &rhead);

					snprintf(tmpstr, TEXTLEN, "%d/%d correct.", res, slen(pwd));
					push_sidebar((const char*) tmpstr, &rhead);
				
				/* correct! */
				} else {
					push_sidebar("Access granted!", &rhead);
				}

			} else {
				if ( (delim_index = find_delim(ptr, game_board)) > 0 ){
					push_sidebar("Dud removed.", &rhead);
					remove_dud(ptr, delim_index, subset, subset_start, subset_len, (char**) &game_board);

					duds++;

					if (duds == 3 && allow_replenish){
						attempts = 4;
						push_sidebar("Allowance", &rhead);
						push_sidebar("replenished.", &rhead);
						allow_replenish = 0;
					}

				} else {
					attempts--;
					push_sidebar("Entry denied", &rhead);
				}
			}

			break;
		case ESC:
			attempts = 0;
			break;
		}

		// TODO: redraw entire screen?
		system("clear");

		if (attempts == 1)
			printheader("!!! WARNING: LOCKOUT IMMINENT !!!\n\n", attempts);

		print_board(*ptr, game_board, lhead, chead, rhead);
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
ptr_col(int ptr){
	return (ptr % TEXTLEN);
}


int
ptr_row(int ptr){
	return (floor(ptr / BOARDROWS));
}


void
push_sidebar(const char* str, node** head){
	node* cnode = ecalloc(1, sizeof(node));
	cnode->next = NULL;
	snprintf(cnode->str, TEXTLEN, "%s", str);
	attach(cnode, head);
}


// TODO: add delim indices to an array of already used indices
void
remove_dud(int index, int delim_index, gameword** subset, char** game_board){
	int i;
	int to_remove = randnum(num_words[diff]);
	gameword* current, * target;

	/* find subset */
	for (i=0,current=*subset;current && current->next;current=current->next,i++){
		if (i == to_remove){
			target = current;
			break;
		}
	}

	for (i=target->start;i < slen(target->str);i++)
		*game_board[i] = '.';

	popg(target, subset);
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


// TODO: sighandler, need to execute cleanup()
int
main(int argc, char* argv[]){
	int diff;
	struct timeval t;

	if (argc != 2){
		usage();
		return 0;
	}
	
	gettimeofday(&t, NULL);
	srand((unsigned) (t.tv_usec * t.tv_sec));

	diff = atoi(argv[1]);

	printf(ANSI_COLOR_GREEN);

	printstepwise("Welcome to ROBCO Industries (TM) TermLink\n", YES_PAUSE);
	printstepwise("ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\n", YES_PAUSE);
	printpassword(diff);

	printf(ANSI_COLOR_RESET);

	cleanup();

	return 0;
}

