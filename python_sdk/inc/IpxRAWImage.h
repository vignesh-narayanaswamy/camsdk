/////////////////////////////////////////////////////////////////////
// Imperx Imaging API SDK
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File: IpxRAWImage.h
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IpxRAWImage data type declarations and inline functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Created: 26-March-2015
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2015 -2016, Imperx Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef _IPX_RAW_IMAGE_H
#define _IPX_RAW_IMAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IpxImage.h"

#ifdef INCL_IPXRAW  // EXCLUDED FROM BUILD

//////////////////////////////////////////////////////////////////////
/// \defgroup raw_img IpxRawImage Header
/// \ingroup raw_data
/// \brief Defines, macros, and functions for IpxRawImage
///
/// @{

typedef enum {
//	Undefined  = 0,
	BobcatGEV = 0,
	FLeX	  = 1,
	HD_SDI	  = 2,
	VCE		  = 3
}IpxCameraType;

#define II_CAMERATYPE_COUNT      4


#define MAX_TAPS		255
///Format of taps output
#define TCTO_SEGMENTED	0 /**< Segmented tap output */
#define TCTO_ALTERNATE	1 /**< Alternated tap output. Two interleaved taps per segment */
#define TCTO_INTERLEAVED	2 /**< Interleaved tap output */

///Tap direction
#define TCTD_LEFT_RIGHT	0 /**< Left-Right tap direction */
#define TCTD_RIGHT_LEFT	1 /**< Right-Left tap direction */


typedef struct _IpxRAWImageAttributes
{
	_IpxRAWImageAttributes()	{taps = 1; tapOutput = TCTO_SEGMENTED;
								::memset(tapDirection, TCTD_LEFT_RIGHT, MAX_TAPS);
								cameraType = BobcatGEV;interlaced = false;};
	unsigned char taps; /**< Number of taps per channel (1 to MAX_TAPS) */
	unsigned char tapOutput;	/**< Format of taps output
					*<table>
					*<tr><th>value</th><th>meaning</th></tr>
					*<tr><td>TCTO_SEGMENTED</td><td>Segmented tap output TCTO_SEGMENTED</td></tr>
					*<tr><td>TCTO_ALTERNATE</td><td>Alternated tap output. Two interleaved taps per segment TCTO_ALTERNATE</td></tr>
					*<tr><td>TCTO_INTERLEAVED</td><td>Interleaved tap output TCTO_INTERLEAVED</td></tr>
					*</table>
					*/
	unsigned char tapDirection[MAX_TAPS]; /**< Tap direction:
						  *<table>
						  *<tr><th>value</th><th>meaning</th></tr>
						  *<tr><td>TCTD_LEFT_RIGHT</td><td>Left-Right tap direction TCTD_LEFT_RIGHT</td></tr>
						  *<tr><td>TCTD_RIGHT_LEFT</td><td>Right-Left tap direction TCTD_RIGHT_LEFT</td></tr>
						  *</table>
					      */
	IpxCameraType cameraType;

	bool interlaced;/**<	true - image is interlaced, 
					false(default) - not interlaced
					*/
}IpxRAWImageAttributes;

/// Data structure for description of Imperx RAW Image.
//================================================================================
/**
* \note
* IpxImage stores the image data in a char array. The number of bytes per pixel channel
* is defined by Pixel Type. The field 'imageDataOrigin' is used by IpxImage API for memory management
* and should not be changed by user.
*
* \see IpxImage<BR/>IpxPixelTypeDescr<BR/>II_PIXEL_TYPE_DEFINES<BR/>IpxCreateRAWImage<BR/>IpxCreateRAWImageHeader<BR/>IpxReleaseRAWImage
*/
typedef struct _IpxRAWImage: public IpxImage
{
	_IpxRAWImage()	{nSize = sizeof(_IpxRAWImage);
				version = IPX_IMAGE_VERSION;	
				dataOrder = origin = 0;
				width = height = imageSize = rowSize = imageID = 0; 
				imageData = imageDataOrigin = NULL;
				timestamp = 0; userData = NULL;
				/*taps = 1; tapOutput = TCTO_SEGMENTED; 
				tapDirection[0] = tapDirection[1] = tapDirection[2] = tapDirection[3] = TCTD_LEFT_RIGHT;
				cameraType = BobcatGEV;interlaced = false;*/};
	IpxRAWImageAttributes rawAttrs;
}IpxRAWImage;

IPX_INLINE bool IpxInitRAWImageAttributes(unsigned char taps, unsigned char tapOutput, unsigned char * tapDirection, 
										IpxCameraType cameraType, bool interlaced, IpxRAWImageAttributes* attrs)
{
	::memset(attrs, 0, sizeof(IpxRAWImageAttributes));
	if (taps > MAX_TAPS || sizeof(tapDirection) > MAX_TAPS)
		return false;
	attrs->taps = taps;
	attrs->tapOutput = tapOutput;
	memcpy(attrs->tapDirection, tapDirection, taps);
	attrs->cameraType = cameraType;
	attrs->interlaced = interlaced;
	return true;
}

IPX_INLINE  unsigned long IpxGetRAWRowSize(unsigned long pixType, unsigned long width, unsigned char taps, IpxCameraType cameraType, int * nBlockLength = NULL, int * nBlockSize = NULL);

#define	II_GET_RAW_ROW_SIZE(pixType, width, taps, cameraType)				IpxGetRAWRowSize(pixType, width, taps, cameraType)
#define	II_GET_RAW_IMAGE_SIZE(pixType, width, taps, cameraType, height)		IpxGetRAWImageSize(pixType, width, taps, cameraType, height)

