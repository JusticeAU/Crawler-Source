#include "SerialPort.h"
#include <iostream>

SerialPort::SerialPort(std::string comPort)
{
	handleToCOM = CreateFileA(static_cast<LPCSTR>(comPort.c_str()), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD errMsg = GetLastError();
	if (errMsg == 2)
	{
		std::cout << "Device not found.\n";
	}
	else if (errMsg == 5)
	{
		std::cout << "Device already in use.\n";
	}
	else if (errMsg != 0)
	{
		std::cout << "Unknown error.\n";
	}
	else
	{
		DCB dcbSerialParameters = { 0 };
		if (!GetCommState(handleToCOM, &dcbSerialParameters))
		{
			std::cout << "Failed to get current serial parameters.\n";
		}
		else
		{
			dcbSerialParameters.BaudRate = CBR_9600;
			dcbSerialParameters.ByteSize = 8;
			dcbSerialParameters.StopBits = ONESTOPBIT;
			dcbSerialParameters.Parity = NOPARITY;
			dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

			if (!SetCommState(handleToCOM, &dcbSerialParameters))
			{
				std::cout << "Couldn't set serial port parameters!\n";
			}
			else
			{
				std::cout << "Successfully connected to requested COM port!\n";
				connected = true;
				PurgeComm(handleToCOM, PURGE_RXCLEAR | PURGE_TXCLEAR);
			}
		}
	}
}

SerialPort::~SerialPort()
{
	CloseHandle(handleToCOM);
}

bool SerialPort::GetByte(unsigned char& data)
{
	COMSTAT status;
	DWORD bytesRead;
	DWORD errors;
	int toRead = 0;

	ClearCommError(handleToCOM, &errors, &status);

	//TODO we're currently ignoring anything we find in 'errors' which doesn't *feel* like the right approach.

	if (status.cbInQue > 0)
	{
		return ReadFile(handleToCOM, &data, 1, &bytesRead, NULL);
	}
	else
	{
		return false;
	}
}
