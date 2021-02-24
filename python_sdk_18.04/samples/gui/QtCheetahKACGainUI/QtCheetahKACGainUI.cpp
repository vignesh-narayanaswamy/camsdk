#include "QtCheetahKACGainUI.h"
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
//  QtCheetahKACGainUI Constructor
//
//	The Constructor initializes the ThreadDisplay, Camera, IpxDisplay, IpxStream, IpxDeviceInfo
//  modules to null. Then, it loads the Imperx Icon and initializes the System of the
//  GenTL Module Hierachy.
// =============================================================================
QtCheetahKACGainUI::QtCheetahKACGainUI(QWidget *parent)
    : QMainWindow(parent)
    , m_grabThread(nullptr)
    , m_camera(nullptr)
    , m_display(IpxDisplay::CreateComponent())
    , m_stream(nullptr)
    , m_devInfo(nullptr)
    , m_parameterView(nullptr)
    , m_isConnected (false)
    , m_isStartEnabled(false)
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
// The QtCheetahKACGainUI Destructor
//
//	The Destructor is Delete the IpxDisplay Component Module
// =============================================================================
QtCheetahKACGainUI::~QtCheetahKACGainUI()
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
    if(m_system)
        m_system->Release();

    if(m_displayLabel)
        delete m_displayLabel;
}

void QtCheetahKACGainUI::cameraConnect()
{
    if(!m_system)
        m_system = IpxCam::IpxCam_GetSystem();

    // Open Discovery dialog
    IpxGui::IpxCameraSelectorDialog cameraSelectionDlg(m_system);
    IpxCam::DeviceInfo *devInfo = nullptr;
    if(cameraSelectionDlg.exec() == QDialog::Accepted && (devInfo = cameraSelectionDlg.selectedCamera()))
    {
        //const char *intDescr = devInfo->GetInterface()->GetVersion();

        // Create the camera
        //auto *pCamera = IpxCam_CreateDevice(devInfo);
    }
}

bool QtCheetahKACGainUI::SetupToolbarConnects()
{
    // Connect the GUI signals with GuiManager's slots
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

void QtCheetahKACGainUI::closeEvent(QCloseEvent * event)
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

void QtCheetahKACGainUI::InitFromCamera()
{
    // Set ProgFrameTimeAbs GenICam  parameter value
    auto paramArray = m_camera->GetCameraParameters();

    //Set the Combo and Update the Parameter Tree for the Analog Gain
    //This feature controls the Analog Gain for all the taps
    OnCbnSelchangeEnum(m_cbAnalogGain, (char*)"Frame_AnalogGain");

    IpxCamErr err;

    //Digital Gain Course Value
    //It controls the course digital gain for all taps
    int64_t io_digi;
    io_digi = paramArray->GetIntegerValue("Frame_DigitalCourseGain",&err);
    if( err != IPX_CAM_ERR_OK)
    {
       QMessageBox::critical(this, "Error", "FRAME_DIGITAL_GAIN_COURSE set failed");
    }

    //Digital Gain Fine value
    //It controls the course digital gain
    int64_t io_digf;
    io_digf = paramArray->GetIntegerValue("Frame_DigitalFineGain",&err);
    if( err != IPX_CAM_ERR_OK)
    {
        QMessageBox::critical(this, "Error", "FRAME_DIGITAL_GAIN_FINE set failed");
    }

    // Calculate real Gain and put it back to the edit field
    double GainDigital = pow(2., (double)io_digi) * (1.+ (double)io_digf/64.);
    QString strVal;
    strVal.sprintf("%.3f", GainDigital);
    m_edDigitalGain->setText(strVal);


    //////////////////////////////////////////////////////void QtCheetahKACGainUI::InitFromCamera()///////////////////////////////////////
    // Black Level
    int64_t BlackLevel=0;

    //The Frame Black Level controls the Black Level for all taps
    BlackLevel = paramArray->GetIntegerValue("Frame_BlackLevel",&err);
    if( err != IPX_CAM_ERR_OK)
    {
        QMessageBox::critical(this, "Error", "FRAME_BLACK_LEVEL set failed");
    }

    m_edBlackLevel->setText(QString::number(BlackLevel));

    // Enable GUI
    EnableControls(m_isConnected);

}
// =============================================================================
// Connect the Imperx Camera
// =============================================================================
void QtCheetahKACGainUI::Connect()
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
        QMessageBox::critical(this, "ERROR", "Unable to retrieve camera device.\nMake sure an Imperx KAC Camera is connected.\r\n" );
        return;
    }
    // Get the camera basic information and show it on GUI
    IpxCamErr err=IPX_CAM_ERR_OK;
    auto camParamArray = m_camera->GetCameraParameters();

    //Register the Event Sink
    IpxGenParam::Param* param;

    param = camParamArray->GetParam("Frame_AnalogGain", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->RegisterEventSink(this);
    param = camParamArray->GetParam("Frame_DigitalCourseGain", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }

    param->RegisterEventSink(this);
    param = camParamArray->GetParam("Frame_DigitalFineGain", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }
    param->RegisterEventSink(this);
    param = camParamArray->GetParam("Frame_BlackLevel", &err);

    param->RegisterEventSink(this);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }

    IpxGenParam::String* lModelName = camParamArray->GetString("DeviceModelName", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx  KAC Camera is connected.\r\n" );
        return;
    }

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
    CreateEnumComboBox(m_cbAnalogGain, (char*)"Frame_AnalogGain", &m_mapAnalogValues);

    //Gain time, usec
    int64_t offsetVal = 0;

    offsetVal = camParamArray->GetIntegerValue("Frame_BlackLevel", &err);

    m_edBlackLevel->setText(QString::number(offsetVal));
    
    // Create the Buffers queue for video acquisition
    CreateStreamBuffer();

    // Create the grabbing threadOnCbnSelchangeEnum
    m_grabThread = new GrabbingThread(m_stream, m_display);


    //Set State of Imperx Camera connect
    m_isConnected = true; //Imperx Camera is connected

    //Sync up GUI
    UpdateToolbarButtons();

	if(m_camera)
		InitFromCamera();
}

