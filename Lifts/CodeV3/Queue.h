#ifndef QUEUE_H
#define QUEUE_H

#include "defines.h"

class Queue{	// Names
public:
	Queue();

	std::atomic<int>  int_q[N_FLOORS];					// Format: [BUTTON_COMMAND...]
	std::atomic<char> ext_q[N_FLOORS*2]; 				// Format: [BUTTON_UP..., BUTTON_DOWN...]Make atomic and int?

	std::atomic<char> orders_new[1 + N_FLOORS * 2];		// Format: [MSG_ID, BUTTON_UP..., BUTTON_DOWN...]
	std::atomic<char> orders_complete[1 + N_FLOORS * 2]; // ---------------------||-----------------------

	int  get_q(int floor, int button_type);
	void add_new_order(int floor, int button);
	void add_order_from_netw(int button);
	void recycle_order(int button);
	void register_completed_order(int floor);
	void sync_q_with_netw(char netw_master_q[N_FLOORS * 2]);
	int  is_queue_empty();
};

#endif
