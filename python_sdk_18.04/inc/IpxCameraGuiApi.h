////////////////////////////////////////////////////////////////////////////////
// Imperx Camera SDK
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File: IpxCameraGuiApi.h
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Imperx Camera GUI API Function and Constant Declarations
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Created:    14-OCT-2014
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2013-2016 Imperx Inc. All rights reserved. 
////////////////////////////////////////////////////////////////////////////////

#ifndef IPX_CAMERA_GUI_API_H
#define IPX_CAMERA_GUI_API_H

// TODO: generic definitions - should be defined in one common header file
#if defined _WIN32 || defined __CYGWIN__
    #define LIB_HELPER_DLL_IMPORT __declspec(dllimport)
    #define LIB_HELPER_DLL_EXPORT __declspec(dllexport)
    #define LIB_HELPER_DLL_LOCAL
#elif __GNUC__ >= 4
    #define LIB_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define LIB_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define LIB_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
    #define LIB_HELPER_DLL_IMPORT
    #define LIB_HELPER_DLL_EXPORT
    #define LIB_HELPER_DLL_LOCAL
#endif
//////////////////////////////////////////////////////////////////////////

#ifndef IPXCAM_GUI_EXTERN_C
    #ifdef __cplusplus
        #define IPXCAM_GUI_EXTERN_C extern "C"
    #else
        #define IPXCAM_GUI_EXTERN_C
    #endif
#endif // IPXCAM_GUI_EXTERN_C

#ifndef IPXCAM_GUI_CALL
    #ifdef _WIN32
        #define IPXCAM_GUI_CALL __stdcall
    #else
        #define IPXCAM_GUI_CALL
    #endif
#endif // IPXCAM_GUI_CALL

// check if static library is wanted
#ifdef IPXCAM_GUI_STATIC
    // define the static library symbol
    #define IPX_CAMERA_GUI_API
    #define IPX_CAMERA_GUI_LOCAL
#else
    #define IPX_CAMERA_GUI_LOCAL LIB_HELPER_DLL_LOCAL
    #ifdef IPX_CAMERA_GUI_API_EXPORTS
        // define the export symbol
        #define IPX_CAMERA_GUI_API LIB_HELPER_DLL_EXPORT

        #if defined _WIN32 && !defined _M_AMD64
            // For 32b DLL only. If method name is mangled, add unmangled name below.
            //#pragma comment(linker, "/EXPORT:IpxCam_GetSystem=_IpxCam_GetSystem()@0")
        #endif
    #else // IPX_CAMERA_GUI_API_EXPORTS
        // define the import symbol
        #define IPX_CAMERA_GUI_API LIB_HELPER_DLL_IMPORT
    #endif // IPX_CAMERA_GUI_API_EXPORTS
#endif // IPXCAM_GUI_STATIC


// Note For MSVC < 2010
// The C99 standard includes definitions of several new integer types to 
// enhance the portability of programs[2]. The already available basic 
// integer types were deemed insufficient, because their actual sizes are 
// implementation defined and may vary across different systems. The new 
// types are especially useful in embedded environments where hardware supports
// usually only several types and that support varies from system to system. 

// Unfortunately Microsoft Visual C++ before 2010 did not support C99 standard
// In this case you can download this file from
// https://msinttypes.googlecode.com/svn/trunk/stdint.h

#include <stdint.h>

namespace IpxGenParam 
{
    class Array;
}

namespace IpxCam 
{
    class System;
    class Device;
    class DeviceInfo;
}

// define correct name of GenApi namespace
#ifndef IPX_GENAPI_NS
    #ifdef GENAPI_NAMESPACE
        #define IPX_GENAPI_NS GENAPI_NAMESPACE
    #else
        #define IPX_GENAPI_NS GenApi_3_0
    #endif // GENAPI_NAMESPACE
#endif // IPX_GENAPI_NS

// forward declaration must have real name not an alias
namespace IPX_GENAPI_NS { struct INodeMap; }

/*! \namespace IpxGui
    \brief The IpxGUI namespace is a declarative region that provides a scope to the Imperx Camera GUI API classes and functions.

    The IpxGUI namespace includes Imperx Camera GUI API classes and functions, such as:
        IIpxGenParamTreeView,
        SelectCameraA
        SelectCameraW
        IpxGenParamTreeView,
        IpxCameraSelectorDialog

*/

namespace IpxGui
{
	//! An enum of Visibility. Defines the visibility type of features that user will see in the Tree View.
	enum Visibility : uint32_t
	{
		Beginner = 0,    /*!< Enum value Beginner. User has visibility to all the basic features of the device.*/
		Expert,          /*!< Enum value Expert. User has visibility to more advance features of the device.*/
		Guru             /*!< Enum value Guru. User has visibility to even more advance features that if set improperly can cause device to be in an improper state.*/
	};

