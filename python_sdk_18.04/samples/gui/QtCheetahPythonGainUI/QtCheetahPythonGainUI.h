#ifndef QTCHEETAHPYTHONGAINUI_H
#define QTCHEETAHPYTHONGAINUI_H

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
#include "IpxCameraGuiApi.h"
#include "IpxDisplay.h"
#include "../common/inc/GrabbingThread.h"
#include "../common/inc/ipxparammaphelper.h"

class QtCheetahPythonGainUI : public QMainWindow, public IpxGenParam::ParamEventSink
{
	Q_OBJECT

public:
    QtCheetahPythonGainUI(QWidget *parent = 0);
    ~QtCheetahPythonGainUI();

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
    
    void OnParameterUpdate(IpxGenParam::Param* param);

protected Q_SLOTS:
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
    
private:
	Ui::IpxPlayerClass ui;

    //Connect Group (Camera Information)
    QLineEdit *mManufacturerEdit;
    QLineEdit *mDeviceFamilyEdit;
    QLineEdit *mModelEdit;
    QLineEdit *mNameEdit;
    QLineEdit *mVersionEdit;
    QLineEdit *mSerialNbrEdit;

    bool m_bNewXml;

	QComboBox *m_cbAnalogGain;
    QLineEdit *m_edDigitalGain;
    QLineEdit *m_edDigitalGainCode;
    QLineEdit *m_edDigitalGainDB;

	//Program the Black Level Offset Parameter and QLineEdit box
	QLineEdit *m_edBlackLevel;
	
    QPushButton *m_btDigitalGainSet;
    QPushButton *m_btBlackLevelSet;

    QComboBox *m_cbBlackLevelAutoCal;

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
    IpxDisplay *m_display;

    QLabel *m_displayLabel;
    
    //Pointer to IIpxGenParamTreeView Object
    IpxGui::IIpxGenParamTreeView* m_parameterView;
    
    //Retrieve the initial parameter values from the Camera
    void InitFromCamera();

    bool StartGrabbing();

    bool StopGrabbing();

    // help mapper to facilitate development
    IpxGui::IpxCamConfigMapper m_mapData;
};
#endif // QTCHEETAHPYTHONGAINUI_H