template <int alignment>
inline int align(int value)
{
	return (value + alignment - 1) & (~(alignment - 1)); // If the alignment is a power of two
}

IPX_INLINE  unsigned long IpxGetRAWRowSize(unsigned long pixType, unsigned long width, unsigned char taps, IpxCameraType cameraType, int * nBlockLength/* = NULL*/, int * nBlockSize/* = NULL*/)
{
	
	unsigned long rowSize = 0;
	IpxPixelTypeDescr pixelTypeDescr;
	IpxInitPixelTypeDescr(pixType, &pixelTypeDescr);
	int nBS = pixelTypeDescr.pixSize / 8;
	int nBL = 1;
	if(pixelTypeDescr.pixelType == II_PIX_YUV422_10_PACKED)
	{
		nBS = 8;
		nBL = 3;
		return align<8>(width*8/3);	
	}
	else if(pixelTypeDescr.pixelType == II_PIX_RGB8 && cameraType == BobcatGEV)
	{
		return width * 3;
	}
	if(cameraType == FLeX)
	{
		unsigned long fullWidth = width;
		if(taps == 2)
			fullWidth = align<2>(width);
		switch(pixType)
		{
		case II_PIX_MONO8://1232,3 - 1648
		case II_PIX_MONO10://2464,3 - 3288
		case II_PIX_MONO12://2464, 3 - 3288			
		case II_PIX_MONO14://2464
		case II_PIX_MONO16://2464
			{
				nBS = 4 * (pixelTypeDescr.pixSize / 8);
				nBL = 4;
				if(taps == 3)
				{
					nBS = 4 * (pixelTypeDescr.pixSize / 8);
					nBL = 3;
				}
			}
			rowSize = fullWidth * nBS / nBL;
			break;
		case II_PIX_MONO8_PACKED://1232
			nBS = 24;
			nBL = 24;
			rowSize = (int(fullWidth / nBL))*nBS + align<sizeof(uint64_t /*unsigned __int64*/)>(((fullWidth % nBL)*pixelTypeDescr.depth+7)/8);
			break;
		case II_PIX_MONO10_PACKED://1648, 8 - 1544, (1,2,3) < 4 - 1648
			{
				nBS = 8;
				nBL = 6;
				if(taps == 8)
				{
					nBS = 40;
					nBL = 32;
				}
				else if(taps == 4)
				{
					nBS = 16;
					nBL = 12;
				}
			}
			rowSize = (int(fullWidth / nBL))*nBS + align<sizeof(uint64_t /*unsigned __int64*/)>(((fullWidth % nBL)*pixelTypeDescr.depth+7)/8);
			break;
		case II_PIX_MONO12_PACKED:
			{
				nBS = 24;
				nBL = 16;
				if(taps == 1)
				{
					nBS = 8;
					nBL = 5;
				}
				else if(taps == 3)
				{
					nBS = 24;
					nBL = 15;
				}
			}	
			rowSize = (int(fullWidth / nBL))*nBS + align<sizeof(uint64_t /*unsigned __int64*/)>(((fullWidth % nBL)*pixelTypeDescr.depth+7)/8);
			break;
		case II_PIX_RGB8://4000
		case II_PIX_RGB10://8000
		case II_PIX_RGB12://8000
			nBS = 4 * (pixelTypeDescr.pixSize / 24);
			nBL = 1;
			rowSize = (int(fullWidth / nBL))*nBS /*+ align<sizeof(unsigned __int64)>(((fullWidth % nBL)*pixelTypeDescr.depth+7)/8)*/;
			break;
		case II_PIX_RGB8_PACKED://3000
			nBS = 24;
			nBL = 8;
			rowSize = (int(fullWidth / nBL))*nBS + align<sizeof(uint64_t /*unsigned __int64*/)>(((fullWidth % nBL)*pixelTypeDescr.depth+7)/8);
			break;
		case II_PIX_RGB10_PACKED://4000
			nBS = 8;
			nBL = 2;
			rowSize = (int(fullWidth / nBL))*nBS + align<sizeof(uint64_t /*unsigned __int64*/)>(((fullWidth % nBL)*pixelTypeDescr.depth+7)/8);
			break;
		case II_PIX_RGB12_PACKED://4800
			nBS = 24;
			nBL = 5;
			rowSize = (int(fullWidth / nBL))*nBS + align<sizeof(uint64_t /*unsigned __int64*/)>(((fullWidth % nBL)*pixelTypeDescr.depth+7)/8);
			break;
		default:
			break;
		}
	}
	else  
	{
		//rowSize = II_GET_ROW_SIZE(pixelTypeDescr.pixelType, width);
		rowSize = width * II_GET_PIXEL_BITS_SIZE(pixType);
		rowSize = (rowSize % 8) ? ((rowSize >> 3) + 1) : (rowSize >> 3);
	}
	if(nBlockSize)
		*nBlockSize = nBS;
	if(nBlockLength)
		*nBlockLength = nBL;
	return rowSize;

}

IPX_INLINE  unsigned long IpxGetRAWImageSize(unsigned long pixType, unsigned long width, unsigned char taps, IpxCameraType cameraType, unsigned long height)
{
	unsigned long rowSize = IpxGetRAWRowSize(pixType, width, taps, cameraType);
	unsigned long imageSize = rowSize * height;
	if(cameraType != BobcatGEV)
		imageSize = align<4096>(imageSize);
	return imageSize;
}
///@}
//////////////////////////////////////////////////////////////////////

#endif //  INCL_IPXRAW  - EXCLUDED FROM BUILD

#endif // _IPX_RAW_IMAGE_H
