#ifndef QUEUE_H
#define QUEUE_H

/**
Henter element fra kø-array.
@param gjeldende etasje.
@param gjeldende knapp.
@return 1 om det er en ordre i valgt kø-element. 0 om elementet er tomt.
*/
int getQueue(int floor, int button);

/**
Setter elementer i køen til ønsket verdi.
@param gjeldende etasje.
@param gjeldende knapp.
@param ønsket verdi.
*/
void setQueue(int floor, int button, int value);

#endif