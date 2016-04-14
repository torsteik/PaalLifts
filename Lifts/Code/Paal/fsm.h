#ifndef STATEMACHINE_H
#define STATEMACHINE_H

/**
Navnelapper for substates i TEMP_STOP tilstanden i stateMachine().
*/
typedef enum tag_TEMP_STOP_substate{
	ARRIVAL = 0,
	WAIT_FOR_DOOR = 1,
	DETERMINE_NEXT_ACTION = 2
} TEMP_STOP_substate_t;

/**
Gir funksjonalitet avhengig av nåværende tilstand.
Mulige tilstander:
INIT, IDLE, MOVE, TEMP_STOP, EMERGENCY_STOP.
@param array som inneholder alle tilstandsvariablene.
@param int som samsvarer med nåværende tilstand.
*/
void stateMachine(int stateArray[], state_t state);

/**
Prøver å identifisere heisens posisjon.
@param peker til static variabel med info om forrige besøkte etasje.
@return 1 om etasje identifiseres. 0 om ukjent posisjon.
*/
int findPosition(int* prevFloor);

/**
Sjekker om køen er tom.
@return 1 hvis køen er tom. 0 om det er elementer i køen.
*/
int isQueueEmpty();

/**
Bestemmer retning heisen skal kjøre i.
@param array som inneholder alle tilstandsvariablene.
@param peker til static variabel med info om forrige bevegelsesretning.
@param peker til static variabel med info om forrige besøkte etasje.
*/
void findDestination(int stateArray[], int* prevDirection, int* prevFloor);

/**
Bestemmer om passasjerer skal av/på i nåværende posisjon.
@param array som inneholder alle tilstandsvariablene.
@param peker til static variabel med info om forrige bevegelsesretning.
@param peker til static variabel med info om forrige besøkte etasje.
*/
void evaluateFloor(int stateArray[], int* prevDirection, int* prevFloor);

/**
Setter elementer tilhørende nåværende etasje i køen lik 0.
@param nåværende posisjon.
*/
void updateQueue(int floor);

#endif