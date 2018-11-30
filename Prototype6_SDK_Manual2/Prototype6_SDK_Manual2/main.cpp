
#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <Windows.h>

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#include <pxcsensemanager.h>

#include "reaslsenseopencv.h"
#include "tinyxml.h"

using namespace std;

/*---------------Macros----------------*/
#define PI 3.14159265
#define WIDTH 1280
#define HEIGHT 720
/*-----------------------------------------------
			Prototype Functions
------------------------------------------------*/
void drawContourWound();
void getAreaFigure(cv::Mat &imageColor, cv::Mat &depthImage, cv::Mat &thresholdImage);
bool write_simple_doc(double areaTotal, double areaRed = 0, double areaYellow = 0, double areaBlack = 0);
void CreateFolder(const char * path);
/*-----------------------------------------------
			Variables Globales
------------------------------------------------*/
cv::Size imageSizeColor(1280, 720);
cv::Size imageSizeDepth(640, 480);
cv::Point mousePosition(0, 0);
cv::Point inicialPosition;
cv::Point currentPoint;
cv::Point lastPoint;
cv::Mat imagenOriginal;
int numberOfMovements = 0;
double ka = 1;

/*-------------------Flags----------------------*/
volatile bool saveImages = false;
bool selectObject = false;
bool resetImage = false;
bool closeDrawing = false;

void mouseTakePicture(int event, int x, int y, int flags, void* data)
{
	if (event == CV_EVENT_LBUTTONUP)
	{
		saveImages = true;
	}
}

void mouseEventDraw(int event, int x, int y, int flags, void* data)
{
	cv::Mat *imgline = (cv::Mat *)data;
	if (selectObject)
	{
		currentPoint = cv::Point(x, y);
		numberOfMovements++;
	}
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		selectObject = true;
		resetImage = false;
		lastPoint = currentPoint = inicialPosition = cv::Point(x, y);
		numberOfMovements = 0;
		break;
	case CV_EVENT_LBUTTONUP:
		selectObject = false;
		if (numberOfMovements > 100)
		{
			cv::imwrite("Contour.png", *imgline);
			
		}
		else {
			*imgline = cv::Mat::zeros(imageSizeColor, CV_8UC3);
			resetImage = true;
		}
		break;
	}

	if (event == CV_EVENT_RBUTTONDOWN)
	{
		closeDrawing = true;
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
		
		/*Constante*/
		ka = (4 * tan(((device->QueryColorFieldOfView().x) / 2.0)* PI / 180.0) * tan(((device->QueryColorFieldOfView().y) / 2.0)* PI / 180.0)) / (WIDTH * HEIGHT);

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

				cv::Mat imageDepth;
				convertToOpenCVTestDepth(sample->depth, &imageDepth);
				cv::imshow("Death Stream", imageDepth);

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
		drawContourWound();
	}

	return 0;
}


