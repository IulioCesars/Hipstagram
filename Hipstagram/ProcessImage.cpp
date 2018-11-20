#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>

#pragma comment(lib,"opencv_world310.lib")
using namespace cv;
using namespace std;

class ProcessImage
{
	int kernelLaplaciano[9] =
	{
		0, 1, 0,
		1, -4, 1,
		0, 1, 0
	};

public:
	Mat Process(Mat source)
	{


		//int kdata[] = 
		//{
		//	255, 4, 6, 4, 255,
		//	4, 16, 24, 16, 4,
		//	6, 24, -1, 24, 6,
		//	4, 16, 24, 16, 4,
		//	255, 4, 6, 4, 255
		//};
		//Mat kernel(5, 5, -1, kdata);

		//Mat result;
		//filter2D(source, result, source.depth(), kernel);
		return source;
	}


};