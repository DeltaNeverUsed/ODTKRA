#include <iostream>
#include <Windows.h>
#include <csignal>
#include <stdio.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <ctime>
#include <iomanip>
#include <string>

std::string ODTPath = "C:\\Program Files\\Oculus\\Support\\oculus-diagnostics\\";
int memlimit = 512;

int get_pid(const std::wstring& processName) {
	int pid = 0;
	// Take a snapshot of all running processes
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		std::cout << "CreateToolhelp32Snapshot failed: " << GetLastError() << std::endl;
		return pid;
	}

	// Fill in the size of the structure before using it.
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Walk the snapshot of the processes, and for each process,
	// display information
	if (Process32First(hSnapshot, &pe32)) {
		do {
			if (processName == pe32.szExeFile) {
				pid = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);
	return pid;
}

double check_memory_usage() {
	int pid = get_pid(L"OculusDebugTool.exe");
	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	double memory_size_mb = 0;

	if (process == NULL) {
		std::cout << "Couldn't open process" << std::endl;
		return 1;
	}

	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(process, &pmc, sizeof(pmc))) {
		memory_size_mb = (pmc.WorkingSetSize / (1024.0 * 1024.0));
		std::cout << "PID: " << pid << ", Currently using: " << memory_size_mb << " MB" << std::endl;
	}
	CloseHandle(process);
	return memory_size_mb;
}

tm time_now(bool twelve_hour = true) {
	time_t now = time(0);
	tm ltm;
	localtime_s(&ltm, &now);
	if (twelve_hour = true) {
		//convert to 12 hour format
		if (ltm.tm_hour > 12) {
			ltm.tm_hour -= 12;
		}
		else if (ltm.tm_hour == 0) {
			ltm.tm_hour = 12;
		}
	}
	return ltm;
}

int minutes(int minutes) {
	return minutes * 60000;
}

int seconds(int seconds) {
	return seconds * 1000;
}

void executed_at(std::string message = "Executed at: ") {
	tm time = time_now();
	std::cout << message << std::put_time(&time, "%H:%M:%S") << std::endl;
}

HWND get_winhandle(LPCWSTR Target_window_Name) {
	HWND hWindowHandle = FindWindow(NULL, Target_window_Name);
	return hWindowHandle;
}

HWND get_vxwin(HWND hWindowHandle) {
	HWND PropertGrid = FindWindowEx(hWindowHandle, NULL, L"wxWindowNR", NULL);
	HWND wxWindow = FindWindowEx(PropertGrid, NULL, L"wxWindow", NULL);
	return wxWindow;
}

