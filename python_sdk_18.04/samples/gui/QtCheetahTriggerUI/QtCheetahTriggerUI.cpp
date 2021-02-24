#include "QtCheetahTriggerUI.h"

#include "IpxCameraApi.h"
#include "IpxCameraGuiApi.h"
#include "IpxImage.h"
#include "IpxDisplay.h"

#include <QTimer>
#include <QGroupBox>
#include <QSettings>

#include "../common/inc/GrabbingThread.h"
#include "../common/inc/ipxparammaphelper.h"

// ===========================================================================
//  QtCheetahTriggerUI Constructor
//
//	The Constructor initializes the ThreadDisplay, Camera, IpxDisplay, IpxStream, IpxDeviceInfo
//  modules to null. Then, it loads the Imperx Icon and initializes the System of the
//  GenTL Module Hierachy.
// =============================================================================
QtCheetahTriggerUI::QtCheetahTriggerUI(QWidget *parent)
    : QMainWindow(parent)
    , m_timer(nullptr)
    , m_system(nullptr)
    , m_camera(nullptr)
    , m_stream(nullptr)
    , m_devInfo(nullptr)
    , m_grabThread(nullptr)
    , m_display(IpxDisplay::CreateComponent())
    , m_parameterView(nullptr)
    , m_mapData(new IpxGui::IpxCamConfigMapper)
    , m_isConnected (false)
{
    ui.setupUi(this);

    CreateLayout();

    //-------------------- Connect the GUI signals with slots --------------------//
    connect(ui.actionConnect, &QAction::triggered, this, &QtCheetahTriggerUI::Connect);
    connect(ui.actionDisconnect, &QAction::triggered, this, &QtCheetahTriggerUI::Disconnect);

    connect(ui.actionPlay, &QAction::triggered, [this](){
        // Check if the Imperx Camera is Connected
        if (!m_isConnected)
            return;

        StartAcquisition();
        UpdateToolbarButtons();
    });

    connect(ui.actionStop, &QAction::triggered, [this](){
        // Check if the Imperx Camera is Connected
        if (!m_isConnected)
            return;

        StopAcquisition();
        UpdateToolbarButtons();
    });

    connect(ui.actionGenICamTree, &QAction::triggered, [this](){
        if (m_parameterView)
            m_parameterView->show();
    });

    connect(ui.actionZoomIn, &QAction::triggered, this, [this](){
        m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_IN);
    });

    connect(ui.actionZoomOut, &QAction::triggered, this, [this](){
        m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ZOOM_OUT);
    });

    connect(ui.actionFitToWindow, &QAction::triggered, this, [this](){
        m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 1);
    });

    connect(ui.actionSpreadToWindow, &QAction::triggered, [this](){
            m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 2);
    });

    connect(ui.actionActualSize, &QAction::triggered, this, [this](){
        m_display->GetComponent()->SetParamInt((char*)IDP_VIEW_FIT, 3);
    });

    connect(ui.actionCenterImage, &QAction::triggered, [this](){
        m_display->GetComponent()->RunCommand((char*)IDPC_CMD_VIEW_ATCENTER);
    });

    connect(this, &QtCheetahTriggerUI::cameraDisconnected, this, [this](){
        Disconnect();
        QMessageBox::critical(this, "ERROR", "The camera has lost connection!");
        }, Qt::QueuedConnection
    );
    //----------------------------------------------------------------------------//

    //----------------------- Create and initialize System -----------------------//
    if (!(m_system = IpxCam::IpxCam_GetSystem()))
        QMessageBox::critical(this, "Error", "ERROR: Unable to create the System Object.");
    //----------------------------------------------------------------------------//

    //-------------------------- Initialize the display --------------------------//
    if (m_display)
    {
        m_display->Initialize(m_displayLabel);
        ui.actionFitToWindow->trigger();
    }
    else
    {
        QMessageBox::critical(this, "", "ERROR: Unable to create the Display Object.");
    }
    //----------------------------------------------------------------------------//

    UpdateToolbarButtons();
}

