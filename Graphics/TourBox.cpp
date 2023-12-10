#include "TourBox.h"
#include <iostream>
#include "SerialPort.h"
#include <bitset>

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
//#define TOURBOX_DEBUG_PRINT
//#define TOURBOX_DEBUG_PRINT_RAW

bool TourBox::ProcessCode(unsigned char code)
{
	TourBoxCode inputCode = (TourBoxCode) (code & 0b00111111);
	bool pressed = !(code & 0b10000000);

	bool directionFlip = false;

	const bool debugPrint = true;
	
#ifdef TOURBOX_DEBUG_PRINT_RAW
	std::cout << "Raw: " << CodeToString(inputCode) << ' ';
#endif

	if (inputCode == TourBoxCode::KnobShort || inputCode == TourBoxCode::KnobTall)
	{
		//Uniquely, the knob gets its direction reversed when chorded with a tall or short button for some reason, so we undo this.
		directionFlip = true;
	}
	inputCode = UnchordWheel(inputCode);

	if (inputCode == TourBoxCode::Dial && correctDialDirection)
	{
		//The dial reports positive direction for anticlockwise in hardware, which is the opposite of how the knob works, so we can correct it here.
		directionFlip = true;
	}


	if (IsChord(inputCode) && !pressed)
	{
		//Chords generate release codes but they do so *in addition* the release codes we care about, so we can ignore this.
#ifdef TOURBOX_DEBUG_PRINT_RAW
		std::cout << "Ignored.\n";
#endif
		return true;
	}
	inputCode = ChordToSingle(inputCode, buttonPressed[TourBoxCode::Tall], buttonPressed[TourBoxCode::Short]);

	inputCode = DoubleToSingle(inputCode);


	if (IsButton(inputCode))
	{
		if (pressed == buttonPressed[inputCode])
		{
			//This happens for a couple of reasons involving weird chording shenanigans on the c1/c2/tall/short buttons.
#ifdef TOURBOX_DEBUG_PRINT_RAW
			std::cout << "Redundant button state change event ignored.\n";
#endif
			return true;
		}


#ifdef TOURBOX_DEBUG_PRINT
		std::cout << "Processed button: " << CodeToString(inputCode);
		std::cout << (pressed ? " down\n" : " up\n");
#endif
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
			if (directionFlip) positiveDirection = !positiveDirection;
#ifdef TOURBOX_DEBUG_PRINT
			std::cout << "Processed wheel: " << CodeToString(inputCode);
			std::cout << (positiveDirection ? " positive\n" : " negative\n");
#endif
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
		else
		{
#ifdef TOURBOX_DEBUG_PRINT_RAW
			std::cout << " Ignored.\n";
#endif
		}
		return true;
	}
	else
	{
		std::cout << "Warning! TourBox API received byte that it couldn't recognise! Maybe wrong port/device in use, or maybe my library is incomplete.\n";
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

bool TourBox::IsDoubleClick(TourBoxCode code)
{
	switch (code)
	{
	case TourBoxCode::TallDouble:
	case TourBoxCode::ShortDouble:
	case TourBoxCode::SideDouble:
	case TourBoxCode::TopDouble:
		return true;
	default:
		return false;
	}
}


bool TourBox::IsChord(TourBoxCode code)
{
	switch (code)
	{
	case TourBoxCode::C1ShortChord:
	case TourBoxCode::C2ShortChord:
	case TourBoxCode::C1TallChord:
	case TourBoxCode::C2TallChord:
	case TourBoxCode::TallShortChord:
		return true;
	default:
		return false;
	}
}

bool TourBox::IsWheelChord(TourBoxCode code)
{
	switch (code)
	{
	case TourBoxCode::KnobTall:
	case TourBoxCode::KnobShort:
	case TourBoxCode::ScrollTall:
	case TourBoxCode::ScrollShort:
		return true;
	default:
		return false;
	}
}

TourBoxCode TourBox::DoubleToSingle(TourBoxCode code)
{
	switch (code)
	{
		case TourBoxCode::TallDouble: return TourBoxCode::Tall;
		case TourBoxCode::ShortDouble: return TourBoxCode::Short;
		case TourBoxCode::SideDouble: return TourBoxCode::Side;
		case TourBoxCode::TopDouble: return TourBoxCode::Top;
		default: return code;
	}
}

TourBoxCode TourBox::ChordToSingle(TourBoxCode code, bool tallDown, bool shortDown)
{
	switch (code)
	{
	case TourBoxCode::C1ShortChord:
		if (shortDown) return TourBoxCode::C1;
		else return TourBoxCode::Short;
	case TourBoxCode::C2ShortChord:
		if (shortDown) return TourBoxCode::C2;
		else return TourBoxCode::Short;
	case TourBoxCode::C1TallChord:
		if (tallDown) return TourBoxCode::C1;
		else return TourBoxCode::Tall;
	case TourBoxCode::C2TallChord:
		if (tallDown) return TourBoxCode::C2;
		else return TourBoxCode::Tall;
	case TourBoxCode::TallShortChord:
		if (tallDown) return TourBoxCode::Short;
		else return TourBoxCode::Tall;
	default:
		return code;
	}
}

TourBoxCode TourBox::UnchordWheel(TourBoxCode code)
{
	switch (code)
	{
		case TourBoxCode::KnobTall: return TourBoxCode::Knob;
		case TourBoxCode::KnobShort: return TourBoxCode::Knob;
		case TourBoxCode::ScrollTall: return TourBoxCode::Scroll;
		case TourBoxCode::ScrollShort: return TourBoxCode::Scroll;
		default: return code;
	}
}

std::string TourBox::CodeToString(TourBoxCode code)
{
	switch (code)
	{
		case TourBoxCode::Tall:           return "Tall";
		case TourBoxCode::Side:		      return "Side";
		case TourBoxCode::Top:		      return "Top";
		case TourBoxCode::Short:	      return "Short";
		case TourBoxCode::ScrollButton:   return "ScrollButton";
		case TourBoxCode::Up:		      return "Up";
		case TourBoxCode::Down:		      return "Down";
		case TourBoxCode::Left:		      return "Left";
		case TourBoxCode::Right:	      return "Right";
		case TourBoxCode::C1:		      return "C1";
		case TourBoxCode::C2:		      return "C2";
		case TourBoxCode::Tour:		      return "Tour";
		case TourBoxCode::KnobButton:     return "KnobButton";
		case TourBoxCode::DialButton:     return "DialButton";
		case TourBoxCode::Scroll:	      return "Scroll";
		case TourBoxCode::Dial:		      return "Dial";
		case TourBoxCode::Knob:		      return "Knob";
		case TourBoxCode::TallDouble:     return "TallDouble";
		case TourBoxCode::ShortDouble:    return "ShortDouble";
		case TourBoxCode::TopDouble:      return "TopDouble";
		case TourBoxCode::SideDouble:     return "SideDouble";
		case TourBoxCode::C1TallChord:    return "C1TallChord";
		case TourBoxCode::C2TallChord:    return "C2TallChord";
		case TourBoxCode::C1ShortChord:   return "C1ShortChord";
		case TourBoxCode::C2ShortChord:   return "C2ShortChord";
		case TourBoxCode::TallShortChord: return "TallShortChord";
		case TourBoxCode::KnobShort:      return "KnobShort";
		case TourBoxCode::KnobTall:       return "KnobTall";
		case TourBoxCode::ScrollShort:    return "ScrollShort";
		case TourBoxCode::ScrollTall:     return "ScrollTall";
		default:                          return "UNRECOGNISED " + std::to_string(int(code));
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

