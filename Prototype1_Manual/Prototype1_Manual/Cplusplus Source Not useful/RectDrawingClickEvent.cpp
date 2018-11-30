#include <iostream>
#include <vector>

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#include <librealsense\rs.hpp>

using namespace std;
/*------------------------------------------
Prototype Functions
--------------------------------------------*/
int initializeCamaraSR300(int index);//void alignImageDepth(cv::Mat &imageDepth); //Alinar Imagen de profundidad con imagen a colo

/*------------------------------------------
	 Variables Globales
--------------------------------------------*/

//Dimensiones de la imagen de profundidad
cv::Size imageSize(640, 480);
//cv::Size imageSizeColor(1920, 1080);
cv::Size imageSizeColorHD(1280, 720);

//Objetos Camara
rs::context ctx;
rs::device *sr300Device = NULL;

double scalar = 0;

bool selectObject = false;
cv::Rect selection;
cv::Point origin;
int trackObject = 0;
cv::Mat image;


void mouseEvent(int event, int x, int y, int flags, void* data)
{
	if (selectObject)
	{
		selection.x = MIN(x, origin.x);
		selection.y = MIN(y, origin.y);
		selection.width = std::abs(x - origin.x);
		selection.height = std::abs(y - origin.y);
		selection &= cv::Rect(0, 0, image.cols, image.rows);
	}
	switch (event)
	{
		case CV_EVENT_LBUTTONDOWN:
			origin = cv::Point(x, y);
			selection = cv::Rect(x, y, 0, 0);
			selectObject = true;
			break;
		case CV_EVENT_LBUTTONUP:
			selectObject = false;
		break;
	}


}


int main()
{
	//Inicializar Camaraa SR300
	if (initializeCamaraSR300(0)) {
		return -1;
	}

	//Inicar Transmisión
	sr300Device->start();

	//Crear nombre de ventana
	//cv::namedWindow("Depth Image", CV_WINDOW_FULLSCREEN);
	cv::namedWindow("Color Image", CV_WINDOW_FULLSCREEN);


	cv::setMouseCallback("Color Image", mouseEvent, 0);

	while (true)
	{
		//Esperar por una frame
		sr300Device->wait_for_frames();
		//Declarar objecto para almacenar imagen de profundidad
		//cv::Mat depthImageNative(imageSize, CV_16UC1, (uint16_t*)sr300Device->get_frame_data(rs::stream::depth));
		cv::Mat imageColor(imageSizeColorHD, CV_8UC3, (uint8_t*)sr300Device->get_frame_data(rs::stream::color));

		image = imageColor;

		if (selectObject && selection.width > 0 && selection.height > 0)
		{
			cv::Mat roi(image, selection);
			cv::bitwise_not(roi, roi);
			//printf("%d %d %d %d\n", selection.x, selection.y, selection.width, selection.height);
		}

		//Visualizar Imagen
		cv::imshow("Color Image", imageColor);

		//Esperar ENTER
		if (cv::waitKey(1) == 27) break;
	}


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