#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <algorithm>


#pragma comment(lib,"opencv_world310.lib")
using namespace cv;
using namespace std;

const string BLANCO_NEGRO = "A. Escala de Grises";
const string SEPIA = "B. Sepia";

const string MEDIA = "C. Media";
const string MEDIAPONDERADA = "D. Media Ponderada";
const string SUSTRACCIONMEDIA = "E. Sustraccion de la Media";
const string LAPLACIANO = "F. Laplaciano";
const string MENOSLAPLACIANO = "G. Menos Laplaciano";
const string NORTESUR = "H. Norte - Sur";
const string ESTEOESTE = "I. Este - Oeste";
const string SOBEL_C = "J. Sobel C";
const string SOBEL_F = "K. Sobel F";
const string GAUSSIANO = "L. Gaussiano";
const string ECUALIZAR = "M. Ecualizar";
const string SHARPEN = "X1. Sharpen";
const string EMBOSS = "X2. Emboss";
const string SOBELTOTAL = "SOBEL TOTAL";


const string ListaFiltros[16] = 
{
	BLANCO_NEGRO,
	SEPIA,
	MEDIA,
	MEDIAPONDERADA,
	SUSTRACCIONMEDIA,
	LAPLACIANO,
	MENOSLAPLACIANO,
	NORTESUR,
	ESTEOESTE,
	SOBEL_C,
	SOBEL_F,
	GAUSSIANO,
	ECUALIZAR,
	SHARPEN,
	EMBOSS,
	SOBELTOTAL
};

class ProcessImage
{
	vector<string> filters;

	#pragma region Kernels
	int KernelMedia[9]{
		1, 1, 1,
		1, 1, 1,
		1 ,1, 1
	};
	int KernelMediaPonderada[9]{
		1, 1, 1,
		1, 5, 1,
		1, 1, 1
	};
	int KernelSustraccionMedia[9]{
		-1, -1, -1,
		-1, 8, -1,
		-1, -1, -1
	};
	int KernelLaplaciano[9]{
		0, 1, 0,
		1, -4, 1,
		0, 1, 0
	};
	int KernelMenosLaplaciano[9] {
		0, -1, 0,
		-1, 5, -1,
		0, -1, 0
	};
	int KernelNorteSur[9] {
		1, 1, 1,
		1, -2, 1,
		-1, -1, -1
	};
	int KernelEsteOeste[9] {
		-1, 1, 1,
		-1, -2, 1,
		-1, 1, 1
	};
	int KernelSobelC[9]{
		-1, 0, 1,
		-2, 0, 2,
		-1, 0, 1
	};
	int KernelSobelF[9]{
		-1, -2,-1,
		0, 0, 0,
		1, 2, 1
	};
	int KernelGaussiano[9]{
		1, 2, 1,
		2, 4, 2,
		1, 2, 1
	};
	int KernelSharpen[9]{
		0, -1, 0,
		-1, 5, -1,
		0, -1, 0
	};
	int KernelEmboss[9]{
		-2, -1,	 0,
		-1,	 1,  1,
		 0,  1,  2
	};
#pragma endregion

	int FloorPixel(int i)
	{
		if (i >= 255) { i = 255; }
		else if (i <= 0) { i = 0; }
		return i;
	}

	void DoGray(Mat* source)
	{
		/* #Promedio
			(R + G + B)/3
		*/
		int columns = source->cols * source->channels();
		for (int i = 0; i < source->rows; i++)
		{
			uchar *row = source->ptr<uchar>(i);
			for (int j = 0; j < columns; j += 3)
			{
				int promedio = (row[j] + row[j + 1] + row[j + 2]) / 3;
				promedio = FloorPixel(promedio);
				row[j] = promedio;		//B
				row[j + 1] = promedio;	//G
				row[j + 2] = promedio;	//R
			}
		}
	}

	void DoSepia(Mat *source)
	{
		/* #Sepia
			Red = (inputRed * 0.393) + (inputGreen * 0.769) + (inputBlue * 0.189)
			Green = (inputRed * 0.349) + (inputGreen * 0.686) + (inputBlue * 0.168)
			Blue = (inputRed * 0.272) + (inputGreen * 0.534) + (inputBlue * 0.131)
		*/
		int columns = source->cols * source->channels();
		for (int i = 0; i < source->rows; i++)
		{
			uchar *row = source->ptr<uchar>(i);
			for (int j = 0; j < columns; j += 3)
			{
				float blue = (float)row[j];
				float green = (float)row[j + 1];
				float red = (float)row[j + 2];

				int newRed = (red * 0.393) + (green * 0.769) + (blue * 0.189);
				int newGreen = (red * 0.349) + (green * 0.686) + (blue * 0.168);
				int newBlue = (red * 0.272) + (green * 0.534) + (blue * 0.131);

				row[j] = FloorPixel(newBlue);		//B
				row[j + 1] = FloorPixel(newGreen);	//G
				row[j + 2] = FloorPixel(newRed);	//R
			}
		}
	}