// =============================================================================
// Disconnect the Imperx Camera
// =============================================================================
void QtCheetahKACGainUI::Disconnect()
{
    IpxCamErr err;

    // Unregister all events of the parameters in the device's node map
    auto paramArrayMain = m_camera->GetCameraParameters();

    IpxGenParam::Param* param;

    param = paramArrayMain->GetParam("Frame_AnalogGain", &err);
    param->UnregisterEventSink(this);
    param = paramArrayMain->GetParam("Frame_DigitalCourseGain", &err);
    param->UnregisterEventSink(this);
    param = paramArrayMain->GetParam("Frame_DigitalFineGain", &err);
    param->UnregisterEventSink(this);
    param = paramArrayMain->GetParam("Frame_BlackLevel", &err);
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

    m_isConnected = false;

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
void QtCheetahKACGainUI::OnBnClickedGenICamButton()
{
    if(m_parameterView)
    {
        m_parameterView->clearParams();
        DestroyGenParamTreeView(m_parameterView);
    }
    m_parameterView = IpxGui::CreateGenParamTreeViewForArrayW(m_camera->GetCameraParameters(), L"Camera Parameters - Gen<i>Cam");


}
// =============================================================================
// CreateBuffer will create the stream buffer queue
// =============================================================================
void QtCheetahKACGainUI::CreateStreamBuffer()
{
    struct startGrabbing_error : std::runtime_error
    {
        startGrabbing_error()
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
                    throw startGrabbing_error();
                if(!m_stream)
                    m_stream = m_camera->GetStreamByIndex(0);
                if(!m_stream)
                    throw startGrabbing_error();

                // Release buffer queue, just in case, if it was previously allocated
                m_stream->ReleaseBufferQueue();
            }
            catch(const startGrabbing_error&)
            {
                //Release the Camera Device Object
                m_camera->Release();
                m_camera = nullptr;
                return;
            }
    }
}
bool QtCheetahKACGainUI::StartGrabbing()
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
bool QtCheetahKACGainUI::StopGrabbing()
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
void QtCheetahKACGainUI::StartAcquisition()
{
    struct StartAcquisition_error : std::runtime_error
    {
        StartAcquisition_error()
            : std::runtime_error("ERROR: AcquireImages failed")
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
void QtCheetahKACGainUI::StopAcquisition()
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
void QtCheetahKACGainUI::OnBnClickedStart()
{
    //Check if the Imperx Camera is Connected
    if ( !m_isConnected)
    {
        return;
    }

    // Start the video acquisition
    StartAcquisition();

    m_isStartEnabled = true;

    UpdateToolbarButtons();

}

// =============================================================================
// OnBnClickedStop will stop the video acquisition process
// =============================================================================
void QtCheetahKACGainUI::OnBnClickedStop()
{
    //Check if the Imperx Camera is Connected
    if ( !m_isConnected )
    {
        return;
    }

    // Stop the video acquisition
    StopAcquisition();

    // Update the Toolbar state
    m_isStartEnabled = false;

    UpdateToolbarButtons();
}

// =============================================================================
// OnBnClickedConnectButton will call Connect() function that connects the
// camera and enables the dialog box to be able to perform the acquisition and
// control commands
// =============================================================================
void QtCheetahKACGainUI::OnBnClickedConnectButton()
{
    //connects the camera and enables the dialog box to be able to perform actions
    Connect();
}

// =============================================================================
// OnBnClickedDisconnectButton will call DisConnect() function that disconnects
// the camera and disables the dialog box to be able to perform the acquisition
// and control commands
// =============================================================================
void QtCheetahKACGainUI::OnBnClickedDisconnectButton()
{
    //Disconnects the camera and disables the dialog box to be able to perform any actions
    Disconnect();
}

bool QtCheetahKACGainUI::ZoomIn()
{
    return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_IN));
}

