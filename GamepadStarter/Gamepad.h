#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include <windows.h>
#include <XInput.h>

#pragma comment(lib, "XInput.lib")

class Gamepad
{
private:
	XINPUT_STATE _controllerState;
	int _controllerNumber;
public:
	Gamepad(int controllerNumber);
	bool IsConnected();
	XINPUT_STATE GetState();
};

#endif