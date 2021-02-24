Imperx Camera SDK Python API modules are built against Python 3.6 and PyQt5.

Sample scripts include :
    1) CommonStreamConsole.py - console application script, demonstrating how to list the interfaces, 
       connect to the camera, and acquire the images
    2) CommonStreamGUI.py - GUI application script, demonstrating how to open the Device Selector dialog,
       connect to the camera, acquire the images, show the live video, and invoke the Gen<I>Cam Tree dialog 
       to get / set camera parameters.
    3) MultiCamStreamConsole.py - console application script, demonstrating how to list the cameras in the system, 
       connect to the camera, and acquire the images from the multiple cameras
    4) GevEventsConsole.py - console application script, demonstrating how to list the interfaces, 
       connect to the camera, and process the GigE Vision events
    5) CheetahPregiusGevConsole.py - console application script, demonstrating how to list the interfaces, 
       connect to the camera, set/get Cheetah Pregius GEV camera parameters, and acquire the video 

To run the Python scripts following steps should be performed:

On Windows OS: 
    1) Download Windows x86-64 executable installer of Python 3.6, install it, and set as default
    2) Run terminal command to install PyQt5: 
       pip install pyqt5==5.9
    3) Switch to Python samples folder and run the scripts:
       python CommonStreamConsole.py
       python CommonStreamGUI.py

On Linux OS:
    1) Open Terminal windows and install Python3.6:
       sudo apt install python3.6
    2) Install PyQt5:  
       sudo apt install python3-pyqt5
    3) Switch to Python samples folder and set PYTHONPATH environment variable:
       export PYTHONPATH=${PYTHONPATH}:$PWD/../../lib/Linux64_x64/  (systems with x86_64 CPU)
      or
       export PYTHONPATH=${PYTHONPATH}:$PWD/../../lib/Linux64_ARM/  (systems with ARM CPU)
    4) Run the sample scripts:
       python3 CommonStreamConsole.py
       python3 CommonStreamGUI.py


