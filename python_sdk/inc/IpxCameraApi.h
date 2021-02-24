////////////////////////////////////////////////////////////////////////////////
// Imperx Camera SDK
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File: IpxCameraApi.h
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Imperx Camera API Function and Constant Declarations
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Created:	14-OCT-2014
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2013-2016 Imperx Inc. All rights reserved. 
////////////////////////////////////////////////////////////////////////////////

#ifndef IPX_CAMERA_API_H
#define IPX_CAMERA_API_H

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

#ifndef IPXCAM_EXTERN_C
    #ifdef __cplusplus
        #define IPXCAM_EXTERN_C extern "C"
    #else
        #define IPXCAM_EXTERN_C
    #endif
#endif // IPXCAM_EXTERN_C

#ifndef IPXCAM_CALL
    #ifdef _WIN32
        #define IPXCAM_CALL __stdcall
    #else
        #define IPXCAM_CALL
    #endif
#endif // IPXCAM_CALL

// check if static library is wanted
#ifdef IPXCAM_STATIC
    // define the static library symbol
    #define IPX_CAMERA_API
    #define IPX_CAMERA_LOCAL
#else
    #define IPX_CAMERA_LOCAL LIB_HELPER_DLL_LOCAL
    #ifdef IPX_CAMERA_API_EXPORTS
        // define the export symbol
        #define IPX_CAMERA_API LIB_HELPER_DLL_EXPORT

        #if defined _WIN32 && !defined _M_AMD64
            // For 32b DLL only. If method name is mangled, add unmangled name below.
            //#pragma comment(linker, "/EXPORT:IpxCam_GetSystem=_IpxCam_GetSystem()@0")
        #endif
    #else // IPX_CAMERA_API_EXPORTS
        // define the import symbol
        #define IPX_CAMERA_API LIB_HELPER_DLL_IMPORT
    #endif // IPX_CAMERA_API_EXPORTS
#endif // IPXCAM_STATIC

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

#include <stddef.h>
#include <stdint.h>

#include "IpxCameraErr.h"

#ifdef __cplusplus

// define correct name of GenApi namespace
#ifndef IPX_GENAPI_NS
    #ifdef GENAPI_NAMESPACE
        #define IPX_GENAPI_NS GENAPI_NAMESPACE
    #else
        #define IPX_GENAPI_NS GenApi_3_0
    #endif // GENAPI_NAMESPACE
#endif // IPX_GENAPI_NS

// forward declaration must have real name not an alias
namespace IPX_GENAPI_NS { struct INodeMap; struct INode; }

typedef struct _IpxImage IpxImage;

/*! \namespace IpxGenParam
    \brief A namespace provides the scope to the API to access the GenICam parameters.

     \details The IpxGenParam namespace provides the scope to the API to control the GenICam camera parameters of types: Boolean, Enumeration, String, Float, Integer, Commands and Categories.
	 Such parameters may include image Width, Height, Pixel Format, Gain, Exposure, Trigger, I/O settings, etc.
	 Parameters are described in camera GenICam XML file, and documented in appropriate camera user's manual.
*/
namespace IpxGenParam
{
	//! An enumeration of Parameter Types. Parameter Node Types that can access the node object's programming interface.
	enum ParamType : uint32_t
	{
        ParamUnknown,   //!< Enum value ParamUnknown. Unknown Parameter.  
        ParamInt, 	    //!< Enum value ParamInt will access node object's of IInteger interface. 
        ParamFloat,     //!< Enum value ParamFloat will access node object's of IFloat interface. 
        ParamString,    //!< Enum value ParamString will access node object's of IString interface.  
        ParamEnum,      //!< Enum value ParamEnum will access node object's of IEnumeration interface.  
        ParamEnumEntry, //!< Enum value ParamEnumEntry will access the entry of Enum parameter  
        ParamBoolean,   //!< Enum value ParamBoolean will access node object's of IBoolean interface.  
        ParamCommand,   //!< Enum value ParamCommand will access node object's of ICommand interface.  
        ParamCategory   //!< Enum value ParamCategory will access node object's of ICategory interface.  
	};

	//! An enumeration of GenICam NameSpace. Parameter Node Namespace. 
	enum NameSpace : uint32_t
	{
        NameSpaceStandard = 0, /*!< Enum value NameSpaceStandard. Identifies the standard namespace used in the file.  */
        NameSpaceCustom,       /*!< Enum value NameSpaceCustom. Identifies the custom namespace used in the file.  */
        NameSpaceUndefined=999 /*!< Enum value NameSpaceUndefined. Unknown namespace.  */
	};
	
    //! An enumeration of Visibility. This element defines the level of user that has access to the feature.
	enum Visibility	: uint32_t
	{
        VisBeginner = 0,   /*!< Enum value VisBeginner. User has visibility to all the basic features of the device.*/
        VisExpert,         /*!< Enum value VisExpert. User has visibility to more advance features of the device.*/
        VisGuru,           /*!< Enum value VisGuru. User has visibility to even more advance features that if set improperly can cause device to be in an improper state.*/
        VisInvisible,      /*!< Enum value VisInvisible. Not visible.  */  
        VisUndefined = 99, /*!< Enum value VisUndefined. Unknown visibility.  */
	};

	class Param;

	/**  
	\brief  A Class for ParamEventSink notifications handling. 
	\details An Event Sink class designed to receive the notifications from the GenICam parameter Node Updates
	*/
	class ParamEventSink
	{
	public:

        //! ParamEventSink class destructor. Destroys the ParamEventSink object and all its descendants.
        virtual ~ParamEventSink(){}

        //! Update Parameter Node 
        /*!
        \param[in] param The pointer to the Param class node 
        \return Void.
        */
        virtual void OnParameterUpdate( Param *param ) = 0;
	};

	class Category;
	class Boolean;
	class Command;
	class EnumEntry;
	class Enum;
	class Float;
	class Int;
	class String;

	/**  
	*  Class for accessing the GenICam feature node of the Camera parameters
	\brief General class for GenICam parameter 
	*/
	class Param
	{
	public:

        //! Param class destructor. Destroys the Param and all its descendants.
        /*!
        \brief Param class destructor. 
        */
        virtual ~Param(){}

        //! This method returns the Parameter Node Type
        /*!
        * \return return the parameter type.
        */
        virtual ParamType GetType() = 0;

        //! This method returns the parameter node name.
        /*!
        \return If the method succeeds, it will return the parameter node name. Otherwise, it will return a nullptr.
        */
        virtual const char* GetName() = 0;

        //! This method returns a short description of the parameter node.
        /*!
        \return If the method succeeds, it will return a short description of the parameter node. Otherwise, it will return a nullptr.
        */
        virtual const char* GetToolTip() = 0;

        //! This method returns a long description of the parameter node.
        /*!
        \return If the method succeeds, it will return a long description of the parameter node. Otherwise, it will return a nullptr.
        */
        virtual const char* GetDescription() = 0;

        //! This method returns the string to be used for the parameter displaying.
        /*!
        \return If the method succeeds, it will return the parameter display name. Otherwise, it will return a nullptr.
        */
        virtual const char* GetDisplayName() = 0;

        //! This method returns the visibility of the node.
        /*!
        \return It will return the visibility setting of the parameter node. It will be either Basic, Expert, or Guru. 
        */
        virtual Visibility GetVisibility()  = 0;

        //! This method checks if the parameter node is cached.
        /*!
        \return True if the value is cached. False if the value is not cached.
        */
        virtual bool IsValueCached() = 0;

        //! This method checks if parameter node is available.
        /*!
        \return True if the parameter node is available. False if it is not available.
        */
        virtual bool IsAvailable() = 0;

        //! This method checks if parameter node is writable.
        /*!
        \return True if the parameter node is writable. False if it is not writable.
        */
        virtual bool IsWritable() = 0;

        //! This method checks if the parameter node is readable.
        /*!
        \return True if the parameter node is readable. False if it is not readable.
        */
        virtual bool IsReadable() = 0;

        //! This method checks if the parameter node is streamable.
        /*!
        \return True if the parameter node is streamable. False if it is not streamable.
        */
        virtual bool IsStreamable() = 0;

        //! This method checks if the node is visible.
        /*!
         \param[in] vis Visibility of the parameter node
        * \return True if the parameter node is visible. False if it is not visible.
        */
        virtual bool IsVisible( Visibility vis ) = 0;

        //! This method registers the event.
        /*!
        * \param[in] aEventSink pointer to Parameter Event Sink
        * \return Returns the Error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully registers event sink
        */
        virtual IpxCamErr RegisterEventSink( ParamEventSink *aEventSink ) = 0;

        //! This method unregisters the event.
        /*!
        \param[in] aEventSink pointer to Parameter Event Sink
        \return Returns the Error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully unregisters event sink
        */
        virtual IpxCamErr UnregisterEventSink( ParamEventSink *aEventSink ) = 0;

        //! This method returns the callback of the node registered.
        /*!
        \return If the method succeeds, it will return the pointer to the node of the callback that is registered. Otherwise, it will return a value of nullptr.
        */        
        virtual IPX_GENAPI_NS::INode* GetNode() = 0;

        //! This method returns typed representation of param
        /*!
        \return If the method succeeds, it will return pointer to typed param. Otherwise, it will return a value of nullptr
        */
        virtual Category	*ToCategory() = 0;

