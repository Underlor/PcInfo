#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <winsock2.h>
#include "InfoGetter.h"
#include <windows.h>
#include <stdio.h>
#include "sstream"
#include <iostream>
#include <future>
#define INFO_BUFFER_SIZE 32767
#define BUFSIZE 1024
void InfoGetter::ip_getter() {
	WSADATA wsaData;
	if (!WSAStartup(WINSOCK_VERSION, &wsaData)) {
		char chInfo[64];
		if (!gethostname(chInfo, sizeof(chInfo))) {
			hostent *sh;
			sh = gethostbyname((char*)&chInfo);
			if (sh != NULL) {
				int   nAdapter = 0;
				while (sh->h_addr_list[nAdapter]) {
					struct   sockaddr_in   adr;
					memcpy(&adr.sin_addr, sh->h_addr_list[nAdapter], sh->h_length);
					ip = inet_ntoa(adr.sin_addr);
					nAdapter++;
				}
			}
		}
	} else
		ip = "Can't get ip.";
	WSACleanup();
}

void InfoGetter::get_comp_name() {
	TCHAR  infoBuf[INFO_BUFFER_SIZE];
	DWORD  bufCharCount = INFO_BUFFER_SIZE;
	if (GetComputerName(infoBuf, &bufCharCount))
		computer_name = (char*)infoBuf;
}

void InfoGetter::getRealWindowsVersion() {
	RTL_OSVERSIONINFOEXW *pk_OsVer = new RTL_OSVERSIONINFOEXW;
	typedef LONG(WINAPI* tRtlGetVersion)(RTL_OSVERSIONINFOEXW*);

	memset(pk_OsVer, 0, sizeof(RTL_OSVERSIONINFOEXW));
	pk_OsVer->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

	HMODULE h_NtDll = GetModuleHandleW(L"ntdll.dll");
	tRtlGetVersion f_RtlGetVersion = (tRtlGetVersion)GetProcAddress(h_NtDll, "RtlGetVersion");

	if (!f_RtlGetVersion)
		return; // This will never happen (all processes load ntdll.dll)

	LONG Status = f_RtlGetVersion(pk_OsVer);

	if (Status == 0){
		std::stringstream s;
		s << "Windows " << pk_OsVer->dwMajorVersion << "." << pk_OsVer->dwMinorVersion;
		windows = s.str();
	} else
		windows = "Can't get version";

	delete pk_OsVer;
}

void InfoGetter::get_owner() {
	LPTSTR lpszSystemInfo;
	DWORD cchBuff = 256;
	TCHAR tchBuffer[BUFSIZE];
	lpszSystemInfo = tchBuffer;
	if (GetUserName(lpszSystemInfo, &cchBuff)) {
		std::stringstream s;
		s << lpszSystemInfo;
		owner = s.str();
	} else
		owner = "Can't get owner";
}

void InfoGetter::get_core(){
	int CPUInfo[4] = { -1 };
	unsigned   nExIds, i = 0;
	char CPUBrandString[0x40];
	// Get the information associated with each extended ID.
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	for (i = 0x80000000; i <= nExIds; ++i){
		__cpuid(CPUInfo, i);
		// Interpret CPU brand string
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}
	//string includes manufacturer, model and clockspeed
	cpu_model = CPUBrandString;

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	std::stringstream s;
	s << sysInfo.dwNumberOfProcessors;
	cpu_count = s.str();
}

void InfoGetter::get_memory() {
	try {
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		GlobalMemoryStatusEx(&statex);
		std::stringstream s;
		s << statex.ullTotalPhys / (1024 * 1024);
		ram_count = s.str();
		//	std::cout << "Total System Memory: " << (statex.ullTotalPhys / 1024) / 1024 << "MB" << std::endl;
	} catch (std::exception& e) {
		ram_count = "Error";
	}
}

