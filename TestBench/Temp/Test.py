






def FnA():
	print('A')

def FnB():
	print('A')
	FnA()

def FnC():
	print('A')
	FnB()

def FnD():
	print('A')
	FnC()






FnD()