        //! This method returns typed representation of param
        /*!
        \return If the method succeeds, it will return pointer to typed param. Otherwise, it will return a value of nullptr
        */
        virtual Boolean        *ToBoolean() = 0;

        //! This method returns typed representation of param
        /*!
        \return If the method succeeds, it will return pointer to typed param. Otherwise, it will return a value of nullptr
        */
        virtual Command        *ToCommand() = 0;

        //! This method returns typed representation of param
        /*!
        \return If the method succeeds, it will return pointer to typed param. Otherwise, it will return a value of nullptr
        */
        virtual EnumEntry	*ToEnumEntry() = 0;

        //! This method returns typed representation of param
        /*!
        \return If the method succeeds, it will return pointer to typed param. Otherwise, it will return a value of nullptr
        */
        virtual Enum        *ToEnum() = 0;

        //! This method returns typed representation of param
        /*!
        \return If the method succeeds, it will return pointer to typed param. Otherwise, it will return a value of nullptr
        */
        virtual Float        *ToFloat() = 0;

        //! This method returns typed representation of param
        /*!
        \return If the method succeeds, it will return pointer to typed param. Otherwise, it will return a value of nullptr
        */
        virtual Int        	*ToInt() = 0;

        //! This method returns typed representation of param
        /*!
        \return If the method succeeds, it will return pointer to typed param. Otherwise, it will return a value of nullptr
        */
        virtual String        *ToString() = 0;
	};
	
	/**  
	\brief A class containing methods for GenICam Category.
	\details A class containing methods that the user can access the categories of GenICam features.  
	It will access the node object's of an ICategory interface. 
	Each feature of a device will be placed in a <b>Category</b>.  
	The Category feature is used to present the user with a group of features for the named category.  
        
	For example, the mapping below will illustrate the ICategory interfaces categories such as DeviceControl and EventControl. 
	
	\image html Category.png
	*/
	class Category : virtual public Param
	{
	public:

        //! This method returns the node object Category type.
        /*!
        \return Returns the node object Category type 
        */
        virtual ParamType GetType() { return ParamCategory; }

        //! This method returns the number of parameters in the category
        /*!
        \return Returns the number of parameters in the category
        */
        virtual uint32_t GetCount() = 0;
        
        //! This method returns the Parameter by Index.
        /*!
        \param[in] idx index
        \param[out] err returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to the parameter for specified index
        - \c IpxCamErr::IPX_CAM_ERR_INVALID_INDEX - an invalid index for node
        \return Returns the pointer to the parameter object
        */
        virtual Param* GetParamByIndex( uint32_t idx, IpxCamErr *err ) = 0;
	};

	/**  
	\brief  A class containing methods for Boolean GenICam camera parameter. 
	\details A class containing methods that map the integer element value of a GenICam IBoolean interface feature to true or false.
	
	For example, the mapping below will illustrate the IBoolean interfaces of a <b>LUTEnable</b> feature. 
	
	\image html Boolean.png
	*/
	class Boolean : virtual public Param
	{
    public:

        //! This method returns the node object Boolean type.
        /*!
        \return Returns the node object Boolean type  
        */
        virtual ParamType GetType() { return ParamBoolean; }

        //! This method can be used to set the node value to true or false.
        /*!
        \param[in] val The node value to set such as true or false
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully sets the Boolean value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        - \c IpxCamErr::IPX_CAM_GENICAM_OUT_OF_RANGE - the value entered is out of range 
        */
        virtual IpxCamErr SetValue( bool val ) = 0;

        //! This method returns the node value. It can return a true or false value.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the value of the Boolean node
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type 
        \return The node value read.
	    */
        virtual bool GetValue( IpxCamErr *err = nullptr ) = 0;
	};

	/**  
	\brief A class containing methods for Command GenICam camera parameter.   
	\details A class for GenICam Command contains methods that allow the user submit a command for execution as well 
	as poll the command status.
	
	For example, the mapping below will illustrate the ICommand interface for AcquisitionStart. This feature starts the
	Acquisition of the device.
	
	\image html Command.png
	*/
	class Command : virtual public Param
	{
	public:

        //! This method returns the node object Command type.
        /*!
        \return Returns the node object Command type 
        */
        virtual ParamType GetType(){ return ParamCommand; }

        //! This method executes the command.
        /*!
        \return the error code 
        */
        virtual IpxCamErr Execute() = 0;

        //! This method queries whether the command is executed and completed.
        /*!
         \param[out] err returns error code:
         - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully determined that state of execute command
         - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
         - \c IpxCamErr::IPX_CAM_GENICAM_TREE_ERROR - Unable to access tree
         \return If set to TRUE, the Execute command has finished. Otherwise, it returns FALSE.
        */
        virtual bool IsDone( IpxCamErr *err = nullptr ) = 0;
	};

	/**  
	\brief EnumEntry class represents the entry of GenICam Enum parameter
	\details A Class for GenICam Enum Entries has methods to access the Enumeration GenICam parameter entry.
	
	For example, the mapping below illustrates entries of the IEnumeration interface for the AOI2_Select feature. 
	This feature can select the mode of operation for Slave AOI #2.  The enumeration entries could be "Off", "Include", and
	"Exclude".
	
	\image html EnumEntry.png
	*/
	class EnumEntry : virtual public Param
	{
	public:

        //! This method returns the node object EnumEntry type.
        /*!
        \return If the method succeeds, it returns the ParamType object type of the EnumEntry.
        */
        virtual ParamType GetType() { return ParamEnumEntry; }

        //! This method gets the EnumEntry numerical value.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully indicates EnumEntry value was retrieved
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return If the method succeeds, it returns the value read of the EnumEntry.
        */
        virtual int64_t GetValue( IpxCamErr *err = nullptr ) = 0;

        //! This method gets the EnumEntry String value.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully indicates EnumEntry string value was retrieved
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return If the method succeeds, it returns the String value read of the EnumEntry.
        */
        virtual const char* GetValueStr( IpxCamErr *err = nullptr ) = 0;
	};

	/**  
	\brief A class containing methods for Enumeration GenICam camera parameter
	\details A class containing methods to access the Enumeration GenICam camera parameter, using Integer or String value.
	
	For example, the picture below illustrates the enumeration "WhiteBalanceMode". 
	
	\image html Enum.png
	*/
	class Enum : virtual public Param
	{
	public:

        //! This method returns the node object Enum type.
        /*!
        \return If the method succeeds, it will returns the Enum parameter type.
        */
        virtual ParamType GetType(){ return ParamEnum; }

        //! This method gets the number of entry nodes.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the number of EnumEntries
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type 
        \return Returns the number of enum entry nodes. 
        */
        virtual size_t GetEnumEntriesCount( IpxCamErr *err = nullptr ) = 0;

        //! This method gets the Enum Entry node by the Index number.
        /*!
         \param[in] aIndex Index number 
        * \return If the method succeeds, it returns the Enum Entry node.
        */
        virtual EnumEntry* GetEnumEntryByIndex( size_t aIndex ) = 0;

        //! This method gets the Enum Entry node by Name.
        /*!
         \param[in] name Entry Name 
        * \return If the method succeeds, it returns the Enum Entry node.
        */
        virtual EnumEntry* GetEnumEntryByName( const char *name ) = 0;

        //! This method gets the Enum Entry node by Value.
        /*!
         \param[in] val Entry Value 
        * \return If the method succeeds, it returns the Enum Entry node.
        */
        virtual EnumEntry* GetEnumEntryByValue( int64_t val ) = 0;

        //! This method gets the Enum Entry node value as Integer.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Enum Entry node value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return If the method succeeds, it returns the Enum Entry node value.
        */
        virtual int64_t GetValue( IpxCamErr *err = nullptr ) = 0;

        //! This method gets the Enum Entry node value as String.
        /*!
         \param[out] err returns error code:
        * - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully get the Enum Entry node string 
        * - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        * - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type 
        * \return If the method succeeds, it returns the Enum Entry node string.
        */
        virtual const char* GetValueStr( IpxCamErr *err = nullptr ) = 0;

        //! This method sets the Enum Entry node value as Integer.
        /*!
        \param[in] val Enum Entry node value 
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully sets the Enum value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type 
        */
        virtual IpxCamErr SetValue( int64_t val ) = 0;

        //! This method sets the Enum Entry node as String.
        /*!
        \param[in] val Enum Entry node String 
        \return Returns the error code 
        */
        virtual IpxCamErr SetValueStr( const char *val ) = 0;
	};

	/**  
	\brief A class containing methods for Float GenICam camera parameter
	\details A class containing methods to access the Float GenICam camera parameter as floating point value.

	For example, the picture below illustrates the float "ExposureTime".

	\image html float.png

	*/
	class Float : virtual public Param
	{
	public:

        //! This method returns the node object Float type.
        /*!
        \return Returns the parameter type 
        */
        virtual ParamType GetType(){ return ParamFloat; }

        //! This method sets the node value.
        /*!
        \param[in]  val The value to set
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully sets the Float value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        - \c IpxCamErr::IPX_CAM_GENICAM_OUT_OF_RANGE - the value entered is out of range 
        */
        virtual IpxCamErr SetValue( double val ) = 0;

        //! This method gets the Float node value.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully get the Float value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return Gets the Float node value 
        */
        virtual double GetValue( IpxCamErr *err = nullptr ) = 0;

        //! This method gets the minimum value.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Minimum float value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return Returns the minimum 
        */
        virtual double GetMin( IpxCamErr *err = nullptr ) = 0;

