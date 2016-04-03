//FSM_HEIS
//main.c
global ready = 0;
global network_ready = 0

//init{
int main(){
	start(local);
	while(!ready){}

	ready = 0;
	start(network);
	while(!ready){}

	/* 	... 
		...
	*/
	select()
}