	/**
	*
	\brief IIpxGenParamTreeView class represents the GenICam parameters node tree panel.
	\details The IIpxGenParamTreeView class is composed of functions to set and clear parameters of the GenICam parameters
	node tree of the camera. The node tree can be set with the current parameters stored in
	the IpxGenParam::Array and GenApi::INodeMap class.

	For example, we can declare the instance of IpxGui::IIpxGenParamTreeView class as m_parameterView as shown below:

	\code{.cpp}
	IpxGui::IIpxGenParamTreeView* m_ParameterView;
	\endcode

	*/
	class IIpxGenParamTreeView
	{
	public:

		//! A destructor of the IIpxGenParamTreeView class.
		/*!
		\details Destroys the IIpxGenParamTreeView object and all its descendants.
		*/
		virtual ~IIpxGenParamTreeView() {}

		//! Sets the IpxGenParam::Array object to the node tree GUI
		/*!
		This method sets the parameters of the node tree by the information extracted from the IpxGenParam::Array class
		\param[in] genParam The pointer to the IpxGenParam::Array class.
		\return    void

		For example, set the Camera Parameters to the corresponding fields of the TreeView as shown below:
		\code{.cpp}
		// Establish camera device
		m_camera = IpxCam_CreateDevice(m_devInfo);

		// If the camera exist, set the camera parameter to the corresponding fields of the GUI TreeView
		if(m_camera){
		  m_ParameterView->setParams(m_camera->GetCameraParameters());
		}
		\endcode
		*/
		virtual void setParams(IpxGenParam::Array *genParam) = 0;

		//! Sets the GenApi::INodeMap object to the node tree GUI
		/*!
		This method sets the parameters of the node tree with parameters retrieved from the GenApi::INodeMap class
		The INodeMap consists of a list of nodes representing the GenICam compliant camera high-level features.
		\param[in]  nodemap The pointer to the GenApi::INodeMap class.
		\return     Void.

		For example, setting the parameters of the node map.
		\code{.cpp}
		// Instantiate the IpxGui::IIpxGenParamTreeView
		IpxGui::IIpxGenParamTreeView* m_ParameterView;
		...
		auto params = GetCameraParameters(&retErr);
		if(!params) {
		   return retErr;
		}
		GenApi::INodeMap *nodemap = param->GetNodeMap(&retErr);
		if(!nodemap){
			return retErr;
		}
		...
		// Set the nodemap parameters of the GUI TreeView

		m_ParameterView->setParams(nodemap);
		...
		\endcode
		*/
		virtual void setParams(IPX_GENAPI_NS::INodeMap *nodemap) = 0;

		//! Clears the parameters of the node tree GUI
		/*!
		This method clears the parameters of the node tree that have been set by the instance of the IpxGui::IIpxGenParamTreeView class
		\return void.

		For example, clear all the parameters after we disconnect the camera as shown below:
		\code{.cpp}
		// Instantiate the IpxGui::IIpxGenParamTreeView
		IpxGui::IIpxGenParamTreeView* m_ParameterView;

		// Connect the camera
		...
		// Set some camera parameters
		...
		// Perform some actions
		...
		// Clear parameters during disconnecting process of camera
		m_ParameterView->clearParam();
		\endcode
		*/
		virtual void clearParams() = 0;

		//! This method returns the current visibility mode.
		/*!
		This method retrieves the current setting of the user visibility level for the feature
		\return Visibility value
		*/
		virtual Visibility visibility() const = 0;

		//! This method sets visibility mode.
		/*!
		It sets the current visibility level for the feature.
		\param[in] visibility The visibility mode value to set
		\return Void.
		*/
		virtual void setVisibility(Visibility visibility) = 0;

		//! Saves the current state of the Tree View
		/*!
		This method creates the string, representing the current state of the Tree View, and returns the pointer to this string.
		\return If succeeds, the method returns pointer to the state string. Otherwise, the return value is nullptr.
		\brief The string consists of sub-string values separated by the token.
		Just save this data somewhere if you want to restore the state later.
		*/
		virtual const char* saveState() const = 0;

		//! Loads the state of the Tree View.
		/*!
		This method loads the state of the Tree View using the string, created by saveState() method.
		The individual node can be in expanded or collapse state.
		\param[in] state State string to be loaded. The string consists of sub-string values separated by the token.
		*/
		virtual void loadState(const char *state) = 0;

		//! Sets the polling time
		/*!
		This method sets the value of the parameters pooling time. Polling should be enabled by enablePolling() function
		\param[in] pollingTime time in msec to be set
		*/
		virtual void setPollingTime(uint64_t pollingTime) = 0;

		//! Retrieves current polling time
		/*!
		This method retrieves the value of the parameters polling time. Polling should be enabled by enablePolling() function
		\return current polling time in msec
		*/
		virtual uint64_t getPollingTime() = 0;

		//! Enables the polling
		/*
		This method enables the parameters polling with the polling time, set by setPollingTime() function
		\param[in] enable Enable or disable polling
		*/
		virtual void enablePolling(bool enable) = 0;

