import sys
import os
import platform
import threading

# number of frames to acquire from cameras
numFramesToAcquire=50

camerasList=[]
camerasNum=0

s_print_lock = threading.Lock()

# Add IpxCameraSDK binaries folders to PATH environment variables
if platform.system()=='Windows':
    sys.path.append(os.environ['IPX_CAMSDK_ROOT']+'/bin/win64_x64/')
    sys.path.append(os.environ['IPX_CAMSDK_ROOT']+'/bin/win32_i86/')

# Import IpxCameraApiPy Python API library
if platform.system()=='Windows':
    import IpxCameraApiPy
else :
    import libIpxCameraApiPy




# This function connects to the Camera using given IpxCam::DeviceInfo object, and returns the IpxCam::Device object of connected camera
def connectToDevice(devInfo):
	print ('connecting to ', devInfo.GetModel())
	dev1 = IpxCameraApiPy.PyIpxCreateDevice(devInfo) if platform.system()=='Windows' else libIpxCameraApiPy.PyIpxCreateDevice(devInfo) 	
	tCnt = dev1.GetNumStreams()	
	if tCnt < 1 :		
		return 0
	else:		
		return dev1
	
# This function allows user to select the interface and camera to get the IpxCam::DeviceInfo object
def SelectDevice(ipxSystem):
	# Get Interface list
	intLst = ipxSystem.GetInterfaceList()	
	# Get list of cameras connected to each interface 
	devList=[]
	gv_num=0
	for infs in intLst:
		# Get list of cameras connected to the current interface 
		devListInt = infs.GetDeviceInfoList()
		for dv in devListInt:
			devList.append(dv)
			print ('[', gv_num, ']    ',dv.GetModel(), 'at ',  infs.GetDescription())
			gv_num+=1
	
	if(len(intLst)<1):
		print('No device found')	

	print('Please select the camera number to connect to: ')
	dv_select = sys.stdin.readline()[:-1]
	# Return selected camera's DeviceInfo object
	if (int(dv_select)<gv_num) :
		return devList[int(dv_select)]
	else:
		return 0
	
	return 0

# Function to set Exposure parameters
def SetupExposure(camera):

	pars = camera.GetCameraParameters()	
	
	# Set ExposureAuto = Off
	parAec = pars.GetEnum("ExposureAuto")	
	parAec.SetValueStr("Off")

	# Set ExposureMode = Timed 
	parExposureMode = pars.GetEnum("ExposureMode")
	parExposureMode.SetValueStr("Timed")

	# Set ExposureTime in microseconds
	parExposureTime = pars.GetFloat("ExposureTime")
	expMin = parExposureTime.GetMin()
	expMax = parExposureTime.GetMax()
	expCur = parExposureTime.GetValue()

	print('Enter Exposure value in microseconds, Min=',expMin[1], 'Max=',expMax[1], 'Current=',expCur[1])
	expVal = float(sys.stdin.readline()[:-1])
	if expVal > expMax[1] or expVal < expMin[1] :
		print('Value ', expVal, 'is out of range')
	else: 
		parExposureTime.SetValue(expVal)

# This function setups the AEC parameters of the camera
# AGC / AEC function not included in models: POE-C2000, POE-C2400  
def SetupAec(camera):

	pars = camera.GetCameraParameters()	

	print('Setup AEC')

	# Set ExposureAuto = Continuous
	parAec = pars.GetEnum("ExposureAuto")	
	parAec.SetValueStr("Continuous")

	# set AecExposureMin = 300us
	parAecExpMin = pars.GetInt("AecExposureMin")
	parAecExpMin.SetValue(300)

	# set AecExposureMax = (MAX-MIN)/2
	parAecExpMax = pars.GetInt("AecExposureMax")
	aecExpMaxMax = parAecExpMax.GetMax()[1]
	aecExpMaxMin = parAecExpMax.GetMin()[1]
	parAecExpMax.SetValue((aecExpMaxMax-aecExpMaxMin)/2)
	
# This function setups the AGC parameters of the camera
# AGC / AEC function not included in models: POE-C2000, POE-C2400  
def SetupAgc(camera):

	pars = camera.GetCameraParameters()	

	print('Setup AGC')

	# Set GainAuto = Continuous
	parAgc = pars.GetEnum("GainAuto")	
	parAgc.SetValueStr("Continuous")

	# set AgcGainMin = 0.1dB
	parAgcGainMin = pars.GetFloat("AgcGainMin")
	parAgcGainMin.SetValue(0.1)

	# set AgcGainMax = 7dB
	parAgcGainMax = pars.GetFloat("AgcGainMax")
	parAgcGainMax.SetValue(7.0)

