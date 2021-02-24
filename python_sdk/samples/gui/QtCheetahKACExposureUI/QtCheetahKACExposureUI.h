#ifndef QTCHEETAHKACEXPOSUREUI_H
#define QTCHEETAHKACEXPOSUREUI_H

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
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

class QtCheetahKACExposureUI : public QMainWindow, public IpxGenParam::ParamEventSink
{
	Q_OBJECT

public:
    QtCheetahKACExposureUI(QWidget *parent = 0);
    ~QtCheetahKACExposureUI();

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
    void CreateEnumComboBox(QComboBox* paramComboBox, char* paramName, std::map<int64_t, int> *aMap);
    void EnableControls(bool en);
    // UI
    void CreateLayout();
    QGroupBox *CreateConnectGroup();
    QGroupBox *CreateExposure();

    unsigned int GetCurrentExposure();
    void OnCbnSelchangeEnum(QComboBox* cb, char* param);
    void UpdateExposure(unsigned int expVal);
    void OnParameterUpdate(IpxGenParam::Param* param);

protected slots:
    void OnBnClickedConnectButton();
    void OnBnClickedDisconnectButton();
    void OnBnClickedStart();
    void OnBnClickedStop();
    void OnZoomIn() { ZoomIn();}
    void OnZoomOut() { ZoomOut();}
    void OnZoom100() { Zoom100();}
    void OnZoomFitToWnd() { ZoomFitToWnd();}
    void OnZoomSpreadToWnd() { ZoomSpreadToWnd();}
    void OnCenterImage() { CenterImage();}
    void cameraConnect();
    void OnBnClickedGenICamButton();
    void OnCbnSelchangeCbExpMode(int aIndex);
    void OnBnClickedBtExpSet();
    void OnBnClickedBtFtmSet();
    void OnBnClickedBtFpsSet();
    void OnBnClickedChFrameTimeEnable();
private:
	Ui::IpxPlayerClass ui;

    //Connect Group (Camera Information)
    QLineEdit *mManufacturerEdit;
    QLineEdit *mDeviceFamilyEdit;
    QLineEdit *mModelEdit;
    QLineEdit *mNameEdit;
    QLineEdit *mVersionEdit;
    QLineEdit *mSerialNbrEdit;

    //Exposure Group
    QComboBox *m_cbExpMode;
    std::map<int64_t, int> m_mapExpModeValues;
    QLineEdit *m_edExpTime;
    QLineEdit* m_edFFPVal;
    QLineEdit *m_edFPS;
    QPushButton *mPlayButton;
    QPushButton *mStopButton;
    QPushButton *m_bt_exp_set;
    QPushButton *m_bt_ftm_set;
    QPushButton *m_bt_fps_set;
    QCheckBox *m_chFFPEnable;
    QLineEdit *m_edLines;
    QLineEdit *m_edClocks;

    //Indicates whether or not Camera is connected
    bool m_isConnected;
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
    int64_t m_lineTime;
    int64_t m_pixClock;

    bool StartGrabbing();

    bool StopGrabbing();
};

#endif // QTCHEETAHKACEXPOSUREUI_H