		//! Retrieves current polling state
		/*
		This method retrieves the current polling state, previously set by enablePolling() function
		\return current polling state
		*/
		virtual bool isPollingEnabled() = 0;

	};

	//! Creates the panel of the camera GenICam parameters for IpxGenParam::Array object
	/*!
	This function returns the pointer to the IIpxGenParamTreeView class that was created using information extracted from the IpxGenParam::Array class.
	\param[in] genParam       The pointer to the IpxGenParam::Array class.
	\param[in] title          The title of the IIpxGenParamTreeView class as a const char.
	\param[in] parentWindow   A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	\return                   If the function succeeds, the return value is the pointer to the IIpxGenParamTreeView class created. Otherwise, the return value is nullptr.
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API IIpxGenParamTreeView* CreateGenParamTreeViewForArrayA(IpxGenParam::Array *genParam, const char *title, uintptr_t parentWindow = 0);

	//! Creates the panel of the camera GenICam parameters for IpxGenParam::Array object. Unicode version.
	/*!
	This function returns the pointer to the IIpxGenParamTreeView class that was created using information extracted from the IpxGenParam::Array.
	\param[in] genParam        The pointer to the IpxGenParam::Array class.
	\param[in]  title          The title of the IIpxGenParamTreeView class as a wchar_t variable.
	\param[in]  parentWindow   A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	\return                    If the function succeeds, the return value is the pointer to the IIpxGenParamTreeView class created. Otherwise, the return value is nullptr.
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API IIpxGenParamTreeView* CreateGenParamTreeViewForArrayW(IpxGenParam::Array *genParam, const wchar_t *title, uintptr_t parentWindow = 0);

	//! Creates the panel of the camera GenICam parameters for GenApi::INodeMap object
	/*!
	This function returns the pointer to the IIpxGenParamTreeView class that was created using information extracted from the GenApi::INodeMap class.
	\param[in] nodemap         The pointer to the GenApi::INodeMap class.
	\param[in]  title          The title of the IIpxGenParamTreeView class as a wchar_t variable.
	\param[in]  parentWindow   A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	\return                    If the function succeeds, the return value is the pointer to the IIpxGenParamTreeView class created. Otherwise, the return value is nullptr.
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API IIpxGenParamTreeView* CreateGenParamTreeViewForNodemapA(IPX_GENAPI_NS::INodeMap *nodemap, const char *title, uintptr_t parentWindow = 0);

	//! Creates the panel of the camera GenICam parameters for GenApi::INodeMap object. Unicode version.
	/*!
	This function returns the pointer to the IIpxGenParamTreeView that was created using information extracted from the GenApi::INodeMap class.
	\param[in] nodemap          The pointer to the GenApi::INodeMap class.
	\param[in]  title           The title of the IIpxGenParamTreeView as a wchar_t variable.
	\param[in]  parentWindow    A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	\return                     If the function succeeds, the return value is the pointer to the IIpxGenParamTreeView class created. Otherwise, the return value is nullptr.
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API IIpxGenParamTreeView* CreateGenParamTreeViewForNodemapW(IPX_GENAPI_NS::INodeMap *nodemap, const wchar_t *title, uintptr_t parentWindow = 0);

	//! Destroys the GenICam parameters panel.
	/*!
	This function closes the camera GenICam parameters panel and destroys the IIpxGenParamTreeView object previously created with
	CreateGenParamTreeViewForNodemap* or CreateGenParamTreeViewForArray* function
	\param[in] view      A pointer to the IIpxGenParamTreeView class.
	\return              void
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API void DestroyGenParamTreeView(IIpxGenParamTreeView *view);

	//! Pops-up the camera selection dialog. 
	/*!
	This function pops-up the "Select camera" modal dialog, where user can select the Camera and obtain the pointer to IpxCam::DeviceInfo object for the selected camera
	\param[in] pSystem          The pointer to the IpxCam::System class.
	\param[in] title            The title of the selected Camera as a const char variable.
	\param[in] parentWindow     A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	\param[in] poll             Specifies if poll check box should be checked by default, so the System will be polled for new devices to appear
	\return                     If the function succeeds, the return value is the pointer to the IpxCam::DeviceInfo class. Otherwise, the return value is nullptr.
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API IpxCam::DeviceInfo* SelectCameraA(IpxCam::System *pSystem, const char *title, uintptr_t parentWindow = 0, bool poll = true);

	//! Pops-up the camera selection dialog. Unicode version.
	/*!
	This function pops-up the "Select camera" modal dialog, where user can select the Camera and obtain the pointer to IpxCam::DeviceInfo object for the selected camera.
	\param[in] pSystem          The pointer to the IpxCam::System class.
	\param[in] title            The title of the IIpxGenParamTreeView as a wchar_t variable.
	\param[in] parentWindow     A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	\param[in] poll             Specifies if poll check box should be checked by default, so the System will be polled for new devices to appear
	\return                     If the function succeeds, the return value is the pointer to the IpxCam::DeviceInfo class. Otherwise, the return value is nullptr.
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API IpxCam::DeviceInfo* SelectCameraW(IpxCam::System *pSystem, const wchar_t *title, uintptr_t parentWindow = 0, bool poll = true);

	#ifndef IPX_DONT_INCLUDE_TO_DOC

	//! Show Camera Configuration Dialog
	/*!
	\param[in] device           The pointer to the IpxCam::Device class.
	\param[in] parentWindow     A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API void ShowCamConfigDialog(IpxCam::Device *device, uintptr_t parentWindow = 0);

	/*!
	\param[in] device          The pointer to the IpxCam::Device class.
	\param[in] parentWindow     A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API void ShowFrameABDialog(IpxCam::Device *device, uintptr_t parentWindow = 0);

	//! Show Trigger Dialog
	/*!
	\param[in] device           The pointer to the IpxCam::Device class.
	\param[in] parentWindow     A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API void ShowTriggerDialog(IpxCam::Device *device, uintptr_t parentWindow = 0);

	//! Show Pulse Dialog
	/*!
	\param[in] device           The pointer to the IpxCam::Device class.
	\param[in] parentWindow     A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API void ShowPulseDialog(IpxCam::Device *device, uintptr_t parentWindow = 0);

	//! Show Strobe Dialog
	/*!
	\param[in] device           The pointer to the IpxCam::Device class.
	\param[in] parentWindow     A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API void ShowStrobeDialog(IpxCam::Device *device, uintptr_t parentWindow = 0);

	//! Show Output Data Dialog
	/*!
	\param[in] device           The pointer to the IpxCam::Device class.
	\param[in] parentWindow     A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API void ShowOutputDialog(IpxCam::Device *device, uintptr_t parentWindow = 0);

	//! Show Color Dialog
	/*!
	\param[in] device           The pointer to the IpxCam::Device class.
	\param[in] parentWindow     A pointer to the parent Window. If the parent is set to 0, this widget is setup to become a window. If not this widget becomes a child widget
	*/
	IPXCAM_GUI_EXTERN_C IPX_CAMERA_GUI_API void ShowColorDialog(IpxCam::Device *device, uintptr_t parentWindow = 0);
#endif /* IPX_DONT_INCLUDE_TO_DOC */
}

    #if defined(QT_VERSION) || defined(Q_MOC_RUN) || defined(QT_DLL) || defined (QT_CORE_LIB)

