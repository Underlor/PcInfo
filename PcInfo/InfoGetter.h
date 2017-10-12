#pragma once
#include <string>
#include <datetimeapi.h>

#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")
class InfoGetter {
public:
	std::string ip;
	std::string computer_name;
	std::string windows;
	std::string winInstall_date;
	std::string owner;
	std::string CPUSocket;
	std::string cpu_model;
	std::string cpu_count;
	std::string ram_count;
	int hdd_count;
	std::string hdd_Model[10];
	std::string mb_manufacturer;
	std::string mb_model;
	std::string bios_manufacturer;
	std::string bios_version;
	std::string RAM_Manufacturer;

	InfoGetter() : hdd_count(0)
	{
		ip_getter();
		get_comp_name();
		getRealWindowsVersion();
		get_owner();
		get_core();
		get_memory();
		get_wmi_info();
	};
	
	void ip_getter();
	void get_comp_name();
	void getRealWindowsVersion();
	void get_owner();
	void get_core();
	void get_memory();
	void get_wmi_info();
};
