////////////////////////////////////////////////////////////////////////////////
// Imperx Camera SDK C++ Sample Code
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Imperx camera video streaming, and image saving in trigger mode example (Console)
// File: IpxTriggerStreamWritingConsole.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2015-2019 Imperx Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

//
// Shows how to controle the camera trigger and use IpxCam::Stream class object to acquire images
//
#include "IpxCameraApi.h"
#include "IpxImage.h"
#include "IpxImageApi.h"
#include "IpxBayer.h"
#include "IpxImageSerializer.h"

#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <limits>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cinttypes>
#include <algorithm>

// Function Prototypes
IpxCam::Interface *SelectInterface( IpxCam::System *system );
IpxCam::DeviceInfo *SelectDevice( IpxCam::Interface *iface );
void GetSetParams( IpxCam::Device *device );
bool AcquireImages( IpxCam::Device *device, IpxCam::Stream *stream );
void FreeDataStreamBuffers( IpxCam::Stream *stream, std::vector<IpxCam::Buffer *> *buffers );
const char* GetAccessStatusStr( int32_t status );
std::vector<std::string> split(const std::string& s, char delimiter);
void ConfigureTrigger(IpxCam::Device *device);
void WriteImage(IpxCam::Buffer *buff, uint64_t file_idx, int file_ext);
IpxImage* CreateRgbImage(IpxImage *img);

// sync values
std::atomic_bool g_isStop(false);
bool g_result = false;

// Serializer
IpxHandle g_Serializer = nullptr;

// Bayer
IpxHandle g_Bayer = nullptr;

// File types
const char g_FileExt [4][5] = 
{
	".raw", // 0
	".bmp", // 1
	".jpg", // 2
	".tif", // 3
};
// Image file format index
uint32_t g_imgFormIdx = 0;

// Main function
int main()
{
    const char *indent = "    ";

    // Get System
	auto system = IpxCam::IpxCam_GetSystem();
    if (system)
    {
        while (true)
        {
            // Select desired interface
            auto iface = SelectInterface(system);
            if (iface)
            {
                // Select desired device
                auto deviceInfo = SelectDevice(iface);
                if (deviceInfo)
                {
                    auto deviceName = deviceInfo->GetDisplayName();
                    std::cout << "\nConnecting to " << deviceName << " ....\n";

                    if (deviceInfo->GetAccessStatus() == IpxCam::DeviceInfo::AccessStatusReadWrite)
                    {
                        auto device = IpxCam_CreateDevice(deviceInfo);
                        if (device)
                        {
                            std::cout << "Connecting to " << deviceName << " DONE\n\n";
                            GetSetParams(device);
                            std::cout << std::endl;
							
                            ConfigureTrigger(device);
                            std::cout << std::endl;

                            // Get image format index
                            std::cout << indent << "Set image format: RAW[0], BMP[1], JPG[2], TIF[3] (default [0]): ";
                            std::string str;
                            std::getline(std::cin, str, '\n');
                            if (!str.empty())
                                g_imgFormIdx = (uint32_t)std::atoi(str.c_str());
                            g_imgFormIdx = (g_imgFormIdx>4) ? 0 : g_imgFormIdx;

	                    // Create IpxImageSerializer and IpxBayer components
                            g_Serializer = IpxImageSerializer_CreateComponent(false);
                            g_Bayer = IpxBayer_CreateComponent();

                            IpxCam::Stream *stream = nullptr;
                            if (device->GetNumStreams() >= 1 && (stream = device->GetStreamByIndex(0)))
                            {
                                std::vector<IpxCam::Buffer*> bufferList;
                                auto bufSize = stream->GetBufferSize();
                                auto minNumBuffers = stream->GetMinNumBuffers();
                                for (size_t i = 0; i < minNumBuffers; ++i)
                                    bufferList.push_back(stream->CreateBuffer(bufSize, nullptr, nullptr));

                                // set and start separate acquisition thread
                                g_isStop = false;
                                std::thread thread(AcquireImages, device, stream);
                                std::cin.get();
                                g_isStop = true;

                                // cancel I/O for current buffer just in case acquisition thread is waiting it
                                stream->CancelBuffer();

                                thread.join();
                                if (!g_result)
                                    std::cout << "ERROR: AcquireImages failed" << std::endl;

                                stream->FlushBuffers(IpxCam::Flush_AllDiscard);
                                FreeDataStreamBuffers(stream, &bufferList);

                                // Close the stream
                                std::cout << "Closing stream    ...." << std::endl;
                                stream->Release();
                                std::cout << "Closing stream    DONE" << std::endl;

                                // Disconnect the device
                                std::cout << "Disconnecting device(" << deviceName << ")    ...." << std::endl;
                                device->Release();
                                std::cout << "Disconnecting device(" << deviceName << ")    DONE" << std::endl;
                            }
                            else
                                std::cout << "Unable to create a stream on" << deviceName << std::endl;

                            // Delete IpxImageSerializer and IpxBayer components
                            IpxImageSerializer_DeleteComponent(g_Serializer);
                            IpxBayer_DeleteComponent(g_Bayer);
                        }
                        else
                            std::cout << "Unable to connect to" << deviceName << std::endl;
                    }
                    else
                    {
                        std::cout << "Cannot connect due to AccessStatus("
                                  << GetAccessStatusStr(deviceInfo->GetAccessStatus()) << ")";
                        std::cout << "\nConnecting to " << deviceName << " FAIL\n";
                    }
                }
            }
            else
                break;
        }

        std::cout << "Releasing system" << std::endl;
        system->Release();
    }
    else
        std::cout << "Unable to create system!" << std::endl;

    return 0;
}