bool QtCheetahKACGainUI::ZoomOut()
{
  return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_OUT));
}

bool QtCheetahKACGainUI::Zoom100()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 3));
}

bool QtCheetahKACGainUI::ZoomFitToWnd()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 1));
}

bool QtCheetahKACGainUI::ZoomSpreadToWnd()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 2));
}

bool QtCheetahKACGainUI::CenterImage()
{
   return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ATCENTER));
}
void QtCheetahKACGainUI::CreateLayout()
{
    QGroupBox *lConnectionBox = CreateConnectGroup();
    QGroupBox *lExposureBox = CreateGain();

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
    setWindowTitle( tr( "QtCheetahKACGainUI" ) );
}
QGroupBox *QtCheetahKACGainUI::CreateConnectGroup()
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

QGroupBox *QtCheetahKACGainUI::CreateGain()
{

    QLabel *lAnalogGainLabel = new QLabel( tr( "Analog gain:" ) );
    m_cbAnalogGain = new QComboBox;
    m_cbAnalogGain->setEnabled( false );

    QObject::connect( m_cbAnalogGain, SIGNAL( activated( int ) ), this, SLOT( OnCbnSelchangeCbAnalogGain( int ) ) );

    QLabel *lDigitalGainLabel = new QLabel( tr( "Digital Gain:" ) );
    m_edDigitalGain = new QLineEdit;
    m_edDigitalGain->setReadOnly( false );
    m_edDigitalGain->setEnabled( false );
    QObject::connect( m_edDigitalGain, SIGNAL( textChanged(const QString&) ), this, SLOT( setDigitalGainText(const QString&) ) );

    //QLabel *lDigitalGainSetLabel = new QLabel( tr( " " ) );
    m_btDigitalGainSet = new QPushButton( tr( "Set" ) );
    m_btDigitalGainSet->setMinimumHeight( 27 );

    QObject::connect( m_btDigitalGainSet, SIGNAL( clicked() ), this, SLOT( OnBnClickedBtDigitalGain() ) );

    QLabel *lBlackLevelLabel = new QLabel( tr( "Black level:" ) );
    m_edBlackLevel = new QLineEdit;
    m_edBlackLevel->setReadOnly( false );
    m_edBlackLevel->setEnabled( false );
    QObject::connect( m_edBlackLevel, SIGNAL( textChanged(const QString&) ), this, SLOT( setBlackLevelText(const QString&) ) );

    //QLabel *lBlackLevelSetLabel = new QLabel( tr( " "));
    m_btBlackLevelSet = new QPushButton( tr( "Set" ) );
    m_btBlackLevelSet->setMinimumHeight( 27 );

    QObject::connect( m_btBlackLevelSet, SIGNAL( clicked() ), this, SLOT( OnBnClickedBtBlackLevel() ) );

    QGridLayout *lExpGridLayout = new QGridLayout;
    lExpGridLayout->addWidget( lAnalogGainLabel, 0, 0, 1, 3 );
    lExpGridLayout->addWidget( m_cbAnalogGain, 0, 4, 1, 3 );

    lExpGridLayout->addWidget( lDigitalGainLabel, 1, 0, 1, 3 );
    lExpGridLayout->addWidget( m_edDigitalGain, 1, 3, 1, 3 );
    lExpGridLayout->addWidget( m_btDigitalGainSet, 1, 6, 1, 1);

    QGridLayout *lExpGridFrameLayout = new QGridLayout;
    lExpGridFrameLayout->addWidget( lBlackLevelLabel, 0, 0 );
    lExpGridFrameLayout->addWidget( m_edBlackLevel, 0, 1, 1, 1 );
    lExpGridFrameLayout->addWidget( m_btBlackLevelSet, 0, 2, 1, 1);

    QVBoxLayout *lExpLayout = new QVBoxLayout;
    lExpLayout->addLayout( lExpGridLayout );
    lExpLayout->addLayout( lExpGridFrameLayout);
    lExpLayout->addStretch();

    QGroupBox *lGainBox = new QGroupBox( tr( "Gain" ) );
    lGainBox->setLayout( lExpLayout );
    lGainBox->setMinimumWidth( 300 );
    lGainBox->setMaximumWidth( 300 );
   // lGainBox->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

    return lGainBox;

}
void QtCheetahKACGainUI::setDigitalGainText(const QString &text)
{
    if( m_edDigitalGain->text() == text)
    return;

    m_edDigitalGain->setText(text);

}
void QtCheetahKACGainUI::setBlackLevelText(const QString &text)
{
    if( m_edBlackLevel->text() == text)
    return;

    m_edBlackLevel->setText(text);

}
// =============================================================================
// UpdateToolBarButtons will enable/disable the Toolbar buttons according the connection state
// =============================================================================
void QtCheetahKACGainUI::UpdateToolbarButtons()
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
        if(!m_isStartEnabled)
        {
            // Update the Toolbar state
            ui.actionPlay->setEnabled(true);
            ui.actionStop->setEnabled(false);
        }
        else
        {
            // Update the Toolbar state
            ui.actionPlay->setEnabled(false);
            ui.actionStop->setEnabled(true);
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

    }
    EnableControls(m_isConnected);
}
// =============================================================================
// EnableControls will enable/disable the GUI controls
// =============================================================================
void QtCheetahKACGainUI::EnableControls(bool en)
{
    m_btDigitalGainSet->setEnabled(en);
    m_edDigitalGain->setEnabled(en);
    m_btBlackLevelSet->setEnabled(en);
    m_edBlackLevel->setEnabled(en);
}

