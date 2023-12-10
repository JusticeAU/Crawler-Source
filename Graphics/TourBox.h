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

enum class TourBoxCode
{
	Tall = 0,
	Side = 1,
	Top = 2,
	Short = 3,
	Knob = 4,
	Scroll = 9,
	ScrollButton = 10,
	Dial = 15,
	Up = 16,
	Down = 17,
	Left = 18,
	Right = 19,
	C1 = 34,
	C2 = 35,
	Tour = 42,
	KnobButton = 55,
	DialButton = 56
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

	void RegisterButtonEvent(TourBoxCode button, TourboxButtonEvent action);
	void RegisterScrollEvent(TourBoxCode wheel, TourboxWheelEvent action);

	bool IsButtonDown(TourBoxCode button) const;
	bool WasButtonPressed(TourBoxCode button) const;
	bool WasButtonReleased(TourBoxCode button) const;

	int GetWheelPosition(TourBoxCode wheel) const;
	bool GetWheelChange(TourBoxCode wheel) const;

	void ZeroWheelPosition(TourBoxCode wheel);

	void Update();

private:
	std::map<TourBoxCode, bool> buttonPressed;
	std::map<TourBoxCode, TourboxButtonEvent> buttonEvents;
	std::map<TourBoxCode, ButtonStatus> buttonChanges;

	std::map<TourBoxCode, int> wheelPositions;
	std::map<TourBoxCode, TourboxWheelEvent> wheelEvents;
	std::map<TourBoxCode, char> wheelChanges;

	void PopulateStatusMaps();

	bool ProcessCode(char code);

	void ResetStateDeltas();

	SerialPort* serialInterface = nullptr;	//Using a naked pointer for PIMPL because unique pointer seems to cause annoyances.
};