        //! This method gets the maximum value.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Maximum float value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return Returns the maximum 
        */
        virtual double GetMax( IpxCamErr *err = nullptr ) = 0;

        //! This method gets the Unit.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the units
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return Returns the measurement unit string
        */
        virtual const char* GetUnit( IpxCamErr *err = nullptr )  = 0;
	};

	/**  

	\brief A class containing methods for Integer GenICam camera parameter
	\details A class containing methods to access the Integer GenICam camera parameter as integer value.

	For example, the mapping below illustrates "Width" Integer parameter. 
	
	\image html int.png
	*/
	class Int : virtual public Param
	{
	public:

        //! This method returns the node object Int type.
        /*!
        \return Returns the parameter type 
        */
        virtual ParamType GetType(){ return ParamInt; }

        //! This method sets the Int node value.
        /*!
        \param[in] val Int node value 
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully sets the Int value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        - \c IpxCamErr::IPX_CAM_GENICAM_OUT_OF_RANGE - the value entered is out of range  
        */
        virtual IpxCamErr SetValue( int64_t val ) = 0;

        //! This method gets the Int node value.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Int value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return Returns the Int node value 
        */
        virtual int64_t GetValue( IpxCamErr *err = nullptr ) = 0;

        //! This method gets the minimum value.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Minimum int value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type 
        \return Returns the minimum 
        */
        virtual int64_t GetMin( IpxCamErr *err = nullptr )  = 0;

        //! This method gets the maximum value.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Maximum int value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return Returns the maximum 
        */
        virtual int64_t GetMax( IpxCamErr *err = nullptr )  = 0;

        //! This method gets the Increment value.
        /*!
        \param[out] err returns error code :
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the increment value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return Returns the increment 
        */
        virtual int64_t GetIncrement( IpxCamErr *err = nullptr ) = 0;
	};

	/**  

	\brief A class containing methods for String GenICam camera parameter
	\details A class containing methods to access the String GenICam camera parameter as zero-terminated array of characters

	For example, the image below illustrates "DeviceModelName" parameter.

	\image html string.png

	*/
	class String : virtual public Param
	{
	public:

        //! This method returns the node object String type.
        /*!
        \return The parameter type 
        */
        virtual ParamType GetType(){ return ParamString; }

        //! This method gets the Maximum Length of the string.
        /*!
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the maximum length value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return gets the maximum length of the string
        */
        virtual size_t GetMaxLength( IpxCamErr *err = nullptr ) = 0;

        //! This method gets the value of the string node.
        /*!
        \param[out] len return the length of the string
        \param[out] err returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the string 
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return Returns the value
        */
        virtual const char* GetValue( size_t *len = nullptr, IpxCamErr *err = nullptr ) = 0;

        //! This method sets the value of the string node.
        /*!
        \param[in] val Set the value of the string node
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully sets the string 
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        */
        virtual IpxCamErr SetValue( const char *val ) = 0;
	};

	/**  
	\brief  An Array class contains methods to access all GenICam camera parameters
	\details This class contains methods that can access each node from the GenICam camera description XML file by parameters type and name.
	*/
	class Array
	{
	public:

        //! Array class destructor.
        /*!
        \brief Array class destructor. Destroys the Array object and all its descendants.
        */
        virtual ~Array(){}

        //! This method gets the pointer to the Param class object for the specified node name from the node map declared in the camera descriptor XML file
        /*!
        \param[in] name Unique name of a node in node map.  
        \param[out] err Returns an error code:
         - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to Param class of the specified node name
         - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - specified node name not found in camera descriptor XML file
        \return If the method succeeds, it returns the pointer to the Param class for the specific node name. Otherwise, it returns a nullptr.
        */
        virtual Param* GetParam( const char *name, IpxCamErr *err ) = 0;

        //! This method gets the pointer to the Boolean class object for the specified node name of the camera descriptor XML file
        /*!
         \param[in] name A unique name of Boolean type node in the camera descriptor XML file.  
         \param[out] err Returns an error code:
         - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to Boolean class of the specified node name
         - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR - specified node name not found in camera descriptor XML file
         \return If the method succeeds, it returns the pointer to the Boolean class for the specific node name. Otherwise, it returns a nullptr.
        */
        virtual Boolean* GetBoolean( const char *name, IpxCamErr *err ) = 0;

        //! This method gets the pointer to the Command class object for the specified node name of the camera descriptor XML file
        /*!
         \param[in] name Unique name of Command type node in the camera descriptor XML file. 
         \param[out] err returns an error code:
         - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to Command class of the specified node name
         - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR - specified node name not found in camera descriptor XML file
         \return If method succeeds, it returns the pointer to the Command class for the specific node name. Otherwise, it returns a nullptr.
        */
        virtual Command* GetCommand( const char *name, IpxCamErr *err ) = 0;

        //! This method gets the pointer to the Enum class object for the specified node name of the camera descriptor XML file
        /*!
         \param[in] name Unique name of Enumeration type node in the camera descriptor XML file.
         \param[out] err returns an error code:
         - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to Enum class of the specified node name
         - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR - specified node name not found in camera descriptor XML file 
         \return If the method succeeds, it returns the pointer to the Enum parameter class for the specific node name. Otherwise, it returns a nullptr.
        */
        virtual Enum* GetEnum( const char *name, IpxCamErr *err ) = 0;

        //! This method gets the pointer to the Float class object for the specified node name of the camera descriptor XML file
        /*!
         \param[in] name Unique name of Float type node in the camera descriptor XML file. 
         \param[out] err returns an error code:
         - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to Float class of the specified node name
         - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR - specified node name not found in camera descriptor XML file 
         \return If the method succeeds, it returns the pointer to the Float parameter class for the specific node name
        */
        virtual Float* GetFloat( const char *name, IpxCamErr *err ) = 0;

        //! This method gets the pointer to the Int class object for the specified node name of the camera descriptor XML file
        /*!
         \param[in] name Unique name of Integer type node in the camera descriptor XML file.
         \param[out] err returns an error code:
         - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to Int class of the specified node name
         - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR - specified node name not found in camera descriptor XML file 
         \return If the method succeeds, it returns the pointer to the Int class for the specific node name
        */
        virtual Int* GetInt( const char *name, IpxCamErr *err ) = 0;

        //! This method gets the pointer to the String class object for the specified node name of the camera descriptor XML file
        /*!
         \param[in] name Unique name of String type node in the camera descriptor XML file. 
         \param[out] err returns an error code:
         - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to String class of the specified node name
         - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR - specified node name not found in camera descriptor XML file  
         \return If the method succeeds, it returns the pointer to the String class for the specific node name
        */
        virtual String* GetString( const char *name, IpxCamErr *err ) = 0;

        //! This method gets the pointer to the root category node object. The Root node is considered a special node. It has no parent node. In the topology graph, it is the top node which connects to at least one child node. The child node may connect to the device node that provides the connection to the transport layer.
        /*!
        \param[out] err returns an error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to Category class
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR - specified Root node name not found in camera descriptor XML file  
        \return Returns the pointer to the Category (root node) class
        */
        virtual Category* GetRootCategory( IpxCamErr *err ) = 0;

        //! This method gets the pointer to the NodeMap interface. The NodeMap interface will provide methods to retrieves all nodes in the node map.
        /*! 
        \param[out] err returns an error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to GenApi::INodeMap class
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR - the node map does not exist  
        \return nodemap returns the pointer to the NodeMap interface
        */
        virtual IPX_GENAPI_NS::INodeMap* GetNodeMap( IpxCamErr *err ) = 0;

        //! This method gets the number of nodes.
        /*! 
        \return The number of nodes. This number should be greater than 0.
        */
        virtual uint32_t GetCount() = 0;

        //! This method gets the parameter by index.
        /*! 
        \param[in] idx Index
        \param[out] err returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully returns pointer to Param class
        - \c IpxCamErr::IPX_CAM_ERR_INVALID_INDEX  - entered invalid index
        \return Returns param pointer to Parameter class of the specified node referenced by the index value 
        */
        virtual Param* GetParamByIndex( uint32_t idx, IpxCamErr *err ) = 0;

        //! This method sets the Boolean value of the Boolean node.
        /*! 
        \param[in] name Unique name of Boolean node to set
        \param[in] aValue Boolean value to set 
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully set the Boolean value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR  -  Unable to access genicam specified node
        */
        virtual IpxCamErr SetBooleanValue( const char *name, bool aValue ) = 0;

        //! This method gets the Boolean value of the Boolean node.
        /*! 
        \param[in] name Unique name of Boolean node to get
        \param[out] err returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Boolean value
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node 
        \return Returns the Boolean Value 
        */
        virtual bool GetBooleanValue( const char *name, IpxCamErr *err = nullptr ) = 0;

        //! This method sets the Enum node maps and the Enum interface to a name and index value. Each of the enum entries are represented by a name and index pair. This method sets the Enum value String of the corresponding node. The enum nodes map to a drop down box.
        /*! 
        \param[in] name Name of Enum entry node to set
        \param[in] val Enum node string value to set 
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully sets the Enum Value string
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR  -  Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_OUT_OF_RANGE - the value entered is out of range
        */
        virtual IpxCamErr SetEnumValueStr( const char *name, const char *val ) = 0;

