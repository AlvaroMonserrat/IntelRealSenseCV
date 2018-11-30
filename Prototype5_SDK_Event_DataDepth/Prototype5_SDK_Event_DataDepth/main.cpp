/*	
			Inicio
				.
				.
Desplegar transmision cámara a color
				.
				.
				.
Click en ventana de Color Stream
				.
				.
				.
Guardar imagen a Color e imagen de profundidad alineada
				.
				.
				.
Desplegar visualización de imagen a color guardada
				.
				.
				.
Rutina: Esperar evento click button mouse -> imprimir distancia en centimetros

*/

#include <iostream>
#include <math.h>
#include <vector>

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#include <pxcsensemanager.h>
#include "reaslsenseopencv.h"

using namespace std;

/*-----------------------------------------------
				Prototype Functions
------------------------------------------------*/
void measurementDepthDistance();
/*-----------------------------------------------
				Variables Globales
------------------------------------------------*/
cv::Size imageSizeColor(1280, 720);
cv::Size imageSizeDepth(640, 480);
cv::Point mousePosition(0, 0);
/*-------------------Flags----------------------*/
volatile bool saveImages = false;
bool clickButton = false;

void mouseTakePicture(int event, int x, int y, int flags, void* data)
{
	if (event == CV_EVENT_LBUTTONUP)
	{
		saveImages = true;
	}
}

void mouseEvent(int event, int x, int y, int flags, void* data)
{
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		if (clickButton == false)
		{
			mousePosition = cv::Point(x, y);
			cout << "Position x: " << x << " Position y: " << y << endl;
			clickButton = true;
		}
	
	}
}

int main() {

	//Crear una instancia 
	PXCSenseManager *pp = PXCSenseManager::CreateInstance();
	if (pp == NULL) {
		printf("Error al crear la instancia\n");
		return -1;
	}

	//Configure the components
	pp->EnableStream(PXCCapture::STREAM_TYPE_COLOR, imageSizeColor.width, imageSizeColor.height);
	pp->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, imageSizeDepth.width, imageSizeDepth.height);


	if (pp->Init() == PXC_STATUS_NO_ERROR) {

		/* set properties */
		PXCCapture::Device *device = pp->QueryCaptureManager()->QueryDevice();

		/*Projection data*/
		PXCProjection *projection = device->CreateProjection();

		//Crear nombre de ventana
		cv::namedWindow("Color Stream", CV_WINDOW_AUTOSIZE);
		cv::setMouseCallback("Color Stream", mouseTakePicture, NULL);

		while (pp->IsConnected() && (pp->AcquireFrame(true) == PXC_STATUS_NO_ERROR))
		{
			//Retorna la muestra de datos disponible, regresa un NULL si no esta disponible
			const PXCCapture::Sample *sample = pp->QuerySample();

			if (sample)
			{
				cv::Mat imageColor;
				convertToOpenCV(sample->color, &imageColor);
				cv::imshow("Color Stream", imageColor);

				cv::Mat imageAlign;
				PXCImage *img = projection->CreateDepthImageMappedToColor(sample->depth, sample->color);
				convertToOpenCV(img, &imageAlign);
				img->Release();

			
				if (cv::waitKey(1) == 27 || saveImages) {
					cv::imwrite("imageWound.png", imageColor);
					cv::imwrite("imageDepth.png", imageAlign);
					cv::destroyAllWindows();
					break;
				}
			}
			pp->ReleaseFrame();
		}
	}
	else {
		printf("La inicializacion ha fallado\n");
	}
	pp->Release();// Clean Up

	if (saveImages)
	{
		measurementDepthDistance();
	}

	return 0;
}

void measurementDepthDistance() {
	//Leer imagenes
	cv::Mat imageColor = cv::imread("imageWound.png", CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat imageDepth = cv::imread("imageDepth.png", CV_LOAD_IMAGE_UNCHANGED);
	if (imageColor.empty() || imageDepth.empty()) {
		printf("Error to read image");
		return;
	}
	string nameWindowColor = "Display Wound";
	cv::namedWindow(nameWindowColor, CV_WINDOW_FULLSCREEN);
	cv::setMouseCallback(nameWindowColor, mouseEvent, nullptr);

	cv::imshow(nameWindowColor, imageColor);

	//Rutina: esperar evento
	while (true)
	{
		cv::waitKey(10);
		if (clickButton == true)
		{
			clickButton = false;
			float distance = (float)imageDepth.at<uint16_t>(mousePosition);
			cout << "Distancia es: " << distance/10.0 << " centimetros" << endl;

		}
	}

}

