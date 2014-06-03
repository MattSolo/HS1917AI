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

#define MAP_OPTION1 1
#define MAP_OPTION2 2
#define OPTION1_ARC1 {"R", "LRLRLRRLRL", "LRLRL"}
#define OPTION1_ARC2 {"RL", "LRLRLRRLRR", "LRLRR"}
#define OPTION1_CAMPUS {"RL", "LRLRLRRLRR", "LRLRR"}
#define OPTION1_2ND_CAMPUS_ARC {"RR", "LRL", "LRLRLRRL"}
#define OPTION1_2ND_CAMPUS {"RR", "LRLRLRRLR", "LRLR"}
#define OPTION2_ARC1 {"RLRLRLRLRLLL", "RRLRLL", "RRLRLLRLRLL"}
#define OPTION2_ARC2 {"RLRLRLRLRLLLL", "RRLRLLL", "RRLRLLRLRLLL"}
#define OPTION2_CAMPUS {"RLRLRLRLRLLLL", "RRLRLLL", "RRLRLLRLRLLL"}

static action startSpinoff(Game g);
static int spinoffRetrains(Game g, int disciplTo);
static int distFrom7(Game g, int regionID);
static int decideOption(Game g);
static action buildCampus(Game g, path dest);
static action buildFirstCampus(Game g);
static action buildSecondCampus(Game g);
static action buildGO8(Game g);
 
action decideAction(Game g) {
    action nextAction;
    nextAction.actionCode = PASS;
    int player = getWhoseTurn(g);

    // Build first campus on first turn
    if (getCampuses(g, player) < 3 && getGO8s(g, player) == 0) {
        nextAction = buildFirstCampus(g);
    }
    // Upgrade the campus we made above into a GO8
    // Stop trying if it hasn't been made in 85 turns
    else if (getGO8s(g, player) < 1 && getTurnNumber(g) < 85) {
        nextAction = buildGO8(g);
    } else {
        if (decideOption(g) == MAP_OPTION1) {
            nextAction = buildSecondCampus(g);
        }
        // If cant build second campus
        // default to starting spinoffs
        if (nextAction.actionCode == PASS) {
            nextAction = startSpinoff(g);
        }
    }
    return nextAction;
}