        //! This method sets the Enum value of the enum node.
        /*! 
        \param[in] name Unique name of Enum entry to set
        \param[in] val Enum entry integer value to set 
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Enum value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        - \c IpxCamErr::IPX_CAM_GENICAM_OUT_OF_RANGE - the value entered is out of range
        */        
        virtual IpxCamErr SetEnumValue( const char *name, int64_t val ) = 0;

        //! This method gets the Enum value string of the current set Enum value entry.
        /*! 
        \param[in] name Unique name of Enum entry 
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Enum string value
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        \return Get the Enum value String of the current set Enum Value Entry 
        */                
        virtual const char* GetEnumValueStr( const char *name, IpxCamErr *err = nullptr ) = 0;

        //! This method gets the Enum value of the Enum node.
        /*! 
        \param[in] name Unique name of Enum type node in the camera descriptor XML file. 
        \param[out] err returns error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Enum value
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type 
        \return Returns the Enum Value 
        */                
        virtual int64_t GetEnumValue( const char *name, IpxCamErr *err = nullptr ) = 0;

        //! This method sets the Float value of the Float node.
        /*! 
        \param[in] name Unique name of Float type node in the camera descriptor XML file. 
        \param[in] val Float value to set 
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully sets the Float value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        - \c IpxCamErr::IPX_CAM_GENICAM_OUT_OF_RANGE - the value entered is out of range 
        */        
        virtual IpxCamErr SetFloatValue( const char *name, double val ) = 0;

        //! This method gets the Float value of the Float node.
        /*! 
        \param[in] name Unique name of Float type node in the camera descriptor XML file. 
        \param[out] err returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Float value
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type  
        \return Returns the Float value  
        */        
        virtual double GetFloatValue( const char *name, IpxCamErr *err = nullptr ) = 0;

        //! This method sets the Integer value of the Integer node.
        /*! 
        \param[in] name Unique name of Integer type node in the camera descriptor XML file. 
        \param[in] val Integer value to set 
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully sets the Integer value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        - \c IpxCamErr::IPX_CAM_GENICAM_OUT_OF_RANGE - the value entered is out of range  
        */        
        virtual IpxCamErr SetIntegerValue( const char *name, int64_t val ) = 0;

        //! This method gets the Integer value of the Integer node.
        /*! 
        \param[in] name Unique name of Integer type node in the camera descriptor XML file. 
        \param[out] err returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the Integer value
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type 
        \return Returns the Integer value 
        */        
        virtual int64_t GetIntegerValue( const char *name, IpxCamErr *err = nullptr ) = 0;

        //! This method sets the String value of the String node.
        /*! 
        \param[in] name Unique name of String type node in the camera descriptor XML file. 
        \param[in] val String value to set 
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully sets the String value
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type
        - \c IpxCamErr::IPX_CAM_GENICAM_OUT_OF_RANGE - the value entered is out of range 
        */        
        virtual IpxCamErr SetStringValue( const char *name, const char *val ) = 0;

        //! This method gets the String value of the String node.
        /*! 
        \param[in] name Unique name of String type node in the camera descriptor XML file. 
        \param[out] err returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully gets the String value
        - \c IpxCamErr::IPX_CAM_GENICAM_UNKNOWN_PARAM - unknown parameter
        - \c IpxCamErr::IPX_CAM_GENICAM_TYPE_ERROR  -  Unable to access genicam specified node type 
        \return Returns the String value 
        */        
        virtual const char* GetStringValue( const char *name, IpxCamErr *err = nullptr ) = 0;

        //! This method executes/submits the command.
        /*! 
        \param[in] name Unique name of Command type node in the camera descriptor XML file.  
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully determines state of executed command.
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        */        
        virtual IpxCamErr ExecuteCommand( const char *name ) = 0;

        //! This method polls the corresponding executed command to see if the executed command is done or not.
        /*! 
        \param[in] name Unique name of Command type node in the camera descriptor XML file. 
        \param[out] err returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully determines state of executed command.
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node  
        \return Returns true if the Execute command has finished. Otherwise, returns false. 
        */        
        virtual bool IsCommandDone( const char *name, IpxCamErr *err = nullptr ) = 0;

        //! This method fires nodes which have a polling time
        /*!
        \param[in] elapsedTime Time elapsed since last poll in msec
        \return Returns the error code:
        - \c IpxCamErr::IPX_CAM_ERR_OK  -  Successfully determines state of executed command.
        - \c IpxCamErr::IPX_CAM_GENICAM_ACCESS_ERROR - Unable to access genicam specified node
        */
        virtual IpxCamErr Poll(int64_t elapsedTime) = 0;
	};
} // end of namespace IpxGenParam


/*! \namespace IpxCam
    \brief A namespace providing scope to the GenICam GenTL transport layer interface to 
	acquire images with an Imperx Camera.

	\details IpxCam namespace includes classes that represent the base GenTLtransport layer modules:  
	System, Interface, Device, Stream, Buffer. These modules can be used to enumerate the interfaces in the system, 
	enumerate the cameras, connected to each interface, connect to necessary camera, allocate the memory buffers for images, 
	and run the video acquisition.
*/
namespace IpxCam
{
	/**  
	\brief The List class is used as list-like container for the specified template type objects. 
	\details The supported template type objects are <b>Interface</b>,<b>Device</b>,<b>DeviceInfo</b>,<b>Stream</b>, and <b>Buffer</b>.  
        
*	They can be declared as follows:

*	<table style="background-color: #FFFFFF"  border="1">
*	 <tr><th>IpxCam::List<Interface> *interfaceList</th><th>This class represents the list of Interface objects.</th></tr>
*	 <tr><th>IpxCam::List<Device>  *deviceList</th><th>This class represents the list of Device objects.</th></tr>
*	 <tr><th>IpxCam::List<DeviceInfo> *deviceInfoList</th><th>This class represents the list of DeviceInfo objects.</th></tr>
*	 <tr><th>IpxCam::List<Stream> *streamList</th><th>This class represents the list of Data Stream objects</th></tr>
*	 <tr><th>IpxCam::List<Buffer> *bufferList</th><th>This class represents the list of Buffer objects</th></tr>
*	 </table>

*	 Alternatively, you can also use the declared typedef (aliases for specific objects) provided in the IpxCam namespace as shown below:

*	\include snippets/ListTypedefs.cpp

*	They can be declared as follows:

*	<table style="background-color: #FFFFFF"  border="1">
*	 <tr><th>InterfaceList *interfaceList</th><th>This class represents the list of Interface objects.</th></tr>
*	 <tr><th>DeviceInfoList *deviceInfoList</th><th>This class represents the list of DeviceInfo objects.</th></tr>
*	 </table>

*	This class can be used to search through the list of objects discovered.
*	
*	<b>Example using DeviceInfoList</b>

*    In this example, you will see how to use the DeviceInfoList. An example is shown below that demonstrates on how to use the list class methods. The <b>deviceInfoList->GetCount()</b> method
*	is used retrieve the number of devices connected. We confirm that at least one device is available.  Next, the for loop will loop 
*	from the first device information listed using the <b>deviceInfoList->GetFirst()</b> function to the end of the list. During each 
*	iteration the <b>deviceInfoList->GetNext()</b> will increment to the next deviceInfo available.   In the example, you will notice 
*	that we search for a specified device model name.  Once, the specified device is found, we will release the <b>deviceInfoList->Release()</b> 
*    and the create the specified device using the <b>IpxCam::IpxCam_CreateDevice()</b> method.

*	\include snippets/DeviceInfoList.cpp

*	<b> Example using InterfaceList</b>

*	In this example, you will see how to use the InteraceList. You will retrieve the interfaces available for this system. Next, the for
*    loop will loop from the first interface available using the <b>list->GetFirst()</b> method to the end of the list. During each
*	iteration the <b>list->GetNext()</b> will increment to the next interface available.

*	\include snippets/InterfaceListEx.cpp
*	*/

	template<class _T>
	class List
	{
	public:

        //! A destructor of the List class.
        /*!
        \brief Destructor. Destroys the List object and all its descendants.
        */
        virtual ~List(){}

        /** Element Type*/
        typedef _T elem_type;

        /*! \fn "virtual void" Release()
        \brief This method releases the instance of the list of the specified object.
        \return Void.
        */
        virtual void Release() = 0;

        //! This functions gets the number of items in the specified list object 
        /*! 
        \return Returns the number of items in the specified list object.  
        */        
        virtual size_t GetCount() = 0;

        //! This method retrieves the first element in the specified list object
        /*! 
        \return Returns the first element in the specified list object.
        */        
        virtual elem_type* GetFirst() = 0;

        //! This method retrieves the next element in the specified list object
        /*! 
        \return Returns the next element in the specified list object.  
        */        
        virtual elem_type* GetNext() = 0;
	};
	
	//! An enum of Interface Types. Interface Node Types representing physical interface in the system. 
	enum InterfaceType : uint32_t
	{
        USB3Vision   = 1, /*!< Enum value for USB3Vision camera interface. */
        GigEVision  = 2, /*!< Enum value for GigEVision camera interface */
        CameraLink  = 3, /*!< Enum value for CameraLink camera interface */
        CoaxPress   = 4, /*!< Enum value for CoaxPress camera interface */
        HdSdi       = 5, /*!< Enum value for HD-SDI camera interface  */
        AllInterfaces      = 0xff, /*!< Enum value AllInterfaces. */
	};

