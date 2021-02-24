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
	print ('connecting to ', devInfo.GetDisplayName())
	dev1 = IpxCameraApiPy.PyIpxCreateDevice(devInfo) if platform.system()=='Windows' else libIpxCameraApiPy.PyIpxCreateDevice(devInfo) 	
	print ('device model: ', devInfo.GetModel())
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
	devList=[0]
	gv_num=1
	print ('[ 0 ]     NONE - Enter 0 to stop the selection and start the acquisition')
	for infs in intLst:
		# Get list of cameras connected to the current interface 
		devListInt = infs.GetDeviceInfoList()
		for dv in devListInt:
			devList.append(dv)
			print ('[', gv_num, ']    ',dv.GetModel())
			gv_num+=1
			
	print( 'Enter camera number to connect to or [0] to stop the selection and start the acquisition:' )	
	dv_select = sys.stdin.readline()[:-1]
	# Return selected camera's DeviceInfo object
	if (int(dv_select)<gv_num) :
		return devList[int(dv_select)]
	else:
		return 0
	
	return 0

# Thread-safe print function
def locked_print(*a, **b):
    with s_print_lock:
        print(*a, **b)

# Thread function to perform the video acquisition on the given camera
def GrabbingThreadFunction(camera):
	cameraModel = camera.GetInfo().GetModel()

	# create Stream object
	stream = camera.GetStreamByIndex(0)
	locked_print (cameraModel, ': Stream created') 

	# create buffers queue
	bufSize = stream.GetBufferSize() 
	minNumBuffers = stream.GetMinNumBuffers()	# User's defined value should not be less than GetMinNumBuffers() returned
	sbLst = [];
	for x in range(minNumBuffers+1):
		sbLst.append(stream.CreateBuffer(bufSize))
	locked_print (cameraModel, ': Buffers queue created ')

	# start the acquisition and acquire 'numFramesToAcquire' images
	gPars = camera.GetCameraParameters()	
	gPars.SetIntegerValue("TLParamsLocked", 1)	
	stream.StartAcquisition()
	gPars.ExecuteCommand("AcquisitionStart")
	for x in range(numFramesToAcquire):
		buffer = stream.GetBuffer(-1) 
		locked_print (cameraModel,': FrameID: ', x, 'W:', buffer.GetWidth(),' H:',buffer.GetHeight())
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
		stream.RevokeBuffer(b );

	# release the stream and camera
	locked_print (cameraModel,': Closing stream')	
	stream.Release()
	camera.Release()
	return 0
	
# Get PyIpxSystem object
PyIpxSystem1 = IpxCameraApiPy.PyIpxSystem() if platform.system()=='Windows' else libIpxCameraApiPy.PyIpxSystem()
print(PyIpxSystem1.GetDisplayName())

# Connect to cameras until user select 0
deviceInfo = SelectDevice(PyIpxSystem1)
while (deviceInfo):
	dev = connectToDevice(deviceInfo)
	if(dev):
		print ('Connected OK to: ', deviceInfo.GetModel() )
		camerasList.append(dev)
		camerasNum+=1
	else:
		print ('Connection to: ', deviceInfo.GetModel(), ' failed' )
	deviceInfo = SelectDevice(PyIpxSystem1)

# Start camerasNum threads and start the video acquisition in each thread
threadList = []
camerasNum = 0
for camN in camerasList :
	threadCamN = threading.Thread(target=GrabbingThreadFunction, args = (camN,))
	camerasNum+=1
	threadCamN.start()
	threadList.append(threadCamN)

for threaCamN in threadList :
	threaCamN.join()


