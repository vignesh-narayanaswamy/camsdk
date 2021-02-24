import sys
import os
import platform


from PyQt5.QtWidgets import QWidget, QLabel, QHBoxLayout, QVBoxLayout, QGridLayout, QPushButton, QSizePolicy, QTableWidget, QTableWidgetItem
from PyQt5.QtCore import pyqtSlot, pyqtSignal

if platform.system()=='Windows':
    sys.path.append(os.environ['IPX_CAMSDK_ROOT']+'/bin/win64_x64/')
    sys.path.append(os.environ['IPX_CAMSDK_ROOT']+'/bin/win32_i86/')

if platform.system()=='Windows':
    import IpxCameraGuiApiPy as IpxCameraGuiApiPy
else :
    import libIpxCameraGuiApiPy as IpxCameraGuiApiPy

from threading import Thread

class main_Dialog(QWidget):
	parView = None # GenICam Parameter Tree View
	
	def __init__(self):
		super(main_Dialog ,self).__init__()
		self.init_ui()

	def closeEvent(self, event):
		self.camDisconnect()		
		super(main_Dialog, self).closeEvent(event)

	def initInfoTable(self):
		self.infoTable.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
		self.infoTable.setRowCount(5)
		self.infoTable.setColumnCount(2)
		self.infoTable.setItem(0, 0, QTableWidgetItem('Manufacturer'))
		self.infoTable.setItem(1, 0, QTableWidgetItem('Model'))
		self.infoTable.setItem(2, 0, QTableWidgetItem('Name'))
		self.infoTable.setItem(3, 0, QTableWidgetItem('Version'))
		self.infoTable.setItem(4, 0, QTableWidgetItem('Serial Number'))
		h1 = self.infoTable.horizontalHeader()
		h1.setStretchLastSection(1)
		h1.hide()

	def init_ui(self):					
		# Creating widgets

		self.wButtonPanel = QWidget()
		self.wButtonPanel.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Maximum)
		hboxLayout1 = QHBoxLayout(self.wButtonPanel)

		self.pbConnect = QPushButton('Connect')
		self.pbConnect.clicked.connect(self.camConnect)
		hboxLayout1.addWidget(self.pbConnect)
		self.pbDisconnect = QPushButton('Disconnect')
		self.pbDisconnect.clicked.connect(self.camDisconnect)
		self.pbDisconnect.setEnabled(0)
		hboxLayout1.addWidget(self.pbDisconnect)
		self.pbPlay = QPushButton('Play')
		self.pbPlay.clicked.connect(self.camPlay)
		self.pbPlay.setEnabled(0)
		hboxLayout1.addWidget(self.pbPlay)
		self.pbStop = QPushButton('Stop')
		self.pbStop.clicked.connect(self.camStop)
		self.pbStop.setEnabled(0)
		hboxLayout1.addWidget(self.pbStop)
		self.pbTree = QPushButton('GenICam tree')
		self.pbTree.clicked.connect(self.camTree)
		self.pbTree.setEnabled(0)
		hboxLayout1.addWidget(self.pbTree)
		
		self.wBottomPanel = QWidget()
		hboxLayout2 = QGridLayout(self.wBottomPanel)
		self.lInfo = QLabel('Device info')
		self.lInfo.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Maximum)
		hboxLayout2.addWidget(self.lInfo, 0, 0)
		self.lDisplayCap = QLabel('Display')
		self.lInfo.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Maximum)
		hboxLayout2.addWidget(self.lDisplayCap, 0, 1)

		self.infoTable = QTableWidget()		
		self.initInfoTable()

		hboxLayout2.addWidget(self.infoTable, 1, 0)
		v1 = self.infoTable.verticalHeader()
		v1.hide()


		self.WDisplay = QWidget()
		hboxLayout2.addWidget(self.WDisplay, 1, 1)		
		self.WChildDisplay = QWidget(self.WDisplay)		


		# Creating a Vertical Layout to add all the widgets
		self.vboxLayout = QVBoxLayout(self)

		# Adding the widgets
		self.vboxLayout.addWidget(self.wButtonPanel)
		self.vboxLayout.addWidget(self.wBottomPanel)

		self.counter = 0
		self.PyIpxSystem1 = IpxCameraGuiApiPy.PyIpxSystem() 
		print(self.PyIpxSystem1.GetDisplayName())
		IpxCameraGuiApiPy.PyCreateDisplay(self.WDisplay.winId()) 

		# Setting the vBoxLayout as the main layout
		self.setLayout(self.vboxLayout)
		self.setWindowTitle('CommonStreamUI main window')
		self.setMinimumSize(600, 400)
		self.show()
		

	def CreateDataStreamBuffers(self):	
		bufSize = self.data_stream.GetBufferSize()	
		minNumBuffers = self.data_stream.GetMinNumBuffers()	
		self.list1 = []
		for x in range(minNumBuffers+1):
			self.list1.append(self.data_stream.CreateBuffer(bufSize))
		return self.list1

	def GrabNewBuffer(self):		
		buffer = self.data_stream.GetBuffer(1000)                
		print (self.counter, 'W:', buffer.GetWidth(),' H:',buffer.GetHeight())
		if buffer.IsIncomplete() :
			print("incomplete buffer")
			self.data_stream.QueueBuffer(buffer)
			return 0
		img1 = buffer.GetImage()
		IpxCameraGuiApiPy.PyShowImageOnDisplay(img1) 
		self.data_stream.QueueBuffer(buffer)
		self.counter = self.counter + 1
		return 0

	def threaded_function(self):
		self.stop = False
		while True:
			self.GrabNewBuffer()
			if self.stop == True:
				break


	@pyqtSlot(bool)
	def camConnect(self):					
		self.deviceInfo = self.PyIpxSystem1.SelectCamera(self.winId())
		if self.deviceInfo == None:
			return
		self.pbConnect.setEnabled(0)
		self.pbDisconnect.setEnabled(1)
		self.pbPlay.setEnabled(1)
		self.pbStop.setEnabled(0)
		self.pbTree.setEnabled(1)

		self.infoTable.setItem(0, 1, QTableWidgetItem(self.deviceInfo.GetVendor()))
		self.infoTable.setItem(1, 1, QTableWidgetItem(self.deviceInfo.GetModel()))
		self.infoTable.setItem(2, 1, QTableWidgetItem(self.deviceInfo.GetUserDefinedName()))
		self.infoTable.setItem(3, 1, QTableWidgetItem(self.deviceInfo.GetVersion()))
		self.infoTable.setItem(4, 1, QTableWidgetItem(self.deviceInfo.GetSerialNumber()))
		self.device = IpxCameraGuiApiPy.PyIpxCreateDevice(self.deviceInfo) 
		print ('Opening stream to device')
		self.data_stream = self.device.GetStreamByIndex(0)
		print ('buffer size : ', self.data_stream.GetBufferSize())
		self.CreateDataStreamBuffers()			
		self.gPars = self.device.GetCameraParameters()		

	@pyqtSlot(bool)
	def camDisconnect(self):
		self.pbDisconnect.setEnabled(0)	
		if hasattr(self, 'device'):
			if self.device.IsValid():
				self.camStop()
		if hasattr(self, 'data_stream'):
			self.data_stream.FlushBuffers(self.data_stream.Flush_AllDiscard)
		if hasattr(self, 'list1'):	
			for x in self.list1:
				self.data_stream.RevokeBuffer(x)
				print ('Free data stream buffers')
		print ('Closing stream')
		if hasattr(self, 'data_stream'):	
			self.data_stream.Release()
		# we have to release genicam parameters tree before device is released
		if self.parView:
			IpxCameraGuiApiPy.PyDestroyGenParamTreeView(self.parView) 
			self.parView = None
		print ('Disconnecting device')
		if hasattr(self, 'device'):
			self.device.Release()
		self.pbPlay.setEnabled(0)
		self.pbStop.setEnabled(0)
		self.pbConnect.setEnabled(1)
		self.pbTree.setEnabled(0)			

	@pyqtSlot(bool)
	def camPlay(self):
		self.pbPlay.setEnabled(0)				
		self.gPars.SetIntegerValue("TLParamsLocked", 1)	
		print("Send AcquisitionStart command.")
		self.data_stream.StartAcquisition()
		self.gPars.ExecuteCommand("AcquisitionStart")
		self.thread = Thread(target = self.threaded_function)
		self.thread.start()
		self.pbStop.setEnabled(1)

	@pyqtSlot(bool)
	def camStop(self):
		self.pbStop.setEnabled(0)
		self.stop = True
		if hasattr(self.thread, 'join'):
			self.thread.join()
		print("Sending AcquisitionStop command to the device")
		if hasattr(self, 'gPars'):
			self.gPars.ExecuteCommand("AcquisitionStop")
		if hasattr(self, 'data_stream'):
			self.data_stream.StopAcquisition(1)
		if hasattr(self, 'gPars'):
			self.gPars.SetIntegerValue("TLParamsLocked", 0)
		self.pbPlay.setEnabled(1)
		

	@pyqtSlot(bool)
	def camTree(self):		
		print('in slot Tree')
		if self.parView:
			IpxCameraGuiApiPy.PyDestroyGenParamTreeView(self.parView) 
		self.parView = IpxCameraGuiApiPy.PyCreateGenParamTreeViewForArray(self.gPars, self.winId()) 
		
		