// =============================================================================
// CreateEnumComboBox
// =============================================================================

void QtCheetahKACGainUI::CreateEnumComboBox(QComboBox* paramComboBox, char* paramName, std::map<int64_t, int> *aMap)
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
          paramComboBox->setItemData(lIndex /*lIndex*/, (qlonglong)(lEntry->GetValue()));
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
void QtCheetahKACGainUI::OnCbnSelchangeEnum(QComboBox* cb, char* param)
{
    IpxCamErr err;

    // Set Parameter value
    auto paramArray = m_camera->GetCameraParameters();

    IpxGenParam::Enum *lEnum = paramArray->GetEnum(param, &err);
    if(lEnum)
    {
        int64_t cb_value;
        auto val = cb->currentIndex();
        if(val == 0)
            cb_value = 0x20;
        else if(val == 0x1)
            cb_value = 0x10;
        else if(val == 0x2)
            cb_value = 0x8;
        else
            cb_value = 0x4;
        err = lEnum->SetValue(cb_value);
    }
    // Check result
    if( (!lEnum) || (err != IPX_CAM_ERR_OK) )
    {
        QMessageBox::critical(this, "Error", "Error accessing the GenICam enumeration value\r\n");
    }
}
// =============================================================================
// OnBnClickedBtDigitalGain
// =============================================================================
void QtCheetahKACGainUI::OnBnClickedBtDigitalGain()
{
	// Get DigitalGain value form GUI
	QString strVal;
	IpxCamErr err;
    strVal = m_edDigitalGain->text();
    double DigitalGain = strVal.toDouble();

	int io_digi = 0, io_digf = 0;
	if (DigitalGain >= 2.0 && DigitalGain < 4.0) 
		io_digi = 1;
	else if (DigitalGain >= 4.0 && DigitalGain < 8.0) 
		io_digi = 2;
	else if (DigitalGain >= 8.0 && DigitalGain <= 15.875) 
		io_digi = 3;

	if(DigitalGain>15.875  || DigitalGain < 1.0)
	{
		QMessageBox::critical(this, "Error", "DigitalGain value must be in the range of 1.0 to 15.875");
		return;
	}

	io_digf = (long)((DigitalGain/pow((double)2, (double)io_digi)-1) * 64.0 + 0.5);


	//GenICam  parameter value
	auto paramArray = m_camera->GetCameraParameters();
	
	// Write to camera
    // Course Value
    err = paramArray->SetIntegerValue("Frame_DigitalCourseGain",io_digi);
	if( err != IPX_CAM_ERR_OK)
	{	
        QMessageBox::critical(this, "Error", "FRAME_DIGITAL_GAIN_COURSE set failed");
	}

	// Fine value
	err = paramArray->SetIntegerValue("Frame_DigitalFineGain",io_digf);
	if( err != IPX_CAM_ERR_OK)
	{	
		QMessageBox::critical(this, "Error", "FRAME_DIGITAL_GAIN_FINE set failed");
	}

	// Calculate real Gain and put it back to the edit field
	double GainDigital = pow(2., (double)io_digi) * (1.+ (double)io_digf/64.);
	strVal.sprintf("%.6f", GainDigital);
	m_edDigitalGain->setText(strVal);
}

