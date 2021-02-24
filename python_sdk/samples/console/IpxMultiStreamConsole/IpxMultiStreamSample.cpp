////////////////////////////////////////////////////////////////////////////////
// Imperx Camera SDK C++ Sample Code
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Imperx camera video streaming example (Console)
// File: IpxMultiStreamSample.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2015-2018 Imperx Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

//
// Shows how to use IpxCam::Stream class object to acquire images.
//
#include "IpxCameraApi.h"

#include <set>
#include <list>
#include <ctime>
#include <chrono>
#include <cctype>
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <limits>
#include <memory>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

// Macro definitions
////////////////////

// Type Prototypes
//////////////////
struct Camera
{
    Camera(uint32_t camId = std::numeric_limits<uint32_t>::max(),
        IpxCam::Device *d = nullptr, IpxCam::Stream *s = nullptr)
        : id(camId), device(d), stream(s), trigFreq(0.0), isStreaming(false) {}
    ~Camera()
    {
        auto deviceName = device ? device->GetInfo()->GetDisplayName() : "Unknown";
        std::cout << "\nClosing stream and disconnecting device(" << deviceName << ") ....\n";
        if (stream)
        {
            stream->ReleaseBufferQueue();
            stream->Release();
        }

        if (device)
            device->Release();

        std::cout << "Closing stream and disconnecting device(" << deviceName << ") DONE\n";
    }

    Camera(Camera &&other)
        : id(other.id), device(other.device)
        , stream(other.stream), trigFreq(other.trigFreq)
        , isStreaming(other.isStreaming)
    {
        // only these members must be in appropriate state
        // so that destructor works correctly
        other.device = nullptr;
        other.stream = nullptr;
    }

    Camera& operator=(Camera&& other)
    {
        id = other.id;
        device = other.device;
        stream = other.stream;
        trigFreq = other.trigFreq;
        isStreaming = other.isStreaming;

        // only these members must be in appropriate state
        // so that destructor works correctly
        other.device = nullptr;
        other.stream = nullptr;
        return *this;
    }

    uint32_t id;
    IpxCam::Device *device;
    IpxCam::Stream *stream;
    double trigFreq;
    bool isStreaming;

#if __cplusplus >= 201103L
    // it is better to delete them if compiler allows that
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
#else
private:
    Camera(const Camera&);
    Camera& operator=(const Camera&);
#endif
};

struct IPrint
{
    virtual void print(const std::string &str) const = 0;
    virtual void print(uint32_t id, const std::string &str) const = 0;
    virtual void print(uint32_t id, std::time_t timet, uint64_t msec, const std::string &str) const = 0;
};

struct MutexFreeLock
{
    MutexFreeLock() { m_isLocked.clear(); }
    void lock() { while (m_isLocked.test_and_set()); }
    void unlock() { m_isLocked.clear(); }

private:
    std::atomic_flag m_isLocked;
};

// Sync Values
//////////////
bool g_result = false;
std::atomic_bool g_isStop = ATOMIC_VAR_INIT(false);
MutexFreeLock g_printLock;

// IPrint Implementation
////////////////////////
struct MyPrint : IPrint
{
    void print(const std::string &str) const
    {
        g_printLock.lock();
        std::cout <<  str;
        g_printLock.unlock();
    }

    void print(uint32_t id, const std::string &str) const
    {
        g_printLock.lock();
        std::cout << "[" << id << "]: " <<  str;
        g_printLock.unlock();
    }

    void print(uint32_t id, std::time_t timet, uint64_t msec, const std::string &str) const
    {
        g_printLock.lock();
        std::cout << "[" << id << "]:" << std::put_time(std::localtime(&timet), "%H:%M:%S")
            << "." << std::dec << std::left << std::setfill(' ') << std::setw(3) << msec << ": " << str;
        g_printLock.unlock();
    }
};

