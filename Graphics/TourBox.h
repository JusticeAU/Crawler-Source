/*
This is a probably pretty crap API for using the TourBox as a game controller.
You can use it if you want, but you are required to include this disclaimer about
it probably being crap.
Written by Finn Morgan, 2023.
*/

#pragma once

#include <map>
#include <functional>
#include <string>

typedef std::function<void(bool wasPressed)> TourboxButtonEvent;
typedef std::function<void(char delta, int position)> TourboxWheelEvent;
class SerialPort;

//Note: The dial and the knob rotation position is measured in integers. There are exactly thirty increments
//to a full revolution, so the smallest detectable rotation is twelve degrees.


//These are the codes I've figured out for the buttons and wheels.
enum class TourBoxCode
{
	//These are standard button click codes
	Tall = 0,
	Side = 1,
	Top = 2,
	Short = 3,
	ScrollButton = 10,
	Up = 16,
	Down = 17,
	Left = 18,
	Right = 19,
	C1 = 34,
	C2 = 35,
	Tour = 42,
	KnobButton = 55,
	DialButton = 56,

	//These are wheel turn codes and have a direction associated with them.
	Scroll = 9,
	Dial = 15,
	Knob = 4,


	//Everything below is weird TourBox stuff that shouldn't really be relevant to the user of this API.

	//These are codes for buttons that support hardware double click detection.
	//They get treated as a different button event if it happens quickly enough after the first event.
	//I work around these in the API but may add software support for double clicks later if that seems
	//worthwhile.
	TallDouble = 24,
	ShortDouble = 28,
	TopDouble = 31,
	SideDouble = 33,

	//These are codes for button combinations (chords) that are supported in hardware. They displace
	//the press event for whatever button completes them, but they generate release events that *don't*
	//displace the corresponding release event for the button. Weird.
	//I have chosen to work around these for the purpose of this API because I don't think
	//gamedevs will ever really find much use in these and they complicate the interface.
	TallShortChord = 26,
	C1TallChord = 36,
	C2TallChord = 37,
	C1ShortChord = 57,
	C2ShortChord = 58,

	//These are codes that the knob and the wheel will register if tall or short buttons are held at the same
	//time. We *so* don't care about this, just going to convert to its unchorded equivalent when received.
	KnobTall = 5,
	KnobShort = 6,
	ScrollTall = 11,
	ScrollShort = 12,

};


enum class ButtonStatus
{
	NoChange,
	JustPressed,
	JustReleased
};

class TourBox
{
public:
	TourBox();
	TourBox(std::string comPort);
	~TourBox();

	TourBox(const TourBox& other) = delete;
	const TourBox& operator=(const TourBox& other) = delete;
	TourBox(TourBox&& other) noexcept;
	const TourBox& operator=(TourBox&& other) noexcept;

	bool ConnectToCOM(std::string comPort);

	static bool IsButton(TourBoxCode code);
	static bool IsWheel(TourBoxCode code);

	static std::string CodeToString(TourBoxCode code);	//For debugging/instead of reflection

	void RegisterButtonEvent(TourBoxCode button, TourboxButtonEvent action);
	void RegisterScrollEvent(TourBoxCode wheel, TourboxWheelEvent action);

	bool IsButtonDown(TourBoxCode button) const;
	bool WasButtonPressed(TourBoxCode button) const;
	bool WasButtonReleased(TourBoxCode button) const;

	int GetWheelPosition(TourBoxCode wheel) const;
	bool GetWheelChange(TourBoxCode wheel) const;

	void ZeroWheelPosition(TourBoxCode wheel);

	void Update();

	bool correctDialDirection = true;	//The tourbox reports the dial direction as clockwise positive, which is the opposite of the knob direction.

private:
	std::map<TourBoxCode, bool> buttonPressed;
	std::map<TourBoxCode, TourboxButtonEvent> buttonEvents;
	std::map<TourBoxCode, ButtonStatus> buttonChanges;

	std::map<TourBoxCode, int> wheelPositions;
	std::map<TourBoxCode, TourboxWheelEvent> wheelEvents;
	std::map<TourBoxCode, char> wheelChanges;

	void PopulateStatusMaps();

	bool ProcessCode(unsigned char code);

	void ResetStateDeltas();

	static bool IsDoubleClick(TourBoxCode code);
	static bool IsChord(TourBoxCode code);
	static bool IsWheelChord(TourBoxCode code);
	static TourBoxCode DoubleToSingle(TourBoxCode code);
	static TourBoxCode ChordToSingle(TourBoxCode code, bool tallDown, bool shortDown);
	static TourBoxCode UnchordWheel(TourBoxCode code);

	SerialPort* serialInterface = nullptr;	//Using a naked pointer for PIMPL because unique pointer seems to cause annoyances.
};