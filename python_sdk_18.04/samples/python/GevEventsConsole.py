import sys
import os
import platform
import time
import threading

if platform.system()=='Windows':
    sys.path.append(os.environ['IPX_CAMSDK_ROOT']+'/bin/win64_x64/')
    sys.path.append(os.environ['IPX_CAMSDK_ROOT']+'/bin/win32_i86/')

if platform.system()=='Windows':
    import IpxCameraApiPy
else :
    import libIpxCameraApiPy

s_print_lock = threading.Lock()
s_event_lock = threading.Lock()

# Thread-safe print function
def locked_print(*a, **b):
    with s_print_lock:
        print(*a, **b)

mapTEventSelStr=dict()
gotEventsStr='No Event'

def CallBackFunc(eventData, eventSize, ptr1):
	with s_event_lock:
		global gotEventsStr
		if eventSize<1 or eventData[0]!=0x42 :	
			gotEventsStr='Got wrong event';
			return 0				
		eventID = int.from_bytes(eventData[10:12], byteorder='big', signed=False) 
		timestamp =  int.from_bytes(eventData[16:24], byteorder='big', signed=False) 
		eventName = 'Unknown'	
		if eventID in mapTEventSelStr.keys() :
			eventName = mapTEventSelStr[eventID]
		gotEventsStr = 'Got Event ID={} Name={}, Timestamp={}'.format(hex(eventID),eventName,timestamp)
		locked_print(gotEventsStr)
		return 0

def AcquireImages(dev1,data_stream):	
	gPars = dev1.GetCameraParameters()
	frames_cnt = 255
	gPars.SetIntegerValue("TLParamsLocked", 1)	
	print('\nEvents Available:')


	esEnum = gPars.GetEnum('EventSelector')
	if esEnum != None :
		lEntriesCount = esEnum.GetCount()
		for iParam in range(lEntriesCount[1]) :
			lEntry = esEnum.GetEnumEntryByIndex(iParam)
			lAvailable = lEntry.IsAvailable()
			if lAvailable :
				print( iParam + 1 ,'.' , lEntry.GetValueStr()[1])
				lNameStr = lEntry.GetValueStr()
				value = lEntry.GetValue()[1]
				mapTEventSelStr[value] = lNameStr[1]

		print('Select event number: ')
		event_sel = int(sys.stdin.readline()[:-1])
		lEntry = esEnum.GetEnumEntryByIndex(event_sel-1)
		if lEntry :
			esEnum.SetValue(lEntry.GetValue()[1])
			enEnum = gPars.GetEnum('EventNotification')
			if enEnum != None :
					enEnum.SetValue(0) #off
					enEnum.SetValue(1) #on


		print('Set frames count [1-255]: ')
		frames_cnt = int(sys.stdin.readline()[:-1])
		if frames_cnt < 1 or frames_cnt > 255 :
			frames_cnt = 255

		print('Show stream [on=1, off=0]:')
		show_stream = int(sys.stdin.readline()[:-1])

	#Register event handler
	dev1.RegisterEvent(1002, CallBackFunc) #1002 genicam event

	print("Send AcquisitionStart command.")
	data_stream.StartAcquisition()
	gPars.ExecuteCommand("AcquisitionStart")
	for x in range(frames_cnt):
		buffer = data_stream.GetBuffer(1000)
		if show_stream!=0 :
			locked_print ('FrameID: ', x, 'W:', buffer.GetWidth(),' H:',buffer.GetHeight())
		data_stream.QueueBuffer(buffer)
	locked_print("Sending AcquisitionStop command to the device")
	time.sleep(0.002)
	gPars.ExecuteCommand("AcquisitionStop")
	buffer = data_stream.GetBuffer(1000)
	if buffer!=None :
		data_stream.QueueBuffer(buffer)
	data_stream.StopAcquisition(1)
	gPars.SetIntegerValue("TLParamsLocked", 0)


def CreateDataStreamBuffers(data_stream):	
	bufSize = data_stream.GetBufferSize()	
	minNumBuffers = data_stream.GetMinNumBuffers()	
	list1 = []
	for x in range(minNumBuffers+1):
		list1.append(data_stream.CreateBuffer(bufSize))
	return list1

def FreeDataStreamBuffers(data_stream, sbLst):
	for x in sbLst:
		data_stream.RevokeBuffer(x)
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
	
	return 0
	

PyIpxSystem1 = IpxCameraApiPy.PyIpxSystem() if platform.system()=='Windows' else libIpxCameraApiPy.PyIpxSystem()
print(PyIpxSystem1.GetDisplayName())
print('Devices available:' )

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
		FreeDataStreamBuffers(data_stream, sbLst)
		print ('Closing stream')
		data_stream.Release()
		print ('Disconnecting device')
		dev1.Release()
		time.sleep(3)
	else:
		print ('incorrect device')
		sys.exit()
else:
	print ('incorrect choice')
	sys.exit()

