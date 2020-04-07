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
#define LWIDTH			17


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
void printstepwise(const char* str);
int randnum();


/* ---------------------------------------
 * Globals
 * ---------------------------------------
 */

/* Predefine potential words here. Do this by difficulty:
 * Easy: 4-6 chars
 * Med:  6-8 chars
 * Hard: 8-10 chars
 */
const char* ewords[DICTSIZE] = { "test", "panel", "want", "expire", "bases", "pair", "digest",
	"clone", "steps", "color", "beta", "water", "from", "break", "shift", "space",
	"relic" };
const char* mwords[DICTSIZE] = { "incubate", "download", "manual", "reagent", "material",
	"decimal", "pointer", "androids", "trainer", "special", "elated", "unique",
	"busted", "include", "satisfy", "mystic", "entered" };
const char* hwords[DICTSIZE] = { "sequence", "transform", "technical", "produced", "terminal",
	"fantastic", "assembly", "buffered", "terminate", "pressure", "potential",
	"different", "separate", "relentless", "waiting", "passwords", "emphatic" };

const char** pickword[] = { ewords, mwords, hwords };


/* ---------------------------------------
 * Functions
 * ---------------------------------------
 */

const char*
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
	int i, success = 0, attempts = 3;
	int init_hex = randnum();
	const char* pwd = choosepwd(diff);

	printstepwise("ENTER PASSWORD NOW\n\n");
	// TODO: Redraw after failure
	printf("%d", attempts);	printstepwise(" ATTEMPTS LEFT: @ @ @\n\n");
	for (i=0;i < LWIDTH;i++){ /* 17 is the number of lines the password minigame has */
		/* first column's hex */
		printf("0x%X\t", init_hex + i*10);
		/* 12 chars long */
		printf("\t");
		
		/* Start with the one immediately following the last in the first column */
		printf("0x%X\t", init_hex + (i+LWIDTH)*10);
		/* 12 chars long */
		printf("\n");
	}
}

/* Print string one char at a time */
void
printstepwise(const char* str){
	while (*str){
		printf("%c", *str);
		fflush(stdout);
		/* Subtle pause */
		usleep(20000);
		str++;
	}
	/* Dramatic pause */
	usleep(200000);
}

int
randnum(){
	time_t t;
	srand((unsigned) time(&t));
	/* random number between 0 and 0xFFFF */
	return rand() % 0xFFFF;
}

int
main(int argc, char* argv[]){
	int diff;

	if (argc != 2){
		printf("Please enter the correct number of arguments!\n");
		return 0;
	}

	diff = atoi(argv[1]);

	printf(ANSI_COLOR_GREEN);

	printstepwise("Welcome to ROBCO Industries (TM) TermLink\n");
	printstepwise("ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\n");
	printpassword(diff);

	printf(ANSI_COLOR_RESET);

	return 0;
}

