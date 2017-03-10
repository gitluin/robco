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

// A function that prints out each char in a string one at a time
void disp_step(char* string){
	int i = 0; // for iterating through the input string
	while (string[i] != '\0'){ // keep going until you hit a terminator
		printf("%c",string[i]);
		fflush(stdout); // Flush the buffer, else you do all your waiting and then print the whole string at once
		usleep(20000); // Sleep in between each char
		i++;
	}
	usleep(200000); // Pause at the end of the line
}

// A function that displays the welcome text
void welcome(){
	disp_step("Welcome to ROBCO Industries (TM) TermLink\n");
}

// A function that displays the header text in terminal windows
void header(){
	disp_step("ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\n");
}

// A function for choosing a random number
int rand_num(){
	time_t t; // for srand
	srand((unsigned) time(&t)); // cast unsigned to the type returned by time()
	return rand() % 0xFFFF; // return a random number between 0 and 0xFFFF	
}

// A function that chooses a random password from the presets based on the difficulty
char* choose_password(int difficulty){
	int i; // for getting the index of the password array
	time_t t; // for srand
	srand((unsigned) time(&t));
	if (difficulty == 0){ // easy passwords
		i = rand() % DICT_SIZE; // don't go higher than 17
		return easy_words[i];
	} else if (difficulty == 1){ // med passwords
		i = rand() % DICT_SIZE; // don't go over!
		return med_words[i];
	} else if (difficulty == 2){ // if hahd difficulty
		i = rand() % DICT_SIZE;
		return hard_words[i];
	} else { // if somehow, something goofed up
		printf("Something went wrong. Please kill this process, although it will likely SegFault and get killed automatically now\n");
	}
}

// A function for building a random string, sometimes incorporating a password from the proper difficulty
// At some point, you need to incorporate the correct password
// Randomly choose it, and then set a flag when it's done so it doesn't get done again (pass in a pointer to the int in password())


// A function that runs the password minigame
// Does its own error handling
void password(int difficulty){
	int status = 0; // for storing whether the login was a success or not
	int attempts = 3; // track the number of attempts remaining
	int pass_flag; // for tracking whether or not we've printed the real password out
	int i; // for use iterating
	int init_hex = rand_num(); // get a random number that you'll use to initialize the lefthand columns
	char* password = choose_password(difficulty);
	header();
	disp_step("ENTER PASSWORD NOW\n\n");
	printf("%d", attempts);
	disp_step(" ATTEMPTS LEFT: @ @ @\n\n");
	for (i=0;i<17;i++){ // 17 is the number of lines the password minigame has
		printf("0x%X\t", init_hex + i*10); // Print out the first column's hex
		printf("\t"); // 12 chars long
		printf("0x%X\t", init_hex + (i+17)*10); // Start with the one immediately following the last in the first column
		printf("\n"); // 12 chars long
	}
}

// The main function
int main(int argc, char* argv[]){
	int difficulty = 0;
	if (argc != 2){ // if the user passed in the wrong number of args, yell at them
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
	printf(ANSI_COLOR_GREEN); // change all terminal text to green from here on out
	welcome();
	password(difficulty);
	printf(ANSI_COLOR_RESET); // reset the terminal color to default before exiting, else it carries over after program termination
	return 0;
}

