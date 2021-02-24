#include "QtCheetahPythonExposureUI.h"
//Imperx Camera API Functions
#include "IpxCameraApi.h"
//Imperx Image Functions
#include "IpxImage.h"
//Imperx Camera API Functions
#include "IpxCameraGuiApi.h"
//Imperx Display Functions
#include "IpxDisplay.h"

#include <string>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <map>
#include <cmath>

// ===========================================================================
//  QtCheetahPythonExposureUI Constructor
//
//	The Constructor initializes the ThreadDisplay, Camera, IpxDisplay, IpxStream, IpxDeviceInfo
//  modules to null. Then, it loads the Imperx Icon and initializes the System of the
//  GenTL Module Hierachy.
// =============================================================================
QtCheetahPythonExposureUI::QtCheetahPythonExposureUI(QWidget *parent)
    : QMainWindow(parent)
    , m_grabThread(nullptr)
    , m_camera(nullptr)
    , m_display(IpxDisplay::CreateComponent())
    , m_stream(nullptr)
    , m_devInfo(nullptr)
    , m_parameterView(nullptr)
    , m_isConnected (false)
    , m_bNewXml(false)
    , m_lineTime(0)
    , m_pixClock(0)
{
	ui.setupUi(this);

    CreateLayout();

    SetupToolbarConnects();

    UpdateToolbarButtons();

    // Gets The System object that is the entry point to the GenTL Producer software driver
    m_system = IpxCam::IpxCam_GetSystem();

    // Initialize the display
    if(m_display)
    {
        m_display->Initialize(m_displayLabel);
        ZoomFitToWnd();
    }
    else
    {
        QMessageBox::critical(this, "", "ERROR: Unable to create the Display Object.");
    }
}
// =================================GuiManager's ============================================
// The QtCheetahPythonExposureUI Destructor
//
//	The Destructor is Delete the IpxDisplay Component Module
// =============================================================================
QtCheetahPythonExposureUI::~QtCheetahPythonExposureUI()
{
    // Deletes the grabbing thread
    if(m_grabThread)
        delete m_grabThread;

    // Deletes the IpcDisplay Component Module object
    if(m_display)
    {
        IpxDisplay::DeleteComponent(m_display);
        //Reset the pointer to null
        m_display = nullptr;
    }
    if(m_displayLabel)
        delete m_displayLabel;
}

void QtCheetahPythonExposureUI::cameraConnect()
{
    if(!m_system)
        m_system = IpxCam::IpxCam_GetSystem();

    // Open Discovery dialog
    IpxGui::IpxCameraSelectorDialog cameraSelectionDlg(m_system);
    IpxCam::DeviceInfo *devInfo = nullptr;
    if(cameraSelectionDlg.exec() == QDialog::Accepted && (devInfo = cameraSelectionDlg.selectedCamera()))
    {
    }
}

bool QtCheetahPythonExposureUI::SetupToolbarConnects()
{
    // Connect the Toolbar GUI signals with the slots
    connect(ui.actionConnect, SIGNAL(triggered(bool)), this, SLOT(OnBnClickedConnectButton()));
    connect(ui.actionDisconnect, SIGNAL(triggered(bool)), this, SLOT(OnBnClickedDisconnectButton()));
    connect(ui.actionPlay, SIGNAL(triggered(bool)), this, SLOT(OnBnClickedStart()));
    connect(ui.actionStop, SIGNAL(triggered(bool)), this, SLOT(OnBnClickedStop()));
    connect(ui.actionGenICamTree, SIGNAL(triggered(bool)), this, SLOT(OnBnClickedGenICamButton()));
    connect(ui.actionZoomIn, SIGNAL(triggered(bool)), this,   SLOT(OnZoomIn()));
    connect(ui.actionZoomOut, SIGNAL(triggered(bool)), this, SLOT(OnZoomOut()));
    connect(ui.actionFitToWindow, SIGNAL(triggered(bool)), this,  SLOT(OnZoomFitToWnd()));
    connect(ui.actionActualSize, SIGNAL(triggered(bool)), this, SLOT(OnZoom100()));
    connect(ui.actionSpreadToWindow, SIGNAL(triggered(bool)), this, SLOT(OnZoomSpreadToWnd()));
    connect(ui.actionCenterImage, SIGNAL(triggered(bool)), this,  SLOT(OnCenterImage()));
	return true;
}
// =============================================================================
// UpdateToolBarButtons will enable/disable the Toolbar buttons according the connection state
// =============================================================================
void QtCheetahPythonExposureUI::UpdateToolbarButtons()
{
    // Update the toolbar according the camera state
    if(m_isConnected)
    {

        QList<QAction*> actions = (ui).toolBarFile->actions();
        QList<QAction*>::iterator ia;
        for (ia = actions.begin(); ia != actions.end(); ia++)
        {
           (*ia)->setEnabled(true);
           if((*ia)->objectName() == "actionConnect")
               (*ia)->setEnabled(false);
           if((*ia)->objectName() == "actionStop")
               (*ia)->setEnabled(false);
        }

        //Disable Buttons
        actions = (ui).toolBarDisplay->actions();
        for (ia = actions.begin(); ia != actions.end(); ia++)
        {
           (*ia)->setEnabled(true);
        }
        actions = (ui).toolBarView->actions();
        for (ia = actions.begin(); ia != actions.end(); ia++)
        {
           (*ia)->setEnabled(true);
        }

        if(!m_grabThread)
        {
            ui.actionPlay->setEnabled(true);
            ui.actionStop->setEnabled(false);
        }
        else
        {   if(!m_grabThread->IsStarted())
            {
                ui.actionPlay->setEnabled(true);
                ui.actionStop->setEnabled(false);
            }
            else
            {
                ui.actionPlay->setEnabled(false);
                ui.actionStop->setEnabled(true);
            }
        }
    }
    else
    {
        QList<QAction*> actions = (ui).toolBarFile->actions();
        QList<QAction*>::iterator ia;
        for (ia = actions.begin(); ia != actions.end(); ia++)
        {
           (*ia)->setEnabled(false);
           if((*ia)->objectName() == "actionConnect")
               (*ia)->setEnabled(true);
        }

        //Disable Buttons
        actions = (ui).toolBarDisplay->actions();
        for (ia = actions.begin(); ia != actions.end(); ia++)
        {
           (*ia)->setEnabled(false);
        }
        actions = (ui).toolBarView->actions();
        for (ia = actions.begin(); ia != actions.end(); ia++)
        {
           (*ia)->setEnabled(false);
        }
        ui.actionPlay->setEnabled(false);
        ui.actionStop->setEnabled(false);

    }
    EnableControls(m_isConnected);
}
void QtCheetahPythonExposureUI::closeEvent(QCloseEvent * event)
{
    if(m_isConnected)
            Disconnect();
    // Delete the grabbing thread
    if(m_grabThread)
        delete m_grabThread;

    m_grabThread = nullptr;

    // Delete Display object, since main Window to be closed
    if(m_display)
    {
        IpxDisplay::DeleteComponent(m_display);
        m_display = nullptr;
    }

    QMainWindow::closeEvent(event);
}
// =============================================================================
// OnBnClickedConnectButton will call Connect() function that connects the
// camera and enables the dialog box to be able to perform the acquisition and
// control commands
// =============================================================================
void QtCheetahPythonExposureUI::OnBnClickedConnectButton()
{
    //connects the camera and enables the dialog box to be able to perform actions
    Connect();
}

