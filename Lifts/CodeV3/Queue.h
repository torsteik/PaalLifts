#ifndef QUEUE_H
#define QUEUE_H
/**
  queue er satt opp slik:
  queue[4][3] = {
	{ BUTTON_UP1, BUTTON_DOWN1, BUTTON_COMMAND1 },
	{ BUTTON_UP2, BUTTON_DOWN2, BUTTON_COMMAND2 },
	{ BUTTON_UP3, BUTTON_DOWN3, BUTTON_COMMAND3 },
	{ BUTTON_UP4, BUTTON_DOWN4, BUTTON_COMMAND4 },
  }
*/

class Queue{	// Names
public:
	Queue();

	int int_q[N_FLOORS];
	char ext_q[N_FLOORS*2]; // Make atomic

	char orders_new[1 + N_FLOORS * 2];
	char orders_complete[1 + N_FLOORS * 2]; //OBS CHECK SIZE

	int  getQ(int floor, int button_type);
	void add_new_order(int floor, int button);
	void add_order_from_netw(int button);
	void recycle_order(int button);
	void register_completed_order(int floor);
	void sync_q_with_netw(char netw_master_q);
	int  is_queue_empty();
};
/**
  Henter element fra kø-array.
  @param gjeldende etasje.
  @param gjeldende knapp.
  @return 1 om det er en ordre i valgt kø-element. 0 om elementet er tomt.

int getQueue(int floor, int button);
*/
/**
  Setter elementer i køen til ønsket verdi.
  @param gjeldende etasje.
  @param gjeldende knapp.
  @param ønsket verdi.

void setQueue(int floor, int button, int value);
*/
/**
Setter elementer tilhørende nåværende etasje i køen lik 0.
@param nåværende posisjon.

void updateQueue(int floor);
*/

#endif
