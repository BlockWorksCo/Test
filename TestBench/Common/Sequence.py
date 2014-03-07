


import ast


class Sequence:
	"""
	"""



	def __init__(self, fileName):
		"""
		"""
		self.text 	= open(fileName).read()
		self.tree 	= ast.parse(self.text)


	def AddStepAtPosition(self, stepText, position):
		"""
		"""
		stepTree	= ast.parse(stepText)