void QtCheetahTriggerUI::closeEvent(QCloseEvent * event)
{
    // Disconnect first
    Disconnect();

    // because m_parameterView has not been given a parent we need to clean it up
    // make sure that parameter a set to nullptr
    delete m_parameterView;
    m_parameterView = nullptr;

    // Delete the grabbing thread
    delete m_grabThread;
    m_grabThread = nullptr;

    // Delete mapper
    delete m_mapData;
    m_mapData = nullptr;

    // Delete Display object, since main Window to be closed
    IpxDisplay::DeleteComponent(m_display);
    m_display = nullptr;

    QMainWindow::closeEvent(event);
}

void QtCheetahTriggerUI::CreateLayout()
{
    QGroupBox *lInfoBox = CreateInfoGroup();
    QGroupBox *lTriggerBox = CreateTriggerGroup();

    QVBoxLayout *lLayoutLeft = new QVBoxLayout();
    lLayoutLeft->addWidget(lInfoBox);
    lLayoutLeft->addWidget(lTriggerBox);
    lLayoutLeft->addStretch();

    m_displayLabel = new QLabel;

    QVBoxLayout *displayLayout = new QVBoxLayout;
    displayLayout->addWidget(m_displayLabel);

    QGroupBox *lDisplayBox = new QGroupBox(tr("Display"));
    lDisplayBox->setLayout(displayLayout);

    QHBoxLayout *lMainLayout = new QHBoxLayout;
    lMainLayout->addLayout(lLayoutLeft);
    lMainLayout->addWidget(lDisplayBox);

    QFrame *lMainBox = new QFrame;
    lMainBox->setLayout(lMainLayout);

    setCentralWidget(lMainBox);
    setWindowTitle(tr("QtCheetahTriggerUI"));
}

QGroupBox *QtCheetahTriggerUI::CreateInfoGroup()
{
    QLabel *lManufacturerLabel = new QLabel(tr("Manufacturer"));
    mManufacturerEdit = new QLineEdit;
    mManufacturerEdit->setReadOnly(true);
    mManufacturerEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lDeviceFamilyLabel = new QLabel(tr("Device Family"));
    mDeviceFamilyEdit = new QLineEdit;
    mDeviceFamilyEdit->setReadOnly(true);
    mDeviceFamilyEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lModelLabel = new QLabel(tr("Model"));
    mModelEdit = new QLineEdit;
    mModelEdit->setReadOnly(true);
    mModelEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lNameLabel = new QLabel(tr("Name"));
    mNameEdit = new QLineEdit;
    mNameEdit->setReadOnly(true);
    mNameEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lVersionLabel = new QLabel(tr("Version"));
    mVersionEdit = new QLineEdit;
    mVersionEdit->setReadOnly(true);
    mVersionEdit->setFocusPolicy(Qt::NoFocus);

    QLabel *lSerialNbrLabel = new QLabel(tr("SerialNumber"));
    mSerialNbrEdit = new QLineEdit;
    mSerialNbrEdit->setReadOnly(true);
    mSerialNbrEdit->setFocusPolicy(Qt::NoFocus);

    QGridLayout *lGridLayout = new QGridLayout;
    lGridLayout->addWidget(lManufacturerLabel, 0, 0);
    lGridLayout->addWidget(mManufacturerEdit, 0, 1);

    lGridLayout->addWidget(lDeviceFamilyLabel, 1, 0);
    lGridLayout->addWidget(mDeviceFamilyEdit, 1, 1);

    lGridLayout->addWidget(lModelLabel, 2, 0);
    lGridLayout->addWidget(mModelEdit, 2, 1);

    lGridLayout->addWidget(lNameLabel, 3, 0);
    lGridLayout->addWidget(mNameEdit, 3, 1);

    lGridLayout->addWidget(lVersionLabel, 4, 0);
    lGridLayout->addWidget(mVersionEdit, 4, 1);

    lGridLayout->addWidget(lSerialNbrLabel, 5, 0);
    lGridLayout->addWidget(mSerialNbrEdit, 5, 1);

    QGroupBox *lInfoBox = new QGroupBox(tr("Device Info"));
    lInfoBox->setLayout(lGridLayout);
    lInfoBox->setMaximumWidth(300);

    return lInfoBox;
}