#ifndef IPX_DONT_INCLUDE_TO_DOC 

#include <QWidget>
#include <QDialog>
#include <QMainWindow>
#include <QScopedPointer>

    QT_FORWARD_DECLARE_CLASS(QTreeView)

#endif /* IPX_DONT_INCLUDE_TO_DOC */

namespace IpxGui {

    class IpxGenParamTreeViewPrivate;

    //! IpxGenParamTreeView class for QT applications, representing the modeless window of the camera GenICam parameters 
    /*! 
    The IpxGenParamTreeView class is a custom QT class for the GenApi node 
    tree derived from QWidget and IpxGui::IIpxGenParamTreeView class.
    This class is preferable to use with Linux and/or QT-based Windows applications
    */
    class IPX_CAMERA_GUI_API IpxGenParamTreeView : public QDialog, public IIpxGenParamTreeView
    {
        Q_OBJECT
    
    public:

        //! IpxGenParamTreeView class constructor
        /*!
        Constructs a custom Widget of GenICam parameters node tree. This IpxGenParamTreeView class inherits QWidget and IpxGui::IIpxGenParamTreeView class. 
        \param[in] parent pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window. If not this widget becomes a child widget
        */
        IpxGenParamTreeView( QWidget *parent = nullptr );

        //! IpxGenParamTreeView class constructor
        /*!
        Constructs a custom Widget of GenICam parameters node tree. This IpxGenParamTreeView class inherits QWidget and IpxGui::IIpxGenParamTreeView class.
        \param[in] genParam The pointer to IpxGenParam::Array class. This class contains functions that can access each node from the camera descriptor XML file by type and name.
        \param[in] parent   pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window. If not this widget becomes a child widget
        */
        IpxGenParamTreeView( IpxGenParam::Array *genParam, QWidget *parent = nullptr );

        //! IpxGenParamTreeView class constructor
        /*!
        Constructs a custom Widget of GenICam parameters node tree. This IpxGenParamTreeView class inherits QWidget and IpxGui::IIpxGenParamTreeView class.
        \param[in] nodemap The pointer to GenApi::INodeMap class.
        \param[in] parent pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window. If not this widget becomes a child widget
        */
        IpxGenParamTreeView( IPX_GENAPI_NS::INodeMap *nodemap, QWidget *parent = nullptr );

        //! IpxGenParamTreeView class destructor
        /*!
        Destroys the IpxGenParamTreeView and all children widgets.
        */
        ~IpxGenParamTreeView();

        //! This method retrieves the pointer to QTreeView object of the node tree.
        /*!
        \return Returns the pointer to QTreeView object
        */
        QTreeView* treeView() const;

