


import TestBench
import time



def Setup():
	"""
	Setup Code.
	"""
	print('Setup')
	2 + 2

	a = 4


	b = 3

	if b == 3:
		print('HelloWorld.')



def Body():
	"""
	Body code.
	"""
	a=1							# Comment 1.
	print(a)					# Comment 2.
	time.sleep(1.0)				# Comment 3.
	b=a*a						# Comment 4.
	print(b)					# Comment 5.
	time.sleep(1.0)				# Comment 6.
	print('a')					# Comment 7.
	time.sleep(1.0)				# Comment 8.
	print('bla')				# Comment 9.
	time.sleep(1.0)				# Comment 10.


def Cleanup():
	"""
	Cleanup code.
	"""
	print('Cleanup')




if __name__ == '__main__':
	"""
	"""
	Setup()
	Body()
	Cleanup()

