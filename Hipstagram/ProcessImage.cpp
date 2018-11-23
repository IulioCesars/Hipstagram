#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>

#pragma comment(lib,"opencv_world310.lib")
using namespace cv;
using namespace std;

const string BLANCO_NEGRO = "1. Escala de Grises";
const string SEPIA = "2. Sepia";

const string MEDIA = "3. Media";
const string MEDIAPONDERADA = "4. Media Ponderada";
const string SUSTRACCIONMEDIA = "5. Sustraccion de la Media";
const string LAPLACIANO = "6. Laplaciano";
const string MENOSLAPLACIANO = "7. Menos Laplaciano";
const string NORTESUR = "8. Norte - Sur";
const string ESTEOESTE= "9. Este - Oeste";



const string ListaFiltros[9] = 
{
	BLANCO_NEGRO,
	SEPIA,
	MEDIA,
	MEDIAPONDERADA,
	SUSTRACCIONMEDIA,
	LAPLACIANO,
	MENOSLAPLACIANO,
	NORTESUR,
	ESTEOESTE
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
		for (int i = 0; i < sizeof(kernel); i++) { DIV += kernel[i]; }
		
		if (DIV == 0) { DIV = 1; }

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
		}

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
		return result;
	}

	void AddFilter(string filter)
	{ filters.push_back(filter); }

	void ClearFilters()
	{ filters.clear(); }
};