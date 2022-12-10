#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include <sstream>
#include <sys/stat.h>
// I really dont need some of these includes lol

#define SERV_PORT 8080


// Request format:
/*
    POST / HTTP/1.1
    Host:[HOSTNAME]
    Content-Type: application/octet-stream
    Content-Disposition: attachment; path=[PATH]; filename=[FILENAME]

    [DATA]
*/





/*
    Reads data from the socket sfd into the std::vector<char> data.
    Reads until connection is closed
*/
void readData(std::vector<char>& d, int s){
    while(1){
        char buffer[16284]; // A buffer to store each chunk of data in
        int bytesRead = read(s, buffer, 16284); // Read a maximum of 16284 bytes into the buffer
        if(bytesRead == 0){ // If no data was read, the connection was closed
            break; // So exit the loop
        }
        for(int i=0; i<bytesRead; i++){
            d.push_back(buffer[i]); // Push the chunk of data onto the vector (passed by reference)
        }
    }
    printf("Read %d bytes from socket %d\n", d.size(), s);
}

/*
    Extracts the path from a POST request
    STILL NEEDS MORE ERROR HANDLING
*/
std::string extractPath(std::vector<char>& d){
    if(d.size() == 0) {return "ERRORPATH\\ERROR";}
    char* pathStart = strstr(&d[0], "path="); // Get a pointer to the start of the string "path=" inside the data
    pathStart += 5; // Move the pointer up to past the "path=" to the start of the actual path
    char* pathEnd = strstr(&pathStart[0], ";"); // Get a pointer to the ; that comes right after the path
    return std::string(pathStart, pathEnd-pathStart); // Create a string, starting at pathStart, with length pathEnd-pathStart
}

/*
    Extracts the filename from a POST request
    STILL NEEDS MORE ERROR HANDLING
*/
std::string extractFilename(std::vector<char>& d){
    if(d.size() == 0) {return "ERRORFILENAME"; }
    char* filenameStart = strstr(&d[0], "filename="); // Get pointer to start of the string "filename=" inside data
    filenameStart += 9; // Increment to get a pointer to the start of the filename
    char* filenameEnd = strstr(&d[0], "\r\n\r\n"); // Get a pointer to the end of the filename (known to be at \r\n\r\n)
    return std::string(filenameStart, filenameEnd-filenameStart); // Create a string starting at filenameStart with length filenameEnd-filenameStart
}

/*
    Extracts the data from a POST request
*/
std::vector<char> extractData(std::vector<char>& d){
    std::vector<char> extractedData; // Used to store the binary data from the POST request
    char* p = strstr(&d[0], "filename=");
    p = strstr(&p[0], "\r\n\r\n");
    if(p+4 > &d[d.size()-1]){
        return extractedData;
    }
    for(char* i = p+4; i!= &d[d.size()-1]; i++){
        extractedData.push_back(*i);
    }
    return extractedData;
}

/*
    Gets an std::string of the IP of a connection
    NEEDS ERROR HANDLING!
*/
std::string getClientAddr(int s){
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(s, (struct sockaddr *)&addr, &addr_size);
    return std::string(inet_ntoa(addr.sin_addr));
}

void writeToDisk(std::vector<char>& d, std::string p, std::string f, int s){
    std::stringstream ss(p);
    std::string folder;
    std::string currentPath = "./";
    while(std::getline(ss, folder, '\\')){
        currentPath += folder + "/";
        mkdir(currentPath.c_str(), 0777);
    }
    // Now that the path is set up, actuall write the data to the disk
    std::ofstream of(currentPath + f, std::ios::binary);
    of.write(&d[0], d.size());
    of.close();
}

// Handles an incoming connection.
void handleRequest(int sfd){
    std::string clientAddr = getClientAddr(sfd);
    printf("Handling incoming connection from %s\n", clientAddr.c_str());

    // Read into the request vector until the connection is closed
    std::vector<char> request;
    readData(request, sfd);
    close(sfd); // Close the socket,

    // Parse the path from the request
    std::string path = extractPath(request);
    printf("Path: %s\n", path.c_str());
    // Remove filename from the path, add the address of the client to the start
    path = path.substr(0, path.find_last_of("\\"));
    path = clientAddr + "\\" + path;

    // Parse the filename from the request
    std::string filename = extractFilename(request);
    printf("Filename: %s\n", filename.c_str());

    // Get the real binary data from the request
    std::vector<char> data = extractData(request);
    request.clear(); // May as well clear it now, since it isnt needed anymore

    // Write the binary data as a file to disk, at the location "./" + path 
    writeToDisk(data, path, filename, sfd);

    // Cleanup:
    data.clear(); // Clear the data vector (dit goes out of scope here, so this probably is unnecessary)
}



int main(){
    // Create a socket listening for a connection on port 8080
    int listenfd = socket(AF_INET, SOCK_STREAM, 0); // Get a socket file descriptor

    struct sockaddr_in servaddr; // Create a sockaddr_in struct to store the address to listen on
    servaddr.sin_family = AF_INET; // Set the address family to le internet
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Allow any address to connect
    servaddr.sin_port = htons(SERV_PORT); // Listen to connections on port 8080
    
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));// Bind the socket to the port and listen for connections
    listen(listenfd, 128); // Listen for connections, allow 128 connections in the queue

    while(1){ // Accept connections and handle them forever
        int connfd = accept(listenfd, (struct sockaddr *)NULL, NULL); // Listen for a connection, get a new socket FD when one is accepted
        std::thread t(handleRequest, connfd); // Create a new thread to handle the connection
        t.detach(); // Detach the thread so it doesnt need to be joined
    }

    return 0;
}