package main
import (
    . "fmt"
    "runtime"
)

var i int = 0

var mutex_channel chan int
var itUp_channel chan int
var itDown_channel chan int



func threadIterateUp(mutex_channel chan int, itUp_channel chan int){
	for j:=0; j<1000000; j++ {
		mutex_channel <- 1
		i++
		<- mutex_channel
	}
	itUp_channel <- 1
}
func threadIterateDown(mutex_channel chan int, itDown_channel chan int){
	for j:=0; j<1000000; j++ {
		mutex_channel <- 1
		i--
		<- mutex_channel
	}
	itDown_channel <- 1
}

func main(){
	runtime.GOMAXPROCS(runtime.NumCPU())
	
	mutex_channel := make(chan int, 1)
	itDown_channel:= make(chan int, 1)
	itUp_channel:= make(chan int, 1)
		
	
	go threadIterateUp(mutex_channel,itUp_channel)
	go threadIterateDown(mutex_channel, itDown_channel)
	
	for j:= 0; j<2; j++ {
		select{
			case <-itUp_channel:
			
			case <-itDown_channel:
		}
	}
	Println("Value of i: ", i)
}