// =============================================================================
// Connect the Imperx Camera
// =============================================================================
void QtCheetahPythonExposureUI::Connect()
{
    if(m_stream)
        m_stream->Release();
    m_stream = nullptr;

    if(m_camera)
        m_camera->Release();
    m_camera = nullptr;

    // Open camera selection dialog
    m_devInfo = IpxGui::SelectCameraW(m_system, L"Get Device Selection");
    // Check if any camera was selected
    if(m_devInfo == nullptr)
        return;

    // Create the Device object from DeviceInfo of selected Camera
    m_camera = IpxCam::IpxCam_CreateDevice(m_devInfo);
    if(!m_camera)
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve camera device.\nMake sure an Imperx Python Camera is connected.\r\n" );
        return;
    }
    // Get the camera basic information and show it on GUI
    IpxCamErr err=IPX_CAM_ERR_OK;
    auto camParamArray = m_camera->GetCameraParameters();

    //Register the Event Sink
    IpxGenParam::Param* param;

    //Test if the 'ExposureTime' parameter exist in the Xml. If so,
    //use the new XML
    m_bNewXml = (camParamArray->GetParam("ExposureTime", &err))? true:false;

    if(m_bNewXml)
    {
        param = camParamArray->GetParam("ExposureTime", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        {
            QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx Python Camera is connected.\r\n" );
            return;
        }
        param->RegisterEventSink(this);
        param = camParamArray->GetParam("AcquisitionFrameTime", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        {
            QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx Python Camera is connected.\r\n" );
            return;
        }
        param->RegisterEventSink(this);
    }
    else
    {
        //Register the Event Sink for all the parameter fields that can perform callbacks
        param = camParamArray->GetParam("Frame_ExposureTime", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        {
            QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx Python Camera is connected.\r\n" );
            return;
        }
        param->RegisterEventSink(this);
        param = camParamArray->GetParam("Frame_FixedFramePeriodValue", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        {
            QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx Python Camera is connected.\r\n" );
            return;
        }
        param->RegisterEventSink(this);
    }

    IpxGenParam::String* lManufacturer = camParamArray->GetString("DeviceVendorName", &err);
    IpxGenParam::String* lModelName = camParamArray->GetString("DeviceModelName", &err);
    IpxGenParam::String* lNameParam = camParamArray->GetString("DeviceUserDefinedName", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        lNameParam = camParamArray->GetString("DeviceUserID", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "ERROR", "DeviceUserID set failed.\r\n" );
    }

    IpxGenParam::String* lDeviceFamily = camParamArray->GetString("DeviceFamilyName", &err);
    IpxGenParam::String* lVersion = camParamArray->GetString("DeviceVersion", &err);
    IpxGenParam::String* lSerialNbr = camParamArray->GetString("DeviceSerialNumber", &err);

    QString lModelNameStr((lModelName != 0) ? lModelName->GetValue() : "");
    QString lNameStr((lNameParam != 0) ? lNameParam->GetValue() : "");
    QString lManufacturerStr((lManufacturer != 0) ? lManufacturer->GetValue() : "");
    QString lDeviceFamilyStr((lDeviceFamily != 0) ? lDeviceFamily->GetValue() : "");
    QString lVersionStr((lVersion != 0) ? lVersion->GetValue() : "");
    QString lSerialNbrStr((lSerialNbr != 0) ? lSerialNbr->GetValue() : "" );

    //Fill the edit boxes with the information retrieved from the camera
    mModelEdit->setText(lModelNameStr);
    mNameEdit->setText(lNameStr);
    mManufacturerEdit->setText(lManufacturerStr);
    mDeviceFamilyEdit->setText(lDeviceFamilyStr);
    mVersionEdit->setText(lVersionStr);
    mSerialNbrEdit->setText(lSerialNbrStr);

    if(m_bNewXml)
    {
        m_mapData.mapEnumParam(camParamArray->GetEnum("ExposureMode", nullptr), m_cbExpMode);
        m_mapData.mapBooleanParam(camParamArray->GetBoolean("AcquisitionFrameRateEnable", nullptr), m_chFFPEnable);
    }
    else
    {
       m_mapData.mapEnumParam(camParamArray->GetEnum("Frame_ExposureControl", nullptr), m_cbExpMode);
       m_mapData.mapBooleanParam(camParamArray->GetBoolean("Frame_FixedFramePeriodEnable", nullptr), m_chFFPEnable);
    }

    // connect check box with controls
    m_edFFPVal->setEnabled(m_chFFPEnable->isChecked());
    m_edFPS->setEnabled(m_chFFPEnable->isChecked());
    QObject::connect(m_chFFPEnable, &QCheckBox::stateChanged,
        [this](int state){
            bool checked = true;
            if ((Qt::CheckState)state == Qt::Unchecked)
                checked = false;

            m_edFFPVal->setEnabled(checked);
            m_edFPS->setEnabled(checked);
    });

    QString notAvailable(QStringLiteral("N/A"));

    // Get Exposure Time
    int64_t expTime = 0;
    if(m_bNewXml)
        expTime = camParamArray->GetFloatValue("ExposureTime", &err);
    else
        expTime = camParamArray->GetIntegerValue("Frame_ExposureTime", &err);

    if (err == IPX_CAM_ERR_OK)
    {
        m_edExpTime->setText(QString::number(expTime));
        m_edExpTime->setEnabled(true);
    }
    else
    {
        m_edExpTime->setText(notAvailable);
        m_edExpTime->setEnabled(false);
    }

    // Get Fixed Frame Period
    int64_t ffpVal_us=0;
    if(m_bNewXml)
        ffpVal_us = camParamArray->GetIntegerValue("AcquisitionFrameTime", &err);
    else
        ffpVal_us = camParamArray->GetIntegerValue("Frame_FixedFramePeriodValue", &err);

    if (err == IPX_CAM_ERR_OK)
    {
        m_edFFPVal->setText(QString::number(ffpVal_us));

        // Init FPS field
        double fpsVal = 1000000./ffpVal_us;
        m_edFPS->setText(QString::number(fpsVal, 'f', 1));
    }
    else
    {
        m_edFFPVal->setText(notAvailable);
        m_edFPS->setText(notAvailable);
    }

    // Create the Buffers queue for video acquisition
    CreateStreamBuffer();

    // Create the grabbing thread
    m_grabThread = new GrabbingThread(m_stream, m_display);

    //Set State of Imperx Camera connect
    m_isConnected = true; //Imperx Camera is connected

    // Update the toolbar
    UpdateToolbarButtons();
}

// =============================================================================
// OnBnClickedDisconnectButton will call DisConnect() function that disconnects
// the camera and disables the dialog box to be able to perform the acquisition
// and control commands
// =============================================================================
void QtCheetahPythonExposureUI::OnBnClickedDisconnectButton()
{
    //Disconnects the camera and disables the dialog box to be able to perform any actions
    Disconnect();
}

// =============================================================================
// Disconnect the Imperx Camera
// =============================================================================
void QtCheetahPythonExposureUI::Disconnect()
{
    IpxCamErr err;

    // Unregister all events of the parameters in the device's node map
    auto paramCamArray = m_camera->GetCameraParameters();

    // crear mapper in case is not empty
    m_mapData.clear(paramCamArray);

    IpxGenParam::Param* param;

    if(m_bNewXml)
    {
        //Unregister the event sink
        param = paramCamArray->GetParam("ExposureTime", &err);
        param->UnregisterEventSink(this);
        param = paramCamArray->GetParam("AcquisitionFrameTime", &err);
        param->UnregisterEventSink(this);
    }
    else
    {
        //Unregister the event sink
        param = paramCamArray->GetParam("Frame_ExposureTime", &err);
        param->UnregisterEventSink(this);
        param = paramCamArray->GetParam("Frame_FixedFramePeriodValue", &err);
        param->UnregisterEventSink(this);
    }

    mModelEdit->setText("");
    mNameEdit->setText("");
    mManufacturerEdit->setText("");
    mDeviceFamilyEdit->setText("");
    mVersionEdit->setText("");
    mSerialNbrEdit->setText("");

    m_edExpTime->setEnabled(false);
    m_edFFPVal->setEnabled(false);
    m_edFPS->setEnabled(false);

    // Stop the Acquisition
    if(m_grabThread)
    {
        if(m_grabThread->IsStarted())
        {
            OnBnClickedStop();
        }

        delete m_grabThread;
        m_grabThread = nullptr;
    }

    //Set the state of the Imperx Camera
    m_isConnected = false;  //Imperx Camera is Disconnected

    // Update the toolbar
    UpdateToolbarButtons();

    // Destroy the parameter view
    if(m_parameterView)
    {
        m_parameterView->clearParams();
        DestroyGenParamTreeView(m_parameterView);
        m_parameterView = nullptr;
    }

    // Release the Stream object
    if(m_stream)
        m_stream->Release();
    m_stream = nullptr;

    // Release the Camera Device object
    if(m_camera)
        m_camera->Release();
    m_camera = nullptr;
}

// =============================================================================
// OnBnClickedGenICamButton creates/generates the Parameter Tree View of the
// Camera
// =============================================================================
void QtCheetahPythonExposureUI::OnBnClickedGenICamButton()
{
    if(m_parameterView)
    {
        m_parameterView->clearParams();
        DestroyGenParamTreeView(m_parameterView);
    }
    m_parameterView = IpxGui::CreateGenParamTreeViewForArrayW(m_camera->GetCameraParameters(), L"Camera Parameters - Gen<i>Cam");
}
// =============================================================================
// CreateStreamBuffer will create the stream buffer queue
// =============================================================================
void QtCheetahPythonExposureUI::CreateStreamBuffer()
{
    struct CreateStreamBuffer_error : std::runtime_error
    {
        CreateStreamBuffer_error()
            : std::runtime_error("ERROR: Start grab failed")
        {}
    };
    //Check if the Camera Device Object exists
    if ( m_camera != nullptr)
    {
        try
        {
            // Get first Stream from the Camera
            if(m_camera->GetNumStreams() < 1)
                throw CreateStreamBuffer_error();
            if(!m_stream)
                m_stream = m_camera->GetStreamByIndex(0);
            if(!m_stream)
                throw CreateStreamBuffer_error();

            // Release buffer queue, just in case, if it was previously allocated
            m_stream->ReleaseBufferQueue();
        }
        catch(const CreateStreamBuffer_error&)
        {
            //Release the Camera Device Object
            m_camera->Release();
            m_camera = nullptr;
            return;
        }
    }
}
bool QtCheetahPythonExposureUI::StartGrabbing()
{
    //
    // Start the video acquisition on Stream
    IpxCamErr err = m_stream->StartAcquisition();
    if(IPX_CAM_ERR_OK!=err)
    {
        QMessageBox::critical(this, "ERROR", "Sorry! Cannot start streaming\r\nCheck if there is enough memory.\r\n"
                                             "Particularly, on Linux machine!");
        return false;
    }
    m_grabThread->Start();

    return true;
}
bool QtCheetahPythonExposureUI::StopGrabbing()
{

    m_grabThread->Stop();

    // Stop the Acquisition on Stream
    IpxCamErr err = m_stream->StopAcquisition(1);
    if(IPX_CAM_ERR_OK != err)
    {
        return false;
    }

    return true;
}
// =============================================================================
// Start the Acquisition engine on the host. It will call the GenTL consumer to
// start the transfer
// =============================================================================
void QtCheetahPythonExposureUI::StartAcquisition()
{
    struct StartAcquisition_error : std::runtime_error
    {
        StartAcquisition_error()
            : std::runtime_error("ERROR: StartAcquisition failed")
        {}
    };

    try
    {
        IpxCamErr err(IPX_CAM_ERR_OK);

        // Allocate the minimal required buffer queue
        err = m_stream->AllocBufferQueue(this, m_stream->GetMinNumBuffers());
        if(err != ((IpxCamErr)0))
            throw StartAcquisition_error();

        // Get camera parrameters array
        auto genParams = m_camera->GetCameraParameters();
        if(genParams)
        {
            // Set the TLParamsLocked parameter to lock the parameters effecting to the Transport Layer
            err = genParams->SetIntegerValue("TLParamsLocked", 1);
            if(err != ((IpxCamErr)0))
                throw StartAcquisition_error();

            // Start the grabbing thread
            StartGrabbing();

            // Run AcquisitionStart command to start the acquisition
            err = genParams->ExecuteCommand("AcquisitionStart");
            if(err != ((IpxCamErr)0))
                throw StartAcquisition_error();
        }
    }
    catch(const StartAcquisition_error&)
    {
        // reset the camera to non-acquisition state
        m_camera->GetCameraParameters()->ExecuteCommand("AcquisitionAbort");
        StopGrabbing();
        m_camera->GetCameraParameters()->SetIntegerValue("TLParamsLocked", 0);
        m_stream->ReleaseBufferQueue();

        //Release the Stream Object
        m_stream->Release();
        m_stream = nullptr;
        //Release the Camera Device Object
        m_camera->Release();
        m_camera = nullptr;
    }
}

// =============================================================================
// Stop the Acquisition engine on the host. It will call the GenTL consumer to
// stop the transfer
// =============================================================================
void QtCheetahPythonExposureUI::StopAcquisition()
{
    // Get camera parrameters array
    auto genParams = m_camera->GetCameraParameters();
    if(genParams)
    {
        // Run AcquisitionStop command to start the acquisition
        IpxCamErr err = genParams->ExecuteCommand("AcquisitionStop");

        // If any error occured with AcquisitionStop, run AcquisitionAbort
        if(err != IPX_CAM_ERR_OK )
            genParams->ExecuteCommand("AcquisitionAbort");

        StopGrabbing();

        // Reset the TLParamsLocked parameter to unlock the parameters effecting to the Transport Layer
        genParams->SetIntegerValue("TLParamsLocked", 0);

        m_stream->ReleaseBufferQueue();
    }

}

// =============================================================================
// OnBnClickedStart will start the video acquistion process
// =============================================================================
void QtCheetahPythonExposureUI::OnBnClickedStart()
{
    //Check if the Imperx Camera is Connected
    if ( !m_isConnected)
    {
        return;
    }

    // Start the video acquisition
    StartAcquisition();

    UpdateToolbarButtons();

}

// =============================================================================
// OnBnClickedStop will stop the video acquisition process
// =============================================================================
void QtCheetahPythonExposureUI::OnBnClickedStop()
{
    //Check if the Imperx Camera is Connected
    if ( !m_isConnected )
    {
        return;
    }

    // Stop the video acquisition
    StopAcquisition();

    // Update the Toolbar state
    UpdateToolbarButtons();
}


bool QtCheetahPythonExposureUI::ZoomIn()
{
    return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_IN));
}

bool QtCheetahPythonExposureUI::ZoomOut()
{
  return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_OUT));
}

