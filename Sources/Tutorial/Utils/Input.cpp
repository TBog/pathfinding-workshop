#include "pch.h"

#include "Input.h"

static const float INPUT_DEADZONE_RATIO = 0.2f;   // 20%

//-------------------------------------------------------------------

Input::Input()
	: m_altKeyPressed(false)
	, m_ctrlKeyPressed(false)
{
	for (int i = 0; i < kMaxKeysCount; i++)
		m_keysPressed[i] = false;
}

//-------------------------------------------------------------------

Input::~Input()
{

}

//-------------------------------------------------------------------
void Input::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_KEYDOWN ||
		message == WM_SYSKEYDOWN ||
		message == WM_KEYUP ||
		message == WM_SYSKEYUP)
	{
		bool bKeyDown = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);

		DWORD dwAltMask = (1 << 29);
		m_altKeyPressed = ((lParam & dwAltMask) != 0);

		DWORD dwCtrlMask = (1 << 24);
		m_ctrlKeyPressed = ((lParam & dwCtrlMask) != 0);

		m_keysPressed[(BYTE)(wParam & 0xFF)] = bKeyDown;
	}
}

//-------------------------------------------------------------------

void Input::Update()
{
	DWORD dwResult;
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		// Simply get the state of the controller from XInput.
		dwResult = XInputGetState(i, &state);

		if (dwResult == ERROR_SUCCESS)
		{
			m_gamePadStates[i].m_isConnected = true;
			m_gamePadStates[i].m_buttonStates = state.Gamepad.wButtons;
			m_gamePadStates[i].m_leftTrigger = _NormalizeInput(state.Gamepad.bLeftTrigger);
			m_gamePadStates[i].m_rightTrigger = _NormalizeInput(state.Gamepad.bRightTrigger);
			m_gamePadStates[i].m_thumbLeftX = _NormalizeInput(state.Gamepad.sThumbLX);
			m_gamePadStates[i].m_thumbLeftY = _NormalizeInput(state.Gamepad.sThumbLY);
			m_gamePadStates[i].m_thumbRightX = _NormalizeInput(state.Gamepad.sThumbRX);
			m_gamePadStates[i].m_thumbRightY = _NormalizeInput(state.Gamepad.sThumbRY);
		}
		else
		{
			// Controller is not connected 
			m_gamePadStates[i].m_isConnected = false;
			m_gamePadStates[i].Reset();
		}
	}
}

//-------------------------------------------------------------------

float Input::_NormalizeInput(BYTE axisValue)
{
	static const float MAX_BYTE_INPUT_VALUE = 127.f;
	float sign = (axisValue >= 0) ? 1.f : -1.f;
	float f = (fabsf(axisValue) / MAX_BYTE_INPUT_VALUE - INPUT_DEADZONE_RATIO) / (1.f - INPUT_DEADZONE_RATIO);
	f = sign * __max(__min(f, 1.f), 0.f);
	return f;
}

//-------------------------------------------------------------------

float Input::_NormalizeInput(SHORT axisValue)
{
	static const float MAX_SHORT_INPUT_VALUE = 32767.f;
	float sign = (axisValue >= 0) ? 1.f : -1.f;
	float f = (fabsf(axisValue) / MAX_SHORT_INPUT_VALUE - INPUT_DEADZONE_RATIO) / (1.f - INPUT_DEADZONE_RATIO);
	f = sign * __max(__min(f, 1.f), 0.f);
	return f;
}

//-------------------------------------------------------------------