// Retrain students to start a spinoff
static int spinoffRetrains(Game g, int disciplTo) {
    int exchangeRate;
    int player = getWhoseTurn(g);
    int retrainIsOkay = FALSE;
    int retrainFrom = NO_RETRAIN;
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
                // If both MTV and M$ are depleted, only retrain MTV if
                // there is enough left to also retrain M$
                if (disciplTo == STUDENT_MTV &&
                    getStudents(g, player, STUDENT_MMONEY) == 0) {
                    if (retrainFrom == NO_RETRAIN) {
                        retrainFrom = current;
                        if ((getStudents(g, player, current)
                            - exchangeRate) / 3 >= 1) {
                            retrainIsOkay = TRUE;
                        }
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
        spinoffAction.actionCode = START_SPINOFF;
    } else {
        if (getStudents(g, player, STUDENT_MJ) == 0) {
            depletedDiscipl = STUDENT_MJ;
        } else if (getStudents(g, player, STUDENT_MTV) == 0) {
            depletedDiscipl = STUDENT_MTV;
        } else {
            depletedDiscipl = STUDENT_MMONEY;
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

// how close a number is to 7 (and higher probability)
static int distFrom7(Game g, int regionID) {
    int diceValue = 7 - getDiceValue(g, regionID);
    if (diceValue < 0) {
        diceValue *= -1;
    }
    return diceValue;
}

// decide from 2 places to build a campus on first turn
// based on higher probablity of the regions it would be in.
static int decideOption(Game g) {
    int option;
    // sum of the regions' dice value's distance from 7
    // the smaller, the better
    int distances1, distances2;
    int player = getWhoseTurn(g);
    if (player == UNI_A) {
        distances1 = distFrom7(g, 3) +
                    distFrom7(g, 7) + 
                    distFrom7(g, 8);
        distances2 = distFrom7(g, 10) + 
                    distFrom7(g, 11) + 
                    distFrom7(g, 15);
    } else if (player == UNI_B) {
        distances1 = distFrom7(g, 14) +
                    distFrom7(g, 17) +
                    distFrom7(g, 18);
        distances2 = distFrom7(g, 0) +
                    distFrom7(g, 1) + 
                    distFrom7(g, 4);
    } else {
        distances1 = distFrom7(g, 12) +
                    distFrom7(g, 13) +
                    distFrom7(g, 16);
        distances2 = distFrom7(g, 2) +
                    distFrom7(g, 5) +
                    distFrom7(g, 6);
    }
    if (distances2 < distances1) {
        option = MAP_OPTION2;
    } else {
        option = MAP_OPTION1;
    }
    return option;
}

// Build a campus, or spinoff in order to.
static action buildCampus(Game g, path dest) {
    printf("BUILD CAMPUS\n");
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

// Build first campus at start of game
static action buildFirstCampus(Game g) {
    int option;
    int player = getWhoseTurn(g);
    action a;
    path opt1Arc1[] = OPTION1_ARC1;
    path opt2Arc1[] = OPTION2_ARC1;
    path opt1Arc2[] = OPTION1_ARC2;
    path opt2Arc2[] = OPTION2_ARC2;
    path opt1Campus[] = OPTION1_CAMPUS;
    path opt2Campus[] = OPTION2_CAMPUS;
    // Two options for building a campus
    // on two sides on the map
    option = decideOption(g);
    printf("option %d\n", option);
    if (getARCs(g, player) == 0) {
        // build first ARC
        a.actionCode = OBTAIN_ARC;
        if (option == MAP_OPTION1) {
            strcpy(a.destination, opt1Arc1[player - 1]);
        } else {
            strcpy(a.destination, opt2Arc1[player - 1]);
        }
    } else if (getARCs(g, player) == 1) {
        // build second ARC
        a.actionCode = OBTAIN_ARC;
        if (option == MAP_OPTION1) {
            strcpy(a.destination, opt1Arc2[player - 1]);
        } else {
            strcpy(a.destination, opt2Arc2[player - 1]);
        }
    } else {
        // build first campus, or retrain required students
        a.actionCode = BUILD_CAMPUS;
        if (option == MAP_OPTION1) {
            a = buildCampus(g, opt1Campus[player - 1]);
        } else {
            a = buildCampus(g, opt2Campus[player - 1]);
        }
    }
    return a;
}

// for option 1 only: build a second campus if possible
static action buildSecondCampus(Game g) {
    action a;
    int player = getWhoseTurn(g);
    path campusLocation[3] = OPTION1_2ND_CAMPUS;
    path arcLocation[3] = OPTION1_2ND_CAMPUS_ARC;

    // make arc required for 2nd campus if it doesn't already exist
    if (getARC(g, arcLocation[player - 1]) == VACANT_ARC) {
        if (getStudents(g, player, STUDENT_BQN) > 0 &&
            getStudents(g, player, STUDENT_BPS) > 0) {
            a.actionCode = OBTAIN_ARC;
            strcpy(a.destination, arcLocation[player - 1]);
        } else {
            a.actionCode = PASS;
        }
    // build second campus if enough people
    } else if (getStudents(g, player, STUDENT_BQN) > 0 &&
               getStudents(g, player, STUDENT_BPS) > 0 &&
               getStudents(g, player, STUDENT_MJ) > 0 &&
               getStudents(g, player, STUDENT_MTV) > 0) {
        a.actionCode = BUILD_CAMPUS;
        strcpy(a.destination, campusLocation[player - 1]);
    } else {
        a.actionCode = PASS;
    }
    // check that arc or campus is possible
    if (!isLegalAction(g, a)) {
        a.actionCode = PASS;
    }
    return a;
}

// Retrain students in order to build a GO8
// Only retrains to M$ if retraining results in 3+ M$
static int gO8Retrains(Game g, int depleted) {
    int player = getWhoseTurn(g);
    int retrainFrom = NO_RETRAIN;
    int retrainIsOkay = FALSE;
    int potentialMMon = getStudents(g, player, STUDENT_MMONEY);
    int rate;
    int currentCount;

    int current = STUDENT_BPS;
    while (!retrainIsOkay && current <= STUDENT_MMONEY) {
        if (current != depleted) {
            rate = 
                getExchangeRate(g, player, current, depleted);
            // If retraining from MJ or M$, make sure there are
            // enough left to build GO8
            if (current == STUDENT_MJ) {
                rate += 2;
            } else if (current == STUDENT_MMONEY) {
                rate += 3;
            }
            currentCount = getStudents(g, player, current);
            if (currentCount >= rate) {
                // If retraining to M$, make sure that retraining will
                // be enough to create GO8, to avoid waste to ThD's
                if (depleted == STUDENT_MMONEY) {
                    if (retrainFrom == NO_RETRAIN) {
                        retrainFrom = current;
                    }
                    // Add how many times current discipline
                    // can be retrained
                    potentialMMon += currentCount / rate;
                    if (potentialMMon >= 3) {
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
    } else printf("RETRAIN FROM %d\n", retrainFrom);
    return retrainFrom;
}

static action buildGO8(Game g) {
    printf("buildGO8\n");
    action a;
    path opt1Campus[] = OPTION1_CAMPUS;
    path opt2Campus[] = OPTION2_CAMPUS;
    path campus;
    int player = getWhoseTurn(g);
    int retrainFrom;
    if (decideOption(g) == MAP_OPTION1) {
        strcpy(campus, opt1Campus[player - 1]);
    } else {
        strcpy(campus, opt2Campus[player - 1]);
    }
    // If not enough MJ students,
    // find somewhere to retrain from or PASS
    if (getStudents(g, player, STUDENT_MJ) < 2) {
        retrainFrom = gO8Retrains(g, STUDENT_MJ);
        if (retrainFrom == NO_RETRAIN) {
            a.actionCode = PASS;
        } else {
            a.actionCode = RETRAIN_STUDENTS;
            a.disciplineFrom = retrainFrom;
            a.disciplineTo = STUDENT_MJ;
        }
    // If not enough M$ students,
    // find somewhere to retrain from or PASS
    } else if (getStudents(g, player, STUDENT_MMONEY) < 3) {
        retrainFrom = gO8Retrains(g, STUDENT_MMONEY);
        if (retrainFrom == NO_RETRAIN) {
            a.actionCode = PASS;
        } else {
            a.actionCode = RETRAIN_STUDENTS;
            a.disciplineFrom = retrainFrom;
            a.disciplineTo = STUDENT_MMONEY;
        }
    // Enough students
    } else {
        a.actionCode = BUILD_GO8;
        strcpy(a.destination, campus);
    }
    return a;
}