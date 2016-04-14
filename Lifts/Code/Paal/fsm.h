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
Gir funksjonalitet avhengig av n�v�rende tilstand.
Mulige tilstander:
INIT, IDLE, MOVE, TEMP_STOP, EMERGENCY_STOP.
@param array som inneholder alle tilstandsvariablene.
@param int som samsvarer med n�v�rende tilstand.
*/
void stateMachine(int stateArray[], state_t state);

/**
Pr�ver � identifisere heisens posisjon.
@param peker til static variabel med info om forrige bes�kte etasje.
@return 1 om etasje identifiseres. 0 om ukjent posisjon.
*/
int findPosition(int* prevFloor);

/**
Sjekker om k�en er tom.
@return 1 hvis k�en er tom. 0 om det er elementer i k�en.
*/
int isQueueEmpty();

/**
Bestemmer retning heisen skal kj�re i.
@param array som inneholder alle tilstandsvariablene.
@param peker til static variabel med info om forrige bevegelsesretning.
@param peker til static variabel med info om forrige bes�kte etasje.
*/
void findDestination(int stateArray[], int* prevDirection, int* prevFloor);

/**
Bestemmer om passasjerer skal av/p� i n�v�rende posisjon.
@param array som inneholder alle tilstandsvariablene.
@param peker til static variabel med info om forrige bevegelsesretning.
@param peker til static variabel med info om forrige bes�kte etasje.
*/
void evaluateFloor(int stateArray[], int* prevDirection, int* prevFloor);

/**
Setter elementer tilh�rende n�v�rende etasje i k�en lik 0.
@param n�v�rende posisjon.
*/
void updateQueue(int floor);

#endif