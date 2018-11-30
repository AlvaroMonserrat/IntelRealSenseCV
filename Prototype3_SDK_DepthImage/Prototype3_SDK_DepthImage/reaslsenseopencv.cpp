#include "reaslsenseopencv.h"


void convertToOpenCV(PXCImage *image, cv::Mat *imageOut)
{
	PXCImage::ImageInfo info = image->QueryInfo();
	PXCImage::ImageData data;

	pxcRGB *bufferRGB = 0;
	int bufferSize = info.width * info.height;

	if ((info.format == PXCImage::PixelFormat::PIXEL_FORMAT_RGB24)|| (info.format == PXCImage::PixelFormat::PIXEL_FORMAT_YUY2)) {

		image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB24, &data);

		bufferRGB = new pxcRGB[bufferSize];

		memcpy_s(bufferRGB, bufferSize * sizeof(pxcRGB), (pxcRGB *)data.planes[0], bufferSize * sizeof(pxcRGB));

		image->ReleaseAccess(&data);

		cv::Mat imageOpenCV(cv::Size(info.width, info.height), CV_8UC3, bufferRGB);

		*imageOut = imageOpenCV.clone();

		delete[] bufferRGB;

	}


}

void convertToOpenCVTestDepth(PXCImage *image, cv::Mat *imageOut) {

	PXCImage::ImageInfo info = image->QueryInfo();
	PXCImage::ImageData data;

	unsigned char *charBuffer = 0;
	int bufferSize = info.width * info.height;

	if ((info.format == PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH) || (info.format == PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_F32))
	{
		image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &data);

		charBuffer = new unsigned char[info.width*info.height * 4];

		
		convertTo8bpp((unsigned short*)data.planes[0], info.width*info.height, charBuffer);
		

		image->ReleaseAccess(&data);

		cv::Mat imageOpenCV(cv::Size(info.width, info.height), CV_8UC4, charBuffer);

		*imageOut = imageOpenCV.clone();

		delete[] charBuffer;


	}

}

void convertTo8bpp(unsigned short * pSrc, int iSize, unsigned char * pDst)
{
	float fMaxValue = 1500;
	unsigned char cVal;
	for (int i = 0; i < iSize; i++, pSrc++, pDst += 4)
	{
		cVal = (unsigned char)((*pSrc) / fMaxValue * 255);
		if (cVal != 0)
			cVal = 255 - cVal;

		pDst[0] = cVal;
		pDst[1] = cVal;
		pDst[2] = cVal;
		pDst[3] = 255;
	}
}