// =============================================================================
// OnCbnSelchangeCbExpMode programs the Camera Parameter of the Frame Exposure
// Control based on the combobox variable selected. The User can select no
// exposure control, internal exposure timer or trigger pulse width.
// =============================================================================
void QtCheetahKACGainUI::OnCbnSelchangeCbAnalogGain(int aIndex)
{
   // UNREFERENCED_PARAMETER(aIndex);

    OnCbnSelchangeEnum(m_cbAnalogGain,(char*)"Frame_AnalogGain");
}
// =============================================================================
// OnBnClickedBtBlackLevel
// =============================================================================
void QtCheetahKACGainUI::OnBnClickedBtBlackLevel()
{
	IpxCamErr err;
	// Set Black Level
	/*
	int BlackLevel = GetDlgItemInt(IDC_ED_BLACK_LEVEL);
*/
    QString strVal;

    strVal = m_edBlackLevel->text();
    int BlackLevel = strVal.toInt();

	if(BlackLevel > 2048)
	{
		QMessageBox::critical(this, "Error", "The Black Level value must be in the range of 0 to 2048");
		return;
	}

	// Write to camera
	//GenICam  parameter value
	auto paramArray = m_camera->GetCameraParameters();
	
	err = paramArray->SetIntegerValue("Frame_BlackLevel",BlackLevel);
	if( err != IPX_CAM_ERR_OK)
	{	
		QMessageBox::critical(this, "Error", "FRAME_BLACK_LEVEL set failed");
	}

}
// =============================================================================
// OnParameterUpdate occurs during an event callback
// =============================================================================
void QtCheetahKACGainUI::OnParameterUpdate(IpxGenParam::Param* param)
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

				if (strcmp("Frame_AnalogGain", paramName)  == 0)
				{
					// set correct index and check if this value was created before
					if (m_mapAnalogValues.find(value) != m_mapAnalogValues.end())
					{
                        m_cbAnalogGain->setCurrentIndex(m_mapAnalogValues[value]);
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
				
				if (strcmp("Frame_BlackLevel", paramName)  == 0)
				{
					//Update the Edit Box with the Frame_DigitalGain value retrieved
                    m_edBlackLevel->setText(QString::number(value));
				}
                else if (strcmp("Frame_DigitalCourseGain", paramName)  == 0)
				{
					IpxCamErr err;
					auto paramArray = m_camera->GetCameraParameters();
					// Fine value
					int64_t io_digf = paramArray->GetIntegerValue("Frame_DigitalFineGain",&err);
					if( err != IPX_CAM_ERR_OK)
					{	
						QMessageBox::critical(this, "Error", "Frame_DigitalFineGain set failed");
					}
					QString strVal;
					// Calculate real Gain and put it back to the edit field
					double GainDigital = pow(2., (double)value) * (1.+ (double)io_digf/64.);
					strVal.sprintf("%.6f", GainDigital);
                    m_edDigitalGain->setText(strVal);
				}
				else if (strcmp("Frame_DigitalFineGain", paramName)  == 0)
				{
					IpxCamErr err;
					auto paramArray = m_camera->GetCameraParameters();
					// Fine value
                    int64_t io_digi = paramArray->GetIntegerValue("Frame_DigitalCourseGain",&err);
					if( err != IPX_CAM_ERR_OK)
					{	
                        QMessageBox::critical(this, "Error", "Frame_DigitalCourse set failed");
					}
					QString strVal;
					// Calculate real Gain and put it back to the edit field
					double GainDigital = pow(2., (double)io_digi) * (1.+ (double)value/64.);
					strVal.sprintf("%.6f", GainDigital);
                    m_edDigitalGain->setText(strVal);
				}
			}
		}
	}
}
