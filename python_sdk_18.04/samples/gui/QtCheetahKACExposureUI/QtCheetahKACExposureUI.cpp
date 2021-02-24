#include "QtCheetahKACExposureUI.h"
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
//  QtCheetahKACExposureUI Constructor
//
//	The Constructor initializes the ThreadDisplay, Camera, IpxDisplay, IpxStream, IpxDeviceInfo
//  modules to null. Then, it loads the Imperx Icon and initializes the System of the
//  GenTL Module Hierachy.
// =============================================================================
QtCheetahKACExposureUI::QtCheetahKACExposureUI(QWidget *parent)
    : QMainWindow(parent)
    , m_grabThread(nullptr)
    , m_camera(nullptr)
    , m_display(IpxDisplay::CreateComponent())
    , m_stream(nullptr)
    , m_devInfo(nullptr)
    , m_parameterView(nullptr)
    , m_isConnected(false)
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
// =============================================================================
// The QtCheetahKACExposureUI Destructor
//
//	The Destructor is Delete the IpxDisplay Component Module
// =============================================================================
QtCheetahKACExposureUI::~QtCheetahKACExposureUI()
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

void QtCheetahKACExposureUI::cameraConnect()
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

bool QtCheetahKACExposureUI::SetupToolbarConnects()
{
    // Connect the GUI signals with slots
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

void QtCheetahKACExposureUI::closeEvent(QCloseEvent * event)
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
// Connect the Imperx Camera
// =============================================================================
void QtCheetahKACExposureUI::Connect()
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

    // Get the camera basic information and show it on GUI
    IpxCamErr err=IPX_CAM_ERR_OK;
    auto camParamArray = m_camera->GetCameraParameters();

    //Register the Event Sink
    IpxGenParam::Param* param;

    if(!camParamArray)
    {
        QMessageBox::critical(this, "ERROR", "GetCameraParameters Error. This sample support KAC Imperx Cameras Only.\r\n" );
        return;
    }

    param = camParamArray->GetParam("Frame_FixedFramePeriodEnable", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->RegisterEventSink(this);
    param = camParamArray->GetParam("Frame_FixedFramePeriodValue", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->RegisterEventSink(this);
    param = camParamArray->GetParam("Frame_ExposureControl", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->RegisterEventSink(this);
    param = camParamArray->GetParam("Frame_LineTime", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->RegisterEventSink(this);
    param = camParamArray->GetParam("Frame_ExposureLines", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->RegisterEventSink(this);
    param = camParamArray->GetParam("Frame_ExposureClocks", &err);
    param->RegisterEventSink(this);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }

    IpxGenParam::String* lModelName = camParamArray->GetString("DeviceModelName", &err);
    IpxGenParam::String* lNameParam = camParamArray->GetString("DeviceUserDefinedName", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        lNameParam = camParamArray->GetString("DeviceUserID", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "ERROR", "DeviceUserID set failed.\r\n" );
    }

    IpxGenParam::String* lManufacturer = camParamArray->GetString("DeviceManufacturerName", &err);
    if(!lManufacturer)
        lManufacturer = camParamArray->GetString("DeviceVendorName", &err);
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

    // Exposure mode
    CreateEnumComboBox(m_cbExpMode, (char*)"Frame_ExposureControl", &m_mapExpModeValues);

    // Get Frame_LineTime
    m_lineTime = camParamArray->GetIntegerValue("Frame_LineTime", &err);
    if(err != IPX_CAM_ERR_OK)
        return; //ERROR!!!

    // Get PixelClk
    m_pixClock = camParamArray->GetIntegerValue("PixelClock", &err);
    if(err != IPX_CAM_ERR_OK)
        return; //ERROR!!!

    // Get Fixed Frame period enable bit
    bool ffpEn=0;
    ffpEn = camParamArray->GetBooleanValue("Frame_FixedFramePeriodEnable", &err);
    if(err != IPX_CAM_ERR_OK)
        return; ///ERROR!!!

    m_chFFPEnable->setChecked(ffpEn?true:false);

    // Get Fixed Frame Period
    int64_t ffpVal_LT=0;
    ffpVal_LT = camParamArray->GetIntegerValue("Frame_FixedFramePeriodValue", &err);
    if(err != IPX_CAM_ERR_OK)
        return; ///ERROR!!!


    // Get Fixed Frame Period
    m_lineTime = camParamArray->GetIntegerValue("Frame_LineTime", &err);
    if(err != IPX_CAM_ERR_OK)
        return; ///ERROR!!!


    // Fixed Frame Period (FFP) register keeps FFP value as Number of LineTimes,
    // So, to convert it to microseconds, we should use following formula:
    double ffpVal_us = (double)ffpVal_LT*(double)m_lineTime/(double)m_pixClock;
    unsigned int ffp_intVal_us = (unsigned int)floor(ffpVal_us + 0.5f);
    m_edFFPVal->setText(QString::number(ffp_intVal_us));
    m_edFFPVal->setEnabled(ffpEn);

    // Init FPS field
    double fpsVal = 1000000./ffpVal_us;
    m_edFPS->setText(QString::number(fpsVal, 'f', 1));
    m_edFPS->setEnabled(ffpEn);

    int expCtrlVal = m_cbExpMode->currentIndex();

    //ExposureControl (2 - Internal)
    //Read Current state of Exposure fields and update programming

    //Address 0x720 (Exposure Control)
    //Data(1:0) 00 - Off
    //          01 - Trigger Pulse Width
    //          10 - Internal
    //          11 - Reserved
    if ((expCtrlVal&0x3)==0x2)
    {

        unsigned int exp_intVal = GetCurrentExposure();

        if(ffp_intVal_us<exp_intVal)
                UpdateExposure(ffp_intVal_us);
    }
    else
    {   m_edExpTime->setText("N/A");
        m_edExpTime->setEnabled(false);
        m_edLines->setText("N/A");
        m_edLines->setEnabled(false);
        m_edClocks->setText("N/A");
        m_edClocks->setEnabled(false);
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
// Disconnect the Imperx Camera
// =============================================================================
void QtCheetahKACExposureUI::Disconnect()
{
    IpxCamErr err;

    // Unregister all events of the parameters in the device's node map
    auto paramArrayMain = m_camera->GetCameraParameters();

    IpxGenParam::Param* param;

    param = paramArrayMain->GetParam("Frame_LineTime", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->UnregisterEventSink(this);
    param = paramArrayMain->GetParam("Frame_FixedFramePeriodEnable", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->UnregisterEventSink(this);
    param = paramArrayMain->GetParam("Frame_FixedFramePeriodValue", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->UnregisterEventSink(this);
    param = paramArrayMain->GetParam("Frame_ExposureControl", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }

    param->UnregisterEventSink(this);
    param = paramArrayMain->GetParam("Frame_ExposureLines", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }

    param->UnregisterEventSink(this);
    param = paramArrayMain->GetParam("Frame_ExposureClocks", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }

    param->UnregisterEventSink(this);

    if ( this->windowHandle() != 0 )
    {
        mModelEdit->setText("");
        mNameEdit->setText("");
        mManufacturerEdit->setText("");
        mDeviceFamilyEdit->setText("");
        mVersionEdit->setText("");
        mSerialNbrEdit->setText("");
    }

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

    // Release the Camera Device object
    if(m_camera)
        m_camera->Release();
    m_camera = nullptr;


}

// =============================================================================
// OnBnClickedGenICamButton creates/generates the Parameter Tree View of the
// Camera
// =============================================================================
void QtCheetahKACExposureUI::OnBnClickedGenICamButton()
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
void QtCheetahKACExposureUI::CreateStreamBuffer()
{
    struct CreateStreamBuffer_error : std::runtime_error
    {
        CreateStreamBuffer_error()
            : std::runtime_error("ERROR: CreateStreamBuffer() runtime_error")
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
bool QtCheetahKACExposureUI::StartGrabbing()
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
bool QtCheetahKACExposureUI::StopGrabbing()
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
void QtCheetahKACExposureUI::StartAcquisition()
{
    struct StartAcquisition_error : std::runtime_error
    {
        StartAcquisition_error()
            : std::runtime_error("ERROR: StartAcquisition runtime_error")
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
            //

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
void QtCheetahKACExposureUI::StopAcquisition()
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
void QtCheetahKACExposureUI::OnBnClickedStart()
{
    //Check if the Imperx Camera is Connected
    if ( !m_isConnected)
    {
        return;
    }

    // Start the video acquisition
    StartAcquisition();

    // Update the Toolbar state
    UpdateToolbarButtons();

}

// =============================================================================
// OnBnClickedStop will stop the video acquisition process
// =============================================================================
void QtCheetahKACExposureUI::OnBnClickedStop()
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

// =============================================================================
// OnBnClickedConnectButton will call Connect() function that connects the
// camera and enables the dialog box to be able to perform the acquisition and
// control commands
// =============================================================================
void QtCheetahKACExposureUI::OnBnClickedConnectButton()
{
    //connects the camera and enables the dialog box to be able to perform actions
    Connect();
}

// =============================================================================
// OnBnClickedDisconnectButton will call DisConnect() function that disconnects
// the camera and disables the dialog box to be able to perform the acquisition
// and control commands
// =============================================================================
void QtCheetahKACExposureUI::OnBnClickedDisconnectButton()
{
    //Disconnects the camera and disables the dialog box to be able to perform any actions
    Disconnect();
}

bool QtCheetahKACExposureUI::ZoomIn()
{
    return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_IN));
}

bool QtCheetahKACExposureUI::ZoomOut()
{
  return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_OUT));
}

bool QtCheetahKACExposureUI::Zoom100()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 3));
}

bool QtCheetahKACExposureUI::ZoomFitToWnd()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 1));
}

bool QtCheetahKACExposureUI::ZoomSpreadToWnd()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 2));
}