void InfoGetter::get_wmi_info() {
	try {
		HRESULT hres;

		// Initialize COM.
		hres = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (FAILED(hres)) {
			RAM_Manufacturer = "WMIERROR";
			CPUSocket = "WMIERROR";
			bios_manufacturer = "WMIERROR";
			bios_version = "WMIERROR";
			mb_manufacturer = "WMIERROR";
			mb_model = "WMIERROR";
			hdd_Model[0] = "WMIERROR";
		}

		// Initialize 
		hres = CoInitializeSecurity(
			NULL,
			-1,      // COM negotiates service                  
			NULL,    // Authentication services
			NULL,    // Reserved
			RPC_C_AUTHN_LEVEL_DEFAULT,    // authentication
			RPC_C_IMP_LEVEL_IMPERSONATE,  // Impersonation
			NULL,             // Authentication info 
			EOAC_NONE,        // Additional capabilities
			NULL              // Reserved
		);


		if (FAILED(hres)) {
			RAM_Manufacturer = "WMIERROR";
			CPUSocket = "WMIERROR";
			bios_manufacturer = "WMIERROR";
			bios_version = "WMIERROR";
			mb_manufacturer = "WMIERROR";
			mb_model = "WMIERROR";
			hdd_Model[0] = "WMIERROR";
		}

		// Obtain the initial locator to Windows Management
		// on a particular host computer.
		IWbemLocator *pLoc = 0;

		hres = CoCreateInstance(
			CLSID_WbemLocator,
			0,
			CLSCTX_INPROC_SERVER,
			IID_IWbemLocator, (LPVOID *)&pLoc);

		if (FAILED(hres)) {
			RAM_Manufacturer = "WMIERROR";
			CPUSocket = "WMIERROR";
			bios_manufacturer = "WMIERROR";
			bios_version = "WMIERROR";
			mb_manufacturer = "WMIERROR";
			mb_model = "WMIERROR";
			hdd_Model[0] = "WMIERROR";
		}

		IWbemServices *pSvc = 0;

		// Connect to the root\cimv2 namespace with the
		// current user and obtain pointer pSvc
		// to make IWbemServices calls.

		hres = pLoc->ConnectServer(
			_bstr_t(L"ROOT\\CIMV2"), // WMI namespace
			NULL,                    // User name
			NULL,                    // User password
			0,                       // Locale
			NULL,                    // Security flags                 
			0,                       // Authority       
			0,                       // Context object
			&pSvc                    // IWbemServices proxy
		);

		if (FAILED(hres)) {
			RAM_Manufacturer = "WMIERROR";
			CPUSocket = "WMIERROR";
			bios_manufacturer = "WMIERROR";
			bios_version = "WMIERROR";
			mb_manufacturer = "WMIERROR";
			mb_model = "WMIERROR";
			hdd_Model[0] = "WMIERROR";
		}


		// Set the IWbemServices proxy so that impersonation
		// of the user (client) occurs.
		hres = CoSetProxyBlanket(
			pSvc,                         // the proxy to set
			RPC_C_AUTHN_WINNT,            // authentication service
			RPC_C_AUTHZ_NONE,             // authorization service
			NULL,                         // Server principal name
			RPC_C_AUTHN_LEVEL_CALL,       // authentication level
			RPC_C_IMP_LEVEL_IMPERSONATE,  // impersonation level
			NULL,                         // client identity 
			EOAC_NONE                     // proxy capabilities     
		);

		if (FAILED(hres)) {
			RAM_Manufacturer = "WMIERROR";
			CPUSocket = "WMIERROR";
			bios_manufacturer = "WMIERROR";
			bios_version = "WMIERROR";
			mb_manufacturer = "WMIERROR";
			mb_model = "WMIERROR";
			hdd_Model[0] = "WMIERROR";
		}

		// Use the IWbemServices pointer to make requests of WMI. 
		// Make requests here:

		// For example, query for all the running processes
		IEnumWbemClassObject* pEnumerator = NULL;
		hres = pSvc->ExecQuery(
			bstr_t("WQL"),
			bstr_t("SELECT * FROM Win32_DiskDrive"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hres)) {
			hdd_Model[0] = "ERROR";
			// Program has failed.
		} else {
			IWbemClassObject *pclsObj;
			ULONG uReturn = 0;
			while (pEnumerator) {
				hres = pEnumerator->Next(WBEM_INFINITE, 1,
					&pclsObj, &uReturn);

				if (0 == uReturn) {
					break;
				}

				VARIANT vtProp;
				// Get the value of the Name property
				hres = pclsObj->Get(L"Model", 0, &vtProp, 0, 0);
				if (FAILED(hres))
				{
					
				}
				else {
					wstring ws(vtProp.bstrVal);
					hdd_Model[hdd_count] = string(ws.begin(), ws.end());
					hdd_count++;
				}
				VariantClear(&vtProp);
			}
		}
		//---------------------------------------------------
		pEnumerator = NULL;
		hres = pSvc->ExecQuery(
			bstr_t("WQL"),
			bstr_t("SELECT * FROM Win32_BaseBoard"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hres)) {
			mb_manufacturer = "ERROR";
			mb_model = "ERROR";
		} else {
			IWbemClassObject *pclsObj;
			ULONG uReturn = 0;
			while (pEnumerator) {
				hres = pEnumerator->Next(WBEM_INFINITE, 1,
					&pclsObj, &uReturn);

				if (0 == uReturn) {
					break;
				}

				VARIANT vtProp;
				// Get the value of the Name property
				hres = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
				if (FAILED(hres)) {
					mb_manufacturer = "ERROR";
				} else {
					wstring ws(vtProp.bstrVal);
					mb_manufacturer = string(ws.begin(), ws.end());
				}
				//			wcout << "MotherBoard : " << vtProp.bstrVal << endl;
				VariantClear(&vtProp);
				// Get the value of the Name property
				hres = pclsObj->Get(L"Product", 0, &vtProp, 0, 0);
				if (FAILED(hres)) {
					mb_model = "ERROR";
				} else {
					wstring ws(vtProp.bstrVal);
					mb_model = string(ws.begin(), ws.end());
				}
				VariantClear(&vtProp);
			}
		}
		//---------------------------------------------------
		pEnumerator = NULL;
		hres = pSvc->ExecQuery(
			bstr_t("WQL"),
			bstr_t("SELECT * FROM Win32_BIOS"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hres)) {
			bios_manufacturer = "ERROR";
			bios_version = "ERROR";
		} else {
			IWbemClassObject *pclsObj;
			ULONG uReturn = 0;
			while (pEnumerator) {
				hres = pEnumerator->Next(WBEM_INFINITE, 1,
					&pclsObj, &uReturn);

				if (0 == uReturn) { break; }

				VARIANT vtProp;
				// Get the value of the Name property
				hres = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
				if (FAILED(hres)) {
					bios_manufacturer = "ERROR";
				} else {
					wstring ws(vtProp.bstrVal);
					bios_manufacturer = string(ws.begin(), ws.end());
				}

				VariantClear(&vtProp);
				// Get the value of the Name property
				hres = pclsObj->Get(L"Version", 0, &vtProp, 0, 0);
				if (FAILED(hres)) {
					bios_version = "ERROR";
				} else {
					wstring ws(vtProp.bstrVal);
					bios_version = string(ws.begin(), ws.end());
				}
				VariantClear(&vtProp);
			}
		}
		//---------------------------------------------------
		pEnumerator = NULL;
		hres = pSvc->ExecQuery(
			bstr_t("WQL"),
			bstr_t("SELECT * FROM Win32_Processor"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hres)) {
			CPUSocket = "ERROR";
		} else {
			IWbemClassObject *pclsObj;
			ULONG uReturn = 0;
			while (pEnumerator)	{
				hres = pEnumerator->Next(WBEM_INFINITE, 1,
					&pclsObj, &uReturn);

				if (0 == uReturn) {	break; }

				VARIANT vtProp;
				// Get the value of the Name property
				hres = pclsObj->Get(L"SocketDesignation", 0, &vtProp, 0, 0);
				if (FAILED(hres)) {
					CPUSocket = "ERROR";
				} else {
					wstring ws(vtProp.bstrVal);
					CPUSocket = string(ws.begin(), ws.end());
				}
				VariantClear(&vtProp);
			}

		}
		//---------------------------------------------------
		pEnumerator = NULL;
		hres = pSvc->ExecQuery(
			bstr_t("WQL"),
			bstr_t("SELECT * FROM Win32_PhysicalMemory"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hres)) {
			RAM_Manufacturer = "ERROR";
		} else {
			IWbemClassObject *pclsObj;
			ULONG uReturn = 0;
			while (pEnumerator) {
				hres = pEnumerator->Next(WBEM_INFINITE, 1,
					&pclsObj, &uReturn);

				if (0 == uReturn) {	break; }

				VARIANT vtProp;
				// Get the value of the Name property
				hres = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
				if (FAILED(hres)) {
					RAM_Manufacturer = "ERROR";
				} else {
					wstring ws(vtProp.bstrVal);
					RAM_Manufacturer = string(ws.begin(), ws.end());
				}
				VariantClear(&vtProp);
			}
		}
		//---------------------------------------------------
		pEnumerator = NULL;
		hres = pSvc->ExecQuery(
			bstr_t("WQL"),
			bstr_t("SELECT * FROM Win32_OperatingSystem"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hres)) {
			winInstall_date = "ERROR";
		} else {
			IWbemClassObject *pclsObj;
			ULONG uReturn = 0;
			while (pEnumerator) {
				hres = pEnumerator->Next(WBEM_INFINITE, 1,
					&pclsObj, &uReturn);

				if (0 == uReturn) {	break; }

				VARIANT vtProp;
				// Get the value of the Name property
				hres = pclsObj->Get(L"InstallDate", 0, &vtProp, 0, 0);
				if (FAILED(hres)) {
					winInstall_date = "ERROR";
				} else {
					wstring ws(vtProp.bstrVal);
					winInstall_date = string(ws.begin(), ws.end());
					string temp_date = winInstall_date;
					winInstall_date[0] = temp_date[6];
					winInstall_date[1] = temp_date[7];
					winInstall_date[2] = '.';
					winInstall_date[3] = temp_date[4];
					winInstall_date[4] = temp_date[5];
					winInstall_date[5] = '.';
					winInstall_date[6] = temp_date[0];
					winInstall_date[7] = temp_date[1];
					winInstall_date[8] = temp_date[2];
					winInstall_date[9] = temp_date[3];
					winInstall_date[10] = '|';
					winInstall_date = winInstall_date.erase(winInstall_date.find('|'), winInstall_date.length());
					//winInstall_date = string(to_string(winInstall_date[6]) + to_string(winInstall_date[7]) + "." + to_string(winInstall_date[4]) + to_string(winInstall_date[5]));
					//winInstall_date = winInstall_date[5];

				}
				VariantClear(&vtProp);
			}
		}
		//---------------------------------------------------
		pEnumerator = NULL;
		hres = pSvc->ExecQuery(
			bstr_t("WQL"),
			bstr_t("SELECT * FROM Win32_OperatingSystem"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hres)) {
			winInstall_date = "ERROR";
		} else {
			IWbemClassObject *pclsObj;
			ULONG uReturn = 0;
			while (pEnumerator) {
				hres = pEnumerator->Next(WBEM_INFINITE, 1,
					&pclsObj, &uReturn);

				if (0 == uReturn) {	break; }

				VARIANT vtProp;
				// Get the value of the Name property
				hres = pclsObj->Get(L"Version", 0, &vtProp, 0, 0);
				if (FAILED(hres)) {
					windows = "ERROR";
				} else {
					wstring ws(vtProp.bstrVal);
					windows = string(ws.begin(), ws.end());
				}
				VariantClear(&vtProp);
			}
		}
		// Cleanup
		// ========

		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
	}
	catch (std::exception& e) {
		RAM_Manufacturer = "ERROR";
		CPUSocket = "ERROR";
		bios_manufacturer = "ERROR";
		bios_version = "ERROR";
		mb_manufacturer = "ERROR";
		mb_model = "ERROR";
		hdd_Model[0] = "ERROR";
	}
}
