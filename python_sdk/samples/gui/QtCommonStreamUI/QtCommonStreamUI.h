#ifndef QTCOMMONSTREAMUI_H
#define QTCOMMONSTREAMUI_H

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
class QGroupBox;
class QLineEdit;
class QCloseEvent;
//-------------------------------------------------------------------//

class QtCommonStreamUI : public QMainWindow
{
    Q_OBJECT

public:
    QtCommonStreamUI(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent * event);

private:
    // Create controls ans layout
    void CreateLayout();
    QGroupBox *CreateInfoGroup();

    // Connect/Disconnect the camera
    void Connect();
    void Disconnect();

    // Start/Stop the Acquisition
    void StartAcquisition();
    void StopAcquisition();

    // Enable/Disable actions
    void UpdateToolbarButtons();

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

    IpxCam::System *m_system;
    IpxCam::Device *m_camera;
    IpxCam::Stream *m_stream;
    IpxCam::DeviceInfo *m_devInfo;
    GrabbingThread *m_grabThread;
    IpxDisplay *m_display;

    // Pointer to IIpxGenParamTreeView Object
    IpxGui::IpxGenParamTreeView *m_parameterView;

    // Indicates whether or not Camera is connected
    bool m_isConnected;
};

#endif // QTCOMMONSTREAMUI_H
