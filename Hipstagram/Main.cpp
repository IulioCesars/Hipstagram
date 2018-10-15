#include <windows.h>
#include "resource.h"
#include <iostream>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <math.h>
#include <windows.h>
#include <stdio.h>

#pragma comment(lib,"opencv_world310.lib")

using namespace cv;
using namespace std;


HWND ghDlg = 0;
HINSTANCE ghAppInst;

INT_PTR CALLBACK MsgDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void BtnEncenderCamaraOnClick();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int showCmd)
{

	ghAppInst = hInstance; // Create the modeless dialog window.
	ghDlg = CreateDialog(
		hInstance, // Application instance.
		MAKEINTRESOURCE(IDD_MAIN), // Dialog resource ID.
		0, // Parent window--null for no parent ;___;
		MsgDlgProc); // Dialog window procedure.

	ShowWindow(ghDlg, showCmd); // Show the dialog.

	// Enter the message loop.
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (GetMessage(&msg, 0, 0, 0))
	{
		// Is the message a dialog message? If so the function
		// IsDialogMessage will return true and then dispatch
		// the message to the dialog window procedure.
		// Otherwise, we process as the message as normal.
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
		case WM_INITDIALOG:
			return true;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_BTN_ENCENDER_CAMARA: { BtnEncenderCamaraOnClick(); } break;
			}
			return true;
			return 0;
		case WM_CREATE: 
			return 0;
		case WM_CLOSE:
			DestroyWindow(hDlg);
			return true;
		case WM_DESTROY:
			PostQuitMessage(0);
			return true;
	}
	return false;
}

void BtnEncenderCamaraOnClick()
{
	VideoCapture camara(0);
	if (camara.isOpened())
	{
		while (true)
		{
			Mat frame;
			if (camara.read(frame))
			{ 
				imshow("", frame); 
				// Se supone que con eso se convierte a bitmap
				//HBITMAP hBitmap = CreateBitmap(frame.cols, frame.row, 1, 32, frame.data);

				//CStatic* m_picture;

			}
			if (waitKey(16) == 27)
			{ break; }
		}
	}
	else
	{ MessageBox(0, "No se pudo abrir la camara", "Error", MB_OK); }
}