QGroupBox *QtCheetahTriggerUI::CreateTriggerGroup()
{
    QLabel *lModeLabel = new QLabel(tr("Mode:"));
    mTriggerMode = new QComboBox;
    mTriggerMode->setEnabled(false);

    QLabel *lSourceLabel = new QLabel(tr("Source:"));
    mTriggerSource = new QComboBox;
    mTriggerSource->setEnabled(false);

    QLabel *lActivationLabel = new QLabel(tr("Activation:"));
    mTriggerActivation = new QComboBox;
    mTriggerActivation->setEnabled(false);

    QLabel *lDebounceLabel = new QLabel(tr("Debounce:"));
    mTriggerDebounce = new QComboBox;
    mTriggerDebounce->setEnabled(false);

    mFlashSWTrigger = new QPushButton(tr("Software Trigger"));
    mFlashSWTrigger->setEnabled(false);

    QGridLayout *lExpGridLayout = new QGridLayout;
    lExpGridLayout->addWidget(lModeLabel, 0, 0);
    lExpGridLayout->addWidget(mTriggerMode, 0, 1);

    lExpGridLayout->addWidget(lSourceLabel, 1, 0);
    lExpGridLayout->addWidget(mTriggerSource, 1, 1);

    lExpGridLayout->addWidget(lActivationLabel, 2, 0);
    lExpGridLayout->addWidget(mTriggerActivation, 2, 1);

    lExpGridLayout->addWidget(lDebounceLabel, 3, 0);
    lExpGridLayout->addWidget(mTriggerDebounce, 3, 1);

    lExpGridLayout->addWidget(mFlashSWTrigger, 4, 1);

    QGroupBox *lTriggerBox = new QGroupBox(tr("Trigger Control"));
    lTriggerBox->setLayout(lExpGridLayout);
    lTriggerBox->setMaximumWidth(300);
    return lTriggerBox;
}

