/* First attempt at simulating the RobCo Terminals from Fallout */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define DICT_SIZE 17

/* Predefine potential words here. Do this on a difficulty-based scale,
 * So we could have three separate arrays, with different words
 * Make sure each array has the same number of entries, DICT_SIZE
 */

/* Easy: 4-6 chars
 * Med:  6-8 chars
 * Hard: 8-10 chars
 */
const char* easy_words[] = {"test", "panel", "want", "expire", "bases", "pair", "digest", "clone", "steps", "color", "beta", "water", "from", "break",
			"shift", "space", "relic"};

const char* med_words[] = {"incubate", "download", "manual", "reagent", "material", "decimal", "pointer", "androids", "trainer", "special", "elated",
			"unique", "busted", "include", "satisfy", "mystic", "entered"};

const char* hard_words[] = {"sequence", "transform", "technical", "produced", "terminal", "fantastic", "assembly", "buffered", "terminate", "pressure",
			"potential", "different", "separate", "relentless", "waiting", "passwords", "emphatic"};

/* Print string one char at a time */
void disp_step(const char* str){
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

void welcome(){
	disp_step("Welcome to ROBCO Industries (TM) TermLink\n");
}

void header(){
	disp_step("ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\n");
}

int rand_num(){
	time_t t;
	srand((unsigned) time(&t));
	/* random number between 0 and 0xFFFF */
	return rand() % 0xFFFF;
}

const char* choose_password(int difficulty){
	int i;
	time_t t;
	srand((unsigned) time(&t));
	i = rand() % DICT_SIZE;
	switch(difficulty){
	case 0:
		return easy_words[i];
	case 1:
		return med_words[i];
	case 2:
		return hard_words[i];
	default:
		break;
	}
}

/* A function for building a random string, sometimes incorporating a password from the proper difficulty
 * At some point, you need to incorporate the correct password
 * Randomly choose it, and then set a flag when it's done so it doesn't get done again (pass in a pointer to the int in password())
 */

/* Run the password minigame */
void password(int difficulty){
	int i, success = 0, attempts = 3;
	/* printed the real password yet? */
	int pass_flag;
	int init_hex = rand_num();
	const char* password = choose_password(difficulty);

	header();
	disp_step("ENTER PASSWORD NOW\n\n");
	printf("%d", attempts);	disp_step(" ATTEMPTS LEFT: @ @ @\n\n");
	for (i=0;i<17;i++){ /* 17 is the number of lines the password minigame has */
		/* first column's hex */
		printf("0x%X\t", init_hex + i*10);
		/* 12 chars long */
		printf("\t");
		
		/* Start with the one immediately following the last in the first column */
		printf("0x%X\t", init_hex + (i+17)*10);
		/* 12 chars long */
		printf("\n");
	}
}

int main(int argc, char* argv[]){
	int difficulty = 0;

	if (argc != 2){
		printf("Please enter the correct number of arguments!\n");
		return 0;
	}

	if (strcmp("easy",argv[1]) == 0){
		difficulty = 0;

	} else if (strcmp("med",argv[1]) == 0){
		difficulty = 1;

	} else if (strcmp("hard",argv[1]) == 0){
		difficulty = 2;

	} else {
		printf("Please enter the proper type of arguments!\n");
		return 0;
	}

	printf(ANSI_COLOR_GREEN);
	welcome();
	password(difficulty);

	printf(ANSI_COLOR_RESET);

	return 0;
}

