#ifndef NETWORK_FSM_H
#define NETWORK_FSM_H

#define STATUS_PORT 32000
#define 
// Type definitions: (Maybe move this to seperate file set for code quality.)
enum state_t{ FIND_NETWORK, MASTER, SLAVE};

// Functions:
int find_master();
#endif