void Press_key(int key, HWND wxWindow) {
	if (wxWindow != NULL) {
		SendMessage(wxWindow, WM_KEYDOWN, key, 0);
		SendMessage(wxWindow, WM_KEYUP, key, 0);
	}
	else {
		keybd_event(key, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(key, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
}

void ODT_CLI() {
	//Sets "set-pixels-per-display-pixel-override" to 0.01 to decrease performance overhead
	std::string temp = "echo service set-pixels-per-display-pixel-override 0.01 | \"" + ODTPath + "OculusDebugToolCLI.exe\"";
	system(temp.c_str());

	//Turn off ASW, we do not need it
	temp = "echo server: asw.Off | \"" + ODTPath + "OculusDebugToolCLI.exe\"";
	system(temp.c_str());

	//Clear screen
	//system("cls");
}

void killODT(int param) {
	if (param != 0) {
		//Reverse ODT cli commands
		std::cout << "Reversing ODT CLI commands" << std::endl;
		std::string temp = "echo service set-pixels-per-display-pixel-override 1 | \"" + ODTPath + "OculusDebugToolCLI.exe\"";
		system(temp.c_str());
	}

	//Kill ODT
	std::cout << "Killing ODT" << std::endl;
	HWND hWindowHandle;
	while ((hWindowHandle = get_winhandle((LPCWSTR)L"Oculus Debug Tool")) != NULL) {
		SendMessage(hWindowHandle, WM_CLOSE, 0, 0);
		SwitchToThisWindow(hWindowHandle, true);
	}

	//system("cls"); //Clear screen
	std::cout << "ODT Closed!" << std::endl;
}

HWND start_process(std::string path) {
	std::string tempstr = path + "OculusDebugTool.exe";
	std::cout << "Starting: " << tempstr << std::endl;
	//start exe
	ShellExecute(NULL, L"open", (LPCWSTR)std::wstring(tempstr.begin(), tempstr.end()).c_str(), NULL, NULL, SW_SHOWDEFAULT);
	HWND hWindowHandle;

	//wait for window to load
	std::cout << "Waiting for window to load" << std::endl;
	while ((hWindowHandle = get_winhandle((LPCWSTR)L"Oculus Debug Tool")) == NULL) {
		Sleep(500);
	}

	//system("cls"); //Clear screen
	std::cout << "ODT window found!" << std::endl;
	HWND wxWindow = get_vxwin(hWindowHandle);

	std::cout << "Waiting for window to be focused" << std::endl;
	while (GetForegroundWindow() != hWindowHandle) {
		SwitchToThisWindow(hWindowHandle, true);
		Sleep(500);
	}
	std::cout << "ODT window focused!" << std::endl;

	for (int i = 0; i < 7; i++) {
		Press_key(VK_DOWN, wxWindow);
		std::cout << "pressed down" << std::endl;
	}
	Press_key(VK_TAB, wxWindow);
	std::cout << "pressed tab" << std::endl;
	//Set "Bypass Proximity Sensor Check" to OFF just in case
	Press_key(VK_UP, wxWindow);
	std::cout << "pressed up" << std::endl;
	Press_key(VK_DOWN, wxWindow);
	std::cout << "pressed down" << std::endl;

	ShowWindow(hWindowHandle, SW_MINIMIZE);

	return hWindowHandle;
}

void parse_args(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		if (std::string(argv[i]) == "--path") {
			ODTPath = std::string(argv[i+1]) + (argv[i+1][strlen(argv[i+1])] == '\\' ? "" : "\\"); 	// adds \ if user forgot to add it
			ODTPath.erase(std::remove(ODTPath.begin(), ODTPath.end(), '"'), ODTPath.end()); 	// removes " from the input
		}
		if (std::string(argv[i]) == "--leaksize") {
			memlimit = atoi(argv[i + 1]);
		}
	}
}

int main(int argc, char* argv[]) {
	parse_args(argc, argv);
	
	SetWindowPos(GetConsoleWindow(), 0, 900, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	if (get_winhandle((LPCWSTR)L"Oculus Debug Tool") != NULL) {
		killODT(0);
	}
	std::cout << "ODT Path: " << ODTPath << std::endl;
	std::cout << "Max memory allowed: " << memlimit << std::endl;
	ODT_CLI();
	SetConsoleTitleA("ODTKRA Memory Leak Edition");
	HWND hWindowHandle = start_process(ODTPath);

	signal(SIGABRT, killODT);
	signal(SIGTERM, killODT);
	signal(SIGBREAK, killODT);

	ULONGLONG tracking_refresh_timer = GetTickCount64();
	ULONGLONG leak_check_timer = GetTickCount64();

	ULONGLONG refresh_loop = tracking_refresh_timer;
	ULONGLONG memory_leak_loop = leak_check_timer;

	int refresh_tracking = 9; 	//refresh tracking every X minutes
	int check_leak_timer = 5; 	//check memory leak every X minutes
	int refresh_tracking_times = 0;
	while (true) {
		if (GetTickCount64() >= refresh_loop) {
			refresh_tracking_times++;
			executed_at(std::to_string(refresh_tracking_times) + " Tracking refresh at: ");
			HWND wxWindow = get_vxwin(hWindowHandle);

			while (GetForegroundWindow() != hWindowHandle) {
				SwitchToThisWindow(hWindowHandle, true);
				Sleep(500);
			}

			Press_key(VK_UP, wxWindow);
			std::cout << "pressed up" << std::endl;
			Sleep(50);
			Press_key(VK_DOWN, wxWindow);
			std::cout << "pressed down" << std::endl;
			ShowWindow(hWindowHandle, SW_MINIMIZE);
			refresh_loop = GetTickCount64() + minutes(refresh_tracking);
			Sleep(1000);
		}

		if (GetTickCount64() >= memory_leak_loop) {
			if (check_memory_usage() >= memlimit) {
				executed_at("Memory leak detected, restarting ODT at: ");
				killODT(0);
				HWND hWindowHandle = start_process(ODTPath);
				Sleep(1000);
			}
			else {
				executed_at("No memory leak detected at: ");
			}
			memory_leak_loop = GetTickCount64() + minutes(check_leak_timer);
		}
	}
	return 0;
}