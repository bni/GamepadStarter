#include "Gamepad.h"
#include <iostream>
#include <string>
#include <shellapi.h>
#include <objidl.h>
#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;

#pragma comment (lib, "Gdiplus.lib")

void ExecuteApp(wstring workingDir, wstring exeName, wstring arguments) {
	wstring fullPath = workingDir + L"/" + exeName;

	SHELLEXECUTEINFO execInfo = {0};
	execInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	execInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	execInfo.hwnd = NULL;
	execInfo.lpVerb = NULL;
	execInfo.lpFile = fullPath.c_str();		
	execInfo.lpParameters = arguments.c_str();	
	execInfo.lpDirectory = workingDir.c_str();
	execInfo.nShow = SW_SHOWNORMAL;
	execInfo.hInstApp = NULL;

	ShellExecuteEx(&execInfo);

	WaitForSingleObject(execInfo.hProcess, INFINITE);
}

void OnPaint(HDC hdc) {
	Rect rect(0,0,800,600);
	Graphics grpx(hdc);

	Image* image = new Image(L"guide.png");

	grpx.DrawImage(image, rect);

	delete image;
}

void ShutdownSystem() {
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 

	OpenProcessToken(
		GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&hToken);

	LookupPrivilegeValue(
		NULL,
		SE_SHUTDOWN_NAME,
		&tkp.Privileges[0].Luid); 

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 

	ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 
		SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
		SHTDN_REASON_MINOR_UPGRADE |
		SHTDN_REASON_FLAG_PLANNED);
}

void SampleControllerState(Gamepad* controller) {
	if (controller->IsConnected()) {
		if (controller->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_A) {
			ExecuteApp(L"C:/Program Files (x86)/SQUARE ENIX/Deus Ex Human Revolution", L"dxhr.exe", L"");
		}

		if (controller->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_B) {
			ExecuteApp(L"C:/Program Files (x86)/Electronic Arts/Crytek/Crysis 2/bin32", L"Crysis2.exe", L"");
		}

		if (controller->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_X) {
			ExecuteApp(L"C:/Program Files (x86)/Codemasters/DiRT 3", L"dirt3.exe", L"");
		}

		if (controller->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_Y) {
			ExecuteApp(L"C:/Windows/system32", L"notepad.exe", L"");
		}

		if (controller->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_BACK) {
			ShutdownSystem();
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	Gamepad* controller = new Gamepad(0);

	HDC hdc;
	PAINTSTRUCT ps;

	ULONG_PTR gdiplusToken = {0};
	GdiplusStartupInput gdiplusStartupInput;

	switch (message) {
		case WM_CREATE:
			GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

			return 0;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			OnPaint(hdc);
			EndPaint(hWnd, &ps);

			return 0;
		case WM_TIMER:
			SampleControllerState(controller);

			return 0;
		case WM_DESTROY:
			GdiplusShutdown(gdiplusToken);

			PostQuitMessage(0);

			return 0;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	WNDCLASS wndClass = {0};
	wndClass.lpszClassName = TEXT("GamepadStarter");
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	RegisterClass(&wndClass);

	HWND hWnd = CreateWindowEx(
		0,
		wndClass.lpszClassName,
		0,
		WS_POPUP|WS_VISIBLE|WS_SYSMENU,
		GetSystemMetrics(SM_CXSCREEN)/2-400,
		GetSystemMetrics(SM_CYSCREEN)/2-300,
		800,
		600,
		0,
		0,
		0,
		0);

	SetTimer(hWnd, 1, 100, NULL);

	MSG msg;

	while (GetMessage(&msg, 0, 0, 0) > 0) DispatchMessage(&msg);

	return msg.wParam;
}