        //! This method sets the parameters of the node tree by the information extracted from the IpxGenParam::Array class
        /*!
        \param[in] genParam The pointer to the IpxGenParam::Array class.
        \return    Void.
        \brief Set the Camera Parameters to the corresponding fields of the tree view.
        */
        void setParams( IpxGenParam::Array *genParam ) Q_DECL_OVERRIDE;

        //! This method sets the parameters of the node tree with parameters retrieved from the GenApi::INodeMap class
        /*!
        \brief The INodeMap consists of a list of nodes representing the GenICam compliant camera high-level features.
        \param[in]  nodemap The pointer to the GenApi::INodeMap class.
        \return     Void.
        \brief Set the parameters of the node map to be represented as the tree view.
        */
        void setParams( IPX_GENAPI_NS::INodeMap *nodemap ) Q_DECL_OVERRIDE;

        //! This method clears the parameters of the node tree that have been set.
        /*!
        \return Void.
        \brief Clear all the parameters.
        */
        void clearParams() Q_DECL_OVERRIDE;

        //! This method returns the current visibility mode.
        /*!
        \brief This method retrieves the current setting of the user level access feature.
        \return  Returns the current setting of the user level access feature.
        */
        Visibility visibility() const Q_DECL_OVERRIDE;

        //! This method sets visibility mode.
        /*!
        \brief It sets the current visibility mode.
        \param[in] visibility The visibility parameter used to set the visibility mode.
        \return Void.
        */
        void setVisibility( Visibility visibility ) Q_DECL_OVERRIDE;

        //! This method saves the current state of the Tree View
        /*!
        \brief saves the current state of the Tree View.
        \return If the method succeeds, the method returns pointer to the array of data to be saved. Otherwise, the return value is nullptr.
        \brief The array consists of string values separated by token. Just save this data somewhere if you want to restore state later.
        \code{.cpp}
        // Save the settings
        QSettings settings(QStringLiteral("CompanyName"), QStringLiteral("AppName"));
        const char *dataToBeSaved = m_treeView->saveState();
        if (dataToBeSaved)
            settings.setValue(QStringLiteral("ValueName"), QByteArray(dataToBeSaved));
        \endcode
        */
        const char* saveState() const Q_DECL_OVERRIDE;

        //! This method loads the state of the Tree View.
        /*!
        \brief loads the state of the Tree View. The individual node can be in a state of expanded or collapse state.
        \param[in] state to be loaded
        \brief The array consists of string values separated by token. Load previously saved state of the Tree View.
        \code{.cpp}
        // Restore the settings
        QSettings settings(QStringLiteral("CompanyName"), QStringLiteral("AppName"));
        QByteArray dataToBeRestored = settings->value(QStringLiteral("ValueName")).toByteArray();
        if (!dataToBeRestored.isEmpty())
            m_treeView->loadState(dataToBeRestored.constData());
        \endcode
        */
        void loadState( const char *state ) Q_DECL_OVERRIDE;
    
        //! This method sets polling time
        /*!
        \brief sets the polling time. Polling should be enabled by enablePolling() function
        \param[in] pollingTime time in msec to be set
        */
        void setPollingTime(uint64_t pollingTime) Q_DECL_OVERRIDE;

        //! This method retrieves current polling time
        /*!
        \brief retrieves the polling time. Polling should be enabled by enablePolling() function
        \return current polling time in msec
        */
        uint64_t getPollingTime() Q_DECL_OVERRIDE;

        //! This method enables polling
        /*
        \brief enables polling with polling time set by setPollingTime() function
        \param[in] enable Enable or disable polling
        */
        void enablePolling(bool enable) Q_DECL_OVERRIDE;

        //! This method retrieves current polling state
        /*
        \brief retrieves current polling state set by enablePolling() function
        \return current polling state
        */
        bool isPollingEnabled() Q_DECL_OVERRIDE;
    private:
        QScopedPointer<IpxGenParamTreeViewPrivate> d_ptr;
        Q_DECLARE_PRIVATE(IpxGenParamTreeView)
    };

    class IpxCameraSelectorDialogPrivate;

    //! IpxCameraSelectorDialog class for QT applications, representing the modal dialog of the camera selection 
    /**  
    The IpxCameraSelectorDialog class is a custom class, derived from QDialog. 
    This class inherits the QDialog features and contains member functions to select or extract the camera device information.
	This class is preferable to use with Linux and/or QT-based Windows applications
    */
    class IPX_CAMERA_GUI_API IpxCameraSelectorDialog : public QDialog
    {
        Q_OBJECT
    
    public:

