#include "QtCheetahPythonGainUI.h"
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
//  QtCheetahPythonGainUI Constructor
//
//	The Constructor initializes the ThreadDisplay, Camera, IpxDisplay, IpxStream, IpxDeviceInfo
//  modules to null. Then, it loads the Imperx Icon and initializes the System of the
//  GenTL Module Hierachy.
// =============================================================================
QtCheetahPythonGainUI::QtCheetahPythonGainUI(QWidget *parent)
    : QMainWindow(parent)
    , m_grabThread(nullptr)
    , m_camera(nullptr)
    , m_display(IpxDisplay::CreateComponent())
    , m_stream(nullptr)
    , m_devInfo(nullptr)
    , m_parameterView(nullptr)
    , m_isConnected (false)
    , m_bNewXml( false )
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
// The QtCheetahPythonGainUI Destructor
//
//	The Destructor is Delete the IpxDisplay Component Module
// =============================================================================
QtCheetahPythonGainUI::~QtCheetahPythonGainUI()
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

void QtCheetahPythonGainUI::cameraConnect()
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

bool QtCheetahPythonGainUI::SetupToolbarConnects()
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

void QtCheetahPythonGainUI::closeEvent(QCloseEvent * event)
{
    if(m_isConnected)
            Disconnect();

    // Delete the grabbing thread
    if(m_grabThread)
        delete m_grabThread;

    m_grabThread = nullptr;

    if(m_display)
    {
        IpxDisplay::DeleteComponent(m_display);
        m_display = nullptr;
    }

    QMainWindow::closeEvent(event);
}