IpxCam::Interface *SelectInterface( IpxCam::System *system )
{
    std::vector<IpxCam::Interface*> ifaces;

    std::cout << "\nInterfaces Available:" << std::endl;

    // list has to be released, do this by using of unique pointer
    auto del = [](IpxCam::InterfaceList *l) { l->Release(); };
    std::unique_ptr<IpxCam::InterfaceList, decltype(del)> list(system->GetInterfaceList(), del);
    for (auto iface = list->GetFirst(); iface; iface = list->GetNext())
    {
        ifaces.push_back(iface);
        std::cout << "[" << (ifaces.size() - 1) << "]" << "\t" << iface->GetDescription() << std::endl;
    }

    uint32_t numIfaces = static_cast<uint32_t>(ifaces.size());
    if (numIfaces == 0)
        std::cout << "No interfaces available!\n";

    std::cout << "[" << ifaces.size() << "]" << "\t" << "To exit application\n";

    std::cout << "Enter Interface Number (default [0]): ";
    uint32_t sel = 0;

    std::string str;
    std::getline(std::cin, str, '\n');
    if (!str.empty())
        sel = (uint32_t)std::atoi(str.c_str());

    if (sel >= numIfaces)
        return nullptr;

    // Select interface
    auto iface = ifaces[sel];
    std::cout << "Interface Chosen: " << iface->GetDescription() << std::endl;
    return iface;
}

IpxCam::DeviceInfo *SelectDevice( IpxCam::Interface *iface )
{
    std::vector<IpxCam::DeviceInfo*> devices;
    const char *indent = "    ";
	
    // re-enumerate just in case new devices were connected
	iface->ReEnumerateDevices(nullptr, 200);

    std::cout << indent << "Devices Available:" << std::endl;

    // list has to be released, do this by using of unique pointer
    auto del = [](IpxCam::DeviceInfoList *l) { l->Release(); };
    std::unique_ptr<IpxCam::DeviceInfoList, decltype(del)> list(iface->GetDeviceInfoList(), del);
    for (auto device = list->GetFirst(); device; device = list->GetNext())
    {
        devices.push_back(device);
        std::cout << indent << "[" << (devices.size() - 1) << "]" << "\t" << device->GetDisplayName() << std::endl;
    }

    uint32_t numDevices = static_cast<uint32_t>(devices.size());
    if (numDevices == 0)
        std::cout << indent << "No devices available!\n";

    std::cout << indent << "[" << devices.size() << "]" << "\t" << "To search for interface\n";

    std::cout << indent << "Enter Device Number (default [0]): ";
    uint32_t sel = 0;

    std::string str;
    std::getline(std::cin, str, '\n');
    if (!str.empty())
        sel = (uint32_t)std::atoi(str.c_str());

    if (sel >= numDevices)
        return nullptr;

    // Select interface
    auto device = devices[sel];
    std::cout << indent << "Device Chosen: " << device->GetDisplayName() << std::endl;
    return device;
}

