// Dominic He
// Matthew Solomonson
// 30/05/2014
// mechanical turk for KI

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Game.h"
#include "mechanicalTurk.h"

#define NO_RETRAIN 0

static int spinoffRetrains(Game g, int disciplTo) {
    int exchangeRate;
    int player = getWhoseTurn(g);
    int retrainIsOkay = FALSE;
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
                retrainIsOkay = TRUE;
            } else {
                current++;
            }
        } else {
            current++;
        }
    }
    if (!retrainIsOkay) {
        current = NO_RETRAIN;
    }
    return current;
}
 
action decideAction (Game g) {
    action nextAction;
    int retrainFrom;
    int depletedDiscipl;
    int player = getWhoseTurn(g);

    if (getStudents(g, player, STUDENT_MJ) >= 1 &&
        getStudents(g, player, STUDENT_MTV) >= 1 &&
        getStudents(g, player, STUDENT_MMONEY) >= 1) {
        printf("Creating spinoff\n");
        nextAction.actionCode = START_SPINOFF;
    } else {
        if (getStudents(g, player, STUDENT_MJ) == 0) {
            depletedDiscipl = STUDENT_MJ;
            printf("MJ depleted\n");
        } else if (getStudents(g, player, STUDENT_MTV) == 0) {
            depletedDiscipl = STUDENT_MTV;
            printf("MTV depleted\n");
        } else {
            depletedDiscipl = STUDENT_MMONEY;
            printf("M$ depleted\n");
        }

        retrainFrom = spinoffRetrains(g, depletedDiscipl);
        if (retrainFrom == NO_RETRAIN) {
            nextAction.actionCode = PASS;
        } else {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = retrainFrom;
            nextAction.disciplineTo = depletedDiscipl;
        }
    }
    return nextAction;
}