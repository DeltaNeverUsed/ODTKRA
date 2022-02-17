#include <iostream>
#include <Windows.h>
#include <csignal>
#include <stdio.h>

void killODT(int param)
{
	LPCWSTR Target_window_Name = L"Oculus Debug Tool";
	HWND hWindowHandle = FindWindow(NULL, Target_window_Name);
	if (hWindowHandle != NULL) {
		SendMessage(hWindowHandle, WM_CLOSE, 0, 0);
		SwitchToThisWindow(hWindowHandle, true);
	}
	Sleep(500);
}

void start_ODT(HWND& hWindowHandle, LPCWSTR& Target_window_Name)
{
	// Starts ODT
	ShellExecute(NULL, L"open", L"C:\\Program Files\\Oculus\\Support\\oculus-diagnostics\\OculusDebugTool.exe", NULL, NULL, SW_SHOWDEFAULT);
	Sleep(1000); // not sure if needed

	hWindowHandle = FindWindow(NULL, Target_window_Name);
	SwitchToThisWindow(hWindowHandle, true);

	Sleep(100); // not sure if needed

	// Goes to the "Bypass Proximity Sensor Check" toggle
	for (int i = 0; i < 7; i++) {
		keybd_event(VK_DOWN, 0xE0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(VK_DOWN, 0xE0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}

	ShowWindow(hWindowHandle, SW_MINIMIZE);
}

void ODT_CLI()
{
	//Sets "set-pixels-per-display-pixel-override" to 0.0001 to decrease performance overhead
	system("echo service set-pixels-per-display-pixel-override 0.0001 | \"C:\\Program Files\\Oculus\\Support\\oculus-diagnostics\\OculusDebugToolCLI.exe\"");

	//Turn off ASW, we do not need it
	system("echo server: asw.Off | \"C:\\Program Files\\Oculus\\Support\\oculus-diagnostics\\OculusDebugToolCLI.exe\"");

	//Clear screen
	system("cls");
}

int main()
{

	LPCWSTR Target_window_Name = L"Oculus Debug Tool";
	HWND hWindowHandle = FindWindow(NULL, Target_window_Name);

	// Close ODT if it's already open because we have no idea what is currently selected
	if (hWindowHandle != NULL) {
		SendMessage(hWindowHandle, WM_CLOSE, 0, 0);
		SwitchToThisWindow(hWindowHandle, true);
	}

	Sleep(500); // not sure if needed

	//Sends commands to the Oculus Debug Tool CLI to decrease performance overhead
	//Unlikely to do much, but no reason not to.
	ODT_CLI();

	//Starts Oculus Debug Tool
	start_ODT(hWindowHandle, Target_window_Name);

	Sleep(1000);

	hWindowHandle = FindWindow(NULL, Target_window_Name);

	HWND PropertGrid = FindWindowEx(hWindowHandle, NULL, L"wxWindowNR", NULL);
	HWND wxWindow = FindWindowEx(PropertGrid, NULL, L"wxWindow", NULL);

	signal(SIGABRT, killODT);
	signal(SIGTERM, killODT);
	signal(SIGBREAK, killODT);

	//User friendly information
	std::cout << "ODTKRA has started\n" << std::endl;
	std::cout << "ODTKRA uses Oculus Debug Tool to keep your rift alive.\n It is basically a \"advanced macro\", \nwhich means you interacting with the debug tool can cause ODTKRA to fail. \n If ODTKRA stops working then close and reopen it." << std::endl;
	std::cout << "The Oculus Debug Tool is supposed to be minimized, let it stay so." << std::endl;
	std::cout << "\nLog:" << std::endl;

	SYSTEMTIME st;

	// Presses up arrow key and then down every 600 seconds
	while (true) {
		SendMessage(wxWindow, WM_KEYDOWN, VK_UP, 0);
		SendMessage(wxWindow, WM_KEYUP, VK_UP, 0);
		Sleep(50);
		SendMessage(wxWindow, WM_KEYDOWN, VK_DOWN, 0);
		SendMessage(wxWindow, WM_KEYUP, VK_DOWN, 0);
		Sleep(600000);

		//Log keeping
		GetSystemTime(&st);
		std::cout << "Tracking refreshed at " << st.wHour << ":" << st.wMinute << std::endl;
	}

	return 0;
}