void ConfigureTrigger(IpxCam::Device *device)
{
	const char *indent = "    ";
	auto genParams = device->GetCameraParameters();
	if (genParams)
	{
		try
		{
			// Trigger mode
			std::cout << indent << "Set trigger mode: On[0], Off[1] (default [0]): ";
			uint32_t sel = 0;
			std::string str;
			std::getline(std::cin, str, '\n');
			if (!str.empty())
				sel = (uint32_t)std::atoi(str.c_str());

			IpxCamErr err = (sel==0)? genParams->SetEnumValueStr("TriggerMode", "On"):genParams->SetEnumValueStr("TriggerMode", "Off");
                        if(err)
                             std::cout << "Error, can not set TriggerMode, error code = " << err << std::endl;

			// Trigger mode : Line1, Line2, PulseGenerator
			std::cout << indent << "Set trigger source: Line1[0], Line2[1], PulseGenerator[2] (default [0]): ";
			sel = 0;
			std::getline(std::cin, str, '\n');
			if (!str.empty())
				sel = (uint32_t)std::atoi(str.c_str());
			
			if(sel== 0)
				err = genParams->SetEnumValueStr("TriggerSource", "Line1");
			else if(sel== 1)
				err = genParams->SetEnumValueStr("TriggerSource", "Line2");
			else if(sel== 2)
				err = genParams->SetEnumValueStr("TriggerSource", "PulseGenerator");
			
			// Check error code
			if(err)
                            std::cout << "Error, can not set TriggerSource, error code = " << err << std::endl;

			// Trigger activation: RisingEdge, FallingEdge
			std::cout << indent << "Set trigger activation: RisingEdge[0], FallingEdge[1] (default [0]): ";
			sel = 0;
			std::getline(std::cin, str, '\n');
			if (!str.empty())
                            sel = (uint32_t)std::atoi(str.c_str());
			err = (sel == 0) ? genParams->SetEnumValueStr("TriggerActivation", "RisingEdge") : genParams->SetEnumValueStr("TriggerActivation", "FallingEdge");

			// Check error code
                        if(err)
                            std::cout << "Error, can not set TriggerActivation, error code = " << err << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << indent << "Exception occured: " << e.what();
		}
	}

}


