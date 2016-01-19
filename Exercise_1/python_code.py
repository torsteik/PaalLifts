from threading import Thread

i = 0
def threadIterateUp():
	global i
	for j in xrange(0,1000000):
		i+=1

def threadIterateDown():
	global i
	for j in xrange(0,1000000):
		i-=1

def main():
	
	thread1 = Thread(target = threadIterateUp, args = (),)
	thread2 = Thread(target = threadIterateDown, args = (),)
	
	thread1.start()
	thread2.start()
	
	thread1.join()
	thread2.join()
	print(i)

main()