	void DoKernel(Mat *source, int kernel[])
	{
		Mat matClone = source->clone();
		int kernelRadius = sqrt(sizeof(kernel)+1);
		int kernelCenter = (kernelRadius / 2) + 1;
		int DIV = 0;

		// Generar DIV
		for (int i = 0; i <= sizeof(kernel); i++) 
		{ DIV += kernel[i]; }
		
		if (DIV <= 0) { DIV = 1; }

		int columns = matClone.cols * matClone.channels();
		for (int matRow = 1; matRow < matClone.rows - 1; matRow++)
		{
			uchar *sourceCurrentRow = source->ptr<uchar>(matRow);
			for (int matColumn = 3; matColumn < columns - 3; matColumn += 3)
			{
				int Red = 0, Blue = 0, Green = 0;

				// Procesar Kernel
				for (int kernelRow = 0; kernelRow < kernelRadius; kernelRow++)
				{
					int row = matRow + (kernelRow - (kernelCenter - 1));
					uchar *cloneCurrentRow = matClone.ptr<uchar>(row);

					for (int kernelColumn = 0; kernelColumn < kernelRadius; kernelColumn++)
					{
						int column = matColumn + ((kernelColumn - (kernelCenter - 1)) * 3);

						int indexKernel = (kernelRow * kernelRadius) + kernelColumn;
						int currentCellKernel = kernel[indexKernel];

						Blue += (cloneCurrentRow[column] * currentCellKernel);
						Green += (cloneCurrentRow[column + 1] * currentCellKernel);
						Red += (cloneCurrentRow[column + 2] * currentCellKernel);
					}
				}

				Blue = Blue / DIV;
				Green = Green / DIV;
				Red = Red / DIV;

				sourceCurrentRow[matColumn] = FloorPixel(Blue);
				sourceCurrentRow[matColumn + 1] = FloorPixel(Green);
				sourceCurrentRow[matColumn + 2] = FloorPixel(Red);
			}
		}
	}



	void EdgeSobel(Mat inbuf, Mat outbuf, int height, int width) {
		short filter_x[3][3] =
		{ { -1, 0,  1},
		{-2,  0, 2},
		{ -1, 0,  1} };
		short filter_y[3][3] =
		{ { -1, -2,  -1},
		{0,  0, 0},
		{ 1, 2,  1} };
		int sum_x = 0;
		int sum_y = 0;
		int sum = 0;
		for (int i = 1; i < height - 1; i++)
			for (int j = 1; j < width - 1; j++)
			{
				sum_x = 0;
				sum_y = 0;
				for (int a = -1; a <= 1; a++)
					for (int b = -1; b <= 1; b++) {
						uchar *buff = inbuf.ptr<uchar>((i + a)*width);

						sum_x += buff[j + b] * filter_x[a + 1][b + 1];
						sum_y += buff[j + b] * filter_y[a + 1][b + 1];
					}
				sum = (abs(sum_x) + abs(sum_y)) / 6;
				if (sum < 0)   sum = 0;
				if (sum > 255) sum = 255;

				uchar *otro = outbuf.ptr<uchar>(i*width);
				otro[j] = sum;
			}
	}

	int CalcularPixelEcualizado(int cdf, int cdfmin, int pixels)
	{
		float result = ((float)(cdf - cdfmin) / (float)(pixels - cdfmin));
		result *= 255;

		if (result < 0)
		{ result *= -1; }

		return FloorPixel(result);
	}

	int GetMinValue(int arr[], int size)
	{
		int smallest = size;
		for (int i = 1; i < size; ++i)
		{
			if (arr[i]  > 0 && arr[i] < smallest)
			{ smallest = arr[i]; }
		}
		return smallest;
	}