void GetSetParams( IpxCam::Device *device )
{
    const char *indent = "    ";
    auto genParams = device->GetCameraParameters();
    while (genParams)
    {
        try
        {
            std::cout << indent << "Set[1], get[2] a parameter, go on[0] (default [0]): ";
            uint32_t sel = 0;

            std::string str;
            std::getline(std::cin, str, '\n');
            if (!str.empty())
                sel = (uint32_t)std::atoi(str.c_str());

            if (!sel)
                break;

            bool isGet = sel > 1;

            std::string paramName, paramValue;

            std::cout << indent << "Enter parameter name: ";
            std::getline(std::cin, paramName);

            IpxGenParam::Param *param = nullptr;
            if (!(param = genParams->GetParam(paramName.c_str(), nullptr)))
            {
                std::cout << indent << "Cannot find '" << paramName << "' parameter!\n";
                continue;
            }

            // check if parameter Available, Readable, Writable
            if (!param->IsAvailable())
            {
                std::cout << indent << "'" << paramName << "' is not available!\n";
                continue;
            }
            else if (isGet)
            {
                if (!param->IsReadable())
                {
                    std::cout << indent << "'" << paramName << "' is not readable!\n";
                    continue;
                }
            }
            else if (!param->IsWritable()) // set
            {
                std::cout << indent << "'" << paramName << "' is not writable!\n";
                continue;
            }

            // convert to genuine type
            switch (param->GetType())
            {
                case IpxGenParam::ParamInt:
                {
                    IpxGenParam::Int *paramInt = dynamic_cast<IpxGenParam::Int*>(param);
                    if (paramInt)
                    {
                        // print limits
                        std::cout << indent << "Range of '" << paramName << "': " << paramInt->GetMin();
                        std::cout << ", " << paramInt->GetMax() << " inc: " << paramInt->GetIncrement() << std::endl;

                        if (isGet)
                            std::cout << indent << "'" << paramName << "' -> " << paramInt->GetValue() << std::endl;
                        else // set
                        {
                            std::cout << indent << "Enter parameter value: ";
                            std::getline(std::cin, paramValue);
                            auto value = std::stoll(paramValue);
                            std::cout << indent << value << " -> '" << paramName << "' errcode = ";
                            std::cout << (int)paramInt->SetValue(value) << std::endl;
                        }
                        continue;
                    }
                    break;
                }
                case IpxGenParam::ParamFloat:
                {
                    IpxGenParam::Float *paramFloat = dynamic_cast<IpxGenParam::Float*>(param);
                    if (paramFloat)
                    {
                        // print limits
                        std::cout << indent << "Range of '" << paramName << "': " << paramFloat->GetMin();
                        std::cout << ", " << paramFloat->GetMax() << std::endl;

                        if (isGet)
                            std::cout << indent << "'" << paramName << "' -> " << paramFloat->GetValue() << std::endl;
                        else // set
                        {
                            std::cout << indent << "Enter parameter value: ";
                            std::getline(std::cin, paramValue);
                            auto value = std::stod(paramValue);
                            std::cout << indent << value << " -> '" << paramName << "' errcode = ";
                            std::cout << (int)paramFloat->SetValue(value) << std::endl;
                        }
                        continue;
                    }
                    break;
                }
                case IpxGenParam::ParamString:
                {
                    IpxGenParam::String *paramString = dynamic_cast<IpxGenParam::String*>(param);
                    if (paramString)
                    {
                        if (isGet)
                            std::cout << indent << "'" << paramName << "' -> " << paramString->GetValue() << std::endl;
                        else // set
                        {
                            std::cout << indent << "Enter parameter value: ";
                            std::getline(std::cin, paramValue);
                            std::cout << indent << paramValue << " -> '" << paramName << "' errcode = ";
                            std::cout << (int)paramString->SetValue(paramValue.c_str()) << std::endl;
                        }
                        continue;
                    }
                    break;
                }
                case IpxGenParam::ParamEnum:
                {
                    IpxGenParam::Enum *paramEnum = dynamic_cast<IpxGenParam::Enum*>(param);
                    if (paramEnum)
                    {
                        if (isGet)
                            std::cout << indent << "'" << paramName << "' -> " << paramEnum->GetValueStr() << std::endl;
                        else // set
                        {
                            std::cout << indent << "Enter parameter value: ";
                            std::getline(std::cin, paramValue);

                            // try to set as string first
                            auto err = paramEnum->SetValueStr(paramValue.c_str());
                            if (err != IPX_CAM_ERR_OK)
                            {
                                auto value = std::stoll(paramValue);
                                err = paramEnum->SetValue(value);
                                std::cout << indent << value << " -> '" << paramName << "' errcode = ";
                            }
                            else
                                std::cout << indent << paramValue << " -> '" << paramName << "' errcode = ";

                            std::cout << (int)err << std::endl;
                        }
                        continue;
                    }
                    break;
                }
                case IpxGenParam::ParamBoolean:
                {
                    IpxGenParam::Boolean *paramBoolean = dynamic_cast<IpxGenParam::Boolean*>(param);
                    if (paramBoolean)
                    {
                        if (isGet)
                            std::cout << indent << "'" << paramName << "' -> " << paramBoolean->GetValue() << std::endl;
                        else // set
                        {
                            std::cout << indent << "Enter parameter value: ";
                            std::getline(std::cin, paramValue);
                            bool value = false;
                            if (paramValue == "true" || paramValue == "True" || paramValue == "TRUE")
                                value = true;
                            else
                                value = (bool)std::stoll(paramValue);
                            std::cout << indent << value << " -> '" << paramName << "' errcode = ";
                            std::cout << (int)paramBoolean->SetValue(value) << std::endl;
                        }
                        continue;
                    }
                    break;
                }
                case IpxGenParam::ParamCommand:
                {
                    IpxGenParam::Command *paramCommand = dynamic_cast<IpxGenParam::Command*>(param);
                    if (paramCommand)
                    {
                        if (isGet)
                            std::cout << indent << "'" << paramName << "' is done -> " << paramCommand->IsDone() << std::endl;
                        else // set
                        {
                            std::cout << indent << "Enter any value to execute command: ";
                            std::getline(std::cin, paramValue);
                            std::cout << indent << "execute '" << paramName << "' errcode = ";
                            std::cout << (int)paramCommand->Execute() << std::endl;
                        }
                        continue;
                    }
                    break;
                }
                case IpxGenParam::ParamCategory:
                {
                    IpxGenParam::Category *paramCategory = dynamic_cast<IpxGenParam::Category*>(param);
                    if (paramCategory)
                    {
                        // print all the parameter names under the category
                        auto end = paramCategory->GetCount();
                        std::cout << indent << "'" << paramName << "' category has:\n";
                        for (decltype(end) i = 0; i < end; ++i)
                        {
                            auto paramTemp = paramCategory->GetParamByIndex(i, nullptr);
                            if (paramTemp)
                                std::cout << indent << indent << "'" << paramTemp->GetDisplayName() << "'\n";
                        }
                        continue;
                    }
                    break;
                }
                default:
                    std::cout << indent << "Not supported parameter!";
                    continue;
            }

            // if we got here, when an error occured
            std::cout << indent << "An error occured while conversion of '" << paramName << "'!\n";
        }
        catch (const std::exception &e)
        {
            std::cout << indent << "Exception occured: " << e.what();
        }
    }
}