void drawContourWound() {

	//Leer imagenes
	cv::Mat imageColor = cv::imread("imageWound.png", CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat imageDepth = cv::imread("imageDepth.png", CV_LOAD_IMAGE_UNCHANGED);
	if (imageColor.empty() || imageDepth.empty()) {
		printf("Error to read image");
		return;
	}
	string nameWindowColor = "Display Wound";
	cv::namedWindow(nameWindowColor, CV_WINDOW_FULLSCREEN);
	
	imagenOriginal = imageColor.clone();

	cv::Mat imgContour = cv::Mat::zeros(imageSizeColor, CV_8UC3);
	cv::setMouseCallback(nameWindowColor, mouseEventDraw, &imgContour);
	//Rutina: esperar evento
	while (!closeDrawing)
	{
		cv::waitKey(10);
		if (selectObject)
		{
			cv::line(imgContour, currentPoint, lastPoint, cv::Scalar(21, 255, 0), 6, CV_FILLED, 0);
			lastPoint = currentPoint;
			imageColor = imgContour + imageColor;
		}

		if (resetImage)
		{
			imageColor = imagenOriginal.clone();
		}

		cv::imshow(nameWindowColor, imageColor);
	}
	
	cv::destroyAllWindows();

	if (closeDrawing)
	{
		cv::Mat imageContour = cv::imread("Contour.png", CV_LOAD_IMAGE_UNCHANGED);
		if (imageContour.empty())
			return;

		/*Umbralización*/
		cv::cvtColor(imageContour, imageContour, CV_BGR2GRAY);
		cv::Mat OtsuThresholdImage;
		cv::threshold(imageContour, OtsuThresholdImage, 0, 255, CV_THRESH_BINARY + CV_THRESH_OTSU);

		getAreaFigure(imageColor, imageDepth, imageContour);

		cv::imshow(nameWindowColor, imageColor);
		//cv::imshow("Contour", imageContour);
		cv::waitKey(0);
	}

}

void getAreaFigure(cv::Mat &imageColor, cv::Mat &depthImage, cv::Mat &thresholdImage) {

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
	cv::drawContours(thresholdImage, contours, position, cv::Scalar(255), CV_FILLED, 8, hierarchy, 0);

	double amount = 0;
	double areaWound = 0;
	for (int i = 0;i < thresholdImage.rows; i++)
	{
		for (int j = 0; j < thresholdImage.cols; j++)
		{
			if (thresholdImage.at<unsigned char>(cv::Point(j,i)) == 255)
			{
				areaWound += pow((double)depthImage.at<uint16_t>(cv::Point(j, i)) / 10.0, 2);
				amount++;
			}
		}
	}

	areaWound *= ka;
	write_simple_doc(areaWound);

}

bool write_simple_doc(double areaTotal, double areaRed, double areaYellow, double areaBlack) {



	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);

	TiXmlElement *analisis = new TiXmlElement("Analisis");
	doc.LinkEndChild(analisis);

	TiXmlElement *wound = new TiXmlElement("Herida");
	analisis->LinkEndChild(wound);

	/*Area Total*/
	TiXmlElement *at = new TiXmlElement("at"); //Área Total
	wound->LinkEndChild(at);

	//
	string area_total = to_string(areaTotal);
	const char *pTotal;
	pTotal = area_total.c_str();

	TiXmlText *text_at = new TiXmlText(pTotal);  //Área Herida total
	at->LinkEndChild(text_at);

	/*Area rojo*/
	TiXmlElement *ar = new TiXmlElement("ar"); //Área rojo
	wound->LinkEndChild(ar);

	//
	string area_rojo = to_string(areaRed);

	TiXmlText *text_ar = new TiXmlText(area_rojo.c_str());  //Área Herida total rojo
	ar->LinkEndChild(text_ar);


	/*Area amarillo*/
	TiXmlElement *aa = new TiXmlElement("aa"); //Área amarillo
	wound->LinkEndChild(aa);

	//
	string area_amarrillo = to_string(areaYellow);

	TiXmlText *text_aa = new TiXmlText(area_amarrillo.c_str());  //Área Herida total amarillo
	aa->LinkEndChild(text_aa);

	/*Area negro*/
	TiXmlElement *an = new TiXmlElement("an"); //Área negro
	wound->LinkEndChild(an);

	//
	string area_negro = to_string(areaBlack);

	TiXmlText *text_an = new TiXmlText(area_negro.c_str());  //Área Herida total negro
	an->LinkEndChild(text_an);

	/*Porcentaje rojo*/
	TiXmlElement *pr = new TiXmlElement("pr"); //Porcentaje rojo
	wound->LinkEndChild(pr);

	TiXmlText *text_pr = new TiXmlText("40 %");  //Porcentaje rojo
	pr->LinkEndChild(text_pr);

	/*Porcentaje Amarillo*/
	TiXmlElement *pa = new TiXmlElement("pa"); //Porcentaje Amarillo
	wound->LinkEndChild(pa);

	TiXmlText *text_pa = new TiXmlText("30 %");  //Porcentaje Amarillo
	pa->LinkEndChild(text_pa);

	/*Porcentaje Negro*/
	TiXmlElement *pm = new TiXmlElement("pm"); //Porcentaje Negro (pa)
	wound->LinkEndChild(pm);

	TiXmlText *text_pm = new TiXmlText("30 %");  //Porcentaje Negro
	pm->LinkEndChild(text_pm);

	CreateFolder("C:\\Image_Nurseye\\");

	if (doc.SaveFile("C:\\Image_Nurseye\\wound.xml"))
	{
		return true;
	}
	return false;

}

void CreateFolder(const char * path)
{
	if (!CreateDirectory(path, NULL))
	{
		return;
	}
}
