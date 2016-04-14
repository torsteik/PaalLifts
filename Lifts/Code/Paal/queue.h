#ifndef QUEUE_H
#define QUEUE_H

/**
Henter element fra k�-array.
@param gjeldende etasje.
@param gjeldende knapp.
@return 1 om det er en ordre i valgt k�-element. 0 om elementet er tomt.
*/
int getQueue(int floor, int button);

/**
Setter elementer i k�en til �nsket verdi.
@param gjeldende etasje.
@param gjeldende knapp.
@param �nsket verdi.
*/
void setQueue(int floor, int button, int value);

#endif