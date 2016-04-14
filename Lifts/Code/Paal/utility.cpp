void find_destination_sub_code(int stateArray[], int* previous_direction, int* prevFloor){
	int dir, next_dir;
	if (*previous_direction){
		dir = 1;
		next_dir = 0;
	}
	else{
		dir = -1;
		next_dir = 1;
	}
	for (int floor = *prevFloor; dir*floor >= dir * 0; floor -= dir){
		for (int button = 0; button < 3; ++button){
			if (getQueue(floor, button)){
				if (floor == *prevFloor){
					if (*prevFloor == currentFloor){
						return;
					}
					///////Dette gjeleder ved noen nødstopp situasjoner///////
					else{
						stateArray[2] = 0;
						elev_set_motor_direction((tag_elev_motor_direction)dir);
						*previous_direction = next_dir;
						return;
					}
					//////////////////////////////////////////////////////////
				}
				else{
					stateArray[2] = 0;
					elev_set_motor_direction((tag_elev_motor_direction)dir);
					return;
				}
			}
		}
	}
	stateArray[2] = 0;
	elev_set_motor_direction(DIRN_UP);
	*prevDirection = 0;
	return;
}