        //! A constructor for the IpxCameraSelectorDialog class
        /*!
         \brief Constructs a custom QDialog. This IpxCameraSelectorDialog class inherits QDialog features, initializes the Camera Tree View with the information pointed to by the IpxCam::System parameters. It also
        passes a pointer to the QWidget parent window. 
        \param[in] pSystem pointer to IpxCam::System class. This system represents the GenICam system.
        \param[in] parent  pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window. If not this widget becomes a child widget.
        \param[in] poll    specifies if "Poll" check box should be checked by default, so the System will be polled for new devices to appear.
        */
        IpxCameraSelectorDialog( IpxCam::System *pSystem, QWidget *parent = nullptr, bool poll = true );

        //! A destructor of the IpxCameraSelectorDialog class
        /*!
        \brief Destructor. Destroys the IpxCameraSelectorDialog and all children widgets.
        */
        ~IpxCameraSelectorDialog();

        //! This method retrieves a pointer to the IpxCam::DeviceInfo class of the selected camera device.
        /*!
        \brief It retrieves the IpxCam::DeviceInfo class of the selected camera. This IpxCam::DeviceInfo class is composed of functions used to retrieve the selected camera information such as vendor and model data. 
        \return If the method succeeds, the method returns the IpxCam::DeviceInfo class. Otherwise, the return value is nullptr.
        */
        IpxCam::DeviceInfo* selectedCamera() const;

        //! This method sets the device information of the selected camera.
        /*!
        \brief It sets the device information of the selected camera.
        \param[in] pDeviceInfo pointer to IpxCam::DeviceInfo class.
        \return Void.
        */
        void setSelectedCamera( IpxCam::DeviceInfo *pDeviceInfo );

        //! This method returns the state of "Poll" check box, before the "Select camera" dialog was closed.
        /*!
        \brief Returns the "Poll" check box state.
        \return true, if "Poll" check box was enabled, false - otherwise
        */
        bool isPollChecked();
    
    private:
        QScopedPointer<IpxCameraSelectorDialogPrivate> d_ptr;
        Q_DECLARE_PRIVATE(IpxCameraSelectorDialog)

    };

#ifndef IPX_DONT_INCLUDE_TO_DOC

    class IpxCamConfigPrivate;

    /**  
    *  A Class for IpxCamConfig. 
    \brief The IpxCamConfig class is a main window containing general information.
    This class inherits the QMainWindow features and contains various dialogs to configure the camera device.
    */
    class IPX_CAMERA_GUI_API IpxCamConfig : public QMainWindow
    {
        Q_OBJECT

    public:
    
        //! A constructor for the IpxCamConfig class
        /*!
          \brief Constructs a custom QMainWindow. This IpxCamConfig class initializes the Camera Config Window
          with the information pointed to by the IpxCam::Device parameters. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
          \param[in] parent pointer to parent window.
          \param[in] flags the widget flags argument, is normally 0, but it can be set to customize the frame of a window
          (i.e. parent must be nullptr). To customize the frame, use a value composed from the bitwise OR of any of the window flags.
        */
        IpxCamConfig( IpxCam::Device *pDevice = nullptr, QWidget *parent = nullptr, Qt::WindowFlags flags = 0 );
    
        //! A destructor of the IpxCamConfig class
        /*!
         \brief Destructor. Destroys the IpxCamConfig and all children widgets.
        */
        ~IpxCamConfig();

        //! Set(reset) device
        /*!
          \brief Initializes the Camera Config Window with the information pointed to by the IpxCam::Device parameters. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
        */
        void SetDevice( IpxCam::Device *pDevice );

    protected:
        void showEvent( QShowEvent *event ) Q_DECL_OVERRIDE;

    protected:
        QScopedPointer<IpxCamConfigPrivate> d_ptr;

    private:
        Q_DECLARE_PRIVATE(IpxCamConfig)
    };

    class IpxCamConfigColorDialogPrivate;

    /**  
    *  A Class for IpxCamConfigColorDialog. 
    \brief The IpxCamConfigColorDialog class is a custom QDialog class.
    This class inherits the QDialog features and contains member functions to configure the camera device.
    It allows to control Color related parameters of a camera.
    */
    class IPX_CAMERA_GUI_API IpxCamConfigColorDialog : public QDialog
    {
        Q_OBJECT

    public:
    
        //! A constructor for the IpxCamConfigColorDialog class
        /*!
          \brief Constructs a custom QDialog. This IpxCamConfigColorDialog class initializes the Camera Config Color Dialog
          with the information pointed to by the IpxCam::Device parameters. It also passes a pointer to the QWidget parent window. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
          \param[in] parent pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window.
          If not this widget becomes a child widget.
        */
        IpxCamConfigColorDialog( IpxCam::Device *pDevice, QWidget *parent = nullptr );
    
        //! A destructor of the IpxCamConfigColorDialog class
        /*!
         \brief Destructor. Destroys the IpxCamConfigColorDialog and all children widgets.
        */
        ~IpxCamConfigColorDialog();

        //! Set(reset) device
        /*!
          \brief Initializes the Camera Config Color Dialog with the information pointed to by the IpxCam::Device parameters. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
        */
        void SetDevice( IpxCam::Device *pDevice );

    Q_SIGNALS:
        void closing();

