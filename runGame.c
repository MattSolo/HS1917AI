// Version 1.4
// Created by Oliver Tan
// 19 May 2011
// Pits your AI against each other
// Must compile with Game.c and ai.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "Game.h"
#include "mechanicalTurk.h"

// Game aspects
#define UNI_CHAR_NAME ('A' - UNI_A)
#define WINNING_KPI 150
#define DICE_AMOUNT 2

// runGame defaults
#define INVALID -1
#define DICE_FACES 6

#define NUM_DISCIPLINES 6

// Action:
#define ACTION_NAMES \
   { "Pass", "Build Campus", "Build GO8", "Obtain ARC", \
     "Start Spinoff", "", "", "Retrain Student" }

#define DISCIPLE_NAMES \
   { "ThD", "BPS", "BQN", "MJobs", "MTV", "M$" }

#define SCREEN_WIDTH 50
#define LINE_BREAK_SEPARATOR '-'

   
#define CYAN STUDENT_BQN
#define PURP STUDENT_MMONEY
#define YELL STUDENT_MJ
#define RED STUDENT_BPS
#define GREE STUDENT_MTV 
#define BLUE STUDENT_THD

#define MAX_PASS 1000

void randomDisciplines(int disciplines[]);
void randomDice(int dice[]);
int rollDice(void);
int checkForWinner(Game g);
void printLineBreak(void);

int main(int argc, char *argv[]) {
   // miscellaneous
   /*int disciplines[NUM_REGIONS];
   int dice[NUM_REGIONS];*/
   Game g;
   
   // store the winner of each game
   int winner;
   
   // store game states within the game
   int keepPlaying;
   int turnFinished;
   int diceRollAmount;
   
   // random
   char *actions[] = ACTION_NAMES;
   int diceRoll;
   
   int passedTurns = 0;
            
   // seed rand!
   srand(time(NULL));
   
   // while the game is wanting to be played, create new game, etc.
   keepPlaying = TRUE;
   while (keepPlaying == TRUE) {
      // create the game
      //randomDisciplines(disciplines);
      //randomDice(dice);
      
      // you can change this to randomiseDisciplines() and randomiseDice() later
      int disciplines[NUM_REGIONS] = {CYAN,PURP,YELL,PURP,YELL,RED ,GREE,GREE, RED ,GREE,CYAN,YELL,CYAN,BLUE,YELL,PURP,GREE,CYAN,RED };
      int dice[NUM_REGIONS] = {9,10,8,12,6,5,3,7,3,11,4,6,4,9,9,2,8,10,5};
      randomDisciplines(disciplines);
      randomDice(dice);
      g = newGame(disciplines, dice);
      
      printf("Game created! Now playing...\n");
      
      // start the game with noone as the winner
      winner = NO_ONE;
      while (winner == NO_ONE) {
         printLineBreak();
         // start new turn by setting turnFinished to false then
         // rolling the dice
         
         diceRollAmount = 0;
         diceRoll = 0;
         while (diceRollAmount < DICE_AMOUNT) {
            diceRoll += rollDice();
            diceRollAmount++;
         }
         
         throwDice(g, /*diceRoll*/getTurnNumber(g) % 11 + 1);
         
         // new turn means new line break!
         printf("[Turn %d] The turn now belongs to University %c!\n", 
            getTurnNumber(g),
            getWhoseTurn(g) + UNI_CHAR_NAME);
         printf("The dice has casted a %d!\n", diceRoll);
         
         printf("\n");
         
         
         // keep going through the player's turn until
         // he/she decided to pass and finish the turn
         turnFinished = FALSE;
         while (turnFinished == FALSE && passedTurns < MAX_PASS) {
            // processes requests and all subrequests for a move and
            // checks if they are legal. only gives a move within the
            // scope of the defined actionCodes that is legal
            int turnPerson = getWhoseTurn(g);
            printf("Stats for %c:\n", turnPerson + UNI_CHAR_NAME);
            printf("KPIs: %d\n", getKPIpoints(g, turnPerson));
            printf("ARCs: %d\n", getARCs(g, turnPerson));
            printf("Campuses: %d\n", getCampuses(g, turnPerson));
            printf("GO8s: %d\n", getGO8s(g, turnPerson));
            printf("Publications: %d\n", 
               getPublications(g, turnPerson));
            printf("Patents: %d\n", 
               getIPs(g, turnPerson));
            int discipleIndex = 0;
            char *discipleNames[] = DISCIPLE_NAMES;
            while (discipleIndex < NUM_DISCIPLINES) {
               printf("Type %s: %d\n", discipleNames[discipleIndex],
                  getStudents(g, turnPerson, discipleIndex));
               discipleIndex++;
            } 
            printf("\n");
            
            action a = decideAction(g);
            
            // if not passing, make the move; otherwise end the turn
            if (a.actionCode == PASS) {
               turnFinished = TRUE;
               printf("You have passed onto the next person.\n");
            } else {
              
               // write what the player did, for a logs sake.
               printf("The action '%s' has being completed.\n", 
                       actions[a.actionCode]);
               if (a.actionCode == BUILD_CAMPUS 
                   || a.actionCode == OBTAIN_ARC 
                   || a.actionCode == BUILD_GO8) {
                  printf(" -> Destination: %s\n", a.destination);
               } else if (a.actionCode == RETRAIN_STUDENTS) {
                  printf(" -> DisciplineTo: %d\n", a.disciplineTo);
                  printf(" -> DisciplineFrom: %d\n", a.disciplineFrom);
               }

               assert(isLegalAction(g, a));                                   
            
               // break this and the code dies. trololol!
               if (a.actionCode == START_SPINOFF) {
                  if (rand() % 3 == 0) {
                     a.actionCode = OBTAIN_PUBLICATION;
                  } else {
                     a.actionCode = OBTAIN_IP_PATENT;               
                  }
               }

               makeAction(g, a);
               
               if (a.actionCode == PASS) {
                  passedTurns++;
               } else {
                  passedTurns = 0;
               }
               
               if (passedTurns >= MAX_PASS) {
                  turnFinished = TRUE;
                  
               }
            }
            
            // if there is not a winner or pass, add a seperating line
            // to seperate actions being clumped together
            if (turnFinished == FALSE) {
               printf("\n");
            }
         }
         
         // check if there is a winner
         winner = checkForWinner(g);
      }
      
      if (passedTurns >= MAX_PASS) {
         printf("AI passes too much.\n");
         return EXIT_FAILURE;
      }
      
      printLineBreak();
      printf("GAME OVER!\n");
      printf("Vice Chanceller %c Won in %d Turns!!\n", 
             winner + UNI_CHAR_NAME,
             getTurnNumber(g));
             
      printf("\n");
      int counter = UNI_A;
      while (counter < NUM_UNIS + UNI_A) {
         printf("Uni %c scored %d KPIs\n", counter + UNI_CHAR_NAME,
                getKPIpoints(g, counter));
         counter++;
      }
      printLineBreak();
      
      disposeGame(g);
      
      // ask to play again
      printf("Ctrl+C will exit the game.\nOtherwise, the game will "
             "recommence by hitting enter.");
      int a = scanf("%*c");
      a++;
   }     
   
   return EXIT_SUCCESS;
}

