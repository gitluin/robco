/* First attempt at simulating the RobCo Terminals from Fallout */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define DICT_SIZE 17

// Predefine potential words here. Do this on a difficulty-based scale,
// So we could have three separate arrays, with different words
// Make sure each array has the same number of entries, DICT_SIZE

// For easy, use words 4-6 chars in length.
char* easy_words[] = {"test", "panel", "want", "expire", "bases", "pair", "digest", "clone", "steps", "color", "beta", "water", "from", "break",
			"shift", "space", "relic"};

// For med, use words 6-8 chars in length
char* med_words[] = {"incubate", "download", "manual", "reagent", "material", "decimal", "pointer", "androids", "trainer", "special", "elated",
			"unique", "busted", "include", "satisfy", "mystic", "entered"};

// For hard, use words 8-10 chars in length
char* hard_words[] = {"sequence", "transform", "technical", "produced", "terminal", "fantastic", "assembly", "buffered", "terminate", "pressure",
			"potential", "different", "separate", "relentless", "waiting", "passwords", "emphatic"};

/* Print string one char at a time */
void disp_step(char* string){
	int i;
	for (i=0;string[i] != '\0';i++){
		printf("%c",string[i]);
		/* Pause after each char */
		fflush(stdout);
		usleep(20000);
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

char* choose_password(int difficulty){
	int i;
	time_t t;
	srand((unsigned) time(&t));
	if (difficulty == 0){
		/* don't go over 17 */
		i = rand() % DICT_SIZE;
		return easy_words[i];

	} else if (difficulty == 1){
		i = rand() % DICT_SIZE;
		return med_words[i];

	} else if (difficulty == 2){
		i = rand() % DICT_SIZE;
		return hard_words[i];
	}
}

// A function for building a random string, sometimes incorporating a password from the proper difficulty
// At some point, you need to incorporate the correct password
// Randomly choose it, and then set a flag when it's done so it doesn't get done again (pass in a pointer to the int in password())


/* Run the password minigame */
void password(int difficulty){
	int success = 0, attempts = 3;
	/* printed the real password yet? */
	int i, pass_flag;
	int init_hex = rand_num();
	char* password = choose_password(difficulty);

	header();
	disp_step("ENTER PASSWORD NOW\n\n");
	printf("%d", attempts);
	disp_step(" ATTEMPTS LEFT: @ @ @\n\n");
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

	if (strcmp("easy",argv[1]) == 0){ // if the user entered easy for the difficulty
		difficulty = 0;

	} else if (strcmp("med",argv[1]) == 0){ // else if the user entered med for the difficulty
		difficulty = 1;

	} else if (strcmp("hard",argv[1]) == 0){ // else if the user entered hard for the difficulty
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