bool QtCheetahKACExposureUI::CenterImage()
{
   return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ATCENTER));
}
void QtCheetahKACExposureUI::CreateLayout()
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
    setWindowTitle( tr( "QtCheetahKACExposureUI" ) );
}
QGroupBox *QtCheetahKACExposureUI::CreateConnectGroup()
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

QGroupBox *QtCheetahKACExposureUI::CreateExposure()
{

    QLabel *lExposureModeLabel = new QLabel( tr( "Exposure mode:" ) );
    m_cbExpMode = new QComboBox;
    m_cbExpMode->setEnabled( false );

    QObject::connect( m_cbExpMode, SIGNAL( activated( int ) ), this, SLOT( OnCbnSelchangeCbExpMode( int ) ) );

    QLabel *lExposure_usecLabel = new QLabel( tr( "Exposure, usec:" ) );
    m_edExpTime = new QLineEdit;
    m_edExpTime->setReadOnly( false );
    m_edExpTime->setEnabled( false );
   // QObject::connect( m_edExpTime, SIGNAL( textChanged(const QString&) ), this, SLOT( setExpText(const QString&) ) );

    //QLabel *lExpSetLabel = new QLabel( tr( " " ) );
    m_bt_exp_set = new QPushButton( tr( "Set" ) );
    m_bt_exp_set->setMinimumHeight( 27 );

    QObject::connect( m_bt_exp_set, SIGNAL( clicked() ), this, SLOT( OnBnClickedBtExpSet() ) );

    QLabel *lLinesLabel = new QLabel( tr( "Lines" ) );
    m_edLines = new QLineEdit;
    m_edLines->setReadOnly( true );
    m_edLines->setEnabled( false );

    QLabel *lClocksLabel = new QLabel( tr( "Clocks" ) );
    m_edClocks = new QLineEdit;
    m_edClocks->setReadOnly( true );
    m_edClocks->setEnabled( false );

    m_chFFPEnable = new QCheckBox("Enable Fixed Frame Time");
    connect(m_chFFPEnable, SIGNAL(stateChanged(int)), this, SLOT(OnBnClickedChFrameTimeEnable()));

    QLabel *lFtm_usecLabel = new QLabel( tr( "Frame Time, usec:" ) );
    m_edFFPVal = new QLineEdit;
    m_edFFPVal->setReadOnly( false );
    m_edFFPVal->setEnabled( false );
    //QObject::connect( m_edFFPVal, SIGNAL( textChanged(const QString&) ), this, SLOT( setFtmText(const QString&) ) );

    m_bt_ftm_set = new QPushButton( tr( "Set" ) );
    m_bt_ftm_set->setMinimumHeight( 27 );

    QObject::connect( m_bt_ftm_set, SIGNAL( clicked() ), this, SLOT( OnBnClickedBtFtmSet() ) );

    QLabel *lFps_usecLabel = new QLabel( tr( "Frame Rate, FPS:" ) );
    m_edFPS = new QLineEdit;
    m_edFPS->setReadOnly( false );
    m_edFPS->setEnabled( false );

    //QObject::connect( m_edFPS, SIGNAL( textChanged(const QString&) ), this, SLOT( setFpsText(const QString&) ) );

    m_bt_fps_set = new QPushButton( tr( "Set" ) );
    m_bt_fps_set->setMinimumHeight( 27 );

    QObject::connect( m_bt_fps_set, SIGNAL( clicked() ), this, SLOT( OnBnClickedBtFpsSet() ) );

    QGridLayout *lExpGridLayout = new QGridLayout;
    lExpGridLayout->addWidget( lExposureModeLabel, 0, 0, 1, 3 );
    lExpGridLayout->addWidget( m_cbExpMode, 0, 4, 1, 3 );

    lExpGridLayout->addWidget( lExposure_usecLabel, 1, 0, 1, 3 );
    lExpGridLayout->addWidget( m_edExpTime, 1, 3, 1, 3 );
    lExpGridLayout->addWidget( m_bt_exp_set, 1, 6, 1, 1);

    lExpGridLayout->addWidget( m_edLines, 2, 0, 1,2);
    lExpGridLayout->addWidget( lLinesLabel, 2, 2, 1, 1);
    lExpGridLayout->addWidget( m_edClocks, 2, 3, 1, 3 );
    lExpGridLayout->addWidget( lClocksLabel, 2, 6, 1, 1);

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
// UpdateToolBarButtons will enable/disable the Toolbar buttons according the connection state
// =============================================================================
void QtCheetahKACExposureUI::UpdateToolbarButtons()
{
    // This method can be called really early or late when the window is not created
   // if ( this->windowHandle() == 0 )
  //  {
   //     return;
  //  }

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
// =============================================================================
// EnableControls will enable/disable the GUI controls
// =============================================================================
void QtCheetahKACExposureUI::EnableControls(bool en)
{
    m_bt_exp_set->setEnabled(en);
    m_bt_ftm_set->setEnabled(en);
    m_bt_fps_set->setEnabled(en);
    m_chFFPEnable->setEnabled(en);
}

unsigned int QtCheetahKACExposureUI::GetCurrentExposure()
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
    m_edExpTime->setText(QString::number(exp_intval));

    return exp_intval;
}
// =============================================================================
// CreateEnumComboBox
// =============================================================================
void QtCheetahKACExposureUI::CreateEnumComboBox(QComboBox* paramComboBox, char* paramName, std::map<int64_t, int> *aMap)
{
    paramComboBox->clear();

    IpxCamErr err = IPX_CAM_ERR_OK;
    auto paramArray = m_camera->GetCameraParameters();

    IpxGenParam::Enum *lEnum = paramArray->GetEnum(paramName, &err);

    if(lEnum == NULL)
      return;

    auto lEntriesCount = lEnum->GetEnumEntriesCount(&err);

    for(size_t iParam = 0; iParam < lEntriesCount; iParam++)
    {
        auto lEntry = lEnum->GetEnumEntryByIndex(iParam);

        bool lAvailable = lEntry->IsAvailable();
        if(lAvailable)
        {
          QString lNameStr(lEntry->GetValueStr());
          paramComboBox->addItem(lNameStr);
          int lIndex = paramComboBox->findText(lNameStr);
          int64_t lValue = lEntry->GetValue(&err);
          paramComboBox->setItemData(lIndex , (qlonglong)(lEntry->GetValue()));
          if (aMap)
              (*aMap)[lValue] = lIndex;
        }
    }


    //Set mode combo box to value currently used by the device
    auto lValue = lEnum->GetValue();
    for(int i = 0; i < paramComboBox->count(); i++)
    {
        QVariant lData = paramComboBox->itemData(i);
        int64_t lComboValue = lData.value<int64_t>();

        if(lValue == lComboValue)
        {
            paramComboBox->setCurrentIndex(i);
            break;
        }

    }
    paramComboBox->setEnabled(true);

}

// =============================================================================
// OnCbnSelchangeEnum calls the GetCameraParameter and sets the cooresponding
// parameter based on the combobox variable selected
// =============================================================================
void QtCheetahKACExposureUI::OnCbnSelchangeEnum(QComboBox* cb, char* param)
{
    IpxCamErr err;

    // Set Parameter value
    auto paramArray = m_camera->GetCameraParameters();

    IpxGenParam::Enum *lEnum = paramArray->GetEnum(param, &err);
    if(lEnum)
        err = lEnum->SetValue(cb->currentIndex());

    // Check result
    if( (!lEnum) || (err != IPX_CAM_ERR_OK) )
    {
        QMessageBox::critical(this, "Error", "Error accessing the GenICam enumeration value\r\n");
    }
}
// =============================================================================
// OnCbnSelchangeCbExpMode programs the Camera Parameter of the Frame Exposure
// Control based on the combobox variable selected. The User can select no
// exposure control, internal exposure timer or trigger pulse width.
// =============================================================================
void QtCheetahKACExposureUI::OnCbnSelchangeCbExpMode(int aIndex)
{
   // UNREFERENCED_PARAMETER(aIndex);
    OnCbnSelchangeEnum(m_cbExpMode,(char*)"Frame_ExposureControl");
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
void  QtCheetahKACExposureUI::UpdateExposure(unsigned int expVal)
{

    IpxCamErr err;
    // Convert exposure value in microseconds to value in Camera LineTimes and Clocks
    int expLineTimes = (int)(expVal * m_pixClock / m_lineTime);
    int expClocks = (int)((expVal * m_pixClock) % m_lineTime);

    //GenICam  parameter value
    auto paramArray = m_camera->GetCameraParameters();

    err = paramArray->SetIntegerValue("Frame_ExposureLines",expLineTimes);
    if( err != IPX_CAM_ERR_OK)
    {
        QMessageBox::critical(this, "Error","FRAME_EXPOSURE_LINES set failed\r\n");
    }
    err = paramArray->SetIntegerValue("Frame_ExposureClocks",expClocks);
    if( err != IPX_CAM_ERR_OK)
    {
        QMessageBox::critical(this, "Error", "FRAME_EXPOSURE_CLOCKS set failed\r\n");
    }

    // Set expClocks and expLineTimes values to GUI
    m_edExpTime->setText(QString::number(expLineTimes));
    m_edExpTime->setEnabled(0);
    m_edClocks->setText(QString::number(expClocks));
    m_edClocks->setEnabled(0);

    // Calculate the Exposure value in microseconds, using Formula:
    double exposure = (double)(m_lineTime*expLineTimes+expClocks)/m_pixClock;
    unsigned int exp_intval = (unsigned int)floor(exposure + 0.5);

    // Set Exposure value back to the GUI, since real value is rounded to expClocks and expLineTimes
    m_edExpTime->setText(QString::number(exp_intval));

}
// =============================================================================
// OnBnClickedBtExpSet updates the Exposure Time
// =============================================================================
void QtCheetahKACExposureUI::OnBnClickedBtExpSet()
{

    QString strVal = m_edExpTime->text();
    unsigned int val = (unsigned int)strVal.toLongLong();

    UpdateExposure(val);
}
// =============================================================================
// OnBnClickedBtFtmSet updates the Frame_FixedFramePeriodValue
// =============================================================================
void QtCheetahKACExposureUI::OnBnClickedBtFtmSet()
{

    QString strVal = m_edFFPVal->text();

    unsigned int val = strVal.toLongLong();

    /////////////////////////////////////////////////////////////////////////
    // Set FFP to camera in units of LineTime.
    // Convert microseconds to LineTime-s.
    int64_t ffpVal_LT = (int64_t)((double)val*(double)m_pixClock/(double)m_lineTime);

    //GenICam  parameter value
    auto paramArray = m_camera->GetCameraParameters();
    IpxCamErr err = paramArray->SetIntegerValue("Frame_FixedFramePeriodValue",ffpVal_LT);

    if(err != IPX_CAM_ERR_OK)
    {
        ffpVal_LT = paramArray->GetIntegerValue("Frame_FixedFramePeriodValue",&err);

        QMessageBox::critical(this, "Error", "Frame_FixedFramePeriodValue set to a value that is out of range\r\n");
    }

    // Set real FFP value back to panel.
    // Since FFP value is stored in units of LineTimes, value in microseconds,
    // specified in GUI is not "real", so we re-calculate it and put back to GUI
    double ffpVal_us = (double)ffpVal_LT*(double)m_lineTime/(double)m_pixClock;
    unsigned int ffp_intVal_us = (unsigned int)floor(ffpVal_us + 0.5f);
    m_edFFPVal->setText(QString::number(ffp_intVal_us));

    // Set FPS field
    double fpsVal = 1000000./ffpVal_us;
    m_edFPS->setText(QString::number(fpsVal, 'f', 1));

    if(m_cbExpMode->currentIndex() == 2) //Internal
    {
        unsigned int exp_intVal = GetCurrentExposure();

        if(ffp_intVal_us<exp_intVal)
                UpdateExposure(ffp_intVal_us);


    }

}
// =============================================================================
// OnBnClickedBtFpsSet retrieves the current FFP value in microseconds. Then,
// it calculates the FFP in LineTimes. The Frame_FixedFramePeriodValue field
// is updated with the FFP in LineTimes value and then displays the FFP real
// value in microseconds
// =============================================================================
void QtCheetahKACExposureUI::OnBnClickedBtFpsSet()
{

    QString fpsStr;

    double fpsVal=0.0;
    fpsStr = m_edFPS->text();
    fpsVal = fpsStr.toDouble();

    if(fpsVal == 0.0)
        return;

    // Get FFP value in microseconds
    unsigned int ffpVal_us = (unsigned int)(1000000 / fpsVal);
    // FFP in LineTimes
    unsigned int ffpVal_LT= (unsigned int)(ffpVal_us*(double)m_pixClock/(double)m_lineTime);

    // Set ProgFrameTimeAbs GenICam  parameter value
    auto paramArray = m_camera->GetCameraParameters();

    if(!IPX_ERR_SUCCEEDED(paramArray->SetIntegerValue("Frame_FixedFramePeriodValue",ffpVal_LT)))
        QMessageBox::critical(this, "Error","Error writing the GenICam Frame_FixelFramePeriodValue\r\n");

    // Get FFP real value in microseconds
    ffpVal_us = (unsigned int)(ffpVal_LT*m_lineTime/m_pixClock);
    m_edFFPVal->setText(QString::number(ffpVal_us));

    ///////////////////////////////////////////////////////////////////////////////
    // Update Exposure value, if necessary
    if(m_cbExpMode->currentIndex() == 2) //Internal
    {
        unsigned int exp_intVal = GetCurrentExposure();

        if(ffpVal_us<exp_intVal)
            UpdateExposure(ffpVal_us);
    }
}
// =============================================================================
// OnBnClickedChFrameTimeEnable sets the Frame Fixed Frame Period Enable to
// Enabled or Disabled
// =============================================================================
void QtCheetahKACExposureUI::OnBnClickedChFrameTimeEnable()
{
    IpxCamErr err;

    bool ftmEnVal = (m_chFFPEnable->isChecked() ? 1 : 0) ;

    // Set ProgFrameTimeEnable GenICam parameter value
    auto paramArray = m_camera->GetCameraParameters();

    err = paramArray->SetBooleanValue("Frame_FixedFramePeriodEnable",ftmEnVal);

    if(err != IPX_CAM_ERR_OK )
        QMessageBox::critical(this, "Error", "Error accessing the GenICam Frame_FixedFramePeriodEnable value\r\n");

    m_edFFPVal->setEnabled(ftmEnVal);
    m_edFPS->setEnabled(ftmEnVal);

}
// =============================================================================
// OnParameterUpdate occurs during an event callback
// =============================================================================
void QtCheetahKACExposureUI::OnParameterUpdate(IpxGenParam::Param* param)
{
    if(m_camera)
    {
        if (param->GetType() == IpxGenParam::ParamEnum)
        {
            IpxGenParam::Enum *parameter = dynamic_cast<IpxGenParam::Enum *>(param);
            if (parameter && parameter->IsReadable())
            {
                const char *paramName = parameter->GetDisplayName();
                int64_t value = parameter->GetValue();

                if (strcmp("Frame_ExposureControl", paramName)  == 0)
                {
                    // set correct index and check if this value was created before
                    if (m_mapExpModeValues.find(value) != m_mapExpModeValues.end())
                        m_cbExpMode->setCurrentIndex(m_mapExpModeValues[value]);

                    //Update the Current Exposure Fields ExposureCtrl => Internal
                    m_edExpTime->setEnabled(m_mapExpModeValues[value] == 2);
                    if(m_mapExpModeValues[value] == 2)
                        GetCurrentExposure();
                    else
                    {
                        m_edExpTime->setText("N/A");
                        m_edClocks->setText("N/A");
                        m_edLines->setText("N/A");
                    }
                }
            }
        }
        else if(param->GetType() == IpxGenParam::ParamInt)
        {
            IpxGenParam::Int *parameter = dynamic_cast<IpxGenParam::Int *>(param);
            if (parameter && parameter->IsReadable())
            {
                const char *paramName = parameter->GetDisplayName();
                int64_t value = parameter->GetValue();

                if (strcmp("Frame_FixedFramePeriodValue", paramName)  == 0)
                {
                    QString strVal;

                    // Get FFP real value in microseconds
                    unsigned int ffpVal_us = (unsigned int)(value*m_lineTime/m_pixClock);
                    m_edFFPVal->setText(QString::number(ffpVal_us));

                    // Set FPS field
                    double fpsVal = 1000000./ffpVal_us;
                    m_edFPS->setText(QString::number(fpsVal,'f', 1));

                    ///////////////////////////////////////////////////////////////////////////////
                    // Update Exposure value, if necessary
                    if(m_cbExpMode->currentIndex() == 2) //Internal
                    {
                        unsigned int exp_intVal = GetCurrentExposure();

                        if(ffpVal_us<exp_intVal)
                            UpdateExposure(ffpVal_us);
                    }

                }
                else if (strcmp("Frame_LineTime", paramName)  == 0)
                {
                        // Get Frame_LineTime
                        m_lineTime = value;

                        GetCurrentExposure();
                }
                else if (strcmp("Frame_ExposureLines", paramName)  == 0)
                {
                    m_edLines->setText(QString::number(value));

                    auto paramArray = m_camera->GetCameraParameters();
                    IpxCamErr err;
                    // Get Exposure Clocks
                    int64_t expClocks=0;
                    expClocks = paramArray->GetIntegerValue("Frame_ExposureClocks", &err);

                    m_edClocks->setText(QString::number(expClocks));

                    // Calculate the Exposure value in microseconds, using Formula:
                    double exposure = (double)(m_lineTime*value+expClocks)/m_pixClock;
                    unsigned int exp_intval = (unsigned int)floor(exposure + 0.5);
                    // Set to values GUI fields
                    m_edExpTime->setText(QString::number(exp_intval));
                }
                else if (strcmp("Frame_ExposureClocks", paramName)  == 0)
                {

                    auto paramCamArray = m_camera->GetCameraParameters();
                    IpxCamErr err;
                    // ExposureLines value
                    int64_t expLines=0;
                    expLines = paramCamArray->GetIntegerValue("Frame_ExposureLines", &err);

                    m_edLines->setText(QString::number(expLines));

                    // Get Exposure Clocks
                    int64_t expClocks=0;
                    expClocks = value;
                    m_edClocks->setText(QString::number(expClocks));

                    // Calculate the Exposure value in microseconds, using Formula:
                    double exposure = (double)(m_lineTime*expLines+expClocks)/m_pixClock;
                    unsigned int exp_intval = (unsigned int)floor(exposure + 0.5);
                    // Set to values GUI fields
                    m_edExpTime->setText(QString::number(exp_intval));
                }
            }
        }
        else if(param->GetType() == IpxGenParam::ParamBoolean)
        {
            IpxGenParam::Boolean *parameter = dynamic_cast<IpxGenParam::Boolean *>(param);
            if (parameter && parameter->IsReadable())
            {
                const char *paramName = parameter->GetDisplayName();
                bool bvalue = parameter->GetValue();

                if (strcmp("Frame_FixedFramePeriodEnable", paramName)  == 0)
                {
                    m_chFFPEnable->setChecked((int)bvalue);

                    m_edFFPVal->setEnabled(bvalue);
                    m_edFPS->setEnabled(bvalue);
                }
            }
        }
    }
}