void QtCheetahPythonGainUI::InitFromCamera()
{
    //////////////////////////////////////////////////////////////////////
    /// example how to map enum parameter with combo box
    //////////////////////////////////////////////////////////////////////

    // Get GenICam parameter value
    auto paramArray = m_camera->GetCameraParameters();
    if (!paramArray)
        return;

    if(m_bNewXml)
    {
        m_mapData.mapEnumParam(paramArray->GetEnum("AnalogGain", nullptr), m_cbAnalogGain);
        m_mapData.mapEnumParam(paramArray->GetEnum("BlackLevelAuto", nullptr), m_cbBlackLevelAutoCal);
    }
    else
    {
        m_mapData.mapEnumParam(paramArray->GetEnum("Frame_AnalogGain", nullptr), m_cbAnalogGain);
        m_mapData.mapEnumParam(paramArray->GetEnum("Frame_BlackLevelAutoCalibration", nullptr), m_cbBlackLevelAutoCal);
    }

    IpxCamErr err;

    //Retrieve the Digital Gain Value
    int64_t DigitalGainCode=0;
    double DigitalGain=0., DigitalGainDB=0.;
    if(m_bNewXml)
    {
        DigitalGainCode = paramArray->GetIntegerValue("DigitalGainRaw",&err);
        if( err != IPX_CAM_ERR_OK)
            QMessageBox::critical(this, "Error", "DigitalGain get failed");
    }
    else
    {
        DigitalGainCode = paramArray->GetIntegerValue("Frame_DigitalGain",&err);
        if( err != IPX_CAM_ERR_OK)
            QMessageBox::critical(this, "Error","Frame_DigitalGain get failed");
    }

    // Update the Edit Box with the Frame_DigitalGain value retrieved
    m_edDigitalGainCode->setText(QString::number(DigitalGainCode));

    // Calculate the DigitalGain in times(x)
    DigitalGain = floor(10.*.0009765625*DigitalGainCode + 0.5f)/10;

    // Display Digital Gain Value
    m_edDigitalGain->setText(QString::number(DigitalGain, 'f', 1));

    //Convert and display Digital Gain Decibels
    DigitalGainDB = 20.0*log10(DigitalGain);
    m_edDigitalGainDB->setText(QString::number(DigitalGainDB, 'f', 1));

    int calEn = m_cbBlackLevelAutoCal->currentIndex();

    //If BlackLevelAutoCalibration is Off, then retrieve the Black Level Offset and display its value
    if(calEn == 0)
    {
        int64_t prev_black_level = 0;
        int64_t int_prev_black_level=0;
        IpxCamErr err=IPX_CAM_ERR_OK;
        if(m_bNewXml)
        {
            prev_black_level = (int64_t)paramArray->GetFloatValue("BlackLevel", &err);

            if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            {
                QMessageBox::critical(this, "ERROR", "Unable to retrieve BlackLevel\r\n" );
                return;
            }
        }
        else
        {
            prev_black_level = paramArray->GetIntegerValue("Frame_BlackLevelOffset", &err);
            if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            {
                QMessageBox::critical(this, "ERROR", "Unable to retrieve Frame_BlackLevelOffset\r\n" );
                return;
            }

        }
        int signed_value = (prev_black_level&0x200) >> 9;
        if(signed_value)
            int_prev_black_level = -1*(prev_black_level&0x1ff);
        else
            int_prev_black_level = prev_black_level&0x1ff;

        m_edBlackLevel->setText(QString::number(int_prev_black_level));
        m_edBlackLevel->setEnabled(true);
    }
    else
    {
        m_edBlackLevel->setText("N/A");
        m_edBlackLevel->setEnabled(0);
    }

    // Enable GUI
    EnableControls(true);
}
// =============================================================================
// Connect the Imperx Camera
// =============================================================================
void QtCheetahPythonGainUI::Connect()
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
    auto paramArray = m_camera->GetCameraParameters();
    if(!paramArray)
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve Camera Parameters\r\n" );
        return;
    }

    //Register the Event Sink
    IpxGenParam::Param* param;

    m_bNewXml = (paramArray->GetParam("AnalogGain", &err))? true:false;
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        QMessageBox::critical(this, "ERROR", "Unable to get AnalogGain\r\n" );
        return;
    }

    if(m_bNewXml)
    {
        param = paramArray->GetParam("DigitalGainRaw", &err);// new XML
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        {
            QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx Python Camera is connected.\r\n" );
            return;
        }
        param->RegisterEventSink(this);
        param = paramArray->GetParam("BlackLevel", &err);// new XML
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        {
            QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx Python Camera is connected.\r\n" );
            return;
        }
        param->RegisterEventSink(this);
    }
    else
    {
        // Old XML
        param = paramArray->GetParam("Frame_DigitalGain", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        {
            QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx Python Camera is connected.\r\n" );
            return;
        }
        param->RegisterEventSink(this);
        param = paramArray->GetParam("Frame_BlackLevelOffset", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        {
            QMessageBox::critical(this, "ERROR", "Unable to retrieve proper camera parameters.\nMake sure an Imperx Python Camera is connected.\r\n" );
            return;
        }

        param->RegisterEventSink(this);
    }

    IpxGenParam::String* lManufacturer = paramArray->GetString("DeviceVendorName",  &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        QMessageBox::critical(this, "Error", "Unable to retrieve DeviceVendorName");

    IpxGenParam::String* lModelName = paramArray->GetString("DeviceModelName", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        QMessageBox::critical(this, "Error", "Unable to retrieve DeviceModelName");

    IpxGenParam::String* lNameParam = paramArray->GetString("DeviceUserDefinedName", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
    {
        lNameParam = paramArray->GetString("DeviceUserID", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "DeviceUserID set failed");
    }
    IpxGenParam::String* lDeviceFamily = paramArray->GetString("DeviceFamilyName", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        QMessageBox::critical(this, "Error", "Unable to retrieve DeviceFamilyName");

    IpxGenParam::String* lVersion = paramArray->GetString("DeviceVersion", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        QMessageBox::critical(this, "Error", "Unable to retrieve DeviceVersion");

    IpxGenParam::String* lSerialNbr = paramArray->GetString("DeviceSerialNumber", &err);
    if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
        QMessageBox::critical(this, "Error", "Unable to retriever DeviceSerialNumber");

    QString lModelNameStr((lModelName != 0) ? lModelName->GetValue() : "");
    QString lNameStr((lNameParam != 0) ? lNameParam->GetValue() : "");
    QString lManufacturerStr((lManufacturer != 0) ? lManufacturer->GetValue(): "");
    QString lDeviceFamilyStr((lDeviceFamily != 0) ? lDeviceFamily->GetValue(): "");
    QString lVersionStr((lVersion != 0) ? lVersion->GetValue() : "");
    QString lSerialNbrStr((lSerialNbr != 0) ? lSerialNbr->GetValue() : "" );

    //Fill the edit boxes with the information retrieved from the camera
    mModelEdit->setText(lModelNameStr);
    mNameEdit->setText(lNameStr);
    mManufacturerEdit->setText(lManufacturerStr);
    mDeviceFamilyEdit->setText(lDeviceFamilyStr);
    mVersionEdit->setText(lVersionStr);
    mSerialNbrEdit->setText(lSerialNbrStr);

    // Create the Buffers queue for video acquisition
    CreateStreamBuffer();

    // Create the grabbing thread
    m_grabThread = new GrabbingThread(m_stream, m_display);

    //Set State of Imperx Camera connect
    m_isConnected = true; //Imperx Camera is connected

    // Sync up GUI
    UpdateToolbarButtons();

    //Camera Connected
    if(m_camera)
        InitFromCamera();
}

// =============================================================================
// Disconnect the Imperx Camera
// =============================================================================
void QtCheetahPythonGainUI::Disconnect()
{
    IpxCamErr err;

    // Unregister all events of the parameters in the device's node map
    auto paramArray = m_camera->GetCameraParameters();

    // crear mapper in case is not empty
    m_mapData.clear(paramArray);

    IpxGenParam::Param* param;

    if(m_bNewXml)
    {
        param = paramArray->GetParam("DigitalGainRaw", &err); // new XML
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "Unable to retrieve DigitalGainRaw");
        param->UnregisterEventSink(this);
        param = paramArray->GetParam("BlackLevel", &err); // new XML
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "Unable to retrieve BlackLevel");
        param->UnregisterEventSink(this);
    }
    else
    {
        //UnRegisters the Event Sink
        param = paramArray->GetParam("Frame_DigitalGain", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "Unable to retrieve Frame_DigitalGain");

        param->UnregisterEventSink(this);
        param = paramArray->GetParam("Frame_BlackLevelOffset", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "Unable to retrieve Frame_BlackLevelOffset");
        param->UnregisterEventSink(this);
    }

    mModelEdit->setText("");
    mNameEdit->setText("");
    mManufacturerEdit->setText("");
    mDeviceFamilyEdit->setText("");
    mVersionEdit->setText("");
    mSerialNbrEdit->setText("");

    // Stop the Acquisition
    if(m_grabThread->IsStarted())
        OnBnClickedStop();

    // Delete the grabbing thread
    if(m_grabThread)
    {
        delete m_grabThread;
        m_grabThread = nullptr;
    }

    m_isConnected = false;

    //Update the toolbar
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

    // Release the Camera object
    if(m_camera)
        m_camera->Release();
    m_camera = nullptr;
}

// =============================================================================
// OnBnClickedGenICamButton creates/generates the Parameter Tree View of the
// Camera
// =============================================================================
void QtCheetahPythonGainUI::OnBnClickedGenICamButton()
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
void QtCheetahPythonGainUI::CreateStreamBuffer()
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
bool QtCheetahPythonGainUI::StartGrabbing()
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
bool QtCheetahPythonGainUI::StopGrabbing()
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
void QtCheetahPythonGainUI::StartAcquisition()
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
void QtCheetahPythonGainUI::StopAcquisition()
{
    // Get camera parrameters array
    auto genParams = m_camera->GetCameraParameters();
    if(genParams)
    {
        // Run AcquisitionStop command to start the acquisition
        IpxCamErr err = genParams->ExecuteCommand("AcquisitionStop");

        // If any error occured with AcquisitionStop, run AcquisitionAbort
        if(err != IPX_CAM_ERR_OK )
            err = genParams->ExecuteCommand("AcquisitionAbort");
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "Unable to execute AcquisitionAbort");

        StopGrabbing();

        // Reset the TLParamsLocked parameter to unlock the parameters effecting to the Transport Layer
        genParams->SetIntegerValue("TLParamsLocked", 0);

        m_stream->ReleaseBufferQueue();
    }

}

// =============================================================================
// OnBnClickedStart will start the video acquistion process
// =============================================================================
void QtCheetahPythonGainUI::OnBnClickedStart()
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
void QtCheetahPythonGainUI::OnBnClickedStop()
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
void QtCheetahPythonGainUI::OnBnClickedConnectButton()
{
    //connects the camera and enables the dialog box to be able to perform actions
    Connect();
}

// =============================================================================
// OnBnClickedDisconnectButton will call DisConnect() function that disconnects
// the camera and disables the dialog box to be able to perform the acquisition
// and control commands
// =============================================================================
void QtCheetahPythonGainUI::OnBnClickedDisconnectButton()
{
    //Disconnects the camera and disables the dialog box to be able to perform any actions
    Disconnect();
}

bool QtCheetahPythonGainUI::ZoomIn()
{
    return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_IN));
}

bool QtCheetahPythonGainUI::ZoomOut()
{
  return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_OUT));
}