	//! An enum of Flush Operations. Flush Operations Types. 
	enum FlushOperation : uint32_t
	{
        Flush_OutputDiscard   = 1, /*!< Enum value Flush_OutputDiscard.  Discards all buffers in the output queue and if necessary remove the entries from the event data queue. */
        Flush_AllToInput      = 2, /*!< Enum value Flush_AllToInput.  Puts all buffers in the input pool. Even those in the output queue and discard entries in the event data queue. */
        Flush_UnqueuedToInput = 3, /*!< Enum value Flush_UnqueuedToInput. Puts all buffers that are not in the input pool or the output queue in the input pool. */
        Flush_AllDiscard      = 4  /*!< Enum value Flush_AllDiscard. Discards all buffers in the input pool and output queue. */
	};

	class Interface;
	class DeviceInfo;
	class Device;
	class Stream;
	class Buffer;
	
	/** 
	List of Interface objects
	*/
	typedef List<Interface>    InterfaceList;

    /** 
	List of DeviceInfo objects
	*/
	typedef List<DeviceInfo>   DeviceInfoList;

    /** 
	List of Device objects
	*/
	typedef List<Device>       DeviceList;


    /** 
	EventCallback
	\deprecated Use EventCallback2 instead
	*/
	typedef void IPXCAM_CALL EventCallback(const void *eventData, size_t eventSize, void *pPrivate);

	//! EventCallback2
	/** 
	Callback function type for Event handling
	param[in] eventType type of the arrived event 
	param[in] eventData pointer to event Data
	param[in] eventSize event Size
	param[in] pPrivate pointer to the context Data
	*/
    typedef void IPXCAM_CALL EventCallback2(uint32_t eventType, const void *eventData, size_t eventSize, void *pPrivate);
    

	/**  
    \brief The System class represents an abstraction of the System module of the GenTL module hierarchy. 
	
	\details  This class provides member functions to enumerate and instantiate the available interfaces reachable. It also provides a method for the configuration of the device module. 
	This system module is the root of the GenTL Module hierarchy. 
	<b>IpxCam::System</b>  class has member functions to find all the interfaces, display the user readable name and producer version of the GenTL system.
	The <b>IpxCam::System</b> class can be used to obtain <b>IpxCam::InterfaceList</b>, then get the list <b>IpxCam::DeviceInfo</b> objects on the <b>IpxCam::Interface</b>,  
	and create <b>IpxCam::Device</b> object, representing the camera device .

	The following is an example on how to use some of the public Member Functions.

	\include snippets/IpxStreamInfo.cpp 
	*/
	class System
	{
	public:

        //!  System class Destructor
        /** 
        Destroys the System object and all its descendants. 
        */
        virtual ~System(){}

        //!   This method releases the instance of the system object.
        /**
        \return void.
        
        The following shows an example on how to use the <b>Release</b> method to release the system object instantiated.
        \include snippets/ReleaseExample.cpp 
         */
        virtual void Release() = 0;

        //!  This method returns the list of all the interfaces of the system object.
        /**
        \details GetInterfaceList method lists all the available hardware interfaces with the transport layers technologies, 
        supported by GenTL producer library
        \param[in] type interface type
        \return Returns the interface list
        
        The following is an example on how to use the <b>GetInterfaceList</b> method.
        \include snippets/InterfaceListEx.cpp 
        */
        virtual InterfaceList* GetInterfaceList( InterfaceType type = AllInterfaces ) = 0;
        
        	
        //! Retrieves the interface specified by interface identifier.
        /**
        \details This method returns the interface by unique string identifier of the system object.
        \param[in] ifaceId Interface identifier
        \return Returns the Interface or nullptr if no such interface is found

         For example, the const char *ifaceId interface identification name could be as shown below:
         \include snippets/GetInterfaceById.cpp
         This method will retrieve the available interface list of the system.
        */
        virtual Interface* GetInterfaceById( const char *ifaceId ) = 0;

        //! Retrieves the name of the GenTL Producer.
        /*! 
        \details This method returns the User readable name of the GenTL Producer of the system object.
        \return Returns the Display Name string

        The following is an example on how to use the GetDisplayName method
        \include snippets/SystemDisplayName.cpp
        */        
        virtual const char* GetDisplayName() = 0;

        //!Returns the GenTL Producer version.
        /*! 
        This method returns the version of the GenTL Producer of the system object.
        \return Returns the Version string

        The following is an example on how to use the GetVersion method
        \include snippets/SystemVersion.cpp
        */        
        virtual const char* GetVersion() = 0;

        //! Creates the Device object from configuration file
        /*!
        This method creates, configures and sets up the device using the information retrieved from the specified configuration file
        \param[in] fileName Configuration file to open
        \param[out] err returns the error code
        \return Returns Device or nullptr if device cannot be instantiated
        */
        virtual Device* CreateDeviceFromConfig( const char *fileName, IpxCamErr *err = nullptr ) = 0;

        //! Registers the GenTL CTI library
        /*!
        This method registers the 3rd party GenTL provider CTI library in the System. 
        \param[in] fileName path to GenTL CTI file to add
        \return Returns the error code
        */
        virtual IpxCamErr RegisterGenTLProvider( const char *fileName ) = 0;
	};

	//! Returns the System object pointer
	/*! 
	 \details This method returns the System module object. System object is being created as soon as API library is loaded.
	 It is the entry point to the GenTL Module hierarchy.
	 \return Returns the pointer to system. 
	*/
    IPXCAM_EXTERN_C IPX_CAMERA_API System* IpxCam_GetSystem();

	/**  
	*\brief The Interface class represents a interface module in the GenTL module hierarchy. 
	\details This class represents an individual physical interface in the System. For
	example, a network interface card (NIC) for GigE Vision connection, CXP or Camera Link frame grabber board, 
	or USB3 Vision driver in the GenTL system. 
	The Interface class includes methods to enumerate the available devices on the physical interface in the system.
	*/
	class Interface
	{
    public:

        //! Interface class destructor.
        /*!
        Destroys the Interface object and all its descendants.
        */
        virtual ~Interface(){}

        //! This method retrieves the list of DeviceInfo objects for the camera devices, available on this Interface.
        /*! 
        \return Returns the pointer to DeviceInfoList object 

        For example,

        \include snippets/DeviceInfoList.cpp
        */        
        virtual DeviceInfoList* GetDeviceInfoList() = 0;

        //! This method retrieves the DeviceInfo object for the first device available on this Interface.
        /*! 
        \return Returns the pointer to DeviceInfo object or nullptr if no device found

        For example, 

        \code{.cpp}
        //Retrieve the first device available for the specified interface.
        lDeviceInfo = iface->GetFirstDeviceInfo();
        
        std::cout << "First Device Info ModelName" << lDeviceInfo->GetModel() << endl;
        
        \endcode
        */        
        virtual DeviceInfo* GetFirstDeviceInfo() = 0;

        //! This method retrieves the DeviceInfo object pointer for the specified device identifier.
        /*!
        \param[in] deviceId Device identifier
        \return Returns the pointer to DeviceInfo object or nullptr if no such device found

        */
        virtual DeviceInfo* GetDeviceInfoById( const char *deviceId ) = 0;

        //!  This method re-enumerates the devices.
        /*! 
        The ReEnumerateDevices method allows the user to re-enumerate the devices connected to the Interface and update 
        the DeviceInfoList object returned by subsequent GetDeviceInfoList() method calls.
        \param[in] pChanged Change in Device
        \param[in] iTimeout Timeout allowed to search for available camera devices
        \return Returns error code
        */        
        virtual IpxCamErr ReEnumerateDevices( bool *pChanged, uint64_t iTimeout ) = 0;

        //! This method returns the description of the interface
        /*! 
        The GetDescription method gets the user readable string description of the interface.
        \return Returns the Description of the interface  
        */        
        virtual const char* GetDescription() = 0;

        //! This method gets the type of interface 
        /*! 
        The GetType method returns the Interface Type (Transport Layer Technology) of this interface object
        \return Returns Interface Type

        The interface type return can be the following:

        \include snippets/InterfaceType.cpp
        */        
        virtual InterfaceType GetType() = 0;

        //! This method gets the identifier of the interface .
        /*! 
        The GetId method returns the interface identifier that could be used to instantiate the interface object
        \return Returns interface identifier
        */        
        virtual const char* GetId() = 0;

        //! This method gets the version of Interface driver
        /*! 
        Returns the pointer to the string with the version of the interface driver
        \return Returns the version of the interface driver
        */        
        virtual const char* GetVersion() = 0;

       
        //! This method registers the Interface class method as a callback method to be called when a eventType occurs.
        /*!
        \param[in] eventType Event Type
        \param[in] eventCallback pointer to event CallBack method
        \param[in] pPrivate pointer to user's data
        \return Returns Error code
        */
        virtual IpxCamErr RegisterEvent2( uint32_t eventType, IpxCam::EventCallback2 *eventCallback, void *pPrivate ) = 0;

        //! This method registers the Interface class method as a callback method to be called when a eventType occurs.
	    /*!
	    \deprecated Use RegisterEvent2 instead
	    */
        virtual IpxCamErr RegisterEvent(uint32_t eventType, IpxCam::EventCallback *eventCallback, void *pPrivate) = 0;


        //! This method unregisters the Interface class callback method for the eventType.
        /*! 
        \param[in] eventType Event Type
        \param[in] eventCallback pointer to event CallBack method
        \param[in] pPrivate pointer to user's data
        \return Returns Error code
        */        
        virtual IpxCamErr UnRegisterEvent2( uint32_t eventType, IpxCam::EventCallback2 *eventCallback, void *pPrivate ) = 0;

