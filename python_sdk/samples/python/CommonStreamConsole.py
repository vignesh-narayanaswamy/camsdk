import sys
import os
import platform

if platform.system()=='Windows':
    sys.path.append(os.environ['IPX_CAMSDK_ROOT']+'/bin/win64_x64/')
    sys.path.append(os.environ['IPX_CAMSDK_ROOT']+'/bin/win32_i86/')

if platform.system()=='Windows':
    import IpxCameraApiPy
else :
    import libIpxCameraApiPy


def AcquireImages(dev1,data_stream):	
	gPars = dev1.GetCameraParameters()
	gPars.SetIntegerValue("TLParamsLocked", 1)
	print("Send AcquisitionStart command.")
	data_stream.StartAcquisition()
	gPars.ExecuteCommand("AcquisitionStart")
	for x in range(255):
		buffer = data_stream.GetBuffer(1000)
		print ('FrameID: ', x, 'W:', buffer.GetWidth(),' H:',buffer.GetHeight())
		if x%16==1:
			image = buffer.GetBufferPtr()			
			with open("PyFrame%d.raw" % (x), 'wb') as f:
				f.write(bytearray(image))
		data_stream.QueueBuffer(buffer)
	print("Sending AcquisitionStop command to the device")
	gPars.ExecuteCommand("AcquisitionStop")
	buffer = data_stream.GetBuffer(1000)
	if buffer!=None :	
		data_stream.QueueBuffer(buffer)
	data_stream.StopAcquisition(1);
	gPars.SetIntegerValue("TLParamsLocked", 0);


def CreateDataStreamBuffers(data_stream):	
	bufSize = data_stream.GetBufferSize()	
	minNumBuffers = data_stream.GetMinNumBuffers()	
	list1 = [];
	for x in range(minNumBuffers+1):
		list1.append(data_stream.CreateBuffer(bufSize));
	return list1

def FreeDataStreamBuffers(data_stream, sbLst):
	for x in sbLst:
		data_stream.RevokeBuffer(x);
	print ('Free data stream buffers')


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
	devList=[]
	gv_num=0
	for infs in intLst:
		# Get list of cameras connected to the current interface 
		devListInt = infs.GetDeviceInfoList()
		for dv in devListInt:
			devList.append(dv)
			print ('[', gv_num, ']    ',dv.GetModel(), 'at',  infs.GetDescription())
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
	

PyIpxSystem1 = IpxCameraApiPy.PyIpxSystem() if platform.system()=='Windows' else libIpxCameraApiPy.PyIpxSystem()
print(PyIpxSystem1.GetDisplayName())
print('Interfaces available:' )

deviceInfo = SelectDevice(PyIpxSystem1)
if deviceInfo != 0:
	dev1 = connectToDevice(deviceInfo)
	if dev1!=0:
		print ('Opening stream to device')
		data_stream = dev1.GetStreamByIndex(0)
		print ('buffer size : ', data_stream.GetBufferSize())
		sbLst = CreateDataStreamBuffers(data_stream)			
		AcquireImages(dev1,data_stream)
		data_stream.FlushBuffers(data_stream.Flush_AllDiscard)	
		FreeDataStreamBuffers(data_stream, sbLst);
		print ('Closing stream')	
		data_stream.Release()
		print ('Disconnecting device')
		dev1.Release()		
	else:
		print ('incorrect device')
		sys.exit()
else:
	print ('incorrect choice')
	sys.exit()