// ----- game creation -----

// Allocates a set of random disciplines inside disciplines[]
void randomDisciplines(int disciplines[]) {
   int disciplineIndex;
   
   disciplineIndex = 0;
   while (disciplineIndex < NUM_REGIONS) {
      // allocate each discipline with a random one
      disciplines[disciplineIndex] = rand() % NUM_DISCIPLINES;
      disciplineIndex++;
   }
}

// Allocates a set of random dice inside disciplines[]
void randomDice(int dice[]) {
   int diceIndex;
   int diceRolled;
   int totalRoll;
   
   diceIndex = 0;
   while (diceIndex < NUM_REGIONS) {
      totalRoll = 0;

      // roll a dice DICE_AMOUNT and add the total
      diceRolled = 0;
      while (diceRolled < DICE_AMOUNT) {
         totalRoll += rollDice();
         diceRolled++;
      }
      
      // allocate the totalRoll
      dice[diceIndex] = totalRoll;
      diceIndex++;
   }
}

// return a number between 1...DICE_FACES 
int rollDice(void) {
   // modding returns between 0...(DICE_FACES-1), so add 1
   return (rand() % DICE_FACES) + 1;
}

// ----- game actions -----

// check all players' KPI and returns the winner (if any)
int checkForWinner(Game g) {
   int winner = NO_ONE;
   int playerIndex;
   
   playerIndex = UNI_A;
   while (playerIndex < NUM_UNIS + UNI_A) {
      // check if the player is over or equal the WINNING_KPI
      if (getKPIpoints(g, playerIndex) >= WINNING_KPI) {
         winner = playerIndex;
      }
      playerIndex++;
   }
   
   return winner;
}

// prints a new line, line break, then new line again
void printLineBreak(void) {
   int counter;
   
   printf("\n");
   
   // print the line break (SCREEN_WIDTH amount of 
   // LINE_BREAK_SEPARATOR)
   counter = 0;
   while (counter < SCREEN_WIDTH) {
      printf("%c", LINE_BREAK_SEPARATOR);
      counter++;
   }
   
   printf("\n\n");
}