bool QtCheetahPythonExposureUI::Zoom100()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 3));
}

bool QtCheetahPythonExposureUI::ZoomFitToWnd()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 1));
}

bool QtCheetahPythonExposureUI::ZoomSpreadToWnd()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 2));
}

bool QtCheetahPythonExposureUI::CenterImage()
{
   return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ATCENTER));
}
void QtCheetahPythonExposureUI::CreateLayout()
{
    QGroupBox *lConnectionBox = CreateConnectGroup();
    QGroupBox *lExposureBox = CreateExposure();

    QVBoxLayout *lLayoutLeft = new QVBoxLayout();
    lLayoutLeft->addWidget( lConnectionBox, Qt::AlignTop );
    lLayoutLeft->addStretch();
    lLayoutLeft->addWidget( lExposureBox, Qt::AlignTop );

    m_displayLabel = new QLabel;
    m_displayLabel->setMinimumSize(240, 160);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *displayLayout = new QVBoxLayout;
    displayLayout->addWidget(m_displayLabel);

    QGroupBox *lDisplayBox = new QGroupBox(tr("Display"));
    lDisplayBox->setLayout(displayLayout);
    lDisplayBox->setMinimumWidth(600);

    QHBoxLayout *lMainLayout = new QHBoxLayout;
    lMainLayout->addLayout( lLayoutLeft, Qt::AlignLeft );
    lMainLayout->addWidget( lDisplayBox);

    QFrame *lMainBox = new QFrame;
    lMainBox->setLayout( lMainLayout );

    setCentralWidget( lMainBox );
    setWindowTitle( tr( "QtCheetahPythonExposureUI" ) );

}
QGroupBox *QtCheetahPythonExposureUI::CreateConnectGroup()
{
    QLabel *lManufacturerLabel = new QLabel( tr( "Manufacturer" ) );
    mManufacturerEdit = new QLineEdit;
    mManufacturerEdit->setReadOnly( true );
    mManufacturerEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lDeviceFamilyLabel = new QLabel( tr( "Device Family" ) );
    mDeviceFamilyEdit = new QLineEdit;
    mDeviceFamilyEdit->setReadOnly( true );
    mDeviceFamilyEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lModelLabel = new QLabel( tr( "Model" ) );
    mModelEdit = new QLineEdit;
    mModelEdit->setReadOnly( true );
    mModelEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lNameLabel = new QLabel( tr( "Name" ) );
    mNameEdit = new QLineEdit;
    mNameEdit->setReadOnly( true );
    mNameEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lVersionLabel = new QLabel( tr( "Version" ) );
    mVersionEdit = new QLineEdit;
    mVersionEdit->setReadOnly( true );
    mVersionEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lSerialNbrLabel = new QLabel( tr( "SerialNumber" ) );
    mSerialNbrEdit = new QLineEdit;
    mSerialNbrEdit->setReadOnly( true );
    mSerialNbrEdit->setFocusPolicy(Qt::NoFocus);

    QGridLayout *lGridLayout = new QGridLayout;
    lGridLayout->addWidget( lManufacturerLabel, 0, 0 );
    lGridLayout->addWidget( mManufacturerEdit, 0, 1 );
    lGridLayout->addWidget( lDeviceFamilyLabel, 1, 0 );
    lGridLayout->addWidget( mDeviceFamilyEdit, 1, 1 );
    lGridLayout->addWidget( lModelLabel, 2, 0 );
    lGridLayout->addWidget( mModelEdit, 2, 1 );
    lGridLayout->addWidget( lNameLabel, 3, 0 );
    lGridLayout->addWidget( mNameEdit, 3, 1 );
    lGridLayout->addWidget( lVersionLabel, 4, 0 );
    lGridLayout->addWidget( mVersionEdit, 4, 1 );
    lGridLayout->addWidget( lSerialNbrLabel, 5, 0 );
    lGridLayout->addWidget( mSerialNbrEdit, 5, 1 );

    QVBoxLayout *lBoxLayout = new QVBoxLayout;
    lBoxLayout->addLayout( lGridLayout );
    lBoxLayout->addStretch();

    QGroupBox *lConnectionBox = new QGroupBox( tr( "Device Info" ) );
    lConnectionBox->setLayout( lBoxLayout );
    lConnectionBox->setMinimumWidth( 300);
    lConnectionBox->setMaximumWidth( 300 );
    lConnectionBox->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

    return lConnectionBox;
}

