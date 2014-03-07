


import Sequence
import subprocess
import threading
import time
import os
import fcntl
import sys
import re





class Debugger(threading.Thread):
	"""
	Main class to control the python debugger (Pdb). Provides methods to control stepping
	and to get status information from the target code.
	"""



	def __init__(self, firstSequence):
		"""
		Create a Debugger instance starting with the given sequence. This spawns off a thread to handle
		the asynchronous pipe I/O to/from the target Pdb process.
		"""
		threading.Thread.__init__(self)
		self.quitFlag 	= False

		self.sequence 		= Sequence.Sequence(firstSequence)

		(self.fromTargetReadFD, self.fromTargetWriteFD)	= os.pipe()
		(self.toTargetReadFD, self.toTargetWriteFD)		= os.pipe()

		self.toTargetRead 		= os.fdopen(self.toTargetReadFD, 'r')
		self.toTargetWrite 		= os.fdopen(self.toTargetWriteFD, 'w')
		self.fromTargetRead 	= os.fdopen(self.fromTargetReadFD, 'r')
		self.fromTargetWrite 	= os.fdopen(self.fromTargetWriteFD, 'w')

		self.targetProcess 		= subprocess.Popen(	args=['python', '-u', '-m','pdb', firstSequence],
			                    					stdin=self.toTargetRead,
			                    					stdout=self.fromTargetWrite,
			                    					stderr=self.fromTargetWrite)	

		self.response 		= None
		self.showStdout 	= False
		self.start()




	def SignalStop(self):
		"""
		Stops the asynchronous pipe I/O thread.
		"""
		self.quitFlag 	= True


	def WaitForEnd(self):
		"""
		Waits for the target Pdb process to quit. It then joins the Asynchronous pipe I/O thread to clean up
		after itself.
		"""
		self.targetProcess.wait()

		self.SignalStop()
		self.join()


	def TargetCommand(self, command, showStdout=True):
		"""
		Sends a command to the debugger. See the Pdb documentation for details.
		"""
		self.response 		= None
		self.showStdout 	= showStdout
		self.toTargetWrite.write(command+'\n')
		self.toTargetWrite.flush()


	def WaitForResponse(self):
		"""
		Blocks the calling process while Pdb comes back with a response terminating with '(Pdb) '.
		"""
		while self.response == None:
			time.sleep(0.1)

		return self.response


	def StepInto(self):
		"""
		"""
		self.TargetCommand('step')
		temp 	= self.WaitForResponse()

	def StepOver(self):
		"""
		"""
		self.TargetCommand('next')
		temp 	= self.WaitForResponse()

	def Go(self):
		"""
		"""
		self.TargetCommand('cont')
		#temp 	= self.WaitForResponse()

	def Stop(self):
		"""
		"""
		self.TargetCommand('quit')
		#temp 	= self.WaitForResponse()


	def CallStack(self):
		"""
		"""
		self.TargetCommand('where', showStdout=False)
		callStackText 	= self.WaitForResponse()

		matches 	= re.compile(' (.*\.py)\(([0-9]+)\).*').findall(callStackText)

		return [{'File':fileName,'LineNumber':int(lineNumber)} for fileName,lineNumber in matches]


	def Locals(self):
		"""
		"""
		self.TargetCommand('!locals()')
		localsText 	= self.WaitForResponse()

	def Globals(self):
		"""
		"""
		self.TargetCommand('!globals()')
		globalsText 	= self.WaitForResponse()

	def StationGlobals(self):
		"""
		"""
		return {}


	def Ready(self, text):
		"""
		"""
		print('\n[READY]\n')

	def Parse(self, text):
		"""
		"""
		if text[-6:] == '(Pdb) ':
			self.response 	= text
			self.Ready(self.text)
			self.text 	= ''


	def run(self):
		"""
		"""

		fcntl.fcntl(self.fromTargetRead, fcntl.F_SETFL, os.O_NONBLOCK)

		self.text 	= ''

		while True:
			try:
				newText 	= self.fromTargetRead.read()
				self.text 	= self.text + newText	
				self.Parse(self.text)

				if self.showStdout == True:
					sys.stdout.write(newText)
					sys.stdout.flush()

			except(IOError):
				pass

			if self.quitFlag == True:
				break

			time.sleep(0.1)



