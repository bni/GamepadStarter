#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>
#include <objidl.h>
#include <gdiplus.h>
#include <string>

#include "Gamepad.h"

using namespace std;
using namespace Gdiplus;

#pragma comment (lib, "Gdiplus.lib")

typedef struct t_path {
	char button[30];
	char dir[256];
	char exe[50];
};

struct t_path paths[512];
int nr_paths;

void ExecuteApp(string workingDir, string exeName, string arguments) {
	string fullPath = workingDir + "/" + exeName;

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

void ReadPaths() {
	nr_paths = 0;

	FILE *file;
	fopen_s(&file, "paths.cfg", "r");

	char buffer[1024];
	char seps[] = ",\n";
	char *next_token;

	while (fgets(buffer, 1024, file)) {
		strcpy_s(paths[nr_paths].button, 30, strtok_s(buffer, seps, &next_token));
		strcpy_s(paths[nr_paths].dir, 256, strtok_s(NULL, seps, &next_token));
		strcpy_s(paths[nr_paths].exe, 50, strtok_s(NULL, seps, &next_token));

		nr_paths++;
	}

	fclose(file);
}

void SampleControllerState(HWND hWnd, Gamepad* controller) {
	if (controller->IsConnected()) {
		WORD buttonState = controller->GetState().Gamepad.wButtons;

		for (int i = 0; i < nr_paths; i++) {
			t_path path = paths[i];

			string button = path.button;
			string workingDir = path.dir;
			string exeName = path.exe;

			if ((button.compare("BUTTON_DPAD_UP") == 0 && (buttonState & XINPUT_GAMEPAD_DPAD_UP)) ||
				(button.compare("BUTTON_DPAD_DOWN") == 0 && (buttonState & XINPUT_GAMEPAD_DPAD_DOWN)) ||
				(button.compare("BUTTON_DPAD_LEFT") == 0 && (buttonState & XINPUT_GAMEPAD_DPAD_LEFT)) ||
				(button.compare("BUTTON_DPAD_RIGHT") == 0 && (buttonState & XINPUT_GAMEPAD_DPAD_RIGHT)) ||
				(button.compare("BUTTON_START") == 0 && (buttonState & XINPUT_GAMEPAD_START)) ||
				(button.compare("BUTTON_BACK") == 0 && (buttonState & XINPUT_GAMEPAD_BACK)) ||
				(button.compare("BUTTON_LEFT_THUMB") == 0 && (buttonState & XINPUT_GAMEPAD_LEFT_THUMB)) ||
				(button.compare("BUTTON_RIGHT_THUMB") == 0 && (buttonState & XINPUT_GAMEPAD_RIGHT_THUMB)) ||
				(button.compare("BUTTON_LEFT_SHOULDER") == 0 && (buttonState & XINPUT_GAMEPAD_LEFT_SHOULDER)) ||
				(button.compare("BUTTON_RIGHT_SHOULDER") == 0 && (buttonState & XINPUT_GAMEPAD_RIGHT_SHOULDER)) ||
				(button.compare("BUTTON_A") == 0 && (buttonState & XINPUT_GAMEPAD_A)) ||
				(button.compare("BUTTON_B") == 0 && (buttonState & XINPUT_GAMEPAD_B)) ||
				(button.compare("BUTTON_X") == 0 && (buttonState & XINPUT_GAMEPAD_X)) ||
				(button.compare("BUTTON_Y") == 0 && (buttonState & XINPUT_GAMEPAD_Y))) {
				if (workingDir.compare("SHUTDOWN") == 0 && exeName.compare("SHUTDOWN") == 0) {
					ShutdownSystem();
				} else {
					ShowWindow(hWnd, SW_MINIMIZE);
					ExecuteApp(workingDir, exeName, "");
					ShowWindow(hWnd, SW_RESTORE);
				}
			}
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

			ReadPaths();

			return 0;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			OnPaint(hdc);
			EndPaint(hWnd, &ps);

			return 0;
		case WM_TIMER:
			SampleControllerState(hWnd, controller);

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
	wndClass.lpszClassName = "GamepadStarter";
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