///
/// \brief Create Gain group box
///

QGroupBox *QtCheetahPythonExposureUI::CreateExposure()
{
    QLabel *lExposureModeLabel = new QLabel( tr( "Exposure mode:" ) );
    m_cbExpMode = new QComboBox;
    m_cbExpMode->setEnabled( false );

    QLabel *lExposure_usecLabel = new QLabel( tr( "Exposure, usec:" ) );
    m_edExpTime = new QLineEdit;
    m_edExpTime->setReadOnly( false );
    m_edExpTime->setEnabled( false );

    m_bt_exp_set = new QPushButton( tr( "Set" ) );
    m_bt_exp_set->setMinimumHeight( 27 );

    QObject::connect( m_bt_exp_set, SIGNAL( clicked() ), this, SLOT( OnBnClickedBtExpSet() ) );

    m_chFFPEnable = new QCheckBox("Enable Fixed Frame Time");

    QLabel *lFtm_usecLabel = new QLabel( tr( "Frame Time, usec:" ) );
    m_edFFPVal = new QLineEdit;
    m_edFFPVal->setReadOnly( false );
    m_edFFPVal->setEnabled( false );

    m_bt_ftm_set = new QPushButton( tr( "Set" ) );
    m_bt_ftm_set->setMinimumHeight( 27 );

    QObject::connect( m_bt_ftm_set, SIGNAL( clicked() ), this, SLOT( OnBnClickedBtFtmSet() ) );

    QLabel *lFps_usecLabel = new QLabel( tr( "Frame Rate, FPS:" ) );
    m_edFPS = new QLineEdit;
    m_edFPS->setReadOnly( false );
    m_edFPS->setEnabled( false );

    m_bt_fps_set = new QPushButton( tr( "Set" ) );
    m_bt_fps_set->setMinimumHeight( 27 );

    QObject::connect( m_bt_fps_set, SIGNAL( clicked() ), this, SLOT( OnBnClickedBtFpsSet() ) );

    QGridLayout *lExpGridLayout = new QGridLayout;
    lExpGridLayout->addWidget( lExposureModeLabel, 0, 0, 1, 3 );
    lExpGridLayout->addWidget( m_cbExpMode, 0, 4, 1, 3 );

    lExpGridLayout->addWidget( lExposure_usecLabel, 1, 0, 1, 3 );
    lExpGridLayout->addWidget( m_edExpTime, 1, 3, 1, 3 );
    lExpGridLayout->addWidget( m_bt_exp_set, 1, 6, 1, 1);

    QHBoxLayout *lButtonsLayout = new QHBoxLayout;
    lButtonsLayout->addWidget( m_chFFPEnable );

    QGridLayout *lExpGridFrameLayout = new QGridLayout;
    lExpGridFrameLayout->addWidget( lFtm_usecLabel, 0, 0 );
    lExpGridFrameLayout->addWidget( m_edFFPVal, 0, 1, 1, 1 );
    lExpGridFrameLayout->addWidget( m_bt_ftm_set, 0, 2, 1, 1);
    lExpGridFrameLayout->addWidget( lFps_usecLabel, 1, 0 );
    lExpGridFrameLayout->addWidget( m_edFPS, 1, 1, 1, 1 );
    lExpGridFrameLayout->addWidget( m_bt_fps_set, 1, 2, 1, 1);

    QVBoxLayout *lExpLayout = new QVBoxLayout;
    lExpLayout->addLayout( lExpGridLayout );
    lExpLayout->addLayout( lButtonsLayout );
    lExpLayout->addLayout( lExpGridFrameLayout);
    lExpLayout->addStretch();

    QGroupBox *lExposureBox = new QGroupBox( tr( "Exposure" ) );
    lExposureBox->setLayout( lExpLayout );
    lExposureBox->setMinimumWidth( 300 );
    lExposureBox->setMaximumWidth( 300 );

    return lExposureBox;
}
// =============================================================================
// EnableControls will enable/disable the GUI controls
// =============================================================================
void QtCheetahPythonExposureUI::EnableControls(bool en)
{
    m_bt_exp_set->setEnabled(en);
    m_bt_ftm_set->setEnabled(en);
    m_bt_fps_set->setEnabled(en);
    m_chFFPEnable->setEnabled(en);
}