bool AcquireImages( IpxCam::Device *device, IpxCam::Stream *stream )
{
    g_result = false;
    uint64_t images = std::numeric_limits<uint64_t>::max();
    //uint64_t every = 0; // to save images. nothing by default
	uint64_t every = 1;   // save each image

    // determine how many ticks in one second
    // used to calculate FPS
    IpxCamErr err;
    double timestampFreq = 1000000000.0; // second in nanoseconds
    IpxGenParam::Int *tsFreqParam = device->GetCameraParameters()->GetInt("GevTimestampTickFrequency", &err);
    if (tsFreqParam && err == IPX_CAM_ERR_OK)
    {
        timestampFreq = (double)tsFreqParam->GetValue(&err);
        if (err != IPX_CAM_ERR_OK)
            timestampFreq = 1000000000.0;
    }

    // set TLParamsLocked to 1
    auto genParams = device->GetCameraParameters();
    if (genParams && genParams->SetIntegerValue("TLParamsLocked", 1) == IPX_CAM_ERR_OK)
    {
        std::cout << "'TLParamsLocked' set to 1\n";

        // tell the device to start streaming images.
        if (stream->StartAcquisition() == IPX_CAM_ERR_OK)
        {
            std::cout << "Streaming started\n";

            if (genParams->ExecuteCommand("AcquisitionStart") == IPX_CAM_ERR_OK)
            {
                std::cout << "'AcquisitionStart' command sent\n";

                g_result = true;
                uint64_t timestamp = 0, frameId = 0, frameIdPrev = 0, droped = 0, incomplete = 0;
                double bandwidth = .0, fps = .0;
                for (decltype(images) i = 0; i < images; ++i)
                {
                    if (g_isStop)
                        break;

                    auto* buffer = stream->GetBuffer(UINT64_MAX, &err);
                    if (buffer && err == IPX_CAM_ERR_OK)
                    {
                        //auto imagePtr = static_cast<char*>(buffer->GetBufferPtr()) + buffer->GetImageOffset();

                        frameId = buffer->GetFrameID();
                        if (frameIdPrev < frameId)
                            droped += (frameId - frameIdPrev - 1);

                        // timestamp of the first frame
                        if (timestamp)
                        {
                            // FPS and bandwidth as provided by camera
                            // not how many frames have been processed
                            uint64_t diff = timestamp;
                            timestamp = buffer->GetTimestamp();
                            diff = timestamp - diff;

                            fps = diff ? timestampFreq / diff : .0; // camera provides
                            bandwidth = fps * buffer->GetBufferSize() / 1048576.0;
                        }
                        else
                        {
                            timestamp = buffer->GetTimestamp();
                        }

                        frameIdPrev = frameId;
                        if (buffer->IsIncomplete())
                            ++incomplete;

                        std::cout << "OK FID:"  << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << frameId << " "
                            << std::dec << std::setfill(' ') << std::setw(4) << buffer->GetWidth()<< "W "
                            << std::dec << std::setfill(' ') << std::setw(4) << buffer->GetHeight()<< "H "
                            << std::setfill(' ') << std::setw(7) << std::setprecision(2) << std::fixed << fps << "FPS "
                            << std::setfill(' ') << std::setw(7) << bandwidth << "MB/s"
                            << " inc:" << incomplete << " dr:" << droped << "\r" << std::flush;

                        // save each i(th) file
                        if (every && (i % every) == 0)
                        {
                            WriteImage(buffer, i, g_imgFormIdx);
                        }

                        // re-queue the buffer in the stream object
                        stream->QueueBuffer(buffer);
                    }
                    else
                    {
                        // re-queue the buffer in the stream object
                        if (buffer)
                            stream->QueueBuffer(buffer);

                        std::cout << "ERR\r";
                    }
                }

                std::cout << std::endl;

                if (genParams->ExecuteCommand("AcquisitionStop") == IPX_CAM_ERR_OK)
                {
                    std::cout << "'AcquisitionStop' command sent\n";
                }
                else
                {
                    genParams->ExecuteCommand("AcquisitionAbort");
                    std::cout << "'AcquisitionAbort' command sent\n";
                }
            }

            // tell the device to stop streaming images.
            stream->StopAcquisition(1);
            std::cout << "Streaming stoped\n";
        }

        // set TLParamsLocked to 0
        genParams->SetIntegerValue("TLParamsLocked", 0);
        std::cout << "'TLParamsLocked' set to 0\n";
    }

    return g_result;
}

