# Imperx Camera SDK



Sample code to
- Initialize the camera
- View camera parameters
- Change camera parameters
- Start and stop acquisition

Connection is established with the camera when Camera object is initialized

```python
cam = IpxCamera()
```

    [ 0 ]     POE-C2400M-RC0000000 at wlp0s20f3[192.168.1.126]
    Please select the camera number to connect to: 
    0
    connecting to  POE-C2400M-RC0000000


Once connection is established, camera parameters can be viewed and changed

##### Viewing camera parameters

```python
cam.params.GetFloat("ExposureTime").GetValue()
```




    (True, 100000.0)



```python
cam.params.GetInt("Width").GetValue()
```




    (True, 1240)




##### Setting camera parameters

```python
cam.params.GetInt("Width").SetValue(1240)
cam.params.GetInt("Height").SetValue(720)
```




    True



```python
cam.params.GetEnum("ExposureMode").SetValueStr("Timed")
```




    True



```python
cam.params.GetBoolean("AcquisitionFrameRateEnable").SetValue(True)
cam.params.GetFloat("AcquisitionFrameRate").SetValue(10)
```




    True



Starting Image Acquisition. Camera will keep capturing images till a Keyboard Interrupt is issued

```python
cam.start_acquisition()
```

    POE-C2400M-RC0000000 : Stream created
    POE-C2400M-RC0000000 : Buffers queue created 
    POE-C2400M-RC0000000 : Closing stream





    0


