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

#define PATHS_FILE "paths.cfg"
#define GUIDE_FILE L"guide.png"

int guide_width;
int guide_height;

#define MAX_BUTTON_IDENT 30

struct t_path {
	char button[MAX_BUTTON_IDENT];
	char dir[_MAX_DRIVE+_MAX_DIR];
	char exe[_MAX_FNAME+_MAX_EXT];
};

#define MAX_PATHS 100

struct t_path paths[MAX_PATHS];
int nr_paths;

#define LINE_BUFFER_SIZE 1024

void ExecuteApp(string workingDir, string exeName, string arguments) {
	string fullPath = workingDir + "\\" + exeName;

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
	Rect rect(0, 0, guide_width, guide_height);
	Graphics grpx(hdc);

	Image* image = new Image(GUIDE_FILE);

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
				if (workingDir.compare("") == 0 && exeName.compare("SHUTDOWN") == 0) {
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

void ReadPaths() {
	nr_paths = 0;

	FILE *file;
	fopen_s(&file, PATHS_FILE, "r");

	char buffer[LINE_BUFFER_SIZE];
	char seps[] = ",\n";
	char *next_token;

	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	while (fgets(buffer, LINE_BUFFER_SIZE, file)) {
		strcpy_s(paths[nr_paths].button, MAX_BUTTON_IDENT, strtok_s(buffer, seps, &next_token));
		strcpy_s(path_buffer, _MAX_PATH, strtok_s(NULL, seps, &next_token));

		_splitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

		strcpy_s(paths[nr_paths].dir, _MAX_DRIVE, drive);
		strcat_s(paths[nr_paths].dir, _MAX_DIR, dir);

		strcpy_s(paths[nr_paths].exe, _MAX_FNAME, fname);
		strcat_s(paths[nr_paths].exe, _MAX_EXT, ext);

		nr_paths++;
	}

	fclose(file);
}

void FindDimensions(ULONG_PTR gdiplusToken) {
	GdiplusStartupInput gdiplusStartupInput;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	Image* image = new Image(GUIDE_FILE);

	guide_width = image->GetWidth();
	guide_height = image->GetHeight();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	Gamepad* controller = new Gamepad(0);

	HDC hdc;
	PAINTSTRUCT ps;

	switch (message) {
		case WM_CREATE:

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
			PostQuitMessage(0);

			return 0;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	ULONG_PTR gdiplusToken = {0};

	ReadPaths();

	FindDimensions(gdiplusToken);

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
		GetSystemMetrics(SM_CXSCREEN)/2-guide_width/2,
		GetSystemMetrics(SM_CYSCREEN)/2-guide_height/2,
		guide_width,
		guide_height,
		0,
		0,
		0,
		0);

	SetTimer(hWnd, 1, 100, NULL);

	MSG msg;

	while (GetMessage(&msg, 0, 0, 0) > 0) DispatchMessage(&msg);

	GdiplusShutdown(gdiplusToken);

	return msg.wParam;
}