// =============================================================================
// Connect the Imperx Camera
// =============================================================================
void QtCheetahTriggerUI::Connect()
{
    Disconnect();

    // Open camera selection dialog
    m_devInfo = IpxGui::SelectCameraA(m_system, "Get Device Selection");

    // Check if any camera was selected
    if (!m_devInfo)
        return;

    // Create the Device object from DeviceInfo of selected Camera
    m_camera = IpxCam::IpxCam_CreateDevice(m_devInfo);
    if (!m_camera)
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve camera device.\r\nMake sure an Imperx Camera is connected." );
        return;
    }

    // Get the camera basic information and show it on GUI
    auto camParamArray = m_camera->GetCameraParameters();
    if (!camParamArray)
    {
        QMessageBox::critical(this, "ERROR", "Unable to retrieve camParamArray.");
        return;
    }

    IpxGenParam::String* lModelName = camParamArray->GetString("DeviceModelName", nullptr);
    if (lModelName)
    {
        mModelEdit->setText(lModelName->GetValue());
        mModelEdit->setCursorPosition(0); // text may not fit into edit
    }
    else
        QMessageBox::warning(this, "Warning", "Retrieving DeviceModelName failed.");

    IpxGenParam::String* lNameParam = camParamArray->GetString("DeviceUserDefinedName", nullptr);
    if (!lNameParam)
        lNameParam = camParamArray->GetString("DeviceUserID", nullptr);

    if (lNameParam)
    {
        mNameEdit->setText(lNameParam->GetValue());
        mNameEdit->setCursorPosition(0); // text may not fit into edit
    }
    else
        QMessageBox::warning(this, "Warning", "Retrieving DeviceUserID failed.");

    IpxGenParam::String* lManufacturer = camParamArray->GetString("DeviceManufacturerName", nullptr);
    if (!lManufacturer)
        lManufacturer = camParamArray->GetString("DeviceVendorName", nullptr);

    if (lManufacturer)
    {
        mManufacturerEdit->setText(lManufacturer->GetValue());
        mManufacturerEdit->setCursorPosition(0); // text may not fit into edit
    }
    else
        QMessageBox::warning(this, "Warning", "Retrieving DeviceVendorName failed.");

    IpxGenParam::String* lDeviceFamily = camParamArray->GetString("DeviceFamilyName", nullptr);
    if (lDeviceFamily)
    {
        mDeviceFamilyEdit->setText(lDeviceFamily->GetValue());
        mDeviceFamilyEdit->setCursorPosition(0); // text may not fit into edit
    }
    else
        QMessageBox::warning(this, "Warning", "Retrieving DeviceFamilyName failed.");

    IpxGenParam::String* lVersion = camParamArray->GetString("DeviceVersion", nullptr);
    if (lVersion)
    {
        mVersionEdit->setText(lVersion->GetValue());
        mVersionEdit->setCursorPosition(0); // text may not fit into edit
    }
    else
        QMessageBox::warning(this, "Warning", "Retrieving DeviceVersion failed.");

    IpxGenParam::String* lSerialNbr = camParamArray->GetString("DeviceSerialNumber", nullptr);
    if (lSerialNbr)
    {
        mSerialNbrEdit->setText(lSerialNbr->GetValue());
        mSerialNbrEdit->setCursorPosition(0); // text may not fit into edit
    }
    else
        QMessageBox::warning(this, "Warning", "Retrieving DeviceSerialNumber failed.");

    //////////////////////////////////////////////////////////////////////
    /// example how to map enum parameter with combo box
    //////////////////////////////////////////////////////////////////////

    m_mapData->mapEnumParam(camParamArray->GetEnum("TriggerMode", nullptr), mTriggerMode);
    m_mapData->mapEnumParam(camParamArray->GetEnum("TriggerSource", nullptr), mTriggerSource);
    m_mapData->mapEnumParam(camParamArray->GetEnum("TriggerActivation", nullptr), mTriggerActivation);
    m_mapData->mapEnumParam(camParamArray->GetEnum("TriggerDebounce", nullptr), mTriggerDebounce);

    //////////////////////////////////////////////////////////////////////
    // example how to use timer to disable button while IsDone is not true
    //////////////////////////////////////////////////////////////////////
    auto command = camParamArray->GetCommand("TriggerSoftware", nullptr);
    if (command)
    {
        // release an old one in case it exists
        if (m_timer)
        {
            m_timer->stop();
            delete m_timer;
        }

        m_timer = new QTimer(this);
        m_timer->setInterval(500);
        QObject::connect(m_timer, &QTimer::timeout, [this, command](){
            if (command->IsDone())
            {
                mFlashSWTrigger->setEnabled(true);
                m_timer->stop();
            }
        });
    }

    //////////////////////////////////////////////////////////////////////
    /// example how to map command parameter with push button
    //////////////////////////////////////////////////////////////////////
    m_mapData->mapCommandParam(command, mFlashSWTrigger, m_timer);

    //////////////////////////////////////////////////////////////////////
    /// other stuff
    //////////////////////////////////////////////////////////////////////

    // Get first Stream from the Camera
    m_stream = m_camera->GetStreamByIndex(0);
    if (!m_stream)
        QMessageBox::critical(this, "ERROR", "Unable to retrieve camera stream." );

    // Create the grabbing thread
    m_grabThread = new GrabbingThread(m_stream, m_display);

    //////////////////////////////////////////////////////////////////////
    /// Create tree view of the parameters
    //////////////////////////////////////////////////////////////////////
    if (!m_parameterView)
        // do not set parent so m_parameterView becomes a dialog
        m_parameterView = new IpxGui::IpxGenParamTreeView();

    m_parameterView->setParams(camParamArray);

    // restore the settings of tree view
    QSettings settings(QStringLiteral("MyCompanyName"), QStringLiteral("MyApplicationName"));
    auto dataToBeRestored = settings.value(mModelEdit->text() + QStringLiteral("treeViewState")).toByteArray();
    if (!dataToBeRestored.isEmpty())
        m_parameterView->loadState(dataToBeRestored.constData());

    m_parameterView->restoreGeometry(settings.value(QStringLiteral("treeViewGeometry")).toByteArray());

    //////////////////////////////////////////////////////////////////////
    /// Register callback
    //////////////////////////////////////////////////////////////////////
    // Because we register event for a device we cannot release this device in the callback function
    // QtCommonStreamUI::CameraDisconnectCallback. So we will use signal slot connection with type
    // Qt::QueuedConnection. In other words callback should return before releasing the device object.
    // SEE connection in the constructor!
    m_camera->RegisterEvent(IpxCam::Device::CameraDisconnected, &QtCheetahTriggerUI::CameraDisconnectCallback, this);

    // Also you can register callback on an interface for device arrival and device left event
    // You will receive device id in the parameters eventData and eventSize of the callback function
    // Let us save the device's id to check later if we get notification for our device
    // Do not forget to comment registration line above so not to get notified twice if you want to use next line
    // And make sure you use correct QtCommonStreamUI::CameraDisconnectCallback method
    //m_camera->GetInfo()->GetInterface()->RegisterEvent(IpxCam::Device::CameraDisconnected,
    //                                                   &QtCheetahTriggerUI::CameraDisconnectCallback, this);

    m_isConnected = true;
    UpdateToolbarButtons();
}

