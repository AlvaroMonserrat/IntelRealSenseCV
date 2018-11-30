#include <iostream>
#include <vector>
#include <math.h>
#include <Windows.h>

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>


#include <rs.hpp>

#define PI 3.14159265
#define WIDTH 640
#define HEIGHT 480

using namespace std;

/*------------------------------------------
Prototype Functions
--------------------------------------------*/
int initializeCamaraSR300(int index);//void alignImageDepth(cv::Mat &imageDepth); //Alinar Imagen de profundidad con imagen a colo
int captureContourAndImageData();
int getContourWithAnalisis();
double getDistance(cv::Point centerPoint, cv::Mat &imageAlignDepth, double scale);	//obtener distancia
double getAreaFigure(cv::Mat &imageColor, cv::Mat &depthImage, cv::Mat &thresholdImage);
void drawCountour();
/*------------------------------------------
Variables Globales
--------------------------------------------*/

//Dimensiones de la imagen de profundidad
cv::Size imageSize(640, 480);
//cv::Size imageSizeColor(1920, 1080);
cv::Size imageSizeColorHD(1920, 1080);

//Objetos Camara
rs::context ctx;
rs::device *sr300Device = NULL;

double scalar = 0;
double ka = (4 * tan((71.5 / 2.0)* PI / 180.0) * tan((55.0 / 2.0)* PI / 180.0)) / (WIDTH * HEIGHT);

bool selectObject = false;
bool contourOK = false;
bool takePictureFlag = false;

//cv::Mat image = cv::Mat::zeros(imageSizeColorHD, CV_8UC3);
cv::Point origin;
cv::Point pts;
cv::Point last;



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
		*imgline = cv::Mat::zeros(imageSize, CV_8UC3);
		break;
	}


}

void mouseTakePicture(int event, int x, int y, int flags, void* data)
{
	if (event == CV_EVENT_LBUTTONUP)
	{
		takePictureFlag = true;
	}
}


int main()
{
	//Capturar imagenes
	if (captureContourAndImageData())
		return -1;

	sr300Device->stop();

	drawCountour();
	//Obtener área
	if (getContourWithAnalisis())
		return -2;

	cv::waitKey(0);

	return 0;
}

int initializeCamaraSR300(int index) {

	//Verificar dispositivo(s) conectados
	if (!ctx.get_device_count()) { return -1; }

	//Crear dispositivo
	sr300Device = ctx.get_device(index);

	//Verificar obtención de la cámara
	if (sr300Device == NULL) { return -1; };

	//Factor scalar
	scalar = sr300Device->get_depth_scale();

	//Habilitar transmision
	sr300Device->enable_stream(rs::stream::depth, imageSize.width, imageSize.height, rs::format::z16, 0);
	//sr300Device->enable_stream(rs::stream::depth, imageSize.width, imageSize.height, rs::format::z16, 30);
	sr300Device->enable_stream(rs::stream::color, imageSizeColorHD.width, imageSizeColorHD.height, rs::format::bgr8, 0);

	return 0;

}

int captureContourAndImageData() {

	//Inicializar Camaraa SR300
	if (initializeCamaraSR300(0)) {
		return -1;
	}

	//Inicar Transmisión
	sr300Device->start();

	//Crear nombre de ventana
	//cv::namedWindow("Depth Image", CV_WINDOW_FULLSCREEN);
	cv::namedWindow("Color Image", CV_WINDOW_FULLSCREEN);

	cv::setMouseCallback("Color Image", mouseTakePicture, NULL);
	

	while (true)
	{
		//Esperar por una frame
		sr300Device->wait_for_frames();

		//Declarar objecto para almacenar imagen de profundidad
		//cv::Mat depthImageNative(imageSize, CV_16UC1, (uint16_t*)sr300Device->get_frame_data(rs::stream::depth));

		cv::Mat imageColor(imageSize, CV_8UC3, (uint8_t*)sr300Device->get_frame_data(rs::stream::color_aligned_to_depth));
		//Visualizar Imagen
		cv::imshow("Color Image", imageColor);

		//Esperar ENTER
		if (cv::waitKey(1) == 27 || takePictureFlag) {
			cv::Mat depthImageNative(imageSize, CV_16UC1, (uint16_t*)sr300Device->get_frame_data(rs::stream::depth));
			cv::imwrite("depth.png", depthImageNative);
			cv::imwrite("wound.png", imageColor);
			cv::destroyAllWindows();
			break;
		}
	}
	return 0;
}

void drawCountour() {

	cv::Mat imageColor = cv::imread("wound.png", CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat imageDepth = cv::imread("depth.png", CV_LOAD_IMAGE_UNCHANGED);

	cv::namedWindow("Imagen herida", CV_WINDOW_FULLSCREEN);

	cv::Mat imgLines = cv::Mat::zeros(imageSize, CV_8UC3);

	cv::setMouseCallback("Imagen herida", mouseEvent, &imgLines);
	

	while (1)
	{
		cv::waitKey(10);//waits

		if (selectObject)
		{
			cv::line(imgLines, pts, last, cv::Scalar(20, 180, 10), 3, CV_AA, 0);

			imageColor = imageColor + imgLines;
			last = pts;
		}
		cv::imshow("Imagen herida", imageColor);

		if (contourOK)
			break;
	}
	/*
	
	if (cv::waitKey(0) || contourOK) {
		cv::Mat depthImageNative(imageSize, CV_16UC1, (uint16_t*)sr300Device->get_frame_data(rs::stream::depth));
		cv::imwrite("depth.png", depthImageNative);
		cv::imwrite("wound.png", imageColor);
		cv::destroyAllWindows();
	}
	*/
}

int getContourWithAnalisis() {

	cv::Mat imageContour = cv::imread("Contour.png", CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat imageColor = cv::imread("wound.png", CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat imageDepth = cv::imread("depth.png", CV_LOAD_IMAGE_UNCHANGED);

	if (imageContour.empty())
		return -1;


	/*
	cv::Mat result = imageColor.clone();

	cv::pyrUp(result, result, cv::Size(640*2, 480*2), 4);

	cv::Mat result2;
	cv::bilateralFilter(result, result2, 10, 80, 100);
	*/

	cv::cvtColor(imageContour, imageContour, CV_BGR2GRAY);

	/*Umbralización*/
	cv::Mat OtsuThresholdImage;
	cv::threshold(imageContour, OtsuThresholdImage, 0, 255, CV_THRESH_BINARY + CV_THRESH_OTSU);

	int erosion_size = 1;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1), cv::Point(erosion_size, erosion_size));
	//Erosionar y dilatar (Filtro para eliminar particulas pequeñas)
	erode(OtsuThresholdImage, OtsuThresholdImage, element);


	getAreaFigure(imageColor, imageDepth, OtsuThresholdImage);

	cv::imshow("imageColor", imageColor);

	return 0;
}

double getDistance(cv::Point centerPoint, cv::Mat &imageAlignDepth, double scale) {

	double distance = 0;

	if (centerPoint.x > 0 && centerPoint.y > 0) {
		distance = imageAlignDepth.at<uint16_t>(centerPoint) * scale * 100;
	}


	return distance;

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
					distance.push_back((double)depthImage.at<uint16_t>(cv::Point(i, j)) * scalar * 100.0);
					areaWound += pow((double)depthImage.at<uint16_t>(cv::Point(i, j)) * scalar * 100.0, 2);
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