void FreeDataStreamBuffers(IpxCam::Stream *stream, std::vector<IpxCam::Buffer*> *buffers )
{
   // Go through the buffer vector
    std::for_each(std::begin(*buffers), std::end(*buffers),
        [stream](IpxCam::Buffer* buffer){ stream->RevokeBuffer(buffer); }
    );

	//Clear the buffer List
    buffers->clear();
}

const char *GetAccessStatusStr( int32_t status )
{
    switch (status)
    {
    case IpxCam::DeviceInfo::AccessStatusUnknown:
        return "Unknown";
    case IpxCam::DeviceInfo::AccessStatusReadWrite:
        return "ReadWrite";
    case IpxCam::DeviceInfo::AccessStatusReadOnly:
        return "ReadOnly";
    case IpxCam::DeviceInfo::AccessStatusNoAccess:
        return "NoAccess";
    case IpxCam::DeviceInfo::IpSubnetMismatch:
        return "SubnetMismatch";
    default:
        return "Unknown";
    }
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
      tokens.push_back(token);

   return tokens;
}

void WriteImage(IpxCam::Buffer *buff, uint64_t file_idx, int file_ext)
{
	IpxError err =IPX_CAM_ERR_OK;
	
	// Get IpxImage pointer
	IpxImage *img = buff->GetImage();

	// Get extention string
	const char* extention = g_FileExt[file_ext];

	char filename[0x100];
	snprintf(filename, 0x100, "Frame%" PRIu64 "%s", file_idx, extention);
	
	// RAW file saving 
	if(file_ext == 0)
	{
		FILE *fp = fopen(filename, "wb");
		if (fp)
		{
                        auto imagePtr = static_cast<char*>(buff->GetBufferPtr()) + buff->GetImageOffset();
			fwrite(imagePtr, sizeof(char), buff->GetBufferSize(), fp);
			fclose(fp);
		}
		return; // OK
	}
	
	// Check if we have Mono format
	if(img->pixelTypeDescr.pixelType == II_PIX_MONO8)
	{
		err = IpxImageSerializer_Save(g_Serializer, img, filename);
		if (err != IPX_CAM_ERR_OK)
			std::cout << "IpxImageSerializer_Save failed, error code: " << err << std::endl;
		return; // OK
	}
	
	if(img->pixelTypeDescr.pixelType == II_PIX_BAYBG8  || 
		img->pixelTypeDescr.pixelType == II_PIX_BAYRG8 || 
		img->pixelTypeDescr.pixelType == II_PIX_BAYGB8 || 
		img->pixelTypeDescr.pixelType == II_PIX_BAYGR8 )
	{
		// Create RGB image 
		IpxImage *imgRgb = CreateRgbImage(img);
		if (imgRgb)
		{
			// Convert the image to RGB24
			IpxError err = IpxBayer_ConvertImage(g_Bayer, img, imgRgb);
                        if (err != IPX_CAM_ERR_OK)
				std::cout << "IpxBayer_ConvertImage failed, error code: " << err << std::endl;
			
			// Write converted image
			err = IpxImageSerializer_Save(g_Serializer, imgRgb, filename);
			if (err != IPX_CAM_ERR_OK)
				std::cout << "IpxImageSerializer_Save failed, filename: " << filename << " error code: " << err << std::endl;
			
			// Release RGB image
			IpxReleaseImage(&imgRgb);
		}
		else 
			std::cout << "Error, unable to create RGB image"<< std::endl;
		
		return; // OK
	}
				

	// Pixel format not supported
    std::cout << "Error, PixelFormat not supported for specified file type"<< std::endl;
};

IpxImage* CreateRgbImage(IpxImage* inImg)
{
	// Create converted image 
	IpxImage *outImg = nullptr;
	IpxSize imgSize;
	imgSize.height = inImg->height;
	imgSize.width = inImg->width;
	uint32_t pixelType = II_PIX_RGB8;


	// Create the IpxImage object
	IpxCreateImage(&outImg, imgSize, pixelType);

	return outImg;
};
