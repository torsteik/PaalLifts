#ifndef STATEMACHINE_H
#define STATEMACHINE_H

//------STATES-------------------
#define INIT 					0
#define IDLE 					1
#define MOVE					2
#define TEMP_STOP 				3
//------TEMP_STOP_SUBSTATEs------
#define ARRIVAL					0
#define WAIT_FOR_DOOR 			1
#define DETERMINE_NEXT_ACTION	2

/**
Gir funksjonalitet avhengig av nåværende tilstand.
Mulige tilstander:
INIT, IDLE, MOVE, TEMP_STOP, EMERGENCY_STOP.
@param array som inneholder alle tilstandsvariablene.
@param int som samsvarer med nåværende tilstand.
*/
void stateMachine();

/**
Prøver å identifisere heisens posisjon.
@param peker til static variabel med info om forrige besøkte etasje.
@return 1 om etasje identifiseres. 0 om ukjent posisjon.
*/
int findPosition();

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
void findDestination();

/**
Bestemmer om passasjerer skal av/på i nåværende posisjon.
@param array som inneholder alle tilstandsvariablene.
@param peker til static variabel med info om forrige bevegelsesretning.
@param peker til static variabel med info om forrige besøkte etasje.
*/
void evaluateFloor();



#endif