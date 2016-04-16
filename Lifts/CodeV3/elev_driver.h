#ifndef MAIN_H
#define MAIN_H

#define N_FLOORS 4;
/**
  Navnelapper for de ulike tilstandene i stateMacine().
*/


/**
  Bestemmer tilstand ut i fra innhold i stateArray[].
  @param array som inneholder alle tilstandsvariablene.
  @return int som samsvarer med tilstand i tilstandsmaskinen.
*/
state_t determineState(int stateArray[]);

/**
  Behandler alle knappetrykk; oppdaterer kø, gjør klart for nødstopp- 
  @param array som inneholder alle tilstandsvariablene.
*/
void handleButtonPress(int stateArray[], int prevPresses[][3]);



#define INITIALIZE 		0
#define EMERGENCY_STOP 	1
#define STOP_ELEVATOR 	2
#define IS_QUEUE_EMPTY 	3
#endif
