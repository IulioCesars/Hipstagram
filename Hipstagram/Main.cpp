#include <windows.h>
#include "resource.h"
#include "Main.h"
#include "ProcessImage.cpp"
#include "HumanDetection.cpp"
#include <iostream>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <math.h>
#include <windows.h>
#include <stdio.h>
#include <cassert>
#include <windowsx.h>
#include "wtypes.h"

#pragma comment(lib,"opencv_world310.lib")
using namespace cv;
using namespace std;

HWND ghDlg = 0;
HINSTANCE ghAppInst;

UINT TimerID = 0;
enum ModoOperacion { ArchivoImagen, ArchivoVideo, Camara, Ninguno };
ModoOperacion OperacionActual = ModoOperacion::Ninguno;
Mat MatOriginal, MatFiltro;
VideoCapture vCapture;
VideoWriter vWritter;
std::string pathVideo="";
BOOL Pause = false;
ProcessImage pImage = ProcessImage();
HumanDetection hDetector = HumanDetection();

int displayWith = 0;
int displayHeight = 0;

bool volverACalcularRedimencion = true;
Size proporcionRedimencion = Size(600,400);

#pragma region Prototipos
INT_PTR CALLBACK MsgDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
VOID OnTimer(HWND hWnd, UINT id);
VOID OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify);
BOOL OnInitDialog(HWND hWnd, HWND hWndCtl, LPARAM lParam);
#pragma endregion

