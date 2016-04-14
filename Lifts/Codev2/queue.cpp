/**
queue er satt opp slik:
queue[4][3] = {
{ BUTTON_UP1, BUTTON_DOWN1, BUTTON_COMMAND1 },
{ BUTTON_UP2, BUTTON_DOWN2, BUTTON_COMMAND2 },
{ BUTTON_UP3, BUTTON_DOWN3, BUTTON_COMMAND3 },
{ BUTTON_UP4, BUTTON_DOWN4, BUTTON_COMMAND4 },
}
*/
static int queue[4][3] = {
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 }
};

int getQueue(int floor, int button){
	return queue[floor][button];
}

void setQueue(int floor, int button, int value){
	queue[floor][button] = value;
}