// =============================================================================
// Disconnect the Imperx Camera
// =============================================================================
void QtCheetahTriggerUI::Disconnect()
{
    if (!m_isConnected)
        return;

    // Stop the Acquisition
    if (m_grabThread)
    {
        if (m_grabThread->IsStarted())
            StopAcquisition();

        delete m_grabThread;
        m_grabThread = nullptr;
    }

    // crear mapper in case is not empty
    if (m_mapData)
    {
        IpxGenParam::Array *paramArray = nullptr;
        if (m_camera)
            paramArray = m_camera->GetCameraParameters();

        m_mapData->clear(paramArray);
    }

    if (m_parameterView)
    {
        // save the settings of the tree view
        QSettings settings(QStringLiteral("MyCompanyName"), QStringLiteral("MyApplicationName"));
        auto dataToBeSaved = m_parameterView->saveState();
        if (dataToBeSaved)
            settings.setValue(mModelEdit->text() + QStringLiteral("treeViewState"), QByteArray(dataToBeSaved));

        // save geometry of the tree view
        settings.setValue(QStringLiteral("treeViewGeometry"), m_parameterView->saveGeometry());

        // clear parameters in tree view before m_camera(IpxDevice) is dead
        m_parameterView->clearParams();
    }

    QString empty;
    mModelEdit->setText(empty);
    mNameEdit->setText(empty);
    mManufacturerEdit->setText(empty);
    mDeviceFamilyEdit->setText(empty);
    mVersionEdit->setText(empty);
    mSerialNbrEdit->setText(empty);

    // Release the Stream object
	if(m_stream)
		m_stream->Release();
	m_stream = nullptr;
	
    // Release the Camera Device object
    if(m_camera)
        m_camera->Release();
    m_camera = nullptr;

    m_isConnected = false;
    UpdateToolbarButtons();
}

// =============================================================================
// Start the Acquisition engine on the host. It will call the GenTL consumer to
// start the transfer
// =============================================================================
void QtCheetahTriggerUI::StartAcquisition()
{
    QString errMsg("Sorry! Cannot start streaming.");

    // Allocate the minimal required buffer queue
    if (m_stream && m_stream->AllocBufferQueue(this, m_stream->GetMinNumBuffers()) == IPX_CAM_ERR_OK)
    {
        auto genParams = m_camera->GetCameraParameters();
        if (genParams)
        {
            // Set the TLParamsLocked parameter to lock the parameters effecting to the Transport Layer
            if (genParams->SetIntegerValue("TLParamsLocked", 1) == IPX_CAM_ERR_OK)
            {
                if (m_stream->StartAcquisition() == IPX_CAM_ERR_OK)
                {
                    m_grabThread->Start();
                    if (genParams->ExecuteCommand("AcquisitionStart") == IPX_CAM_ERR_OK)
                        return;

                    errMsg += "\r\nAcquisitionStart command failed.";
                    m_stream->StopAcquisition();
                }
                else
                    errMsg += "\r\nCheck if there is enough memory.\r\nParticularly, on Linux machine!";

                genParams->SetIntegerValue("TLParamsLocked", 0);
            }
        }

        m_stream->ReleaseBufferQueue();
    }

    QMessageBox::critical(this, "ERROR", errMsg);
}

