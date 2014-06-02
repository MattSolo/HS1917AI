// Dominic He
// Matthew Solomonson
// 30/05/2014
// mechanical turk for KI

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Game.h"
#include "mechanicalTurk.h"

#define NO_RETRAIN 0

static int spinoffRetrains(Game g, int disciplTo) {
    int exchangeRate;
    int player = getWhoseTurn(g);
    int retrainIsOkay = FALSE;
    int retrainFrom = -1;
    int current = STUDENT_BPS;

    while (!retrainIsOkay && current <= STUDENT_MMONEY) {
        if (current != disciplTo) {
            exchangeRate = 
                getExchangeRate(g, player, current, disciplTo);
            if (current == STUDENT_MJ ||
                current == STUDENT_MTV ||
                current == STUDENT_MMONEY) {
                // If retraining from MJ, MTV, M$, make sure that
                // there is at least one left to start spinoff
                exchangeRate++;
            }
            if (getStudents(g, player, current) >= exchangeRate) {
                if (disciplTo == STUDENT_MTV &&
                    getStudents(g, player, STUDENT_MMONEY) == 0) {
                    if (retrainFrom == -1) {
                        retrainFrom = current;
                    } else {
                        retrainIsOkay = TRUE;
                    }
                } else {
                    retrainFrom = current;
                    retrainIsOkay = TRUE;
                }
            }
        }
        current++;
    }
    if (!retrainIsOkay) {
        retrainFrom = NO_RETRAIN;
    }
    return retrainFrom;
}

// Start a spinoff, or retrain
static action startSpinoff(Game g) {
    action spinoffAction;
    int player = getWhoseTurn(g);
    int depletedDiscipl;
    int retrainFrom;

    if (getStudents(g, player, STUDENT_MJ) >= 1 &&
        getStudents(g, player, STUDENT_MTV) >= 1 &&
        getStudents(g, player, STUDENT_MMONEY) >= 1) {
        //printf("Creating spinoff\n");
        spinoffAction.actionCode = START_SPINOFF;
    } else {
        if (getStudents(g, player, STUDENT_MJ) == 0) {
            depletedDiscipl = STUDENT_MJ;
            //printf("MJ depleted\n");
        } else if (getStudents(g, player, STUDENT_MTV) == 0) {
            depletedDiscipl = STUDENT_MTV;
            //printf("MTV depleted\n");
        } else {
            depletedDiscipl = STUDENT_MMONEY;
            //printf("M$ depleted\n");
        }

        retrainFrom = spinoffRetrains(g, depletedDiscipl);
        if (retrainFrom == NO_RETRAIN) {
            spinoffAction.actionCode = PASS;
        } else {
            spinoffAction.actionCode = RETRAIN_STUDENTS;
            spinoffAction.disciplineFrom = retrainFrom;
            spinoffAction.disciplineTo = depletedDiscipl;
        }
    }
    return spinoffAction;
}

action firstTurn(Game g) {
    action nextAction;
    int player = getWhoseTurn(g);
    if (player == UNI_A) {
        if (getARC(g, "R") != UNI_A) {
            nextAction.actionCode = OBTAIN_ARC;
            strcpy(nextAction.destination, "R");
        } else if (getARC(g, "RL") != UNI_A) {
            nextAction.actionCode = OBTAIN_ARC;
            strcpy(nextAction.destination, "RL");
        } else {
            nextAction.actionCode = BUILD_CAMPUS;
            strcpy(nextAction.destination, "RL");
        }
    } else if (player == UNI_B) {
        if (getARC(g, "RRLRLL") != UNI_B) {
            nextAction.actionCode = OBTAIN_ARC;
            strcpy(nextAction.destination, "RRLRLL");
        } else if (getARC(g, "RRLRLLL") != UNI_B) {
            nextAction.actionCode = OBTAIN_ARC;
            strcpy(nextAction.destination, "RRLRLLL");
        } else {
            nextAction.actionCode = BUILD_CAMPUS;
            strcpy(nextAction.destination, "RRLRLLL");
        }
    } else {
        if (getARC(g, "LRLRL") != UNI_C) {
            nextAction.actionCode = OBTAIN_ARC;
            strcpy(nextAction.destination, "LRLRL");
        } else if (getARC(g, "LRLRR") != UNI_C) {
            nextAction.actionCode = OBTAIN_ARC;
            strcpy(nextAction.destination, "LRLRR");
        } else {
            nextAction.actionCode = BUILD_CAMPUS;
            strcpy(nextAction.destination, "LRLRR");
        }
    }
    return nextAction;
}

int distFrom7(Game g, int regionID) {
    int diceValue = 7 - getDiceValue(g, regionID);
    if (diceValue < 0) {
        diceValue *= -1;
    }
    return diceValue;
}

int decideOption(Game g) {
    int option;
    int distances1, distances2;
    int player = getWhoseTurn(g);
    if (player == UNI_A) {
        distances1 = distFrom7(g, 3) + distFrom7(g, 7) + distFrom7(g, 8);
        distances2 = distFrom7(g, 10) + distFrom7(g, 11) + distFrom7(g, 15);
    } else if (player == UNI_B) {
        distances1 = distFrom7(g, 14) + distFrom7(g, 17) + distFrom7(g, 18);
        distances2 = distFrom7(g, 0) + distFrom7(g, 1) + distFrom7(g, 4);
    } else {
        distances1 = distFrom7(g, 12) + distFrom7(g, 13) + distFrom7(g, 16);
        distances2 = distFrom7(g, 2) + distFrom7(g, 5) + distFrom7(g, 6);
    }
    if (distances2 < distances1) {
        option = 2;
    } else {
        option = 1;
    }
    return option;
}