// Function Prototypes
//////////////////////
std::vector<IpxCam::Interface*> SelectInterfaces( IpxCam::System *system );
std::vector<IpxCam::DeviceInfo*> FindDevices( const std::vector<IpxCam::Interface*> &ifaces );
std::vector<Camera> ConnectDevices( const std::vector<IpxCam::DeviceInfo*> &deviceInfos );
void SwitchOnTriggerMode( const std::vector<Camera> &cameras, IPrint *myPrint );
void StartStreaming( std::vector<Camera> &cameras, IPrint *myPrint );
void StopStreaming( const std::vector<Camera> &cameras, IPrint *myPrint );
void GetSetParams( IpxCam::Device *device );
bool AcquireImages( const Camera &camera, IPrint *myPrint );
std::string GetInterfaceTypeStr( IpxCam::Interface *iface );
std::string GetAccessStatusStr( int32_t status );
std::vector<std::string> split(const std::string& s, char delimiter);
bool SetIpAddress( IpxCam::DeviceInfo* devInfo );
uint32_t ValidateIpAddress( const std::string &ipAddress );

// Main function
////////////////
int main()
{
    // Get System
    auto system = IpxCam::IpxCam_GetSystem();
    if (system)
    {
        while (true)
        {
            // Select desired interface
            auto ifaces = SelectInterfaces(system);
            if (ifaces.size())
            {
                // Find all the devices
                auto deviceInfos = FindDevices(ifaces);
                if (deviceInfos.size())
                {
                    // Connect and Create Streams //
                    ////////////////////////////////

                    MyPrint myPrint; // used for synchronous printing

                    // vector of active(connected) devices
                    std::vector<Camera> cameras = ConnectDevices(deviceInfos);

                    // if there are some cameras connected
                    if (cameras.size())
                    {
                        // set into Trigger Mode if needed
                        SwitchOnTriggerMode(cameras, &myPrint);
                        std::cout << std::endl;

                        // set default values for threads
                        g_isStop = false;
                        std::vector<std::thread> acqThreads;
                        acqThreads.reserve(cameras.size());

                        // start of the streaming
                        // set all the parameters necessary for start of the streaming
                        StartStreaming(cameras, &myPrint);
                        std::cout << std::endl;

                        // start of acquisition threads
                        for (auto &camera: cameras) // std::ref - so not to copy Camera object
                            acqThreads.push_back(std::thread(AcquireImages, std::ref(camera), &myPrint));

                        // wait for stop signal
                        // and stop acquisition threads
                        std::cin.get();
                        g_isStop = true;

                        // stop of the streaming
                        StopStreaming(cameras, &myPrint);

                        // wait while all the threads stoped
                        for (auto &acqThread: acqThreads)
                            acqThread.join();
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

std::vector<IpxCam::Interface*> SelectInterfaces( IpxCam::System *system )
{
    std::vector<IpxCam::Interface*> ifaces;
    const char *indent = "\t";

    //---------- output all the interfaces available ----------//
    std::cout << "\nInterfaces Available:" << std::endl;

    // list has to be released, do this by using of unique pointer
    auto del = [](IpxCam::InterfaceList *l) { l->Release(); };
    std::unique_ptr<IpxCam::InterfaceList, decltype(del)> list(system->GetInterfaceList(), del);
    for (auto iface = list->GetFirst(); iface; iface = list->GetNext())
    {
        ifaces.push_back(iface);
        std::cout << "[" << (ifaces.size() - 1) << "]" << "\t"
            << iface->GetDescription()<< " - " << GetInterfaceTypeStr(iface) << std::endl;
    }

    uint32_t numIfaces = static_cast<uint32_t>(ifaces.size());
    if (numIfaces == 0)
        std::cout << "No interfaces available!\n";
    //---------------------------------------------------------//

    //------------------ ask what user wants ------------------//
    std::cout << "[" << ifaces.size() << "]" << "\t" << "To exit application\n";
    std::cout << "Select Interface Ids or type a for all(default [0]): ";

    // set is used so not to have duplicates
    std::set<uint32_t> selectedIds;
    selectedIds.insert(0); // default

    // analyze data from user
    std::string str;
    std::getline(std::cin, str, '\n');
    if (!str.empty())
    {
        if (str == "a")
            return ifaces;

        selectedIds.clear();
        auto ids = split(str, ' ');
        for (const auto& id: ids)
            selectedIds.insert((uint32_t)std::atoi(id.c_str()));
    }
    //---------------------------------------------------------//

    if (*selectedIds.cbegin() >= numIfaces)
        return std::vector<IpxCam::Interface*>();

    //---------- output all the interfaces selected -----------//
    std::vector<IpxCam::Interface*> result;
    std::cout << "\nInterfaces Selected: " << std::endl;
    for (auto &id: selectedIds)
    {
        if (id >= numIfaces)
            break;

        result.push_back(ifaces[id]);
        std::cout << indent << ifaces[id]->GetDescription() << std::endl;
    }
    //---------------------------------------------------------//

    return result;
}

std::vector<IpxCam::DeviceInfo*> FindDevices( const std::vector<IpxCam::Interface*> &ifaces )
{
    std::vector<IpxCam::DeviceInfo*> devices;
    const char *indent = "\t";

    std::cout << "\nDevices Found:" << std::endl;
    for (auto &iface: ifaces)
    {
        // re-enumerate devices just in case new devices were connected
        iface->ReEnumerateDevices(nullptr, 200);

        // list has to be released, do this by using of unique pointer
        auto del = [](IpxCam::DeviceInfoList *l) { l->Release(); };
        std::unique_ptr<IpxCam::DeviceInfoList, decltype(del)> list(iface->GetDeviceInfoList(), del);
        for (auto device = list->GetFirst(); device; device = list->GetNext())
        {
            devices.push_back(device);
            std::cout << indent << "[" << (devices.size() - 1) << "]" << "\t"
                << device->GetDisplayName() << " - " << device->GetSerialNumber() << std::endl
                << indent << "id=" << device->GetID() << std::endl;
        }
    }

    if (devices.size() == 0)
        std::cout << indent << "No devices have been found!\n";

    return devices;
}

std::vector<Camera> ConnectDevices( const std::vector<IpxCam::DeviceInfo*> &deviceInfos )
{
    // vector of connected devices
    std::vector<Camera> cameras;
    cameras.reserve(deviceInfos.size());

    // Connect to the device and insert it to devices vector
    auto end = deviceInfos.size();
    for (auto i = (decltype(end))0; i < end; ++i)
    {
        auto deviceName = deviceInfos[i]->GetDisplayName();
        std::cout << "\nConnecting to and creating stream on " << deviceName << " ....\n";

        // check if Ip Address can be adjusted
        if (deviceInfos[i]->GetAccessStatus() == IpxCam::DeviceInfo::IpSubnetMismatch)
        {
            std::cout << "Cannot connect due to Ip Subnet Mismatch error\n";
            SetIpAddress(deviceInfos[i]);
        }

        // if access status is still wrong
        if (deviceInfos[i]->GetAccessStatus() != IpxCam::DeviceInfo::AccessStatusReadWrite)
        {
            std::cout << "Cannot connect due to wrong AccessStatus("
                << GetAccessStatusStr(deviceInfos[i]->GetAccessStatus()) << ")\n";
            std::cout << "Connecting to and creating stream on " << deviceName << " FAIL\n";
            continue;
        }

        auto device = IpxCam_CreateDevice(deviceInfos[i]);
        if (device)
        {
            IpxCam::Stream *stream = nullptr;
            if (device->GetNumStreams() >= 1 && (stream = device->GetStreamByIndex(0)))
            {
                std::cout << "Connecting to and creating stream on " << deviceName << " DONE\n";

                //-------------------- create buffers --------------------//
                auto bufSize = stream->GetBufferSize();
                auto minNumBuffers = stream->GetMinNumBuffers();
                for (auto i = (decltype(minNumBuffers))0; i < minNumBuffers; ++i)
                    stream->CreateBuffer(bufSize, nullptr, nullptr);
                //--------------------------------------------------------//
            }
            else
                std::cout << "Unable to create a stream on" << deviceName << std::endl;

            // add device to the vector
            cameras.emplace_back((uint32_t)i, device, stream);
        }
        else
            std::cout << "Unable to connect to" << deviceName << std::endl;
    }

    return cameras;
}

void SwitchOnTriggerMode( const std::vector<Camera> &cameras, IPrint *myPrint )
{
    // ask if user wants to switch cameras into trigger mode
    std::cout << "\nWould you like to set cameras into trigger mode?";
    std::cout << "\nPress enter to leave as is or type [yn]: ";
    std::string str;
    std::getline(std::cin, str, '\n');
    if (!str.empty())
    {
        auto ans = str.c_str()[0];
        if (ans == 'y' || ans == 'Y')
        {
            // assuming there is at least one camera
            IpxCamErr err;
            uint64_t trSrcValue = 0;
            auto genParams = cameras[0].device->GetCameraParameters();
            if (genParams)
            {
                auto trSrc = genParams->GetEnum("TriggerSource", &err);
                if (trSrc && err == IPX_CAM_ERR_OK)
                {
                    uint32_t sel = 0;
                    auto end = trSrc->GetEnumEntriesCount();
                    for (auto i = (decltype(end))0; i < end; ++i)
                    {
                        auto entry = trSrc->GetEnumEntryByIndex(i);
                        std::cout << "\t" << i << ": " << entry->GetValueStr() <<  std::endl;
                    }
                    std::cout << "What trigger source do you want(default [0]): ";
                    std::getline(std::cin, str, '\n');
                    if (!str.empty())
                        sel = (uint32_t)std::atoi(str.c_str());

                    // if index is correct get value of entry with index 'sel'
                    if ( sel < end)
                        trSrcValue = trSrc->GetEnumEntryByIndex(sel)->GetValue();
                    else // get current value
                        trSrcValue = trSrc->GetValue();
                }
            }

            for (auto &camera: cameras)
            {
                auto genParams = camera.device->GetCameraParameters();
                if (!genParams)
                {
                    myPrint->print(camera.id, "Cannot get camera parameters!\n");
                    continue;
                }

                // start Pulse Generator first
                if (genParams->SetEnumValue("PulseGenGranularity", 0) != IPX_CAM_ERR_OK ||
                    genParams->SetIntegerValue("PulseGenWidth", 100000) != IPX_CAM_ERR_OK ||
                    genParams->SetIntegerValue("PulseGenPeriod", 1000000) != IPX_CAM_ERR_OK ||
                    genParams->SetEnumValueStr("PulseGenMode", "Continuous") != IPX_CAM_ERR_OK ||
                    genParams->SetBooleanValue("PulseGenEnable", true) != IPX_CAM_ERR_OK)
                {
                    myPrint->print(camera.id, "Cannot start Pulse Generator!\n");
                    continue;
                }

                // start Out1 and Out2 to Pulse Generator
                if (genParams->SetEnumValueStr("OUT1Selector", "PulseGenerator") != IPX_CAM_ERR_OK ||
                    genParams->SetEnumValueStr("OUT2Selector", "PulseGenerator") != IPX_CAM_ERR_OK)
                {
                    myPrint->print(camera.id, "Cannot set 'OUT1Selector' or 'OUT2Selector'!\n");
                    continue;
                }

                // switch to Trigger Mode
                if (genParams->SetEnumValue("TriggerSource", trSrcValue) != IPX_CAM_ERR_OK ||
                    genParams->SetEnumValueStr("TriggerMode", "On") != IPX_CAM_ERR_OK)
                {
                    myPrint->print(camera.id, "Cannot set 'TriggerSource' or 'TriggerMode'!\n");
                    continue;
                }
            }
        }
        else
        {
            for (auto &camera: cameras)
            {
                auto genParams = camera.device->GetCameraParameters();
                if (!genParams)
                {
                    myPrint->print(camera.id, "Cannot get camera parameters!\n");
                    continue;
                }

                // switch to Freerun Mode
                genParams->SetEnumValueStr("TriggerMode", "Off");
                genParams->SetBooleanValue("PulseGenEnable", false);
            }
        }
    }
}

void StartStreaming( std::vector<Camera> &cameras, IPrint *myPrint )
{
    for (auto &camera: cameras)
    {
        auto genParams = camera.device->GetCameraParameters();
        if (!genParams)
        {
            myPrint->print(camera.id, "Cannot get camera parameters!\n");
            continue;
        }

        // calculate frequency of trigger pulses if any
        auto valStr = genParams->GetEnumValueStr("TriggerMode");
        if (valStr && std::string("On") == valStr)
        {
            IpxCamErr err;

            // first of all find granularity of period value
            camera.trigFreq = 1.0;
            valStr = genParams->GetEnumValueStr("PulseGenGranularity", &err);
            if (valStr && err == IpxCamErr::IPX_CAM_ERR_OK)
            {
                // find first digit
                while (*valStr != '\0' && !std::isdigit(*valStr))
                    ++valStr;

                if (*valStr != '\0')
                    camera.trigFreq = std::atoi(valStr);
            }

            // lets find period
            auto period = genParams->GetIntegerValue("PulseGenPeriod", &err);
            camera.trigFreq = err == IpxCamErr::IPX_CAM_ERR_OK ? 1000000.0 / (camera.trigFreq * period) : 1.0;

            myPrint->print(camera.id, "Pulse Generator is enabled with " + std::to_string(camera.trigFreq) + "Hz pulses\n");
            valStr = genParams->GetEnumValueStr("TriggerSource");
            if (valStr)
                myPrint->print(camera.id, "Trigger Source is '" + std::string(valStr) + "'\n");
            else
                myPrint->print(camera.id, "Trigger Source is 'Unknown'\n");
        }

        if (genParams->SetIntegerValue("TLParamsLocked", 1) != IPX_CAM_ERR_OK)
        {
            myPrint->print(camera.id, "Cannot start streaming!\n");
            continue;
        }

        if (camera.stream->StartAcquisition() != IPX_CAM_ERR_OK)
        {
            genParams->SetIntegerValue("TLParamsLocked", 0);
            myPrint->print(camera.id, "Cannot start streaming!\n");
            continue;
        }

        if (genParams->ExecuteCommand("AcquisitionStart") != IPX_CAM_ERR_OK)
        {
            camera.stream->StopAcquisition();
            genParams->SetIntegerValue("TLParamsLocked", 0);
            myPrint->print(camera.id, "Cannot start streaming!\n");
            continue;
        }

        camera.isStreaming = true;
        myPrint->print(camera.id, "Streaming started\n");
    }
}

void StopStreaming( const std::vector<Camera> &cameras, IPrint *myPrint )
{
    for (auto &camera: cameras)
    {
        if (!camera.isStreaming)
            continue;

        auto genParams = camera.device->GetCameraParameters();
        if (genParams)
        {
            if (genParams->ExecuteCommand("AcquisitionStop") != IPX_CAM_ERR_OK)
                genParams->ExecuteCommand("AcquisitionAbort");

            camera.stream->StopAcquisition();

            // just in case cancel I/O for current buffer so not to wait it infinitely
            camera.stream->CancelBuffer();

            genParams->SetIntegerValue("TLParamsLocked", 0);
            myPrint->print(camera.id, "Streaming stoped\n");
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

bool AcquireImages( const Camera &camera, IPrint *myPrint )
{
    auto timePoint = std::chrono::system_clock::now();
    auto id = camera.id;
    myPrint->print(id, "Acquisition thread has started\n");

    g_result = false;
    auto device = camera.device;
    auto stream = camera.stream;
    uint64_t images = std::numeric_limits<uint64_t>::max();

    // determine how many ticks in one second
    // used to calculate FPS
    IpxCamErr err;
    double timestampFreq = 1000000000.0; // second in nanoseconds
    auto genParams = device->GetCameraParameters();
    IpxGenParam::Int *tsFreqParam = genParams->GetInt("GevTimestampTickFrequency", &err);
    if (tsFreqParam && err == IPX_CAM_ERR_OK)
    {
        timestampFreq = (double)tsFreqParam->GetValue(&err);
        if (err != IPX_CAM_ERR_OK)
            timestampFreq = 1000000000.0;
    }

    // check if camera is in Trigger Mode
    auto valStr = genParams->GetEnumValueStr("TriggerMode");
    bool isInTrigger = valStr && std::string("On") == valStr;

    // calculate how many ticks should be between consecutive triggers
    uint64_t ticksTrigFreq = isInTrigger ? (uint64_t)(timestampFreq * camera.trigFreq) : 0;
    ticksTrigFreq += (ticksTrigFreq >> 1); // increase by half

    // results
    // just to have statistics and dump it at the end
    std::list<uint64_t> drops;
    std::list<uint64_t> trigLosts;
    std::list<uint64_t> incompletes;

    // real acquisition
    g_result = true;
    uint64_t timestamp = 0, frameId = 0, frameIdPrev = 0, droped = 0, diff = 0, incomp = 0;
    double bandwidth = .0, fps = .0;
    for (decltype(images) i = 0; i < images; ++i)
    {
        if (g_isStop)
            break;

        auto buffer = stream->GetBuffer(UINT64_MAX, &err);
        if (buffer && err == IPX_CAM_ERR_OK)
        {
            //auto imagePtr = static_cast<char*>(buffer->GetBufferPtr()) + buffer->GetImageOffset();

            frameId = buffer->GetFrameID();
            if (frameIdPrev < frameId)
            {
                diff = frameId - frameIdPrev - 1;
                if (diff)
                {
                    droped += diff;
                    drops.push_back(frameId);
                }
            }

            std::string trigLost;

            // timestamp of the first frame
            if (timestamp)
            {
                // FPS and bandwidth as provided by camera
                // not how many frames have been processed
                diff = timestamp;
                timestamp = buffer->GetTimestamp();
                diff = timestamp - diff;

                fps = diff ? timestampFreq / diff : .0; // camera provides
                // (buffer->GetBufferSize() << 3) - in bits
                bandwidth = fps * (buffer->GetBufferSize() << 3) / 1048576.0; // 1073741824

                if (isInTrigger && diff > ticksTrigFreq)
                {
                    trigLost = " ER";
                    trigLosts.push_back(frameId);
                }
            }
            else
            {
                timestamp = buffer->GetTimestamp();
            }

            if (buffer->IsIncomplete())
            {
                ++incomp;
                incompletes.push_back(frameId);
            }

            frameIdPrev = frameId;

            auto curTimePoint = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(curTimePoint - timePoint).count();
            timePoint = curTimePoint;

            std::ostringstream buf;
            buf << "FID:"  << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << (frameId&0xFFFFFFFF) << " "
                << std::dec << std::setfill(' ') << std::setw(4) << buffer->GetWidth()<< "W "
                << std::dec << std::setfill(' ') << std::setw(4) << buffer->GetHeight()<< "H "
                << std::setfill(' ') << std::setw(7) << std::setprecision(2) << std::fixed << fps << "FPS "
                << std::setfill(' ') << std::setw(7) << bandwidth << "MiB/s"
                << " inc:" << incomp << " dr:" << droped << " dur:" << duration
                << trigLost << "\n" << std::flush;

            myPrint->print(id, std::chrono::system_clock::to_time_t(timePoint),
                std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count() % 1000,
                buf.str());

            // re-queue the buffer in the stream object
            stream->QueueBuffer(buffer);
        }
        else
        {
            // re-queue the buffer in the stream object
            if (buffer)
                stream->QueueBuffer(buffer);

            std::ostringstream buf;
            buf << "ERR = " << err << " while receiving buffer\n";
            myPrint->print(id, buf.str());
        }
    }

    // dump statistics here
    for (auto& drop: drops)
    {
        std::ostringstream buf;
        buf << "---DROPED FRAMES AFTER: " << std::uppercase << std::hex
            << std::setfill('0') << std::setw(16) << drop << std::endl;
        myPrint->print(id, buf.str());
    }
    for (auto& trigLost: trigLosts)
    {
        std::ostringstream buf;
        buf << "---LOOKS LIKE TRIGGER PULSE LOST/IGNORED BEFORE: " << std::uppercase
            << std::hex << std::setfill('0') << std::setw(16) << trigLost << std::endl;
        myPrint->print(id, buf.str());
    }
    for (auto& incomplete: incompletes)
    {
        std::ostringstream buf;
        buf << "---INCOMPLETE FRAME ID: " << std::uppercase
            << std::hex << std::setfill('0') << std::setw(16) << incomplete << std::endl;
        myPrint->print(id, buf.str());
    }

    myPrint->print(id, "Acquisition thread has stoped\n");

    return g_result;
}

std::string GetInterfaceTypeStr( IpxCam::Interface *iface )
{
    if (iface)
    {
        switch (iface->GetType())
        {
        case IpxCam::USB3Vision:
            return "USB3Vision";
        case IpxCam::GigEVision:
            return "GigEVision";
        case IpxCam::CameraLink:
            return "CameraLink";
        case IpxCam::CoaxPress:
            return "CoaxPress";
        case IpxCam::HdSdi:
            return "HdSdi";
        case IpxCam::AllInterfaces:
            return "AllInterfaces";
        default:
            return "Unknown";
        }
    }

    return "Unknown";
}

std::string GetAccessStatusStr( int32_t status )
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

bool SetIpAddress( IpxCam::DeviceInfo* devInfo )
{
    std::cout << "Would you like to set camera Ip Address [Ny]: ";
    std::string str;
    std::getline(std::cin, str, '\n');
    if (!str.empty())
    {
        auto ans = str.c_str()[0];
        if (ans == 'y' || ans == 'Y')
        {
            auto iface = devInfo->GetInterface();
            auto ifaceParams = iface->GetParameters();
            if (!ifaceParams)
                return false;

			IpxCamErr err;
			std::string description;
			auto ipAddress = ifaceParams->GetIntegerValue("GevInterfaceSubnetIPAddress", &err);
			if (err == IpxCamErr::IPX_CAM_ERR_OK)
			{
				description =  std::to_string(ipAddress >> 24 & 0xFF) + ".";
				description += std::to_string(ipAddress >> 16 & 0xFF) + ".";
				description += std::to_string(ipAddress >> 8  & 0xFF) + ".";
				description += std::to_string(ipAddress       & 0xFF);
			}
			else
			{
				description = iface->GetDescription();
			}

            // ask and validate Ip Address
			std::cout << "Enter Ip Address (Iface:" << description << ")[x.x.x.x]: ";
            std::string str;
            std::getline(std::cin, str, '\n');
            auto ipAdr = ValidateIpAddress(str);
            if (!ipAdr)
                return false;

            // set the same SubnetMask as the interface has
            auto subMask = ifaceParams->GetIntegerValue("GevInterfaceSubnetMask", &err);
            if (err != IpxCamErr::IPX_CAM_ERR_OK)
                return false;

			if (devInfo->ForceIP((uint32_t)ipAdr, (uint32_t)subMask, 0) != IpxCamErr::IPX_CAM_ERR_OK)
				return false;

            return true;
        }
    }

    return false;
}

uint32_t ValidateIpAddress( const std::string &ipAddress )
{
    if (ipAddress.empty())
        return 0;

    auto quadro = split(ipAddress, '.');
    if (quadro.size() != 4)
        return 0;

    try
    {
        return ((uint8_t)std::stoi(quadro[0])) << 24 | ((uint8_t)std::stoi(quadro[1])) << 16
             | ((uint8_t)std::stoi(quadro[2])) << 8  | ((uint8_t)std::stoi(quadro[3]));
    }
    catch (std::exception &) // to catch std::stoi exceptions
    {
    }

    return 0;
}