// =============================================================================
// Stop the Acquisition engine on the host. It will call the GenTL consumer to
// stop the transfer
// =============================================================================
void QtCheetahTriggerUI::StopAcquisition()
{
    // Get camera parrameters array
    auto genParams = m_camera->GetCameraParameters();
    if (genParams)
    {
        /// If any error occured with AcquisitionStop, run AcquisitionAbort
        if (genParams->ExecuteCommand("AcquisitionStop") != IPX_CAM_ERR_OK)
            genParams->ExecuteCommand("AcquisitionAbort");

        // Reset the TLParamsLocked parameter to unlock the parameters effecting to the Transport Layer
        genParams->SetIntegerValue("TLParamsLocked", 0);

        m_grabThread->Stop();
        if (m_stream)
        {
            m_stream->StopAcquisition();
            m_stream->ReleaseBufferQueue();
        }
    }
}

// =============================================================================
// UpdateToolBarButtons will enable/disable the Toolbar buttons according the connection state
// =============================================================================
void QtCheetahTriggerUI::UpdateToolbarButtons()
{
    auto actions = ui.toolBarFile->actions();
    for (QAction *action: actions)
        action->setEnabled(m_isConnected);

    actions = ui.toolBarDisplay->actions();
    for (QAction *action: actions)
        action->setEnabled(m_isConnected);

    actions = ui.toolBarView->actions();
    for (QAction *action: actions)
        action->setEnabled(m_isConnected);

    // connect action should be opposite
    ui.actionConnect->setEnabled(!ui.actionConnect->isEnabled());

    if (m_isConnected)
    {
        if (!m_grabThread || !m_grabThread->IsStarted())
            ui.actionStop->setEnabled(false);
        else
            ui.actionPlay->setEnabled(false);
    }

    EnableControls(m_isConnected);
}

// =============================================================================
// EnableControls will enable/disable the GUI controls
// =============================================================================
void QtCheetahTriggerUI::EnableControls(bool en)
{
    mTriggerActivation->setEnabled(en);
    mTriggerDebounce->setEnabled(en);
    mTriggerSource->setEnabled(en);
    mTriggerMode->setEnabled(en);
    mFlashSWTrigger->setEnabled(en);
}

// =============================================================================
// Callback to be called when camera disconnects
// =============================================================================
void IPXCAM_CALL QtCheetahTriggerUI::CameraDisconnectCallback(const void *eventData, size_t eventSize, void *pPrivate)
{
    QtCheetahTriggerUI *pThis = static_cast<QtCheetahTriggerUI*>(pPrivate);
    Q_EMIT pThis->cameraDisconnected();
}

// =============================================================================
// Callback to be called when camera disconnects
// =============================================================================
// remove comments if you registered callback for interface
// and comment the method above
/*void IPXCAM_CALL QtCheetahTriggerUI::CameraDisconnectCallback(const void *eventData, size_t eventSize, void *pPrivate)
{
    auto pThis = static_cast<QtCommonStreamUI*>(pPrivate);
    auto idEvent = static_cast<const char*>(eventData);
    auto id = pThis->m_camera->GetInfo()->GetID();
    if (id && idEvent && eventSize)
    {
        QByteArray str;
        str.setRawData(idEvent, (uint)eventSize);
        if (str.startsWith(id))
            Q_EMIT pThis->cameraDisconnected();
    }
}*/