bool QtCheetahPythonGainUI::Zoom100()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 3));
}

bool QtCheetahPythonGainUI::ZoomFitToWnd()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 1));
}

bool QtCheetahPythonGainUI::ZoomSpreadToWnd()
{
   return (0 == m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 2));
}

bool QtCheetahPythonGainUI::CenterImage()
{
   return (0 == m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ATCENTER));
}
void QtCheetahPythonGainUI::CreateLayout()
{
    QGroupBox *lConnectionBox = CreateConnectGroup();
    QGroupBox *lGainBox = CreateGain();

    QVBoxLayout *lLayoutLeft = new QVBoxLayout();
    lLayoutLeft->addWidget( lConnectionBox, Qt::AlignTop );
    lLayoutLeft->addStretch();
    lLayoutLeft->addWidget( lGainBox, Qt::AlignTop );

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
    setWindowTitle( tr( "QtCheetahPythonGainUI" ) );
}
QGroupBox *QtCheetahPythonGainUI::CreateConnectGroup()
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

QGroupBox *QtCheetahPythonGainUI::CreateGain()
{

    QLabel *lAnalogGainLabel = new QLabel( tr( "Analog gain:" ) );
    m_cbAnalogGain = new QComboBox;
    m_cbAnalogGain->setEnabled( false );

    QLabel *lDigitalGainLabel = new QLabel( tr( "Digital Gain Value:" ) );
    m_edDigitalGainCode = new QLineEdit;
    m_edDigitalGainCode->setReadOnly( false );
    m_edDigitalGainCode->setEnabled( false );

    m_btDigitalGainSet = new QPushButton( tr( "Set" ) );
    m_btDigitalGainSet->setMinimumHeight( 27 );

    QLabel *lDigitalGainLabel2   = new QLabel(tr("=> Digital Gain"));

    m_edDigitalGain = new QLineEdit;
    m_edDigitalGain->setReadOnly( true );
    m_edDigitalGain->setEnabled( false );

    QLabel *lDigitalGainDecibels = new QLabel(tr("=> Decibels"));

    m_edDigitalGainDB = new QLineEdit;
    m_edDigitalGainDB->setReadOnly( true );
    m_edDigitalGainDB->setEnabled( false );

    QLabel *lDigitalGainDB       = new QLabel(tr("dB"));

    QLabel *lBlackLevelAutoCalLabel = new QLabel(tr("BlackLevelAutoCalibration:"));
    m_cbBlackLevelAutoCal = new QComboBox;
    m_cbBlackLevelAutoCal->setEnabled( false );

    QLabel *lBlackLevelLabel = new QLabel( tr( "Black level:" ) );
    m_edBlackLevel = new QLineEdit;
    m_edBlackLevel->setReadOnly( false );
    m_edBlackLevel->setEnabled( false );

    m_btBlackLevelSet = new QPushButton( tr( "Set" ) );
    m_btBlackLevelSet->setMinimumHeight( 27 );

    connect( m_btDigitalGainSet,    SIGNAL( clicked() ), this, SLOT( OnBnClickedBtDigitalGain() ) );
    connect( m_btBlackLevelSet,     SIGNAL( clicked() ), this, SLOT( OnBnClickedBtBlackLevel() ) );

    QGridLayout *lExpGridLayout = new QGridLayout;
    lExpGridLayout->addWidget( lAnalogGainLabel, 0, 0, 1, 3 );
    lExpGridLayout->addWidget( m_cbAnalogGain, 0, 4, 1, 4 );

    lExpGridLayout->addWidget( lDigitalGainLabel, 1, 0, 1, 3 );
    lExpGridLayout->addWidget( m_edDigitalGainCode, 1, 4, 1, 3 );
    lExpGridLayout->addWidget( m_btDigitalGainSet, 1, 7, 1, 1);
    lExpGridLayout->addWidget( lDigitalGainLabel2, 2, 0, 1, 4 );
    lExpGridLayout->addWidget( m_edDigitalGain, 2, 4, 1, 3);
    lExpGridLayout->addWidget( lDigitalGainDecibels, 3, 0, 1, 4 );
    lExpGridLayout->addWidget( m_edDigitalGainDB, 3, 4, 1, 3 );
    lExpGridLayout->addWidget( lDigitalGainDB, 3, 7, 1, 1);

    QGridLayout *lExpGridFrameLayout = new QGridLayout;
    lExpGridFrameLayout->addWidget( lBlackLevelAutoCalLabel, 0, 0, 1, 3);
    lExpGridFrameLayout->addWidget( m_cbBlackLevelAutoCal, 0, 4, 1, 4);
    lExpGridFrameLayout->addWidget( lBlackLevelLabel, 1, 0, 1, 3);
    lExpGridFrameLayout->addWidget( m_edBlackLevel, 1, 2, 1, 2 );
    lExpGridFrameLayout->addWidget( m_btBlackLevelSet, 1, 4, 1, 1);

    QVBoxLayout *lExpLayout = new QVBoxLayout;
    lExpLayout->addLayout( lExpGridLayout );
    lExpLayout->addLayout( lExpGridFrameLayout);
    lExpLayout->addStretch();

    QGroupBox *lGainBox = new QGroupBox( tr( "Gain" ) );
    lGainBox->setLayout( lExpLayout );
    lGainBox->setMinimumWidth( 300 );
    lGainBox->setMaximumWidth( 300 );
    
    return lGainBox;
}