        //! This method unregisters the Interface class callback method for the eventType.
        /*!
        \deprecated Use UnRegisterEvent2 instead
        */
        virtual IpxCamErr UnRegisterEvent(uint32_t eventType, IpxCam::EventCallback *eventCallback, void *pPrivate) = 0;


        //! This method returns the parameter array used to control the Imperx Camera device.
        /*! 
        \param[out] err returns error code
        \return Returns the pointer to IpxGenParam::Array object, used to control the Imperx Camera device  
        */        
        virtual IpxGenParam::Array* GetParameters( IpxCamErr *err = nullptr ) = 0;

        //! Creates the Device object from configuration file
	    /*!
        This method creates, configures and sets up the device using the information retrieved from the specified configuration file
        \param[in] fileName Configuration file to open
        \param[out] err returns error code
        \return Returns Device or nullptr if device cannot be instantiated
        */
        virtual Device *CreateDeviceFromConfig( const char *fileName, IpxCamErr *err = nullptr ) = 0;
	};

	//! DeviceInfo class provides the information about the camera device.
	/**  
	 \details The DeviceInfo class can be used to retrieve the information about the device, 
	  and to create the IpxCam::Device object by IpxCam_CreateDevice() call
	*/
	class DeviceInfo
	{
    public:

        //! DeviceInfo class destructor.
        /*!
        	Destroys the DeviceInfo object and all its descendants.
        */
        virtual ~DeviceInfo(){}

        //! This method returns the interface of the device object.
        /*! 
        Returns the IpxCam::Interface object pointer for the camera device, associated with the DeviceInfo object
        \return Returns the Interface   
        */        
        virtual Interface* GetInterface() = 0;

        //! This method returns the unique device identifier string for the Imperx Camera device object.
        /*!
        \return Returns the unique device identifier string for the Imperx Camera device
        */
        virtual const char* GetID() = 0;

        //! This method returns the vendor name of the camera device object.
        /*! 
        \return Returns the camera device vendor name  
        */        
        virtual const char* GetVendor() = 0;

        //! This method returns the model name of the camera device object.
        /*! 
        \return Returns the Camera device model name
        */        
        virtual const char* GetModel() = 0;

        //! This method returns the user readable display name of the Camera device object.
        /*! 
        \return Returns the name of the Camera device
        */        
        virtual const char* GetDisplayName() = 0;

        //! This method returns the user defined name of the Camera device
        /*! 
        \return Returns the user defined name of the Camera device
        */        
        virtual const char* GetUserDefinedName() = 0;

        //! This method returns the serial number of the Camera device .
        /*! 
        \return Returns the serial number of the Camera device 
        */        
        virtual const char* GetSerialNumber() = 0;

        //! This method returns the device version of the device object.
        /*! 
        \return Returns the Device version 
        */        
        virtual const char* GetVersion() = 0;

        //! Returns the device access status 
        /*! 
        This method returns the information about the current access status of the Camera device
        \return Status Access Code, can receive one of the following values:
        - \c <b>AccessStatusUnknown</b> [0]  - The current availability of the device is unknown.
        - \c <b>AccessStatusReadWrite</b> [1] - The device is available for Read/Write access
        - \c <b>AccessStatusReadOnly</b> [2] - The device is available for Read only access
        - \c <b>AccessStatusNoAccess</b> [3] - The device is not available either because it is already open or because it is not reachable.
        - \c <b>IpSubnetMismatch</b> [1001] - The device is available, but IP address does not match to the host subnet mask.
        */        
        virtual int32_t GetAccessStatus() = 0;

        //! Returns the information about USB3 host controller
        /*! 
        This method returns the information about USB3 host controller where the camera device is connected to.
        \return Returns the pointer to string structure or nullptr for non-USB camera
        */        
        virtual const char* GetUSB3HostInfo() = 0;

        //! Returns the IP address of the GEV camera
        /*!
         This method returns the IP address of the GEV camera, retrieved from DISCOVERY_ACK packet, 
         received from the camera device
        \param[out] err Error code
        \return Returns IP Address string or nullptr for non-GEV camera
        */
        virtual const char* GetIPAddress(IpxCamErr * err) = 0;

        //! Returns the IP subnet mask of the GEV camera.
        /*!
         This method returns the IP subnet mask of the GEV camera, retrieved from DISCOVERY_ACK packet,
         received from the camera device
         \param[out] err Error code
        \return Returns IP subnet mask string or nullptr for non-GEV camera
        */
        virtual const char* GetIPMask(IpxCamErr * err) = 0;

        //! Returns the IP gateway of GEV camera.
        /*!
        This method returns the IP gateway of the GEV camera, retrieved from DISCOVERY_ACK packet,
        received from the camera device
        \param[out] err Error code
        \return Returns IP gateway string or nullptr for non-GEV camera
        */
        virtual const char* GetIPGateway(IpxCamErr * err) = 0;


        //! Gets IP information from the GEV camera.
        /*!
        This method returns the IP address, netmask, and gateway of the GEV camera, from DISCOVERY_ACK packet, 
        received from the camera
        \param[out] addr IP Address
        \param[out] netmask IP Address subnet mask 
        \param[out] gateway Gateway address 
        \return Returns Error code
        */
        virtual IpxCamErr GetIP(uint32_t *addr, uint32_t *netmask, uint32_t *gateway) = 0;

        //! Set the IP address to GEV camera.
        /*!
        This method sets the specified IP address to the GEV camera, using ForceIP GVCP command
        \param[in] addr IP Address string to set
        \param[in] netmask IP Address subnet mask string 
        \param[in] gateway Gateway address string 
        \return Returns Error code
        */
        virtual IpxCamErr ForceIP(const char *addr, const char *netmask, const char *gateway) = 0;

		//! Set IP address to GEV camera.
		/*!
		This method sets the specified IP address to the GEV camera, using ForceIP GVCP command
        \param[in] addr IP Address to set (host byte order)
        \param[in] netmask IP Address subnet mask (host byte order)
        \param[in] gateway Gateway address (host byte order)
        \return Returns Error code
        */
        virtual IpxCamErr ForceIP(uint32_t addr, uint32_t netmask, uint32_t gateway) = 0;
        
        static const uint32_t AccessStatusUnknown   = 0;
        static const uint32_t AccessStatusReadWrite = 1;
        static const uint32_t AccessStatusReadOnly  = 2;
        static const uint32_t AccessStatusNoAccess  = 3;
        static const uint32_t IpSubnetMismatch      = 1001;
	};

	//! An enum of Device Access.
	enum DeviceAccess : uint32_t
	{
        ReadOnly = 0, /*!< Enum value ReadOnly. */ 
        Control = 1,  /*!< Enum value Control. */ 
        Exclusive = 2 /*!< Enum value Exclusive. */ 
	};

    /*! \fn "IPX_CAMERA_API Device*" IpxCam_CreateDevice(DeviceInfo* info = nullptr, DeviceAccess access = Exclusive);
	* \param[in] info
	  \param[in] access
	  \param[out] err returns the error code
	  \return Returns the pointer to Device
      \brief If info is nullptr, first default device created
	*/
    IPXCAM_EXTERN_C IPX_CAMERA_API Device* IpxCam_CreateDevice(DeviceInfo *info = nullptr, DeviceAccess access = Exclusive, IpxCamErr *err = nullptr);

    /**
    \brief The Device class represents the device module in the GenTL module hierarchy.
    \details This Device class provides methods to enable the communication and control of the Imperx device and enumerate/instantiate
    data stream objects. The methods can be used to enumerate and instantiate the Data Stream module objects. The device must
    must correspond to the interface transport layer technology. For example,the device could be an Imperx GEV Camera and the transport layer
    technology would be GEV. The Device class can be used to retrieve data information about the device by returning the pointer to the DeviceInfo class.
    It can be used to retrieve the pointer to the Stream object and save / load the camera configurations to / from file.
    */
	class Device
	{
	public:

        // An enum of events used during uploading files to a device.
        enum UploadEventType : uint32_t
        {
            FlashSectorErase,/*!< Enum value FlashSectorErase. */
            FlashPageWrite,/*!< Enum value FlashPagewrite. */
            FlashPageRead/*!< Enum value FlashPageRead. */
        };

        //! An enum of endianness types of underlying protocol
        enum Endianness : uint8_t
        {
        	BigEndian, /*!< Enum value Big-endian. */
        	LittleEndian /*!< Enum value Little-endian*/
        };

        //! A destructor of the Device class.
        /*!
        \brief Destructor. Destroys the Device and all its descendants.
        */
        virtual ~Device(){}

        /*! \fn "virtual void" Release()
        \brief This method releases the instance of the device object.
        * This method releases the device object.
        */
        virtual void Release() = 0;

        //! This method retrieves the number of the data streams, provided by the Device
        /*!
        \return returns the number of the data streams
        */
        virtual uint32_t GetNumStreams() = 0;

        //! This retrieves the pointer to the Stream object by stream index
        /*!
        \param[in] idx stream index value
        \return Returns the pointer to the Stream object
        */        
        virtual Stream* GetStreamByIndex( uint32_t idx = 0 ) = 0;