// Build a campus, or spinoff in order to.
action buildCampus(Game g, path dest) {
    int exchangeRate;
    int player = getWhoseTurn(g);
    int retrainIsOkay = FALSE;
    int current;
    int depleted = -1;
    action a;

    if (getStudents(g, player, STUDENT_BPS) == 0) {
        depleted = STUDENT_BPS;
    } else if (getStudents(g, player, STUDENT_BQN) == 0) {
        depleted = STUDENT_BQN;
    } else if (getStudents(g, player, STUDENT_MJ) == 0) {
        depleted = STUDENT_MJ;
    } else if (getStudents(g, player, STUDENT_MTV) == 0) {
        depleted = STUDENT_MTV;
    } else {
        a.actionCode = BUILD_CAMPUS;
        strcpy(a.destination, dest);
    }

    if (depleted != -1) {
        current = STUDENT_BPS;
        while (!retrainIsOkay && current <= STUDENT_MMONEY) {
            if (current != depleted) {
                exchangeRate = 
                    getExchangeRate(g, player, current, depleted);
                if (current == STUDENT_BQN ||
                    current == STUDENT_BPS ||
                    current == STUDENT_MJ ||
                    current == STUDENT_MTV) {
                    // If retraining from B?, BPS, MJ, MTV make sure
                    // there is at least one left to build campus
                    exchangeRate++;
                }
                if (getStudents(g, player, current) >= exchangeRate) {
                    a.actionCode = RETRAIN_STUDENTS;
                    a.disciplineFrom = current;
                    a.disciplineTo = depleted;
                    retrainIsOkay = TRUE;
                }
            }
            current++;
        }
        if (!retrainIsOkay) {
            a.actionCode = PASS;
        }
    }
    return a;
}

action buildFirstCampus(Game g) {
    int option;
    int player = getWhoseTurn(g);
    action a;
    path opt1Arc1[] = {"R", "LRLRLRRLRL", "LRLRL"};
    path opt2Arc1[] = {"RLRLRLRLRLLL", "RRLRLL", "RRLRLLRLRLL"};
    path opt1Arc2[] = {"RL", "LRLRLRRLRR", "LRLRR"};
    path opt2Arc2[] = {"RLRLRLRLRLLLL", "RRLRLLL", "RRLRLLRLRLLL"};
    path opt1Campus[] = {"RL", "LRLRLRRLRR", "LRLRR"};
    path opt2Campus[] = {"RLRLRLRLRLLLL", "RRLRLLL", "RRLRLLRLRLLL"};
    option = decideOption(g);
    printf("option %d\n", option);
    if (getARCs(g, player) == 0) {
        a.actionCode = OBTAIN_ARC;
        if (option == 1) {
            strcpy(a.destination, opt1Arc1[player - 1]);
        } else {
            strcpy(a.destination, opt2Arc1[player - 1]);
        }
    } else if (getARCs(g, player) == 1) {
        a.actionCode = OBTAIN_ARC;
        if (option == 1) {
            strcpy(a.destination, opt1Arc2[player - 1]);
        } else {
            strcpy(a.destination, opt2Arc2[player - 1]);
        }
    } else {
        a.actionCode = BUILD_CAMPUS;
        if (option == 1) {
            //strcpy(a.destination, opt1Campus[player - 1]);
            a = buildCampus(g, opt1Campus[player - 1]);
        } else {
            //strcpy(a.destination, opt2Campus[player - 1]);
            a = buildCampus(g, opt2Campus[player - 1]);
        }
    }
    return a;
}

action buildSecondCampus(Game g) {
    action a;
    int player = getWhoseTurn(g);
    path locations[3] = {"RR", "RRLRLLR", "LRLR"};
    if (getARCs(g, player) < 3) {
        if (getStudents(g, player, STUDENT_BQN) > 0 &&
            getStudents(g, player, STUDENT_BPS) > 0) {
            a.actionCode = OBTAIN_ARC;
            strcpy(a.destination, locations[player - 1]);
        } else {
            a.actionCode = PASS;
        }
    } else if (getStudents(g, player, STUDENT_BQN) > 0 &&
               getStudents(g, player, STUDENT_BPS) > 0 &&
               getStudents(g, player, STUDENT_MJ) > 0 &&
               getStudents(g, player, STUDENT_MTV) > 0) {
        a.actionCode = BUILD_CAMPUS;
        if (player == UNI_C) {
            strcpy(a.destination, "LRL");
        } else {
            strcpy(a.destination, locations[player - 1]);
        }
    } else {
        a.actionCode = PASS;
    }
    if (!isLegalAction(g, a)) {
        a.actionCode = PASS;
    }
    return a;
}
 
action decideAction (Game g) {
    action nextAction;
    nextAction.actionCode = PASS;
    int player = getWhoseTurn(g);

    if (getCampuses(g, player) < 3) {
        nextAction = buildFirstCampus(g);
    }
    else {
        /* if (getCampuses(g, player) < 4) {
            nextAction = buildSecondCampus(g);
        } else {

        } */
        if (nextAction.actionCode == PASS) {

            nextAction = startSpinoff(g);
        }
    }
    return nextAction;
}