// =============================================================================
// UpdateToolBarButtons will enable/disable the Toolbar buttons according the connection state
// =============================================================================
void QtCheetahPythonGainUI::UpdateToolbarButtons()
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
// =============================================================================
// EnableControls will enable/disable the GUI controls
// =============================================================================
void QtCheetahPythonGainUI::EnableControls(bool en)
{
    m_btDigitalGainSet->setEnabled(en);
    m_edDigitalGainCode->setEnabled(en);
    m_btBlackLevelSet->setEnabled(en);
    m_edBlackLevel->setEnabled(en);
    m_edDigitalGain->setEnabled(en);
    m_edDigitalGainDB->setEnabled(en);
}
// =============================================================================
// OnBnClickedBtDigitalGain
// =============================================================================
void QtCheetahPythonGainUI::OnBnClickedBtDigitalGain()
{
    // Get DigitalGain value form GUI
    IpxCamErr err;

    QString strVal = m_edDigitalGainCode->text();

    unsigned int digital_gain_code = (unsigned int)strVal.toLongLong();

    int64_t prev_digital_code = 0;
    // Set Parameter value
    auto paramArray = m_camera->GetCameraParameters();

    if(m_bNewXml)
    {
        prev_digital_code = paramArray->GetIntegerValue("DigitalGainRaw", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "Unable to retrieve DigitalGainRaw");
    }
    else
    {
        prev_digital_code = paramArray->GetIntegerValue("Frame_DigitalGain", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "Unable to retrieve Frame_DigitalGain");
    }

    if(digital_gain_code >= 1024 && digital_gain_code <= 16383)
    {
        if(m_bNewXml)
            err = paramArray->SetIntegerValue("DigitalGainRaw",digital_gain_code);
        else
            err = paramArray->SetIntegerValue("Frame_DigitalGain",digital_gain_code);

        if( err != IPX_CAM_ERR_OK)
        {
            QMessageBox::critical(this,"Error", "Digital Gain set failed");
        }
        else
        {
            //digital gain
            double digital_gainVal = floor(10*.0009765625*digital_gain_code + 0.5f)/10;
            m_edDigitalGain->setText(QString::number(digital_gainVal, 'f', 1));
            //digital gain in decibels
            double digital_gainDB = 20.0*log10(digital_gainVal);
            m_edDigitalGainDB->setText(QString::number(digital_gainDB, 'f', 1));
        }

    }
    else
    {
        m_edDigitalGainCode->setText(QString::number(prev_digital_code));

        //digital gain
        double digital_gainVal = floor(10*.0009765625*prev_digital_code + 0.5f)/10;
        m_edDigitalGain->setText(QString::number(digital_gainVal, 'f', 1));

        //digital gain in decibels
        double digital_gainDB = 20.0*log10(digital_gainVal);
        m_edDigitalGainDB->setText(QString::number(digital_gainDB, 'f', 1));

        QString msg;
        msg.sprintf("The value of %d is not valid.\nThis value is out of the specified range(1024, 16383).\nUnable to update", digital_gain_code);
        QMessageBox::critical(this, "Error", msg);

    }
}

