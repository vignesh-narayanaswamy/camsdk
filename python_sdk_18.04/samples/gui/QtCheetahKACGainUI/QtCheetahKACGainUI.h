#ifndef QTCHEETAHKACGAINUI_H
#define QTCHEETAHKACGAINUI_H

#include <QtWidgets/QMainWindow>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTime>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>

#include "ui_imperx.h"
#include "IpxCameraApi.h"
#include "IpxDisplay.h"
#include "../common/inc/GrabbingThread.h"

//IpxCam namespace consist of several main classes that represent the GenTL modules
namespace IpxCam {
    // The System class is the entry point to the GenTL Producer software driver
    class System;
    // The DeviceInfo class provides a method to access device info
    class DeviceInfo;
    // The Device class provides methods to enable the communication with the device and
    // the enumerate/instantiate data streams.
    class Device;
    // The Stream class purpose is to access the buffer data acquirement from the Acquisition engine.
    class Stream;
    // The Buffer class contains the buffer methods for the data from the acquisition engine
    class Buffer;
}

//The IpxGUI namespace is a declarative region that provides a scope to the IIpxGenParameterTreeView class and functions.
namespace IpxGui {
    // The IIpxGenParamTreeView class is composed of functions to set and clear parameters of the node tree of the camera
    class IIpxGenParamTreeView;
}

class QtCheetahKACGainUI : public QMainWindow, public IpxGenParam::ParamEventSink
{
	Q_OBJECT

public:
    QtCheetahKACGainUI(QWidget *parent = 0);
    ~QtCheetahKACGainUI();

    //Create the stream buffer queue
    void CreateStreamBuffer();
    bool CreateDisplayView();
    bool SetupToolbarConnects();

	virtual void closeEvent(QCloseEvent * event);

    //Creates the Toolbar
    void CreateToolbar();
    //Updates the Toolbar Buttons
    void UpdateToolbarButtons();
    //Connects the camera
    void Connect();
    //Disconnects the camera
    void Disconnect();
    //Start the Acquisition of the Stream
    void StartAcquisition();
    //Stops the Acquisition of the Stream
    void StopAcquisition();

    bool ZoomIn();
    bool ZoomOut();
    bool Zoom100();
    bool ZoomFitToWnd();
    bool ZoomSpreadToWnd();
    bool CenterImage();
    
    void EnableControls(bool en);
    
    void CreateLayout();
    QGroupBox *CreateConnectGroup();
    QGroupBox *CreateGain();
    
	//Create Enum Combo Box
    void CreateEnumComboBox(QComboBox* paramComboBox, char* paramName, std::map<int64_t, int> *aMap);
    //Select and Update the GenParam Treeview Parameter
    void OnCbnSelchangeEnum(QComboBox* cb, char* param);
    
    void OnParameterUpdate(IpxGenParam::Param* param);

protected slots:
	//Connect and Disconnect Features
    void OnBnClickedConnectButton();
    void OnBnClickedDisconnectButton();
    
    //Start and Stop Features
    void OnBnClickedStart();
    void OnBnClickedStop();

    void OnBnClickedBtDigitalGain();
    void OnBnClickedBtBlackLevel();

    //Zoom Features
    void OnZoomIn() { ZoomIn();}
    void OnZoomOut() { ZoomOut();}
    void OnZoom100() { Zoom100();}
    void OnZoomFitToWnd() { ZoomFitToWnd();}
    void OnZoomSpreadToWnd() { ZoomSpreadToWnd();}
    void OnCenterImage() { CenterImage();}

    void cameraConnect();
    void OnBnClickedGenICamButton();
    void OnCbnSelchangeCbAnalogGain(int aIndex);

    void setDigitalGainText(const QString &text);
    void setBlackLevelText(const QString &text);

    
private:
	Ui::IpxPlayerClass ui;

    //Connect Group (Camera Information)
    QLineEdit *mManufacturerEdit;
    QLineEdit *mDeviceFamilyEdit;
    QLineEdit *mModelEdit;
    QLineEdit *mNameEdit;
    QLineEdit *mVersionEdit;
    QLineEdit *mSerialNbrEdit;

    //Gain Group
    std::map<int64_t, int> m_mapAnalogValues;

	QComboBox *m_cbAnalogGain;
    QLineEdit *m_edDigitalGain;
	//Program the Black Level Offset Parameter and QLineEdit box
	QLineEdit *m_edBlackLevel;
	
    QPushButton *m_btDigitalGainSet;
    QPushButton *m_btBlackLevelSet;

    //Indicates whether or not Camera is connected
    bool m_isConnected;
    //Indidates whether or not Grabbing Thread has started
    bool m_isStartEnabled;

    //Pointer to IpxCam System Object
    IpxCam::System* m_system;
    //Pointer to IpxCam Device Object
    IpxCam::Device* m_camera;
    //Pointer to IpxCam Stream Object
    IpxCam::Stream* m_stream;
    //Pointer to IpxCam Device Info Object
    IpxCam::DeviceInfo* m_devInfo;
    //Pointer to GrabbingThread Object
    GrabbingThread *m_grabThread;
    //Pointer to IpxDisplay Object
    IpxDisplay     *m_display;
    QLabel *m_displayLabel;
    //Pointer to IIpxGenParamTreeView Object
    IpxGui::IIpxGenParamTreeView* m_parameterView;
    
    //Retrieve the initial parameter values from the Camera
    void InitFromCamera();

    bool StartGrabbing();

    bool StopGrabbing();
};

#endif // QTCHEETAHKACGAINUI_H
