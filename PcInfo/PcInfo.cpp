// PcInfo.cpp: определяет точку входа для консольного приложения.
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS
#define REMOTE_SERVER 'b1-fileshare'
#define REMOTE_FOLDER 'pcinfo'

#include <winsock2.h>
#include <string>
#include "InfoGetter.h"
#include <iostream>
#include <sstream>
#include <winnt.h>
#include <fileapi.h>
#include <winioctl.h>
#include <ioapiset.h>
#include <future>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#pragma comment(lib, "ws2_32.lib")
//#define BUFSIZE 1024

#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

int main(int argc, TCHAR *argv[]) {
	
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_HIDE);
	InfoGetter inf;

	setlocale(0, "");
	std::ofstream pcinfo_file;
	pcinfo_file.open(inf.ip + ".txt");
	pcinfo_file 
		<< "IP: "<<  inf.ip
		<< "\nOS:"
		<< "\n\tComputer_name: " << inf.computer_name
		<< "\n\tOwner: " << inf.owner
		<< "\n\tWindows Version: "<< inf.windows
		<< "\n\tInstall date: " << inf.winInstall_date
		<< "\nMotherboard:\n" << "\tMotherboard Manufacturer: " << inf.mb_manufacturer
		<< "\n" << "\tMotherboard Model: " << inf.mb_model
		<< "\nCPU:\n" << "\tCPU Socket: " << inf.CPUSocket
		<< "\n\tCpu model: " << inf.cpu_model
		<< "\n\tCpu count: " << inf.cpu_count
		<< "\nRAM:\n\tRam count: " << inf.ram_count
		<< "\n" << "\tRAM_Manufacturer: " << inf.RAM_Manufacturer
		<< "\n" << "HDD:  \n";
	for (int i = 0; i < inf.hdd_count; i++)
	{
		pcinfo_file << "\tDisk#" << i << ": " << inf.hdd_Model[i] << endl;
	}
	pcinfo_file << "\nBIOS:\n" << "\tBios Manufacturer: " << inf.bios_manufacturer
		<< "\n" << "\tBios version: " << inf.bios_version 
		<< "\n";

	pcinfo_file.close();

	return 0;
}


