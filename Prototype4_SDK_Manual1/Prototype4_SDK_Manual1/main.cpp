/*---------------------------------------------------------------------------------------
	Prototipo Manual para calcular área de una herida usando la SDK Intel RealSense
	-------------------------------------------------------------------------------------*/

#include <iostream>
#include <vector>
#include <math.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#include <pxcsensemanager.h>
#include "reaslsenseopencv.h"


using namespace std;

#define PI 3.14159265
#define WIDTH 1280
#define HEIGHT 720


void drawCountour();
int getContourWithAnalisis();
double getAreaFigure(cv::Mat &imageColor, cv::Mat &depthImage, cv::Mat &thresholdImage);

/*------------------------
Variables Globales
--------------------------*/

/*Tamaño de la Imagen a Color*/
cv::Size imageSizeColor(1280, 720);
cv::Size imageSizeDepth(640, 480);

double ka = (4 * tan((71.1 / 2.0)* PI / 180.0) * tan((43.8 / 2.0)* PI / 180.0)) / (WIDTH * HEIGHT);

bool selectObject = false;
bool contourOK = false;
bool takePictureFlag = false;

//cv::Mat image = cv::Mat::zeros(imageSizeColorHD, CV_8UC3);
cv::Point origin;
cv::Point pts;
cv::Point last;

volatile bool saveImages = false;

void mouseTakePicture(int event, int x, int y, int flags, void* data)
{
	if (event == CV_EVENT_LBUTTONUP)
	{
		saveImages = true;
	}
}

void mouseEvent(int event, int x, int y, int flags, void* data)
{
	if (selectObject)
	{
		pts = cv::Point(x, y);
	}
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		origin = cv::Point(x, y);
		pts = cv::Point(x, y);
		last = pts;
		selectObject = true;
		break;
	case CV_EVENT_LBUTTONUP:
		selectObject = false;
		cv::Mat *imgline = (cv::Mat *)data;
		if (pts == last) {
			cv::imwrite("Contour.png", *imgline);
			//cv::putText(image, "Ok!", cv::Point(imageSizeColorHD.width / 2, imageSizeColorHD.height / 2), CV_FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(0, 255, 0), 5);
			contourOK = true;
		}
		else {

			//cv::putText(image, "Fault!", cv::Point(imageSizeColorHD.width / 2, imageSizeColorHD.height / 2), CV_FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(0,255,0), 5);
		}
		*imgline = cv::Mat::zeros(imageSizeColor, CV_8UC3);
		break;
	}


}

int main()
{
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

		/* Reset all properties */
		PXCCapture::Device *device = pp->QueryCaptureManager()->QueryDevice();

		pxcF32 valor = device->QueryDepthFocalLengthMM();

		//device->SetMirrorMode(PXCCapture::Device::MirrorMode::MIRROR_MODE_HORIZONTAL);
		//device->SetIVCAMMotionRangeTradeOff(9);
		//device->SetIVCAMAccuracy(PXCCapture::Device::IVCAM_ACCURACY_COARSE);

		PXCProjection *projection = device->CreateProjection();

		//Crear nombre de ventana
		//cv::namedWindow("Depth Image", CV_WINDOW_FULLSCREEN);
		cv::namedWindow("Imagen a color", CV_WINDOW_AUTOSIZE);

		cv::setMouseCallback("Imagen a color", mouseTakePicture, NULL);

		cout << valor << endl;

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
				convertToOpenCV(img, &imageAlign);
				cv::imshow("Imagen Align", imageAlign);
				img->Release();


				if (cv::waitKey(1) == 27 || saveImages) {
					cv::imwrite("ImageDepth.png", imageAlign);
					cv::imwrite("ImageWound.png", imageColor);

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
	// Clean Up
	pp->Release();

	drawCountour();

	getContourWithAnalisis();

	cv::waitKey(0);

	return 0;
}



void drawCountour() {

	cv::Mat imageColor = cv::imread("ImageWound.png", CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat imageDepth = cv::imread("ImageDepth.png", CV_LOAD_IMAGE_UNCHANGED);

	cv::namedWindow("Imagen herida", CV_WINDOW_FULLSCREEN);

	cv::Mat imgLines = cv::Mat::zeros(imageSizeColor, CV_8UC3);

	cv::setMouseCallback("Imagen herida", mouseEvent, &imgLines);


	while (1)
	{
		cv::waitKey(10);//waits

		if (selectObject)
		{
			cv::line(imgLines, pts, last, cv::Scalar(20, 180, 10), 5, CV_FILLED, 0);

			imageColor = imageColor + imgLines;
			last = pts;
		}
		cv::imshow("Imagen herida", imageColor);

		if (contourOK)
			break;
	}

	cv::destroyWindow("Imagen herida");
}


int getContourWithAnalisis() {

	cv::Mat imageContour = cv::imread("Contour.png", CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat imageColor = cv::imread("ImageWound.png", CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat imageDepth = cv::imread("ImageDepth.png", CV_LOAD_IMAGE_UNCHANGED);

	if (imageContour.empty())
		return -1;

	cv::cvtColor(imageContour, imageContour, CV_BGR2GRAY);

	/*Umbralización*/
	cv::Mat OtsuThresholdImage;
	cv::threshold(imageContour, OtsuThresholdImage, 0, 255, CV_THRESH_BINARY + CV_THRESH_OTSU);

	getAreaFigure(imageColor, imageDepth, OtsuThresholdImage);

	cv::imshow("imageColor", imageColor);

	return 0;
}

double getAreaFigure(cv::Mat &imageColor, cv::Mat &depthImage, cv::Mat &thresholdImage) {

	//Dibujar contornos
	vector<vector<cv::Point> > contours;
	vector<cv::Vec4i> hierarchy;

	cv::findContours(thresholdImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));

	//Encontrar contorno mayor
	int position = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours.size() == contours.max_size())
			position = i;
	}

	//Dibujar contorno
	cv::Scalar color = cv::Scalar(0, 0, 255);
	cv::drawContours(imageColor, contours, position, color, 2, 8, hierarchy, 0);

	//Calculo del área
	double areaWound = 0;
	vector<double> distance;

	for (int i = 0;i < thresholdImage.rows; i++)
	{
		for (int j = 0; j < thresholdImage.cols; j++)
		{
			if (cv::pointPolygonTest(contours[position], cv::Point(i, j), false) > 0)
			{
				if ((double)depthImage.at<uint16_t>(cv::Point(i, j)))
				{
					distance.push_back((double)depthImage.at<uint16_t>(cv::Point(i, j))/10.0);
					areaWound += pow((double)depthImage.at<uint16_t>(cv::Point(i, j)) / 10.0, 2);
				}


			}
		}
	}

	double max = *max_element(distance.begin(), distance.end());
	double min = *min_element(distance.begin(), distance.end());

	areaWound *= ka;

	printf("El area de la herida es: %g\n", areaWound);
	printf("La distancia maxima es: %g\n", max);
	printf("La distancia minima es: %g\n", min);
	printf("Profundidad maxima es: %g\n", max - min);

	return areaWound;
}