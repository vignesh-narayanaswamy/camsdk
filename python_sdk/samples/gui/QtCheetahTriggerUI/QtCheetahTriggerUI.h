#ifndef QTCHEETAHTRIGGERUI_H
#define QTCHEETAHTRIGGERUI_H

#include "ui_imperx.h"

#include <QMainWindow>

#ifndef IPXCAM_CALL
    #ifdef _WIN32
        #define IPXCAM_CALL __stdcall
    #else
        #define IPXCAM_CALL
    #endif
#endif // IPXCAM_CALL

//----------------------- forward decloration -----------------------//
namespace IpxCam { class System; class Device; class Stream; class DeviceInfo; }
namespace IpxGui { class IpxGenParamTreeView; class IpxCamConfigMapper; }

class IpxDisplay;
class GrabbingThread;

class QLabel;
class QTimer;
class QGroupBox;
class QLineEdit;
class QComboBox;
class QPushButton;
class QCloseEvent;
//-------------------------------------------------------------------//

class QtCheetahTriggerUI : public QMainWindow
{
	Q_OBJECT

public:
    QtCheetahTriggerUI(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent * event);

private:
    // Create controls ans layout
    void CreateLayout();
    QGroupBox *CreateInfoGroup();
    QGroupBox *CreateTriggerGroup();

    // Connect/Disconnect the camera
    void Connect();
    void Disconnect();

    // Start/Stop the Acquisition
    void StartAcquisition();
    void StopAcquisition();

    // Enable/Disable actions and controls
    void UpdateToolbarButtons();
    void EnableControls(bool en);

    // Callback on Connection Lost event
    static void IPXCAM_CALL CameraDisconnectCallback(const void *eventData, size_t eventSize, void *pPrivate);

Q_SIGNALS:
    void cameraDisconnected();
    
private:
	Ui::IpxPlayerClass ui;

    // Camera Information group box
    QLineEdit *mManufacturerEdit;
    QLineEdit *mDeviceFamilyEdit;
    QLineEdit *mModelEdit;
    QLineEdit *mNameEdit;
    QLineEdit *mVersionEdit;
    QLineEdit *mSerialNbrEdit;

    // Placeholder for displaying
    QLabel *m_displayLabel;

    // Trigger group box
    QComboBox *mTriggerMode;
    QComboBox *mTriggerSource;
    QComboBox *mTriggerActivation;
    QComboBox *mTriggerDebounce;
    QPushButton *mFlashSWTrigger;
    QTimer *m_timer;

    IpxCam::System *m_system;
    IpxCam::Device *m_camera;
    IpxCam::Stream *m_stream;
    IpxCam::DeviceInfo *m_devInfo;
    GrabbingThread *m_grabThread;
    IpxDisplay *m_display;

    // Pointer to IIpxGenParamTreeView Object
    IpxGui::IpxGenParamTreeView *m_parameterView;

    // Help mapper to facilitate development
    IpxGui::IpxCamConfigMapper *m_mapData;

    // Indicates whether or not Camera is connected
    bool m_isConnected;
};

#endif // QTCHEETAHTRIGGERUI_H