        //! This method retrieves the pointer to the Stream object by stream identifier 
        /*!
        \param[in] id pointer to the string representing the stream identifier
        \return Returns the pointer to the Stream object
        */
        virtual Stream* GetStreamById( const char *id ) = 0;

        //! This method returns a pointer to the DeviceInfo object , associated with the Device
        /*! 
        \return Returns the pointer to the DeviceInfo object   
        */        
        virtual DeviceInfo* GetInfo() = 0;

        //! This method reads a number of bytes from a given address of the Device
        /*! 
        \param[in] addr Byte address to read from
        \param[in] data pointer to a user allocated byte data buffer
        \param[in] len  size of the amount of bytes to read from the register map address
        \return Returns ErrorCode 
        */        
        virtual IpxCamErr ReadMem( uint64_t addr, void *data, size_t len ) = 0;

        //! This method writes a number of bytes at a given address.
        /*! 
        \param[in] addr Byte address to read from
        \param[in] data pointer to a user allocated byte data buffer
        \param[in] len  size of the amount of bytes to write to the register map address
        \param[out] written size of bytes written
        \return Returns ErrorCode  
        */        
        virtual IpxCamErr WriteMem( uint64_t addr, const void *data, size_t len, size_t *written ) = 0;

      
        //! This method registers the Device class method as a callback method to be called when a eventType occurs.
        /*! 
        \param[in] eventType Event Type, can receive one of the following values:
        - \c <b>GenICamEvent</b> [1002] - this event occurs, if GenICam event was triggered by the camera
        - \c <b>CameraConnected</b> [1003] - this event occurs, when camera was connected to the System
        - \c <b>CameraDisconnected</b> [1004] - this event occurs, when camera was disconnected from the System
        \param[in] eventCallback event CallBack
        \param[in] pPrivate pointer to user's data
        \return Returns Error code
        */        
        virtual IpxCamErr RegisterEvent2( uint32_t eventType, IpxCam::EventCallback2 *eventCallback, void *pPrivate ) = 0;

        //! RegisterEvent
        /*!
        \deprecated Use Device::RegisterEvent2 instead
        */
        virtual IpxCamErr RegisterEvent(uint32_t eventType, IpxCam::EventCallback *eventCallback, void *pPrivate) = 0;

        static const uint32_t GenICamEvent = 1002; //! This event occurs, if GenICam event was triggered by the camera device
        static const uint32_t CameraConnected = 1003; //! This event occurs, when the camera was connected to the System
        static const uint32_t CameraDisconnected = 1004; //! This event occurs, when the camera was disconnected from the System

        //! This method unregisters the Interface class callback method for the eventType.
        /*! 
        \param[in] eventType Event Type, can receive one of the following values:
        - \c <b>GenICamEvent</b> [1002] - this event occurs, if GenICam event was triggered by the camera
        - \c <b>CameraConnected</b> [1003] - this event occurs, when camera was connected to the System
        - \c <b>CameraDisconnected</b> [1004] - this event occurs, when camera was disconnected from the System
        \param[in] eventCallback event CallBack
        \param[in] pPrivate pointer to user's data
        \return Returns Error code
        */        
        virtual IpxCamErr UnRegisterEvent2( uint32_t eventType, IpxCam::EventCallback2 *eventCallback, void *pPrivate ) = 0;

        //! UnRegisterEvent
        /*!
        \deprecated Use Device::UnRegisterEvent2 instead
        */
        virtual IpxCamErr UnRegisterEvent(uint32_t eventType, IpxCam::EventCallback *eventCallback, void *pPrivate) = 0;

	    //! This method returns the transport parameters IpxGenParam::Array object of the camera device object.
        /*! 
        \param[out] err returns error code
        \return Returns the Transport parameters object pointer   
        */        
        virtual IpxGenParam::Array* GetTransportParameters( IpxCamErr *err = nullptr ) = 0;

        //! This method returns the camera parameters IpxGenParam::Array object of the device object.
        /*! 
        \param[out] err returns error code
        \return Returns the Camera Parameters array object pointer  
        */        
        virtual IpxGenParam::Array* GetCameraParameters( IpxCamErr *err = nullptr ) = 0;

        //! This method saves the camera parameters to the configuration file
        /*!
        \param[in] fileName Configuration file name
        \return Returns Error code
        */
        virtual IpxCamErr SaveConfiguration( const char *fileName ) = 0;

        //! This method loads the configuration from file, and configures the camera with the parameter values, saved to this file.
        /*!
        \param[in] fileName Configuration file name
        \return Returns Error code
        */
        virtual IpxCamErr LoadConfiguration( const char *fileName ) = 0;

       
        //! This method returns endianness of underlying protocol for this camera device 
        /*!
        \return Returns endianness
        */
        virtual Endianness GetEndianness() const = 0;

#ifndef IPX_DONT_INCLUDE_TO_DOC
        // A structure representing data for uploading to a device.
        struct UploadEventData
        {
        	uint64_t addr_min;
        	uint64_t addr_curr;
        	uint64_t addr_max;
        	uint64_t error;
        };
        typedef void IPXCAM_CALL UploadEventCallback(UploadEventType eventType, UploadEventData eventData);
#endif // IPX_DONT_INCLUDE_TO_DOC
	};

	/**  
	*  
	\brief The Stream class represents the data stream module in the GenTL module hierarchy.
	\details This data stream class provides buffer methods. This data stream class purpose is to 
	access the buffer data acquirement from the Acquisition engine.
	*/
	class Stream
	{
	public:

        //! A destructor of the Stream class.
        /*!
        Destroys the Stream object and all its descendants.
        */
        virtual ~Stream(){}

        //! This method releases the instance of the stream object.
        /*! 
        	\return void
       	*/
        virtual void Release() = 0;

        //! Creates the buffer in the data stream object.
        /*! 
        This method allocates the memory for a buffer and announces this buffer to the data stream
        \param[in]  iSize  Size of the buffer 
        \param[in]  pPrivate pointer to private data (user's data) which will be passed to the GenTL Consumer
        \param[out] err returns Error code   
        \return Returns Buffer object pointer of the announced buffer
        */
        virtual IpxCam::Buffer* CreateBuffer( size_t iSize, void *pPrivate, IpxCamErr *err ) = 0;

        //! Sets memory buffer to create the Buffer object 
        /*! 
        This method is used to set the user-allocated memory buffer to create the Buffer object and announce it to the data stream.
        \param[in] pBuffer buffer
        \param[in] iSize size of Buffer
        \param[in] pPrivate pointer to user's data
        \param[out] err returns Error code   
        \returns returns Buffer object pointer
        */        
        virtual IpxCam::Buffer* SetBuffer( void *pBuffer, size_t iSize, void *pPrivate, IpxCamErr *err ) = 0;

        //! Revokes any announced buffer.
        /*!
        This method removes the specified announced Buffer from the acquisition engine's queue 
        \param[in] buff Buffer object pointer
        \return Returns Error code   
        */        
        virtual IpxCamErr RevokeBuffer( IpxCam::Buffer *buff ) = 0;

        //! This method queues specified buffers.
        /*! 
        During the acquisition, this method is used to return the specified buffer to the acquisition engine's queue 
        \param[in] buff Buffer object pointer
        \return Returns Error code   
        */        
        virtual IpxCamErr QueueBuffer( IpxCam::Buffer *buff ) = 0;

        //! This method retrieves the buffer object.
        /*! 
        Retrieves the next acquired buffer entry from the acquisition engine's queue and returns the acquired Buffer object
        \param[in] iTimeout timeout in ms
        \param[in] err error code
        \return Returns the pointer to the acquired Buffer object
        */        
        virtual IpxCam::Buffer* GetBuffer( uint64_t iTimeout, IpxCamErr *err = nullptr ) = 0;

        //! Terminates the waiting operation on a previously queued Buffer 
        /*! 
        This method cancels the waiting operation on a previously queued Buffer in the acquisition engine's queue 
        \return Returns Error code   
        */        
        virtual IpxCamErr CancelBuffer() = 0;

        //! This method flushes the buffers of the data stream object.
        /*! 
        Performs the specified Flush Operation on the acquisition engine's queue. Operations type is defined in FlushOperations enum.
        \param[in] operation FlushOperation
        \return Returns Error code   
        */        
        virtual IpxCamErr FlushBuffers( FlushOperation operation ) = 0;

        //! Starts the Acquisition Engine
        /*! 
        This method starts the acquisition engine of the stream to acquire the image data frames to the queued buffers 
        \param[in] iNumFramesToAcquire number of Frames to Acquire. Set UINT64_MAX for the infinite acquisition
        \param[in] flags flags. Set to 0 by default
        \return Returns Error code   
        */        
        virtual IpxCamErr StartAcquisition( uint64_t iNumFramesToAcquire = UINT64_MAX, uint32_t flags = 0 ) = 0;

        //! Stops the stream's acquisition engine
        /*! 
        This method stops the acquisition engine of the stream and terminates the image data frames acquisition
        \param[in] flags flags: 
          - ACQ_STOP_FLAGS_DEFAULT=0, Stop the acquisition engine when the currently running tasks like filling a buffer are completed (default behavior). 
          - ACQ_STOP_FLAGS_KILL=1, Stop the acquisition engine immediately and leave buffers currently being filled in the Input Buffer Pool. 
        \return Returns Error code   
        */        
        virtual IpxCamErr StopAcquisition( uint32_t flags = 0 ) = 0;

