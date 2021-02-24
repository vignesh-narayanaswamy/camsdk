#ifndef QTCHEETAHPYTHONEXPOSUREUI_H
#define QTCHEETAHPYTHONEXPOSUREUI_H

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
#include "IpxCameraGuiApi.h"
#include "IpxDisplay.h"
#include "../common/inc/GrabbingThread.h"
#include "../common/inc/ipxparammaphelper.h"

class QtCheetahPythonExposureUI : public QMainWindow, public IpxGenParam::ParamEventSink
{
	Q_OBJECT

public:
    QtCheetahPythonExposureUI(QWidget *parent = 0);
    ~QtCheetahPythonExposureUI();

    //Create the stream buffer
    void CreateStreamBuffer();

    //Toolbar Features
    bool SetupToolbarConnects(); //Setup the Toolbar Connections
    void UpdateToolbarButtons(); //Updates the Toolbar Buttons

    //Close Event
    virtual void closeEvent(QCloseEvent * event);

    //Establish connection/disconnection with the Imperx Camera
    void Connect();     //Connect to the Camera
    void Disconnect();  //Disconnect the Camera

    //Acquisition Features
    void StartAcquisition();   //Start the Acquisition of the Stream
    void StopAcquisition();    //Stops the Acquisition of the Stream

    //Zoom Features
    bool ZoomIn();
    bool ZoomOut();
    bool Zoom100();
    bool ZoomFitToWnd();
    bool ZoomSpreadToWnd();
    bool CenterImage();

    void EnableControls(bool en);

    // UI
    void CreateLayout();
    QGroupBox *CreateConnectGroup();
    QGroupBox *CreateExposure();

    unsigned int GetCurrentExposure();
    void UpdateExposure(unsigned int expVal);
    void OnParameterUpdate(IpxGenParam::Param* param);

protected Q_SLOTS:
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
    void OnBnClickedBtExpSet();
    void OnBnClickedBtFtmSet();
    void OnBnClickedBtFpsSet();
private:
	Ui::IpxPlayerClass ui;

    //Connect Group (Camera Information)
    QLineEdit *mManufacturerEdit;
    QLineEdit *mDeviceFamilyEdit;
    QLineEdit *mModelEdit;
    QLineEdit *mNameEdit;
    QLineEdit *mVersionEdit;
    QLineEdit *mSerialNbrEdit;

    QComboBox *m_cbExpMode;

    QPushButton *mPlayButton;
    QPushButton *mStopButton;

    QCheckBox *m_chFFPEnable;

    QLineEdit *m_edExpTime;
    QPushButton *m_bt_exp_set;

    QLineEdit* m_edFFPVal;
    QPushButton *m_bt_ftm_set;

    QLineEdit *m_edFPS;
    QPushButton *m_bt_fps_set;

    QLineEdit *m_edLines;
    QLineEdit *m_edClocks;
    int64_t m_lineTime;
    int64_t m_pixClock;

    //Indicates whether or not Camera is connected
    bool m_isConnected;

    IpxCam::System* m_system;      //Pointer to IpxCam System Object
    IpxCam::Device* m_camera;      //Pointer to IpxCam Device Object
    IpxCam::Stream* m_stream;      //Pointer to IpxCam Stream Object
    IpxCam::DeviceInfo* m_devInfo; //Pointer to IpxCam Device Info Object

    GrabbingThread *m_grabThread;  //Pointer to GrabbingThread Object
    IpxDisplay     *m_display;     //Pointer to IpxDisplay Object
    QLabel *m_displayLabel;

    IpxGui::IIpxGenParamTreeView* m_parameterView; //Pointer to IIpxGenParamTreeView Object

    bool m_bNewXml;

    bool StartGrabbing();
    bool StopGrabbing();

    // help mapper to facilitate development
    IpxGui::IpxCamConfigMapper m_mapData;
};

#endif // QTCHEETAHPYTHONEXPOSUREUI_H
