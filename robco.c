/* First attempt at simulating the RobCo Terminals from Fallout */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Predefine potential words here. Do this on a difficulty-based scale,
// So we could have three separate arrays, with different words



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
	disp_step(ANSI_COLOR_GREEN "Welcome to ROBCO Industries (TM) TermLink\n");
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

// A function that runs the password minigame
// Does its own error handling
void password(){
	int status = 0; // for storing whether the login was a success or not
	int attempts = 3; // track the number of attempts remaining
	int i; // for use iterating
	int init_hex = rand_num(); // get a random number that you'll use to initialize the lefthand columns
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
int main(){
	welcome();
	password();
	printf(ANSI_COLOR_RESET);
	return 0;
}