        //! Allocates the Buffer Queue 
        /*! 
        This method allocates the buffers in the queue of the acquisition engine of the data stream object.
        \param[in] pPrivate pointer to user's data
        \param[in] iNum number of the buffers to allocate
        \return Returns Error code   
        */        
        virtual IpxCamErr AllocBufferQueue( void *pPrivate, size_t iNum ) = 0;

        //! Releases the Buffer Queue 
        /*! 
        This method releases the buffer queue of the data stream object.
        \return Returns Error code   
        */        
        virtual IpxCamErr ReleaseBufferQueue() = 0;

        //! Retrieves the Buffer Queue size
        /*! 
        This functions returns the buffer queue size of the data stream object.
        \return Returns the Buffer Queue size   
        */        
        virtual size_t GetBufferQueueSize() = 0;

        //! Registers the EventCallback.
        /*! 
        This method registers the data Stream class method as a callback method to be called when event of the specified type occurs.
        \param[in] eventType Event Type 
        \param[in] eventCallback event CallBack function pointer
        \param[in] pPrivate pointer to the user's data
        \return Returns Error code
        */        
        virtual IpxCamErr RegisterEvent( uint32_t eventType, IpxCam::EventCallback *eventCallback, void *pPrivate ) = 0;
        //
        virtual IpxCamErr RegisterEvent2( uint32_t eventType, IpxCam::EventCallback2 *eventCallback, void *pPrivate ) = 0;

        //! Unregisters the EventCallback. 
        /*! 
        This method unregisters the data Stream class callback method for the specified event type
        \param[in] eventType Event Type
        \param[in] eventCallback event CallBack function pointer
        \param[in] pPrivate pointer to the user's data
        \return Returns Error code
        */        
        virtual IpxCamErr UnRegisterEvent( uint32_t eventType, IpxCam::EventCallback *eventCallback, void *pPrivate ) = 0;
        //
        virtual IpxCamErr UnRegisterEvent2( uint32_t eventType, IpxCam::EventCallback2 *eventCallback, void *pPrivate ) = 0;

        //! Returns the GenICam parameters array
        /*! 
        This method returns the pointer to IpxGenParam::Array object of the GenICam parameters array for the data stream object
        \param[out] err returns the error code
        \return Returns the data stream GenICam parameters array   
        */        
        virtual IpxGenParam::Array* GetParameters( IpxCamErr *err = nullptr ) = 0;
	
        //! Returns the number of the delivered buffers
        /*! 
        This method returns the number of the delivered buffers since the start of the last acquisition 
        \return Returns the number of the delivered buffers 
        */        
        virtual uint64_t GetNumDelivered() = 0;

        //! Returns the number under-run frames.
        /*! 
        This method returns the number of the lost frames due to the acquisition queue being under-run.
        \return Returns the number of lost frames due to queue under-run   
        */        
        virtual uint64_t GetNumUnderrun() = 0;

        //! Returns the number of announced buffers
        /*! 
        This method returns the number of announced buffers in the data stream acquisition queue
        \return Returns number of announced buffers   
        */        
        virtual size_t GetNumAnnounced() = 0;

        //! Returns the number of queued buffers
        /*! 
        This method returns the number of queued buffers in the data stream object acquisition queue
        \return Returns the number of buffers in the input pool and the number of buffers currently being filled   
        */        
        virtual size_t GetNumQueued() = 0;

        //! Returns the number of buffers awaiting delivery
        /*! 
        This method returns the number of buffers awaiting the delivery from the data stream object acquisition queue 
        to the client application
        \return Returns the number of buffers in the output buffer queue   
        */        
        virtual size_t GetNumAwaitDelivery() = 0;

        //! Returns the buffer size
        /*! 
        This method returns the buffer size of the data stream object.
        \return Returns the buffer size   
        */        
        virtual size_t GetBufferSize() = 0;

        //! This method returns a flag indicating if the data stream is grabbing or not.
        /*! 
        \return Flag indicating the state of the acquisition engine. If true, acquisition engine has stared. Otherwise, the acquisition engine is off.   
        */        
        virtual bool IsGrabbing() = 0;

        //! Returns the minimum number of buffers to be announced
        /*!
        This method returns the minimum number of buffers to be announced in the data stream object acquisition queue to perform the grabbing
        \return Returns the minimum number of buffers to announce   
        */        
        virtual size_t GetMinNumBuffers() = 0;

        //! Returns the buffer alignment size
        /*! 
        This method returns the alignment size of the buffers in the stream object acquisition queue
        \return Returns the alignment size in bytes of the stream buffers 
        */        
        virtual size_t GetBufferAlignment() = 0;
	};

	/**  
	*\brief The Buffer class represents the buffer module in the GenTL module hierarchy. 
	\details The Buffer class contains the methods that can be used to get the pointer to the acquired image data memory and / or
	retrieve the information about the received image data such as timestamp, image size, pixel format, etc
	
	*/
	class Buffer
	{
	public:

        //! Buffer class destructor
        /*!
        Destroys the Buffer object and all its descendants.
       	\return none
        */
        virtual ~Buffer(){}

        //! Returns the pointer to the IpxImage structure
        /*! 
        This method returns the pointer to the IpxImage structure. See IpxTools user's manual for IpxImage structure description.
        \return Returns the pointer to the IpxImage structure. 
        */        
        virtual IpxImage* GetImage() = 0;

        //!  Returns the pointer to the image data
        /*! 
        This method returns the pointer to the memory of the acquired image data.
        \return Returns the pointer to the image data   
        */        
        virtual void* GetBufferPtr() = 0;

        //! Returns the offset of the actual image data start
        /*! 
        This method returns the offset of the actual image data start in the acquired data buffer memory.
        \return Returns the offset of the actual image data start
        */        
        virtual size_t GetImageOffset() = 0;

        //! This method returns the size of the allocated memory buffer in bytes
        /*! 
        \return Returns the buffer size in bytes  
        */        
        virtual size_t GetBufferSize() = 0;

        //! This method returns the pixel format of the buffer object.
        /*! 
        \return Returns the pixel format of the image in the buffer object. 
		This value equals to <b>PixeFormat</b> GenICam parameter
        */        
        virtual uint64_t GetPixelFormat() = 0;

        //! This method returns the user data buffer pointer, associated with the buffer object.
        /*! 
        \return Returns the user data buffer pointer
        */        
        virtual void* GetUserPtr() = 0;

        //! This method returns the timestamp of the acquired buffer.
        /*! 
        This method returns the timestamp of the acquired buffer. Imperx USB3 and GEV cameras have 10ns timestamp granularity.
        GEV cameras timestamp clock frequency can be obtained from <b>GevTimestampTickFrequency</b> GenICam parameter
        \return Returns the timestamp of the acquired buffer.   
        */        
        virtual uint64_t GetTimestamp() = 0;

        //! This method returns the identificator of the image stream block of the buffer object.
        /*! 
        \return Returns the identificator of the image stream block of the buffer object.
        */        
        virtual uint64_t GetFrameID() = 0;

        //! This method returns a flag indicating if the buffer data has been fully transferred or incompleted
        /*! 
        \return Returns True, if buffer transfer was incompleted, False, if transfer was successful
        */        
        virtual bool IsIncomplete() = 0;

        //! Returns the image width
        /*! 
        This method returns the image width of the buffer data in number of pixels.
         Usually the return value equals to <b>Width</b> GenICam parameter value
        \return Returns the image width   
        */        
        virtual size_t GetWidth() = 0;

        //! Returns the image height.
        /*! 
         This method returns the image height of the buffer data in number of lines.
         Usually the return value equals to <b>Height</b> GenICam parameter value
        \return Returns the image height 
        */        
        virtual size_t GetHeight() = 0;

        //! Returns the horizontal offset of the image data in the buffer   
        /*! 
         This method returns the horizontal offset of the image data in the buffer in number of pixels from the image origin.
         Usually the return value equals to <b>OffsetX</b> GenICam parameter value
        \return Returns the horizontal offset in number of pixels   
        */        
        virtual size_t GetXOffset() = 0;

        //! Returns the vertical offset of the image data in the buffer.
        /*! 
         This method returns the vertical offset of the image data in the buffer in number of lines from the image origin.
         Usually the return value equals to <b>OffsetY</b> GenICam parameter value

        \return Returns the vertical offset of the data in the buffer in number of lines from the image origin   
        */        
        virtual size_t GetYOffset() = 0;

        //! This method returns the number of extra bytes padded in the horizontal direction.
        /*! 
        \return Returns the XPadding of the data in the buffer in number of bytes   
        */        
        virtual size_t GetXPadding() = 0;

        //! This method returns the number of extra bytes padded in the vertical direction.
        /*! 
        \return Returns the YPadding of the data in the buffer in number of bytes   
        */        
        virtual size_t GetYPadding() = 0;

        //! This method returns the actual height of delivered data.
        /*!
        This method returns the actual height of delivered data. 
        Can be different than value returned by GetHeight() method, if image transfer was incompleted.
        \return Returns the actual height of delivered data
        */
        virtual size_t GetDeliveredHeight() = 0;

        //! This method indicates if this buffer is Frame A or Frame B, acquired from Cheetah camera with KAC-12040 or KAC-06040 CMOS sensor.
        /*! 
        \return Returns true for Frame B, false - otherwise   
        */        
        virtual bool IsKacFrameB() = 0;
	};

} // end of namespace IpxCam

#endif // __cplusplus

#endif // IPX_CAMERA_API_H