#pragma region Utils
VOID CambiarOperacion(ModoOperacion modoOperacion)
{
	if (vCapture.isOpened())
	{ vCapture.release(); }

	volverACalcularRedimencion = true;

	OperacionActual = modoOperacion;
}
VOID MostrarMensaje(std::string mensaje, std::string titulo)
{ MessageBox(NULL, mensaje.c_str(), titulo.c_str(), MB_OK); }
VOID MostrarExcepcion(Exception ex)
{ 
	MostrarMensaje(ex.msg, "EX");
	CambiarOperacion(ModoOperacion::Ninguno);
}
BOOL OpenFileName(std::string &fileName, LPCSTR filter, LPCSTR ext = "")
{
	CHAR szFile[MAX_PATH];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;;
	ofn.lpstrDefExt = ext;
	BOOL result = GetSaveFileNameA(&ofn);

	if (result)
	{ fileName = std::string(szFile); }

	return result;
}
#pragma endregion

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int showCmd)
{

	ghAppInst = hInstance;
	ghDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, MsgDlgProc);
	ShowWindow(ghDlg, showCmd); 

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	TimerID = SetTimer(ghDlg, ID_MAINTIMER, 42, NULL);

	HWND desk = GetDesktopWindow();
	RECT desktop;
	GetWindowRect(desk, &desktop);
	displayWith = desktop.right * 0.5;
	displayHeight = desktop.bottom * 0.5;

	while (GetMessage(&msg, 0, 0, 0))
	{
		if (ghDlg == 0 || !IsDialogMessage(ghDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

INT_PTR  CALLBACK MsgDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		HANDLE_MSG(hDlg, WM_TIMER, OnTimer);
		HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
		HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
		//case WM_INITDIALOG:
		//	return true;
		case WM_CLOSE:
			DestroyWindow(hDlg);
			return true;
		case WM_DESTROY:
			PostQuitMessage(0);
			return true;
		
	}
	return false;
}

VOID OnTimer(HWND hWnd, UINT id) 
{
	if (Pause)
	{ return; }

	if (vCapture.isOpened() && !vCapture.read(MatOriginal))
	{ return; }

	if (OperacionActual == ModoOperacion::ArchivoVideo && vCapture.get(CV_CAP_PROP_POS_FRAMES) == vCapture.get(CV_CAP_PROP_FRAME_COUNT))
	{ vCapture = VideoCapture(pathVideo); }

	if (MatOriginal.data != NULL)
	{
		if (volverACalcularRedimencion)
		{
			Size sizeMat = MatOriginal.size();
			int matHeight = displayHeight;
			float proporcion = (float)displayHeight / sizeMat.height;
			int matWith = sizeMat.width * proporcion;
			proporcionRedimencion = Size(matWith, matHeight);
			volverACalcularRedimencion = false;
		}

		resize(MatOriginal, MatOriginal, proporcionRedimencion);

		MatFiltro = pImage.Process(MatOriginal);

		int totalPersonas = 0;
		if (SendDlgItemMessage(hWnd, IDC_CHECK, BM_GETCHECK, 0, 0))
		{ MatFiltro = hDetector.Process(MatOriginal, &totalPersonas); }

		SetDlgItemText(hWnd, IDC_TXT_CONTEO, std::to_string(totalPersonas).c_str());

		imshow("Original", MatOriginal);
		imshow("Filtro", MatFiltro);

		if (vWritter.isOpened())
		{ vWritter.write(MatFiltro); }
	}
	else
	{
		destroyWindow("Original");
		destroyWindow("Filtro");
	}
}
BOOL OnInitDialog(HWND hWnd, HWND hWndCtl, LPARAM lParam) 
{
	HWND cmb = GetDlgItem(hWnd, IDC_CMB_FILTROS);
	for (string filtro: ListaFiltros)
	{ SendMessage(cmb, CB_ADDSTRING, 0, (LPARAM)filtro.c_str()); }

	return true;
}
VOID OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify) 
{
	Pause = true;
	switch (id)
	{
	case IDC_BTN_ENCENDER_CAMARA: 
	{
		try
		{ 
			CambiarOperacion(ModoOperacion::Camara); 
			vCapture = VideoCapture(0);
			SetDlgItemText(hWnd, IDC_TXT_ARCHIVO, "Tomando imagen de la camara");
		}
		catch (Exception ex)
		{ MostrarExcepcion(ex); }
	} 
	break;
	case IDC_BTN_CARGAR_IMAGEN:
	{
		try
		{
			std::string filename;
			if (OpenFileName(filename, "Archivos de Imagen (*.png, *.jpg)\0*.png;*.jpg\0"))
			{
				CambiarOperacion(ModoOperacion::ArchivoImagen);
				MatOriginal = imread(filename);

				if (MatOriginal.data == NULL)
				{
					MostrarMensaje("Archivo no valido", "Error");
					CambiarOperacion(ModoOperacion::Ninguno);
				}
				else
				{ SetDlgItemText(hWnd, IDC_TXT_ARCHIVO, filename.c_str()); }
			}
		}
		catch (Exception ex)
		{ MostrarExcepcion(ex); }
	}
	break;
	case IDC_BTN_CARGAR_VIDEO:
	{
		try 
		{
			std::string filename;
			if (OpenFileName(filename, "Archivos de Video (*.mp4 )\0*.mp4\0"))
			{
				CambiarOperacion(ModoOperacion::ArchivoVideo);
				vCapture = VideoCapture(filename);
				pathVideo = filename;
				SetDlgItemText(hWnd, IDC_TXT_ARCHIVO, filename.c_str());
			}
		}
		catch (Exception ex)
		{ MostrarExcepcion(ex); }
	}
	break;
	case IDC_BTN_CAPTURAR_IMAGEN:
	{
		try
		{
			if (OperacionActual == ModoOperacion::Ninguno)
			{ MostrarMensaje("Primero procese una imagen o video.", "Error"); }

			std::string filename;
			if (OpenFileName(filename, "Archivos de Imagen (*.bmp)\0*.bmp\0", "bmp"))
			{
				// Falta guardar imagen
				vector<int> compression_params;
				compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
				imwrite(filename, MatFiltro, compression_params);
			}
		}
		catch (Exception ex)
		{ MostrarExcepcion(ex); }
	}
	break;
	case IDC_BTN_INICIAR_GRABACION:
	{
		try
		{
			if (OperacionActual == ModoOperacion::Ninguno)
			{ MostrarMensaje("Primero procese una imagen o video.", "Error"); }

			if (vWritter.isOpened())
			{ MostrarMensaje("Ya se esta grabando", "Error"); }

			std::string filename;
			if (OpenFileName(filename, "Archivos de Video (*.avi)\0*.avi\0", "avi"))
			{
				vWritter.open(filename,
					CV_FOURCC('M', 'J', 'P', 'G'),
					10,
					Size(vCapture.get(CV_CAP_PROP_FRAME_WIDTH), vCapture.get(CV_CAP_PROP_FRAME_HEIGHT))
				);
				int i = 0;
			}

		}
		catch (Exception ex)
		{
			MostrarExcepcion(ex);
		}
	}
	break;
	case IDC_BTN_DETENER_GRABACION: 
	{
		try
		{
			if (OperacionActual == ModoOperacion::Ninguno)
			{ MostrarMensaje("Primero procese una imagen o video.", "Error"); }

			if (vWritter.isOpened())
			{ 
				vWritter.release(); 
				MostrarMensaje("Se ha guardado el archivo.", ":D");
			}
			else
			{ MostrarMensaje("No se ha iniciado la grabación.","Error"); }
		}
		catch (Exception ex)
		{
			MostrarExcepcion(ex);
		}
	}
	break;
	case IDC_BTN_ANADIR_FILTRO:
	{
		try
		{
			HWND cmb = GetDlgItem(hWnd, IDC_CMB_FILTROS);
			char cmbValue[_MAX_PATH];
			int nIndex = SendMessage(cmb, CB_GETCURSEL, 0, 0);
			SendMessage(cmb, CB_GETLBTEXT, nIndex, (LPARAM)cmbValue);

			pImage.AddFilter(std::string(cmbValue));
		}
		catch (Exception ex)
		{
			MostrarExcepcion(ex);
		}
	}
	break;
	case IDC_BTN_REINICIAR:
	{ pImage.ClearFilters(); }
	break;
	}
	Pause = false;
}