    protected:
        void showEvent( QShowEvent *event ) Q_DECL_OVERRIDE;
        void closeEvent( QCloseEvent *event ) Q_DECL_OVERRIDE;

    protected:
        QScopedPointer<IpxCamConfigColorDialogPrivate> d_ptr;

    private:
        Q_DECLARE_PRIVATE(IpxCamConfigColorDialog)
    };

    class IpxCamConfigOutputDialogPrivate;

    /**  
    *  A Class for IpxCamConfigOutputDialog. 
    \brief The IpxCamConfigOutputDialog class is a custom QDialog class.
    This class inherits the QDialog features and contains member functions to configure the camera device.
    It allows to control Output Data related parameters of a camera.
    */
    class IPX_CAMERA_GUI_API IpxCamConfigOutputDialog : public QDialog
    {
        Q_OBJECT

    public:
    
        //! A constructor for the IpxCamConfigOutputDialog class
        /*!
          \brief Constructs a custom QDialog. This IpxCamConfigOutputDialog class initializes the Camera Config Output Dialog
          with the information pointed to by the IpxCam::Device parameters. It also passes a pointer to the QWidget parent window. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
          \param[in] parent pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window.
          If not this widget becomes a child widget.
        */
        IpxCamConfigOutputDialog( IpxCam::Device *pDevice, QWidget *parent = nullptr );
    
        //! A destructor of the IpxCamConfigOutputDialog class
        /*!
         \brief Destructor. Destroys the IpxCamConfigOutputDialog and all children widgets.
        */
        ~IpxCamConfigOutputDialog();

        //! Set(reset) device
        /*!
          \brief Initializes the Camera Config Output Dialog with the information pointed to by the IpxCam::Device parameters. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
        */
        void SetDevice( IpxCam::Device *pDevice );

    Q_SIGNALS:
        void closing();

    protected:
        void showEvent( QShowEvent *event ) Q_DECL_OVERRIDE;
        void closeEvent( QCloseEvent *event ) Q_DECL_OVERRIDE;

    protected:
        QScopedPointer<IpxCamConfigOutputDialogPrivate> d_ptr;

    private:
        Q_DECLARE_PRIVATE(IpxCamConfigOutputDialog)
    };

    class IpxCamConfigStrobeDialogPrivate;

    /**  
    *  A Class for IpxCamConfigStrobeDialog. 
    \brief The IpxCamConfigStrobeDialog class is a custom QDialog class.
    This class inherits the QDialog features and contains member functions to configure the camera device.
    It allows to control Strobe related parameters of a camera.
    */
    class IPX_CAMERA_GUI_API IpxCamConfigStrobeDialog : public QDialog
    {
        Q_OBJECT

    public:
    
        //! A constructor for the IpxCamConfigStrobeDialog class
        /*!
          \brief Constructs a custom QDialog. This IpxCamConfigStrobeDialog class initializes the Camera Config Strobe Dialog
          with the information pointed to by the IpxCam::Device parameters. It also passes a pointer to the QWidget parent window. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
          \param[in] parent pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window.
          If not this widget becomes a child widget.
        */
        IpxCamConfigStrobeDialog( IpxCam::Device *pDevice, QWidget *parent = nullptr );
    
        //! A destructor of the IpxCamConfigStrobeDialog class
        /*!
         \brief Destructor. Destroys the IpxCamConfigStrobeDialog and all children widgets.
        */
        ~IpxCamConfigStrobeDialog();

        //! Set(reset) device
        /*!
          \brief Initializes the Camera Config Strobe Dialog with the information pointed to by the IpxCam::Device parameters. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
        */
        void SetDevice( IpxCam::Device *pDevice );

    Q_SIGNALS:
        void closing();

    protected:
        void closeEvent( QCloseEvent *event ) Q_DECL_OVERRIDE;

    protected:
        QScopedPointer<IpxCamConfigStrobeDialogPrivate> d_ptr;

    private:
        Q_DECLARE_PRIVATE(IpxCamConfigStrobeDialog)
    };

    class IpxCamConfigPulseDialogPrivate;

    /**  
    *  A Class for IpxCamConfigPulseDialog. 
    \brief The IpxCamConfigPulseDialog class is a custom QDialog class.
    This class inherits the QDialog features and contains member functions to configure the camera device.
    It allows to control Pulse related parameters of a camera.
    */
    class IPX_CAMERA_GUI_API IpxCamConfigPulseDialog : public QDialog
    {
        Q_OBJECT

    public:
    
        //! A constructor for the IpxCamConfigPulseDialog class
        /*!
          \brief Constructs a custom QDialog. This IpxCamConfigPulseDialog class initializes the Camera Config Pulse Dialog
          with the information pointed to by the IpxCam::Device parameters. It also passes a pointer to the QWidget parent window. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
          \param[in] parent pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window.
          If not this widget becomes a child widget.
        */
        IpxCamConfigPulseDialog( IpxCam::Device *pDevice, QWidget *parent = nullptr );
    
