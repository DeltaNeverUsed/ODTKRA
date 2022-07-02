#include <iostream>
#include <Windows.h>
#include <csignal>
#include <stdio.h>

std::string ODTPath = "C:\\Program Files\\Oculus\\Support\\oculus-diagnostics\\";

void killODT(int param)
{
	//Reverse ODT cli commands
	std::string temp = "echo service set-pixels-per-display-pixel-override 1 | \"" + ODTPath + "OculusDebugToolCLI.exe\"";
	system(temp.c_str());

	LPCWSTR Target_window_Name = L"Oculus Debug Tool";
	HWND hWindowHandle = FindWindow(NULL, Target_window_Name);
	if (hWindowHandle != NULL) {
		SendMessage(hWindowHandle, WM_CLOSE, 0, 0);
		SwitchToThisWindow(hWindowHandle, true);
	}
	Sleep(500);
}


// Check if ODT is running
// Returns false if not, and true if it is
bool check_ODT() {
	LPCWSTR Target_window_Name = L"Oculus Debug Tool";
	HWND hWindowHandle = FindWindow(NULL, Target_window_Name);

	if (hWindowHandle == NULL)
		return false;
	return true;
}

void start_ODT(HWND& hWindowHandle, LPCWSTR& Target_window_Name)
{
	// Starts ODT
	std::string tempstr = ODTPath + "OculusDebugTool.exe";
	ShellExecute(NULL, L"open", (LPCWSTR)std::wstring(tempstr.begin(), tempstr.end()).c_str(), NULL, NULL, SW_SHOWDEFAULT);
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
	//Sets "set-pixels-per-display-pixel-override" to 0.01 to decrease performance overhead
	std::string temp = "echo service set-pixels-per-display-pixel-override 0.01 | \"" + ODTPath + "OculusDebugToolCLI.exe\"";
	system(temp.c_str());

	//Turn off ASW, we do not need it
	temp = "echo server: asw.Off | \"" + ODTPath + "OculusDebugToolCLI.exe\"";
	system(temp.c_str());

	//Clear screen
	system("cls");
}

void parse_args(int argc, char* argv[]) {
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--path"))
		{
			ODTPath = std::string(argv[i]) + (argv[i][strlen(argv[i])] == '\\' ? "" : "\\"); // adds \ if user forgot to add it
			ODTPath.erase(std::remove(ODTPath.begin(), ODTPath.end(), '"'), ODTPath.end()); // removes " from the input
		}
	}
}

int main(int argc, char* argv[])
{

	parse_args(argc, argv);

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

	if (check_ODT() == false) {
		std::cout << "Couldn't start Oculus Debug Tool, please check path: " << ODTPath << std::endl;
		return 1;
	}

	hWindowHandle = FindWindow(NULL, Target_window_Name);

	HWND PropertGrid = FindWindowEx(hWindowHandle, NULL, L"wxWindowNR", NULL);
	HWND wxWindow = FindWindowEx(PropertGrid, NULL, L"wxWindow", NULL);

	signal(SIGABRT, killODT);
	signal(SIGTERM, killODT);
	signal(SIGBREAK, killODT);

	//User friendly information
	std::cout << "ODTKRA uses Oculus Debug Tool to keep your rift alive.\n It is basically a \"advanced macro\", \nwhich means you interacting with the debug tool can cause ODTKRA to fail. \n If ODTKRA stops working then close and reopen it." << std::endl;
	std::cout << "The Oculus Debug Tool is supposed to be minimized, let it stay so." << std::endl;
	std::cout << "\nThis program changes the resolution of the Rift CV1 to a very low amount, it will be reversed when program exits or when computer restarts." << std::endl;
	std::cout << "\nLog:" << std::endl;

	SYSTEMTIME st;

	// Presses up arrow key and then down every 600 seconds
	while (true) {
		SendMessage(wxWindow, WM_KEYDOWN, VK_UP, 0);
		SendMessage(wxWindow, WM_KEYUP, VK_UP, 0);
		Sleep(50);
		SendMessage(wxWindow, WM_KEYDOWN, VK_DOWN, 0);
		SendMessage(wxWindow, WM_KEYUP, VK_DOWN, 0);

		//Log keeping
		GetSystemTime(&st);
		std::cout << "Tracking refreshed at " << st.wHour << ":" << st.wMinute << std::endl;

		Sleep(600000);
	}

	return 0;
}