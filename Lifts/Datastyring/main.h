#ifndef MAIN_H
#define MAIN_H

/**
  Navnelapper for de ulike tilstandene i stateMacine().
*/
typedef enum tag_state{
INIT = 0,
IDLE = 1,
MOVE = 2,
TEMP_STOP = 3,
EMERGENCY_STOP = 4,
DEFAULT = -1
} state_t;

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

#endif