// =============================================================================
// OnBnClickedBtBlackLevel
// =============================================================================
void QtCheetahPythonGainUI::OnBnClickedBtBlackLevel()
{
    IpxCamErr err;

    int64_t prev_black_level = 0;

    // Set Parameter value
    auto paramArray = m_camera->GetCameraParameters();

    if(m_bNewXml)
    {
        prev_black_level = (int64_t)paramArray->GetFloatValue("BlackLevel", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "Unable to retrieve BlackLevel");
    }
    else
    {
        //This featurs retrieves the black level offset for all taps
        prev_black_level = paramArray->GetIntegerValue("Frame_BlackLevelOffset", &err);
        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
            QMessageBox::critical(this, "Error", "Unable to retrieve Frame_BlackLevelOffset");
    }

    int signed_value;
    int int_prev_black_level;
    signed_value = (prev_black_level&0x200) >> 9;

    if(signed_value)
        int_prev_black_level = -1*(prev_black_level&0x1ff);
    else
        int_prev_black_level = prev_black_level&0x1ff;

    // Set Black Level
    int BlackLevel =  m_edBlackLevel->text().toInt();

    int64_t black_level_reg_val = 0;

    if(BlackLevel <= 511 && BlackLevel >= -511)
    {
            if(BlackLevel < 0)
            {
                black_level_reg_val = (0x200) | (abs(BlackLevel)&0x1ff);
            }
            else
            {
                black_level_reg_val = BlackLevel&0x1ff;
            }

            if(m_bNewXml)
                err = paramArray->SetFloatValue("BlackLevel",(double)black_level_reg_val);
            else
                err = paramArray->SetIntegerValue("Frame_BlackLevelOffset",black_level_reg_val);

            if( err != IPX_CAM_ERR_OK)
            {
                QMessageBox::critical(this, "Error", "Black Level set failed");
            }
    }
    else
    {
            QString strVal;
            strVal.sprintf("%i", int_prev_black_level);
            m_edBlackLevel->setText(strVal);

            QString msg;
            msg.sprintf("The black level value of %d is not valid.\nThis value is out of the specified range.\nUnable to update", BlackLevel);
            QMessageBox::critical(this, "Error", msg);
            QMessageBox::critical(this, "Error", "The Black Level value must be in the range of -511 to 511");
    }

    return;

}
// =============================================================================
// OnParameterUpdate occurs during an event callback
// =============================================================================
void QtCheetahPythonGainUI::OnParameterUpdate(IpxGenParam::Param* param)
{

    if(m_camera)
    {
        if(param->GetType() == IpxGenParam::ParamFloat)
        {
            IpxGenParam::Float *parameter = dynamic_cast<IpxGenParam::Float *>(param);
            if (parameter && parameter->IsReadable())
            {
                const char *paramName = parameter->GetDisplayName();
                //This feature controls the Digital Gain for all taps
                if (strcmp("Frame_DigitalGain", paramName)  == 0)
                {
                    int64_t DigitalGainCode = parameter->GetValue();

                    //Update the Edit Box with the Frame_DigitalGain value retrieved
                    m_edDigitalGainCode->setText(QString::number(DigitalGainCode));

                    //Convert and display Digital Gain Value
                    double DigitalGain = floor(10*.0009765625*DigitalGainCode + 0.5f)/10;
                    m_edDigitalGain->setText(QString::number(DigitalGain, 'f', 1));

                    //Convert and display Digital Gain Decibels
                    double DigitalGainDB = 20.0*log10(DigitalGain);
                    m_edDigitalGainDB->setText(QString::number(DigitalGainDB));
                    m_edDigitalGainDB->setEnabled(0);

                }
                else if (strcmp("DigitalGainRaw", paramName)  == 0)
                {
                    int64_t DigitalGainCode = parameter->GetValue();

                    //Update the Edit Box with the Frame_DigitalGain value retrieved
                    m_edDigitalGainCode->setText(QString::number(DigitalGainCode));

                    //Convert and display Digital Gain Value
                    double DigitalGain = floor(10*.0009765625*DigitalGainCode + 0.5f)/10;
                    m_edDigitalGain->setText(QString::number(DigitalGain));

                    //Convert and display Digital Gain Decibels
                    double DigitalGainDB = 20.0*log10(DigitalGain);
                    m_edDigitalGainDB->setText(QString::number(DigitalGainDB, 'f', 1));
                    m_edDigitalGainDB->setEnabled(0);

                }
                //This feature controls the Black Level Offset for all taps
                else if((strcmp("Frame_BlackLevelOffset", paramName) == 0) || (strcmp("BlackLevel", paramName) == 0))
                {
                    int calEn = m_cbBlackLevelAutoCal->currentIndex();

                    //If BlackLevelAutoCalibration is Off, then retrieve the Black Level Offset and display its value
                    if(calEn == 0)
                    {
                        int64_t prev_black_level = 0;
                        int64_t int_prev_black_level;
                        IpxCamErr err;
                        auto paramArray = m_camera->GetCameraParameters();

                        if(m_bNewXml)
                            prev_black_level = (int64_t)(paramArray->GetFloatValue("BlackLevel", &err));
                        else
                            prev_black_level = paramArray->GetIntegerValue("Frame_BlackLevelOffset", &err);

                        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
                            QMessageBox::critical(this, "Error", "Unable to retrieve BlackLevel");

                        int signed_value = (prev_black_level&0x200) >> 9;
                        if(signed_value)
                            int_prev_black_level = -1*(prev_black_level&0x1ff);
                        else
                            int_prev_black_level = prev_black_level&0x1ff;

                        m_edBlackLevel->setText(QString::number(int_prev_black_level));
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
                //This feature controls the Digital Gain for all taps
                if (strcmp("Frame_DigitalGain", paramName)  == 0)
                {
                    int64_t DigitalGainCode = parameter->GetValue();

                    //Update the Edit Box with the Frame_DigitalGain value retrieved
                    m_edDigitalGainCode->setText(QString::number(DigitalGainCode));

                    //Convert and display Digital Gain Value
                    double DigitalGain = floor(10*.0009765625*DigitalGainCode + 0.5f)/10;
                    m_edDigitalGain->setText(QString::number(DigitalGain, 'f', 1));

                    //Convert and display Digital Gain Decibels
                    double DigitalGainDB = 20.0*log10(DigitalGain);
                    m_edDigitalGainDB->setText(QString::number(DigitalGainDB, 'f', 1));

                }
                else if (strcmp("DigitalGainRaw", paramName)  == 0)
                {
                    int64_t DigitalGainCode = parameter->GetValue();

                    //Update the Edit Box with the Frame_DigitalGain value retrieved
                    m_edDigitalGainCode->setText(QString::number(DigitalGainCode));

                    //Convert and display Digital Gain Value
                    double DigitalGain = floor(10*.0009765625*DigitalGainCode + 0.5f)/10;
                    m_edDigitalGain->setText(QString::number(DigitalGain, 'f', 1));

                    //Convert and display Digital Gain Decibels
                    double DigitalGainDB = 20.0*log10(DigitalGain);
                    m_edDigitalGainDB->setText(QString::number(DigitalGainDB, 'f', 1));

                }
                //This feature controls the Black Level Offset for all taps
                else if((strcmp("Frame_BlackLevelOffset", paramName) == 0) || (strcmp("BlackLevel", paramName) == 0))
                {
                    int calEn = m_cbBlackLevelAutoCal->currentIndex();

                    //If BlackLevelAutoCalibration is Off, then retrieve the Black Level Offset and display its value
                    if(calEn == 0)
                    {
                        int64_t prev_black_level = 0;
                        int64_t int_prev_black_level;
                        IpxCamErr err;
                        auto paramArray = m_camera->GetCameraParameters();

                        if(m_bNewXml)
                            prev_black_level = (int64_t)(paramArray->GetFloatValue("BlackLevel", &err));
                        else
                            prev_black_level = paramArray->GetIntegerValue("Frame_BlackLevelOffset", &err);

                        if(err != ((IpxCamErr)IPX_CAM_ERR_OK))
                            QMessageBox::critical(this, "Error", "Unable to retrieve BlackLevel");

                        int signed_value = (prev_black_level&0x200) >> 9;
                        if(signed_value)
                            int_prev_black_level = -1*(prev_black_level&0x1ff);
                        else
                            int_prev_black_level = prev_black_level&0x1ff;

                        m_edBlackLevel->setText(QString::number(int_prev_black_level));
                    }
                }
            }
        }

    }
}
