#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#pragma comment(lib,"opencv_world310d.lib")

using namespace cv;
using namespace std;

class HumanDetection
{
private:
	HOGDescriptor hog;
	int escala = 2;
public:
	HumanDetection()
	{ hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector()); }


	Mat Process(Mat source, int* totalPersonas)
	{
		Mat img = source.clone();

		if (!img.data) { return img; }

		Mat aux(img.rows / escala, img.cols / escala, CV_8U);

		resize(img, aux, aux.size(), 0, 0, INTER_LINEAR);
		cvtColor(aux, aux, CV_BGR2GRAY);

		vector<Rect> found, found_filtered;

		hog.detectMultiScale(aux, found, 0, Size(8, 8), Size(32, 32), 1.05, 2);
			
		int i;
		for (i = 0; i < found.size(); i++)
		{
			//dibuja el primero de los filtrados
			Rect r = found[i];
			//dibujemos el rectangulo un poco mas grande de lo normal
			//pa que la racita no quede mal encuadrada, el 3 es de la
			//reduccion que habiamos hecho, estamos compensando
			r.x *= escala;
			r.x += cvRound(r.width*0.1);
			r.width = cvRound(r.width*0.8*escala);
			r.y = r.y*escala;
			r.y += cvRound(r.height*0.07);
			r.height = cvRound(r.height*0.8*escala);
			rectangle(img, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
		}
		*totalPersonas = i;

		return img;
	}
};