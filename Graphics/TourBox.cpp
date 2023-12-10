#include "TourBox.h"
#include <iostream>
#include "SerialPort.h"

void TourBox::PopulateStatusMaps()
{
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::Tall, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::Side, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::Top, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::Short, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::ScrollButton, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::Up, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::Down, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::Left, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::Right, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::C1, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::C2, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::Tour, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::KnobButton, false));
	buttonPressed.insert(std::pair<TourBoxCode, bool>(TourBoxCode::DialButton, false));

	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::Tall, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::Side, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::Top, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::Short, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::ScrollButton, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::Up, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::Down, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::Left, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::Right, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::C1, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::C2, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::Tour, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::KnobButton, ButtonStatus::NoChange));
	buttonChanges.insert(std::pair<TourBoxCode, ButtonStatus>(TourBoxCode::DialButton, ButtonStatus::NoChange));

	wheelPositions.insert(std::pair<TourBoxCode, int>(TourBoxCode::Scroll, 0));
	wheelPositions.insert(std::pair<TourBoxCode, int>(TourBoxCode::Knob, 0));
	wheelPositions.insert(std::pair<TourBoxCode, int>(TourBoxCode::Dial, 0));

	wheelChanges.insert(std::pair<TourBoxCode, char>(TourBoxCode::Scroll, 0));
	wheelChanges.insert(std::pair<TourBoxCode, char>(TourBoxCode::Knob, 0));
	wheelChanges.insert(std::pair<TourBoxCode, char>(TourBoxCode::Dial, 0));
}

TourBox::TourBox()
{
	PopulateStatusMaps();
}

TourBox::TourBox(std::string comPort)
{
	PopulateStatusMaps();
	ConnectToCOM(comPort);
}

TourBox::~TourBox()
{
	delete serialInterface;
}

TourBox::TourBox(TourBox&& other) noexcept
{
	buttonPressed = other.buttonPressed;
	buttonEvents = other.buttonEvents;
	buttonChanges = other.buttonChanges;
	wheelPositions = other.wheelPositions;
	wheelEvents = other.wheelEvents;
	wheelChanges = other.wheelChanges;
	serialInterface = other.serialInterface;
	other.serialInterface = nullptr;
}

const TourBox& TourBox::operator=(TourBox&& other) noexcept
{
	buttonPressed = other.buttonPressed;
	buttonEvents = other.buttonEvents;
	buttonChanges = other.buttonChanges;
	wheelPositions = other.wheelPositions;
	wheelEvents = other.wheelEvents;
	wheelChanges = other.wheelChanges;
	serialInterface = other.serialInterface;
	other.serialInterface = nullptr;
	return *this;
}

bool TourBox::ConnectToCOM(std::string comPort)
{
	if (serialInterface)
	{
		std::cout << "Connected to new COM port, replacing old connection.\n";
		delete serialInterface;
		serialInterface = nullptr;
	}

	serialInterface = new SerialPort(comPort);

	if (!serialInterface->connected)
	{
		delete serialInterface;
		serialInterface = nullptr;
		return false;
	}
	else
	{
		return true;
	}
	
}

bool TourBox::ProcessCode(char code)
{
	TourBoxCode inputCode = (TourBoxCode) (code & 0b00111111);

	bool pressed = !(code & 0b10000000);
	if (IsButton(inputCode))
	{
		if (buttonEvents.count(inputCode) > 0)
		{
			buttonEvents[inputCode](pressed);
		}
		buttonChanges[inputCode] = pressed ? ButtonStatus::JustPressed : ButtonStatus::JustReleased;
		buttonPressed[inputCode] = pressed;
		return true;
	}
	else if (IsWheel(inputCode))
	{
		if (pressed)	//Wheels always generate press and release events in simultaneous pairs, so we just ignore the release event.
		{
			bool positiveDirection = code & 0b01000000;
			if (positiveDirection)
			{
				wheelChanges[inputCode]++;
				wheelPositions[inputCode]++;
			}
			else
			{
				wheelChanges[inputCode]--;
				wheelPositions[inputCode]--;
			}
			if (wheelEvents.count(inputCode) > 0)
			{
				wheelEvents[inputCode](positiveDirection ? 1 : -1, wheelPositions[inputCode]);
			}
		}
		return true;
	}
	return false;
}

void TourBox::ResetStateDeltas()
{
	for (auto& thisButton : buttonChanges)
	{
		thisButton.second = ButtonStatus::NoChange;
	}
	for (auto& thisWheel : wheelChanges)
	{
		thisWheel.second = 0;
	}
}

bool TourBox::IsButton(TourBoxCode code)
{
	switch (code)
	{
	case TourBoxCode::Tall:
	case TourBoxCode::Side:
	case TourBoxCode::Top:
	case TourBoxCode::Short:
	case TourBoxCode::ScrollButton:
	case TourBoxCode::Up:
	case TourBoxCode::Down:
	case TourBoxCode::Left:
	case TourBoxCode::Right:
	case TourBoxCode::C1:
	case TourBoxCode::C2:
	case TourBoxCode::Tour:
	case TourBoxCode::KnobButton:
	case TourBoxCode::DialButton:
		return true;
	default:
		return false;
	}
}

bool TourBox::IsWheel(TourBoxCode code)
{
	switch (code)
	{
	case TourBoxCode::Knob:
	case TourBoxCode::Scroll:
	case TourBoxCode::Dial:
		return true;
	default:
		return false;
	}
}

void TourBox::RegisterButtonEvent(TourBoxCode button, TourboxButtonEvent action)
{
	if (IsButton(button))
	{
		buttonEvents[button] = action;
	}
	else
	{
		std::cout << "Attempted to create button event but code passed in isn't a button.\n";
	}
}

void TourBox::RegisterScrollEvent(TourBoxCode wheel, TourboxWheelEvent action)
{
	if (IsWheel(wheel))
	{
		wheelEvents[wheel] = action;
	}
	else
	{
		std::cout << "Attempted to create wheel event but code passed in isn't a wheel.\n";
	}
}

bool TourBox::IsButtonDown(TourBoxCode button) const
{
	return buttonPressed.at(button);
}

bool TourBox::WasButtonPressed(TourBoxCode button) const
{
	return buttonChanges.at(button) == ButtonStatus::JustPressed;
}

bool TourBox::WasButtonReleased(TourBoxCode button) const
{
	return buttonChanges.at(button) == ButtonStatus::JustReleased;
}

int TourBox::GetWheelPosition(TourBoxCode wheel) const
{
	return wheelPositions.at(wheel);
}

bool TourBox::GetWheelChange(TourBoxCode wheel) const
{
	return wheelChanges.at(wheel);
}

void TourBox::ZeroWheelPosition(TourBoxCode wheel)
{
	wheelPositions.at(wheel) = 0;
}

void TourBox::Update()
{
	ResetStateDeltas();
	if (serialInterface)
	{
		unsigned char thisByte;
		while (serialInterface->GetByte(thisByte))
		{
			ProcessCode(thisByte);
		}
	}
}

