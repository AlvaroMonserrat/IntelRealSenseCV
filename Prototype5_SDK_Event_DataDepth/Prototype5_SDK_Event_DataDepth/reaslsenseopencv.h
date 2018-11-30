#pragma once

#include <pxcsensemanager.h>
#include <opencv2\imgproc\imgproc.hpp>

/*Estructura de un pixel con 24 bits de profundidad*/
struct _pxcRGB24
{
	pxcBYTE pxcRED;
	pxcBYTE pxcGREEN;
	pxcBYTE pxcBLUE;

};

typedef _pxcRGB24 pxcRGB;


struct _pxcZ16
{
	pxcBYTE pxcZ8;
	pxcBYTE pxcZ16;
};

typedef _pxcZ16 pxcZ16;

/*convertir imagen PXCImage en formato Mat*/
void convertToOpenCV(PXCImage *image, cv::Mat *imageOut);

void convertToOpenCVTestDepth(PXCImage *image, cv::Mat *imageOut);

void convertTo8bpp(unsigned short * pSrc, int iSize, unsigned char * pDst);