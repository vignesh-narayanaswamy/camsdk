
#ifndef _IPX_IMG_PROCESSOR_H_
#define _IPX_IMG_PROCESSOR_H_

#include "IpxImage.h"
#include "IpxToolsBase.h"

enum IIDataLocation
{
	CpuMemory=0,
	OpenGlFrameBuffer,
	CudaMemory,
	OpenCLBuffer
//	// etc...
};

/**
    IpxImgProcessor
    @brief Pure virtual base class for image processor
*/
class IpxImgProcessor {
public:

    virtual ~IpxImgProcessor(){}
  
    virtual IpxError            Init(IpxImage* sourceImageFormat, IpxImage* outputImageFormat) { return true; }

    virtual IpxError            ProcessImage(IpxImage *sourceImage, IpxImage *outputImage, void *data) = 0;   
    
    virtual void                Release() = 0;

    virtual IpxComponent*       GetComponent() = 0;

    virtual IpxImage*           Output() = 0;

    virtual IpxImage*           Format(bool _sourceFormat) { return 0; }

    virtual bool                Enable(bool _enabled) { return true; }

    virtual bool                IsEnabled() { return true; }
};

class IpxImgProcessingChain {
public:

    static IpxImgProcessingChain*   CreateComponent();

    static void                     DeleteComponent(IpxImgProcessingChain* component);

public:
    virtual ~IpxImgProcessingChain(){}

    virtual IpxImgProcessor*        GetProcessor(const char* procName) = 0;

    virtual IpxImgProcessor*        GetProcessor(int32_t index) = 0;

    virtual IpxError                AddProcessor(const char* procName, IpxImgProcessor* processor) = 0;

    virtual IpxError                InsertProcessor(char* insertBefore, const char* procName, IpxImgProcessor* processor) = 0;

    virtual IpxError                DeleteProcessor(const char* procName) = 0;

    virtual IpxComponent*           GetComponent() = 0;

    virtual IpxError                Init(IpxImage* sourceImageFormat, IpxImage* outputImageFormat) = 0; 

    virtual IpxError                ProcessImage(IpxImage *src, IpxImage *out, void *data) = 0;

    virtual void                    Release() = 0;

    virtual IpxImage*               Output() = 0;

    virtual IpxImage*               Format(bool _sourceFormat) = 0;
};

class IpxImgProcChainBuilder {
public:

    static IpxImgProcChainBuilder*  CreateComponent();

    static void                     DeleteComponent(IpxImgProcChainBuilder* component);

public:
    virtual ~IpxImgProcChainBuilder(){}

    virtual IpxComponent*           GetComponent() = 0;

    virtual IpxImgProcessingChain*  StartChain(IpxImage* imgIn);

    virtual IpxError                FinishChain(IpxImgProcessingChain* chain);

	virtual IpxError                AddProcessor(IpxImgProcessor* processor); 

	virtual IpxError                InsertProcessor(IpxImgProcessor* insertBefore, IpxImgProcessor* processor); 
};

#endif // _IPX_IMG_PROCESSOR_H_
