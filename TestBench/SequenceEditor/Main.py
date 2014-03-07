


import Debugger
import sys
import time



class SequenceEditor:


	def __init__(self, firstSequence):
		"""
		"""
		print('TestBenchApp.\n')
		self.debugger 	= Debugger.Debugger(firstSequence)


	def CreateStep(self):
		"""
		"""
		pass


	def CreateStep(self):
		"""
		"""
		pass


	def Run(self):
		"""
		"""

		for i in range(0,100):
			time.sleep(1.0)
			self.debugger.StepInto()
			print('\n\n')
			callStack 	= self.debugger.CallStack()
			for position in callStack:
				print('%s : %d'%(position['File'],position['LineNumber']))
			print('\n\n')

		self.debugger.WaitForEnd()



if __name__=="__main__":
	"""
	"""
	app	= SequenceEditor('../ExampleSequences/First.py')
	app.Run()

