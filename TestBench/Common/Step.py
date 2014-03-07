






class Step:
	"""
	"""

	description 	= '(None)'
	text 			= '2+4'


	def __init__(self):
		"""
		"""
		pass


	def Evaluate(self):
		"""
		"""
		self.ast 	= ast.parse(self.text, mode='eval')


	def SyntaxCheck(self):
		"""
		"""
		return True


	def Description(self):
		"""
		"""
		return description



