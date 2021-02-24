import cv2
import numpy as np
import libIpxCameraApiPy as IpxCameraApiPy


class Camera:
    def __init__(self):
        """
        Camera class for initializing, connecting to and operating the camera
        """

        # Initiating IpxCamera and connecting to device on network
        self.ipx_system = IpxCameraApiPy.PyIpxSystem()
        self.camera = self.__select_device()
        self.params = self.camera.GetCameraParameters()

        # Recording video
        # self._fourcc = cv2.VideoWriter_fourcc(*'DIVX')
        # self._out = cv2.VideoWriter('sample_video.mp4', 0x7634706d, 10.0, (img_width, img_height), 0)

    def __select_device(self):
        """
        Select camera device to connect to
        """
        network_interfaces = self.ipx_system.GetInterfaceList()
        active_devices = list()
        dev_id = 0
        for interface in network_interfaces:
            devices = interface.GetDeviceInfoList()
            for dev in devices:
                active_devices.append(dev)
                print('[', dev_id, ']    ', dev.GetModel(), 'at', interface.GetDescription())
                dev_id += 1
        if not network_interfaces or not active_devices:
            print('No devices found')
            return None

        while True:
            dv_select = input('Please select the camera number to connect to: \n')
            if not dv_select.isnumeric():
                print('Please input a valid integer \n')
                continue
            dv_select = int(dv_select)
            if dv_select > dev_id:
                print(f'No device with ID: {dv_select} \n')
                continue
            return self.__establish_connection(active_devices[dv_select])

    @staticmethod
    def __establish_connection(device):
        """
        Connects to camera using device info passed
        :param obj device: Device object to connect to
        """
        print('connecting to ', device.GetDisplayName())
        device = IpxCameraApiPy.PyIpxCreateDevice(device)
        return device if device.GetNumStreams() >= 1 else None

    def start_acquisition(self):
        """
        Starts capturing images from the camera
        """
        try:
            img_height = self.params.GetInt('Height').GetValue()[1]
            img_width = self.params.GetInt('Width').GetValue()[1]

            camera_model = self.camera.GetInfo().GetModel()

            # create Stream object
            stream = self.camera.GetStreamByIndex(0)
            print(camera_model, ': Stream created')

            # create buffers queue
            buf_size = stream.GetBufferSize()
            min_num_buffers = stream.GetMinNumBuffers()
            sb_lst = [];
            for x in range(min_num_buffers + 1):
                sb_lst.append(stream.CreateBuffer(buf_size))
            print(camera_model, ': Buffers queue created ')

            self.params.SetIntegerValue("TLParamsLocked", 1)
            stream.StartAcquisition()
            self.params.ExecuteCommand("AcquisitionStart")

            while True:
                buffer = stream.GetBuffer(-1)
                image = np.frombuffer(buffer.GetBufferPtr(), dtype=np.uint8).reshape((img_height,
                                                                                      img_width))

                # Use DL to check if the current frame has start of a truck rail
                # If it does, start recording images

                cv2.imshow('Cam Stream', image)
                # self._out.write(image)
                cv2.waitKey(1)
                stream.QueueBuffer(buffer)

        except KeyboardInterrupt:
            # self._out.release()
            if self.params.ExecuteCommand("AcquisitionStop") != 0:
                self.params.ExecuteCommand("AcquisitionAbort")
            stream.StopAcquisition(1);
            self.params.SetIntegerValue("TLParamsLocked", 0)
            stream.FlushBuffers(stream.Flush_AllDiscard)
            for b in sb_lst:
                stream.RevokeBuffer(b);

            # release the stream and camera
            print(camera_model, ': Closing stream')
            stream.Release()

        return 0


cam = Camera()

cam.params.SetIntegerValue("Width", 1240)
cam.params.SetIntegerValue("Height", 720)
cam.params.SetEnumValue("ExposureMode", 2)
cam.params.SetFloatValue("ExposureTime", 100000.000000)
cam.params.SetBooleanValue("AcquisitionFrameRateEnable", True)
cam.params.SetFloatValue("AcquisitionFrameRate", 10)

cam.start_acquisition()