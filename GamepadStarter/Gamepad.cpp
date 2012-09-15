#include "Gamepad.h"

Gamepad::Gamepad(int controllerNumber) {
	_controllerNumber = controllerNumber;
}

bool Gamepad::IsConnected() {
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

	DWORD result = XInputGetState(_controllerNumber, &_controllerState);

	if (result == ERROR_SUCCESS) {
		return true;
	} else {
		return false;
	}
}

XINPUT_STATE Gamepad::GetState() {
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

	XInputGetState(_controllerNumber, &_controllerState);

	return _controllerState;
}