        //! A destructor of the IpxCamConfigPulseDialog class
        /*!
         \brief Destructor. Destroys the IpxCamConfigPulseDialog and all children widgets.
        */
        ~IpxCamConfigPulseDialog();

        //! Set(reset) device
        /*!
          \brief Initializes the Camera Config Pulse Dialog with the information pointed to by the IpxCam::Device parameters. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
        */
        void SetDevice( IpxCam::Device *pDevice );

    Q_SIGNALS:
        void closing();

    protected:
        void showEvent( QShowEvent *event ) Q_DECL_OVERRIDE;
        void closeEvent( QCloseEvent *event ) Q_DECL_OVERRIDE;

    protected:
        QScopedPointer<IpxCamConfigPulseDialogPrivate> d_ptr;

    private:
        Q_DECLARE_PRIVATE(IpxCamConfigPulseDialog)
    };

    class IpxCamConfigTriggerDialogPrivate;

    /**  
    *  A Class for IpxCamConfigTriggerDialog. 
    \brief The IpxCamConfigTriggerDialog class is a custom QDialog class.
    This class inherits the QDialog features and contains member functions to configure the camera device.
    It allows to control Trigger related parameters of a camera.
    */
    class IPX_CAMERA_GUI_API IpxCamConfigTriggerDialog : public QDialog
    {
        Q_OBJECT

    public:
    
        //! A constructor for the IpxCamConfigTriggerDialog class
        /*!
          \brief Constructs a custom QDialog. This IpxCamConfigTriggerDialog class initializes the Camera Config Trigger Dialog
          with the information pointed to by the IpxCam::Device parameters. It also passes a pointer to the QWidget parent window. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
          \param[in] parent pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window.
          If not this widget becomes a child widget.
        */
        IpxCamConfigTriggerDialog( IpxCam::Device *pDevice, QWidget *parent = nullptr );
    
        //! A destructor of the IpxCamConfigTriggerDialog class
        /*!
         \brief Destructor. Destroys the IpxCamConfigTriggerDialog and all children widgets.
        */
        ~IpxCamConfigTriggerDialog();

        //! Set(reset) device
        /*!
          \brief Initializes the Camera Config Trigger Dialog with the information pointed to by the IpxCam::Device parameters. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
        */
        void SetDevice( IpxCam::Device *pDevice );

    Q_SIGNALS:
        void closing();

    protected:
        void closeEvent( QCloseEvent *event ) Q_DECL_OVERRIDE;

    protected:
        QScopedPointer<IpxCamConfigTriggerDialogPrivate> d_ptr;

    private:
        Q_DECLARE_PRIVATE(IpxCamConfigTriggerDialog)
    };

    class IpxCamConfigFrameABDialogPrivate;

    /**  
    *  A Class for IpxCamConfigFrameABDialog. 
    \brief The IpxCamConfigFrameABDialog class is a custom QDialog class.
    This class inherits the QDialog features and contains member functions to configure the camera device.
    It allows to control FrameA-FrameB related parameters of a camera.
    */
    class IPX_CAMERA_GUI_API IpxCamConfigFrameABDialog : public QDialog
    {
        Q_OBJECT

    public:
    
        //! A constructor for the IpxCamConfigFrameABDialog class
        /*!
          \brief Constructs a custom QDialog. This IpxCamConfigFrameABDialog class initializes the Camera Config FrameAB Dialog
          with the information pointed to by the IpxCam::Device parameters. It also passes a pointer to the QWidget parent window. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
          \param[in] parent pointer to parent window. If the parent is set to nullptr, this widget is setup to become a window.
          If not this widget becomes a child widget.
        */
        IpxCamConfigFrameABDialog( IpxCam::Device *pDevice, QWidget *parent = nullptr );
    
        //! A destructor of the IpxCamConfigFrameABDialog class
        /*!
         \brief Destructor. Destroys the IpxCamConfigFrameABDialog and all children widgets.
        */
        ~IpxCamConfigFrameABDialog();

        //! Set(reset) device
        /*!
          \brief Initializes the Camera Config Frame A/B Dialog with the information pointed to by the IpxCam::Device parameters. 
          \param[in] pDevice pointer to IpxCam::Device class. This system represents the GenICam device.
        */
        void SetDevice( IpxCam::Device *pDevice );

    Q_SIGNALS:
        void closing();

    protected:
        void showEvent( QShowEvent *event ) Q_DECL_OVERRIDE;
        void closeEvent( QCloseEvent *event ) Q_DECL_OVERRIDE;

    protected:
        QScopedPointer<IpxCamConfigFrameABDialogPrivate> d_ptr;

    private:
        Q_DECLARE_PRIVATE(IpxCamConfigFrameABDialog)
    }; 

#endif // IPX_DONT_INCLUDE_TO_DOC
}

#endif // defined(QT_VERSION) || defined(Q_MOC_RUN) || defined(QT_DLL)

#endif // IPX_CAMERA_GUI_API_H