	void DoEcualizar(Mat *source) {
		// Contador de colores
		int Red[256], Green[256], Blue[256];
		int CDF_Red[256], CDF_Green[256], CDF_Blue[256];
		
		for (int i = 0; i < 256; i++)
		{ 
			Red[i] = Green[i] = Blue[i] = 0; 
			CDF_Red[i] = CDF_Green[i] = CDF_Blue[i] = 0;
		}

		int columns = source->cols * source->channels();
		for (int i = 0; i < source->rows; i++)
		{
			uchar *row = source->ptr<uchar>(i);
			for (int j = 0; j < columns; j += 3)
			{
				Blue[row[j]]++;
				Green[row[j + 1]]++;
				Red[row[j + 2]]++;
			}
		}


		for (int i = 1; i < 256; i++)
		{
			CDF_Blue[i] = CDF_Blue[i - 1] + Blue[i];
			CDF_Green[i] = CDF_Green[i - 1] + Green[i];
			CDF_Red[i] = CDF_Red[i - 1] + Red[i];
		}

		int CDF_MIN_BLUE = GetMinValue(CDF_Blue, 256);
		int CDF_MIN_GREEN = GetMinValue(CDF_Green, 256);
		int CDF_MIN_RED = GetMinValue(CDF_Red, 256);

		int PIXELS = source->rows * source->cols;

		for (int i = 0; i < 256; i++)
		{
			Blue[i] = CalcularPixelEcualizado(CDF_Blue[i], CDF_MIN_BLUE, PIXELS);
			Green[i] = CalcularPixelEcualizado(CDF_Green[i], CDF_MIN_GREEN, PIXELS);
			Red[i] = CalcularPixelEcualizado(CDF_Red[i], CDF_MIN_RED, PIXELS);
		}

		for (int i = 0; i < source->rows; i++)
		{
			uchar *row = source->ptr<uchar>(i);
			for (int j = 0; j < columns; j += 3)
			{
				row[j] = Blue[FloorPixel(row[j])];
				row[j + 1] = Green[FloorPixel(row[j + 1])];
				row[j + 2] = Red[FloorPixel(row[j + 2])];
			}
		}
	}

public:
	Mat Process(Mat source)
	{
		Mat result = source.clone();
		for (auto &filter : filters)
		{
			if (filter.compare(BLANCO_NEGRO) == 0)
			{ DoGray(&result); }
			else if (filter.compare(SEPIA) == 0)
			{ DoSepia(&result); }
			else if (filter.compare(MEDIA) == 0)
			{ DoKernel(&result, KernelMedia); }
			else if (filter.compare(MEDIAPONDERADA) == 0)
			{ DoKernel(&result, KernelMediaPonderada); }
			else if (filter.compare(SUSTRACCIONMEDIA) == 0)
			{ DoKernel(&result, KernelSustraccionMedia); }
			else if (filter.compare(LAPLACIANO) == 0)
			{ DoKernel(&result, KernelLaplaciano); }
			else if (filter.compare(MENOSLAPLACIANO) == 0)
			{ DoKernel(&result, KernelMenosLaplaciano); }
			else if (filter.compare(NORTESUR) == 0)
			{ DoKernel(&result, KernelNorteSur); }
			else if (filter.compare(ESTEOESTE) == 0)
			{ DoKernel(&result, KernelEsteOeste); }
			else if (filter.compare(SOBEL_C) == 0)
			{ DoKernel(&result, KernelSobelC); }
			else if (filter.compare(SOBEL_F) == 0)
			{ DoKernel(&result, KernelSobelF); }
			else if (filter.compare(GAUSSIANO) == 0)
			{ DoKernel(&result, KernelGaussiano); }
			else if (filter.compare(SHARPEN) == 0)
			{ DoKernel(&result, KernelSharpen); }
			else if (filter.compare(EMBOSS) == 0)
			{ DoKernel(&result, KernelEmboss); }
			else if (filter.compare(ECUALIZAR) == 0)
			{ DoEcualizar(&result); }
			else if (filter.compare(SOBELTOTAL) == 0)
			{
				//Mat C = result.clone();
				//Mat F = result.clone();

				//DoKernel(&C, KernelSobelC);
				//DoKernel(&F, KernelSobelF);

				//subtract(C, F, result);
				EdgeSobel(result.clone(), result, result.rows, result.cols * 3);
			}
		}
		return result;
	}

	Mat GetHistograma(Mat source)
	{
		vector<Mat> bgr_planes;
		split(source, bgr_planes);

		int histSize = 256;

		float range[] = { 0, 256 };
		const float* histRange = { range };

		bool uniform = true; bool accumulate = false;

		Mat b_hist, g_hist, r_hist;

		calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
		calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
		calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

		int hist_w = 512; int hist_h = 400;
		int bin_w = cvRound((double)hist_w / histSize);

		Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

		normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
		normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
		normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

		for (int i = 1; i < histSize; i++)
		{
			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i))),
				Scalar(255, 0, 0), 2, 8, 0);
			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(g_hist.at<float>(i))),
				Scalar(0, 255, 0), 2, 8, 0);
			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(r_hist.at<float>(i))),
				Scalar(0, 0, 255), 2, 8, 0);
		}

		return histImage;
	}

	void AddFilter(string filter)
	{ filters.push_back(filter); }

	void ClearFilters()
	{ filters.clear(); }
};