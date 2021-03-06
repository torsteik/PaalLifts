from threading import Thread, RLock

class shared_int:
	value = 0
	lock = RLock()

i = shared_int()

def threadIterateUp():
	global i
	for j in xrange(0,1000000):
		i.lock.acquire()
		i.value+=1
		i.lock.release()

def threadIterateDown():
	global i
	for j in xrange(0,1000000):
		i.lock.acquire()
		i.value-=1
		i.lock.release()

def main():
	
	thread1 = Thread(target = threadIterateUp, args = (),)
	thread2 = Thread(target = threadIterateDown, args = (),)
	
	thread1.start()
	thread2.start()
	
	thread1.join()
	thread2.join()
	print(i.value)

main()
