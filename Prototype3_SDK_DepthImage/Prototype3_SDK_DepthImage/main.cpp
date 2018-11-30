/*Programa para convertir en formato Matr imagen de profundidad usando SDK RealSense*/

#include <iostream>
#include <vector>
#include <math.h>

#include <pxcsensemanager.h>

#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>

#include "reaslsenseopencv.h"


using namespace std;

/*------------------------
	Variables Globales
--------------------------*/

/*Tamaño de la Imagen a Color*/
cv::Size imageSizeColor(1280, 720);
cv::Size imageSizeDepth(640, 480);

int main()
{
	//Crear una instancia
	PXCSenseManager *pp = PXCSenseManager::CreateInstance();
	if (pp == NULL){
		printf("Error al crear la instancia\n");
		return -1;
	}

	//Configure the components
	pp->EnableStream(PXCCapture::STREAM_TYPE_COLOR, imageSizeColor.width, imageSizeColor.height);
	pp->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, imageSizeDepth.width, imageSizeDepth.height);

	
	if (pp->Init() == PXC_STATUS_NO_ERROR) {

		/* Reset all properties */
		PXCCapture::Device *device = pp->QueryCaptureManager()->QueryDevice();

		//device->SetMirrorMode(PXCCapture::Device::MirrorMode::MIRROR_MODE_HORIZONTAL);
		//device->SetIVCAMMotionRangeTradeOff(9);
		//device->SetIVCAMAccuracy(PXCCapture::Device::IVCAM_ACCURACY_COARSE);

		PXCProjection *projection = device->CreateProjection();
		

		while (pp->IsConnected() && (pp->AcquireFrame(true) == PXC_STATUS_NO_ERROR))
		{
			//Retorna la muestra de datos disponible, regresa un NULL si no esta disponible
			const PXCCapture::Sample *sample = pp->QuerySample();
			
			if (sample)
			{
				
				cv::Mat imageColor;

				convertToOpenCV(sample->color, &imageColor);
				cv::imshow("Imagen a color", imageColor);
				
				
				cv::Mat imageDepth;
				convertToOpenCVTestDepth(sample->depth, &imageDepth);
				cv::imshow("Imagen depth", imageDepth);

				cv::Mat imageAlign;
				PXCImage *img = projection->CreateDepthImageMappedToColor(sample->depth, sample->color);
				convertToOpenCVTestDepth(img, &imageAlign);
				cv::imshow("Imagen Align", imageAlign);
				img->Release();
				
			}

			pp->ReleaseFrame();


			if (cv::waitKey(1) == 27)
				break;
		}


	}
	else {
		printf("La inicializacion ha fallado\n");
	}

	// Clean Up
	pp->Release();

	return 0;
}