# This function setups the Gain parameters of the camera
def SetupGain(camera):

	pars = camera.GetCameraParameters()	

	# Set GainAuto = Off
	parAgc = pars.GetEnum("GainAuto")	
	parAgc.SetValueStr("Off")

	# Set Digital Gain =1 dB, DigitalOffset=0
	parDigGain = pars.GetFloat("DigitalGain")
	parDigGain.SetValue(1.0)
	parDigOffset = pars.GetInt("DigitalOffset")
	parDigOffset.SetValue(0)
		
	# Set Gain in dB
	parGain = pars.GetFloat("Gain")
	gainMin = parGain.GetMin()
	gainMax = parGain.GetMax()
	currGain = parGain.GetValue()

	print('Enter Gain value in dB, Min=',gainMin[1], 'Max=',gainMax[1], 'Current=', currGain[1])
	gainVal = float (sys.stdin.readline()[:-1])
	if gainVal > gainMax[1] or gainVal < gainMin[1] :
		print('Value ', gainVal, 'is out of range')
	else: 
		parGain.SetValue(gainVal)

# This function setups the Trigger parameters of the camera
def SetupTrigger(camera):

	pars = camera.GetCameraParameters()	

	print('Setup Trigger')

	# Set TriggerSource = Line1
	parTrgSrc = pars.GetEnum("TriggerSource")	
	parTrgSrc.SetValueStr("Line1") # can be : Line1, Line2, PulseGenerator, Software

	# Set TriggerDelay=20 microseconds
	parTrgDelay = pars.GetInt("TriggerDelay")
	parTrgDelay.SetValue(20)
		
	# Set TriggerActivation = RisingEdge
	parTrgAct = pars.GetEnum("TriggerActivation")
	parTrgAct.SetValueStr("RisingEdge") 

	# Set TriggerMode = On/Off
	parTrgMode = pars.GetEnum("TriggerMode")	
	parTrgMode.SetValueStr("Off") 


# Function to perform the video acquisition on the given camera
def DoAcquisition(camera):
	cameraModel = camera.GetInfo().GetModel()

	# create Stream object
	stream = camera.GetStreamByIndex(0)
	print (cameraModel, ': Stream created') 

	# create buffers queue
	bufSize = stream.GetBufferSize() 
	minNumBuffers = stream.GetMinNumBuffers()	# User's defined value should not be less than GetMinNumBuffers() returned
	sbLst = [];
	for x in range(minNumBuffers+1):
		sbLst.append(stream.CreateBuffer(bufSize))
	print (cameraModel, ': Buffers queue created ')

	# start the acquisition and acquire 'numFramesToAcquire' images
	gPars = camera.GetCameraParameters()	
	gPars.SetIntegerValue("TLParamsLocked", 1)	
	stream.StartAcquisition()
	gPars.ExecuteCommand("AcquisitionStart")
	for x in range(numFramesToAcquire):
		buffer = stream.GetBuffer(-1) 
		print (cameraModel,': FrameID: ', x, 'W:', buffer.GetWidth(),' H:',buffer.GetHeight())
		# save every 10th image
		if x%10==0:
			image = buffer.GetBufferPtr()			
			with open("PyFrame%d.raw" % (x), 'wb') as f:
				f.write(bytearray(image))
		stream.QueueBuffer(buffer)

	# stop the acquisition 
	if( gPars.ExecuteCommand("AcquisitionStop") != 0):
		gPars.ExecuteCommand("AcquisitionAbort") 
	stream.StopAcquisition(1);
	gPars.SetIntegerValue("TLParamsLocked", 0);

	# release all the buffers
	stream.FlushBuffers(stream.Flush_AllDiscard)	
	for b in sbLst:
		stream.RevokeBuffer(b);

	# release the stream and camera
	print (cameraModel,': Closing stream')	
	stream.Release()
	camera.Release()
	return 0
	
# Get PyIpxSystem object
PyIpxSystem1 = IpxCameraApiPy.PyIpxSystem() if platform.system()=='Windows' else libIpxCameraApiPy.PyIpxSystem()
print(PyIpxSystem1.GetDisplayName())

# Connect to the camera 
deviceInfo = SelectDevice(PyIpxSystem1)
if(deviceInfo):
	dev = connectToDevice(deviceInfo)
	if(dev):
		print ('Connected OK to: ', deviceInfo.GetModel() )
	
		# Set Heartbeat Timeout to 30 seconds to have comfortable debugging. 
		# Uncomment the string below to prevent the camera disconnection on Heartbeat timeout during the debugging
		dev.GetCameraParameters().SetIntegerValue("GevHeartbeatTimeout", 30000)

		# Set the PregiusGEV parameters. Comment / Uncomment the function you need
		
		SetupGain(dev)
		SetupExposure(dev)
	
		# SetupAgc(dev)
		# SetupAec(dev)
		# SetupTrigger(dev)

		# Start the Acquisition
		DoAcquisition(dev)
	else:
		print ('Connection to: ', deviceInfo.GetModel(), ' failed' )
	
	