unsigned int QtCheetahPythonExposureUI::GetCurrentExposure()
{
    auto paramCamArray = m_camera->GetCameraParameters();
    IpxCamErr err;
    // ExposureLines value
    int64_t expLines=0;

    //Get the course exposure time in units of lines
    expLines = paramCamArray->GetIntegerValue("Frame_ExposureLines", &err);
    if(err != IPX_CAM_ERR_OK)
        return -1; //ERROR!!!

    m_edLines->setText(QString::number(expLines));
    m_edLines->setEnabled(false);

    // Get Exposure Clocks
    int64_t expClocks=0;

    //Get the fine exposure time in units of clocks
    expClocks = paramCamArray->GetIntegerValue("Frame_ExposureClocks", &err);
    if(err != IPX_CAM_ERR_OK)
        return -1; //ERROR!!!

    m_edClocks->setText(QString::number(expClocks));
    m_edClocks->setEnabled(false);

    // Calculate the Exposure value in microseconds, using Formula:
    double exposure = (double)(m_lineTime*expLines+expClocks)/m_pixClock;
    unsigned int exp_intval = (unsigned int)floor(exposure + 0.5);
    // Set to values GUI fields
    QString strVal;
    strVal.sprintf("%i", exp_intval);
    m_edExpTime->setText(strVal);

        return exp_intval;
}
// =============================================================================
// Updates the Exposure
//
// Note: The expClocks and expLineTimes have been converted from microseconds
// to values
// in Camera LineTimes and Clocks.
// The formula is as follows:
// double exposure = (double)(m_lineTime*expLineTimes+expClocks)/m_pixClock;
// =============================================================================
void  QtCheetahPythonExposureUI::UpdateExposure(unsigned int expVal)
{
    IpxCamErr err;

    //GenICam  parameter value
    auto paramArray = m_camera->GetCameraParameters();

    int64_t cur_integration_time=0;
    if(m_bNewXml)
        cur_integration_time = paramArray->GetFloatValue("ExposureTime", &err);
    else
        cur_integration_time = paramArray->GetIntegerValue("Frame_ExposureTime", &err);
    if(err != IPX_CAM_ERR_OK)
        return; ///ERROR!!!

    int64_t max_exp_time=0;
    max_exp_time = paramArray->GetIntegerValue("Frame_MaxExposureTimeReg", &err);
    if(err != IPX_CAM_ERR_OK)
        return; ///ERROR!!!

    int64_t exp_ctrl=0;
    if(m_bNewXml)
        exp_ctrl = paramArray->GetEnumValue("ExposureMode", &err);
    else
        exp_ctrl = paramArray->GetEnumValue("Frame_ExposureControl", &err);
    if(err != IPX_CAM_ERR_OK)
        return; ///ERROR!!!

    //Address 0x720 (Exposure Control)
    //Data(1:0) 00 - Off
    //          01 - Trigger Pulse Width
    //          10 - Internal
    //          11 - Reserved
    int64_t max_allowed_exp_time;

    if ((exp_ctrl&0x3)==0x1)
        max_allowed_exp_time = 1000000; //max exposure time during trigger mode is 1 sec
    else
        max_allowed_exp_time = max_exp_time;

    if(expVal >= 4 && expVal <= max_allowed_exp_time)
    {
        if(m_bNewXml)
            err = paramArray->SetFloatValue("ExposureTime", (double)expVal);
        else
            err = paramArray->SetIntegerValue("Frame_ExposureTime", (int64_t)expVal);

        if(err != IPX_CAM_ERR_OK)
        {
            QMessageBox::critical(this, "Error","The INTEGRATION_TIME register is invalid");
            return;
        }
    }
    else
    {
        //Replace the field with last valid cur_integration_time value
        m_edExpTime->setText(QString::number(cur_integration_time));
        QString msg;
        msg.sprintf((char*)"The INTEGRATION_TIME register is set to a value of %d.\nThis value is out of the specified range.\nUnable to update", expVal);

        QMessageBox::critical(this, "Error", msg);
    }

}
// =============================================================================
// OnBnClickedBtExpSet updates the Exposure Time
// =============================================================================
void QtCheetahPythonExposureUI::OnBnClickedBtExpSet()
{

    QString strVal = m_edExpTime->text();
    unsigned int val = (unsigned int)strVal.toLongLong();

    UpdateExposure(val);
}
// =============================================================================
// OnBnClickedBtFtmSet updates the Frame_FixedFramePeriodValue
// =============================================================================
void QtCheetahPythonExposureUI::OnBnClickedBtFtmSet()
{
    IpxCamErr err;

    //GenICam  parameter value
    auto paramArray = m_camera->GetCameraParameters();

    int64_t cur_ffpVal_us=0;
    if(m_bNewXml)
        cur_ffpVal_us = paramArray->GetIntegerValue("AcquisitionFrameTime", &err);
    else
        cur_ffpVal_us = paramArray->GetIntegerValue("Frame_FixedFramePeriodValue", &err);
    if(err != IPX_CAM_ERR_OK)
        return; ///ERROR!!!

    int64_t min_frame_time=0;
    min_frame_time = paramArray->GetIntegerValue("Frame_MinFrameTimeReg", &err);
    if(err != IPX_CAM_ERR_OK)
        return; ///ERROR!!!
    QString strVal = m_edFFPVal->text();
    int ffpVal_us = strVal.toInt();
    if(ffpVal_us >= min_frame_time && ffpVal_us <= 1000000)
    {
        if(m_bNewXml)
            err = paramArray->SetIntegerValue("AcquisitionFrameTime",(int64_t)ffpVal_us);
        else
            err = paramArray->SetIntegerValue("Frame_FixedFramePeriodValue",(int64_t)ffpVal_us);

        if(IPX_ERR_SUCCEEDED(err))
        {
            QString strVal;
            strVal.sprintf("%d", ffpVal_us);
            m_edFFPVal->setText(strVal);

            // Set FPS field
            double fpsVal = 1000000./(double)ffpVal_us;
            strVal.sprintf("%.1f", fpsVal);
            m_edFPS->setText(strVal);
        }
        else
        {
            QMessageBox::critical(this, "Error","FRAME_FIXED_FRAME_PERIOD_VALUE set failed");
            return;
        }

    }
    else
    {
        m_edFFPVal->setText(QString::number(cur_ffpVal_us));
        // Set FPS field
        double fpsVal = 1000000./(double)ffpVal_us;
        m_edFPS->setText(QString::number(fpsVal, 'f', 1));
        QString msg;
        msg.sprintf((char*)"The FRAME_FIXED_FRAME_PERIOD_VALUE is set to a value of %d.\nThis value is out of the specified range.\nUnable to update", ffpVal_us);
        QMessageBox::critical(this, "Error", msg);
    }
}
// =============================================================================
// OnBnClickedBtFpsSet retrieves the current FFP value in microseconds. Then,
// it calculates the FFP in LineTimes. The Frame_FixedFramePeriodValue field
// is updated with the FFP in LineTimes value and then displays the FFP real
// value in microseconds
// =============================================================================
void QtCheetahPythonExposureUI::OnBnClickedBtFpsSet()
{
    IpxCamErr err;

    //GenICam  parameter value
    auto paramArray = m_camera->GetCameraParameters();

    int64_t old_ffpVal_us=0;
    if(m_bNewXml)
        old_ffpVal_us = paramArray->GetIntegerValue("AcquisitionFrameTime", &err);
    else
        old_ffpVal_us = paramArray->GetIntegerValue("Frame_FixedFramePeriodValue", &err);
    if(err != IPX_CAM_ERR_OK)
        return; ///ERROR!!!

    // Get FPS value
    QString fpsSTR;
    QString strVal = m_edFPS->text();
    double fps = (unsigned int)strVal.toDouble();
    if(fps == 0)
        return;

    int64_t min_frame_time=0;
    min_frame_time = paramArray->GetIntegerValue("Frame_MinFrameTimeReg", &err);
    if(err != IPX_CAM_ERR_OK)
    {
        QMessageBox::critical(this, "Error", "Error Reading Register Frame_MinFrameTimeReg failed");
        return; ///ERROR!!!
    }

    // Get FFP value in microseconds
    unsigned int ffpVal_us = (unsigned int)(1000000 / fps);

    if(ffpVal_us >= min_frame_time && ffpVal_us <= 1000000)
    {
        if(m_bNewXml)
            err = paramArray->SetIntegerValue("AcquisitionFrameTime",ffpVal_us);
        else
            err = paramArray->SetIntegerValue("Frame_FixedFramePeriodValue",ffpVal_us);

        if(IPX_ERR_SUCCEEDED(err))
        {
            QString strVal;
            strVal.sprintf("%d", ffpVal_us);
            m_edFFPVal->setText(strVal);

            // Set FPS field
            double fpsVal = 1000000./(double)ffpVal_us;
            strVal.sprintf("%.1f", fpsVal);
            m_edFPS->setText(strVal);
        }
        else
        {
            QMessageBox::critical(this, "Error", "The Frame_FixedFramePeriodValue register is invalid");
            return;
        }
    }
    else
    {
        // Set FPS field
        m_edFPS->setText(QString::number(fps, 'f', 1));
        m_edFFPVal->setText(QString::number(old_ffpVal_us));
        QString msg;
        msg.sprintf("The value of %.1f fps is not valid.\nThis value is out of the specified range.\nUnable to update", fps);
        QMessageBox::critical(this, "Error", msg);
    }
}
// =============================================================================
// OnParameterUpdate occurs during an event callback
// =============================================================================
void QtCheetahPythonExposureUI::OnParameterUpdate(IpxGenParam::Param* param)
{
    if(m_camera)
    {
        if(param->GetType() == IpxGenParam::ParamInt)
        {
            IpxGenParam::Int *parameter = dynamic_cast<IpxGenParam::Int *>(param);
            if (parameter && parameter->IsReadable())
            {
                const char *paramName = parameter->GetDisplayName();
                int64_t value = parameter->GetValue();

                if ((strcmp("Frame_FixedFramePeriodValue", paramName)  == 0) || // old XML
                    (strcmp("AcquisitionFrameTime", paramName)  == 0)) // new  XML
                {
                      m_edFFPVal->setText(QString::number(value));

                      // Set FPS field
                      double fpsVal = 1000000./(double)value;
                      m_edFPS->setText(QString::number(fpsVal,'f', 1));
                }
                else if (strcmp("Frame_ExposureTime", paramName)  == 0) // old XML
                      m_edExpTime->setText(QString::number(value));

            }
        }
        else if(param->GetType() == IpxGenParam::ParamFloat)
        {
            IpxGenParam::Float *parameter = dynamic_cast<IpxGenParam::Float *>(param);
            if (parameter && parameter->IsAvailable())
            {
                const char *paramName = parameter->GetDisplayName();
                double fvalue = parameter->GetValue();

                if ((strcmp("ExposureTime", paramName)  == 0))  //new XML
                {
                    m_edExpTime->setText(QString::number((int)fvalue));
                    m_edExpTime->setEnabled(true);
                }
            }
            else
            {
                m_edExpTime->setText(QStringLiteral("N/A"));
                m_edExpTime->setEnabled(false);
            }
        }
    }
}

