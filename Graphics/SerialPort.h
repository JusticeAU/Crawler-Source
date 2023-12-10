#pragma once
#include <Windows.h>
#include <string>


//This is a kind of crap implementation of a serial port class, but it's separated out for basic PIMPLness
//(to save you including Windows.h everywhere you need controller support)

class SerialPort
{
public:
	HANDLE handleToCOM;

	bool connected = false;

	SerialPort(std::string comPort);
	~SerialPort();
	SerialPort(const SerialPort& other) = delete;
	const SerialPort& operator=(const SerialPort& other) = delete;

	bool GetByte(unsigned char& data);
};