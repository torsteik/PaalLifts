package main
import (
    . "fmt"
    "runtime"
    "time"
)
var i int = 0

func threadIterateUp(){
	for j:=0; j<1000000; j++ {
		i++
	}
}
func threadIterateDown(){
	for j:=0; j<1000000; j++ {
		i--
	}
}

func main(){
	runtime.GOMAXPROCS(runtime.NumCPU())
	
	go threadIterateUp()
	go threadIterateDown()
	
	time.Sleep(100*time.Millisecond)
	Println("Value of i: ", i)
}
