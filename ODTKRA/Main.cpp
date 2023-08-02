#include <iostream>
#include <Windows.h>
#include <csignal>
#include <stdio.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <ctime>
#include <iomanip>
#include <string>
#include <thread>

std::string ODTPath = "C:\\Program Files\\Oculus\\Support\\oculus-diagnostics\\";

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

void forceForegroundWindow(HWND hwnd) { // https://stackoverflow.com/questions/19136365/win32-setforegroundwindow-not-working-all-the-time
	DWORD windowThreadProcessId = GetWindowThreadProcessId(GetForegroundWindow(), LPDWORD(0));
	DWORD currentThreadId = GetCurrentThreadId();
	DWORD CONST_SW_SHOW = 5;
	AttachThreadInput(windowThreadProcessId, currentThreadId, true);
	BringWindowToTop(hwnd);
	ShowWindow(hwnd, CONST_SW_SHOW);
	AttachThreadInput(windowThreadProcessId, currentThreadId, false);
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
}

void killODT(int param) {
	if (param != 0) {
		//Reverse ODT cli commands
		std::cout << "Reversing ODT CLI commands" << std::endl;
		std::string temp = "echo service set-pixels-per-display-pixel-override 1 | \"" + ODTPath + "OculusDebugToolCLI.exe\"";
		system(temp.c_str());
	}

	//Kill ODT
	int attempts = 100;

	std::cout << "Killing ODT" << std::endl;
	HWND hWindowHandle;
	while ((hWindowHandle = get_winhandle((LPCWSTR)L"Oculus Debug Tool")) != NULL) {
		forceForegroundWindow(hWindowHandle);
		SendMessage(hWindowHandle, WM_CLOSE, 0, 0);
		Sleep(15);

		attempts--;
		if (attempts <= 0)
			break;
	}

	std::cout << "ODT Closed!" << std::endl;
}

bool doKillODTThread = false;

void start_process(std::string path) {
	if (get_winhandle((LPCWSTR)L"Oculus Debug Tool") != NULL) {
		killODT(0);
		Sleep(100);
	}

	std::string tempstr = path + "OculusDebugTool.exe";
	std::cout << "Starting: " << tempstr << std::endl;
	//start exe
	ShellExecute(NULL, L"open", (LPCWSTR)std::wstring(tempstr.begin(), tempstr.end()).c_str(), NULL, NULL, SW_SHOWDEFAULT);
	HWND hWindowHandle;

	Sleep(500);

	//wait for window to load
	std::cout << "Waiting for window to load" << std::endl;
	while ((hWindowHandle = get_winhandle((LPCWSTR)L"Oculus Debug Tool")) == NULL) {
		Sleep(100);
	}

	// Kill the thing if user starts doing stuff
	if (doKillODTThread) return;

	//system("cls"); //Clear screen
	std::cout << "ODT window found!" << std::endl;
	HWND wxWindow = get_vxwin(hWindowHandle);

	std::cout << "Waiting for window to be focused" << std::endl;
	while (GetForegroundWindow() != hWindowHandle) {
		if (doKillODTThread) return;
		forceForegroundWindow(hWindowHandle);
		Sleep(250);
	}
	std::cout << "ODT window focused!" << std::endl;

	// Kill the thing if user starts doing stuff
	if (doKillODTThread) return;

	for (int i = 0; i < 7; i++) {
		Press_key(VK_DOWN, wxWindow);
		std::cout << "pressed down" << std::endl;
	}
	Sleep(50);
	Press_key(VK_TAB, wxWindow);
	std::cout << "pressed tab" << std::endl;
	Sleep(50);

	// Kill the thing if user starts doing stuff
	if (doKillODTThread) return;

	Press_key(VK_UP, wxWindow);
	std::cout << "pressed toggle up" << std::endl;
	Sleep(100);
	Press_key(VK_DOWN, wxWindow);
	std::cout << "pressed toggle down" << std::endl;
	Sleep(100);
}

void parse_args(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		if (std::string(argv[i]) == "--path") {
			ODTPath = std::string(argv[i+1]) + (argv[i+1][strlen(argv[i+1])] == '\\' ? "" : "\\"); 	// adds \ if user forgot to add it
			ODTPath.erase(std::remove(ODTPath.begin(), ODTPath.end(), '"'), ODTPath.end()); 	// removes " from the input
		}
	}
}

bool threadRunning;

void doToggle() {
	threadRunning = true;
	start_process(ODTPath);
	if (!doKillODTThread) 
		Sleep(100);
	killODT(0);

	threadRunning = false;
}

int main(int argc, char* argv[]) {
	parse_args(argc, argv);
	
	SetWindowPos(GetConsoleWindow(), 0, 900, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	std::cout << "ODT Path: " << ODTPath << std::endl;
	ODT_CLI();
	SetConsoleTitleA("ODTKRA Memory Leak Edition");

	signal(SIGABRT, killODT);
	signal(SIGTERM, killODT);
	signal(SIGBREAK, killODT);

	ULONGLONG tracking_refresh_timer = GetTickCount64();
	ULONGLONG refresh_loop = tracking_refresh_timer;
	ULONGLONG lastIdle = GetTickCount64();


	int refresh_tracking = 9; 	//refresh tracking every X minutes
	int refresh_tracking_times = 0;
	bool createdThread = false;
	std::thread killThread;

	POINT lastCursor;
	GetCursorPos(&lastCursor);
	while (true) {
		auto tk = GetTickCount64();

		POINT p;
		GetCursorPos(&p);
		// Check if mouse moved
		if (p.x != lastCursor.x || p.y != lastCursor.y)
			lastIdle = tk;
		lastCursor = p;

		if (tk - lastIdle < seconds(15))
		{
			if (threadRunning) {
				doKillODTThread = true;
				refresh_loop = tk;
			}
		}
		else
		{
			doKillODTThread = false;
		}
		

		if (tk >= refresh_loop && tk - lastIdle > seconds(15)) {
			refresh_tracking_times++;
			executed_at(std::to_string(refresh_tracking_times) + " Tracking refresh at: ");


			// Start another thread that does the refreshing, so that we can kill it if the users does anything
			if (createdThread)
				killThread.join();

			killThread = std::thread(doToggle);
			createdThread = true;

			refresh_loop = tk + minutes(refresh_tracking);
			std::cout << "next tk: " << refresh_loop << std::endl;
		}


		Sleep(100);
	}
	return 0;
}