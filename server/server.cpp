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

#define SERV_PORT 8080

// Handles an incoming connection.
void handleRequest(int sfd){
    // The client will send the following:
    /*
    POST / HTTP/1.1
    Host:[HOSTNAME]
    Content-Type: application/octet-stream
    Content-Disposition: attachment; filename=[FILENAME]

    [DATA]
    */
    printf("Handling incoming connection... Reading data...\n");
    std::vector<char> incomingData; // The data from the client
    while(1){
        char buffer[16284]; // Create a buffer to store a chunk of data
        int bytesRead = read(sfd, buffer, 16284); // Read the data from the socket connection into the buffer.
        // Note: The socket isnt set to non-blocking, so this will block until data is available or the connection is closed.
        // This should ideally be changed, or this function should be run in a separate thread (DOS attack on this server would be easy right now)
        if(bytesRead == 0){ // If the connection is closed, break out of the loop
            break;
        }
        // If we do have some bytes, push them onto the vector
        // I should probably use reserve, might add that later
        for(int i = 0; i < bytesRead; i++){
            incomingData.push_back(buffer[i]); // Push the data onto the vector byte by byte
        }
    }
    // Now we are out of the loop, we have the entire HTTP POST request in the vector (probably!)
    printf("Read %d bytes total\n", incomingData.size());
    // Parse the filename from the request
    std::string filename; // An std::string to store the filename in
    char* filenameStart = strstr(&incomingData[0], "filename="); // Get a pointer to the start of the filename (well, this actually points to the start of the "filename=" string)
    char* filenameEnd = strstr(filenameStart, "\r\n\r\n"); // Get a pointer to the end of the filename (actually the start of the "\r\n\r\n")
    filename = std::string(filenameStart + 9, filenameEnd - filenameStart - 9); // Copy the filename into the string using the pointers. Args: start, length
    printf("Filename: %s\n", filename.c_str());
    // Parse the data from the request into a new vector
    std::vector<char> data;
    for(char* i = filenameEnd + 4; i != &incomingData[incomingData.size() - 1]; i++){
        data.push_back(*i); // Go through the vector byte by byte and copy the data into the new vector
        // In this loop, i is a pointer to the current byte in the vector instead of an iterator
        // So we use *i to get the actual value of the byte
    }
    incomingData.clear(); // May as well clear it now, since it isnt needed anymore
    // Write the data to disk using the filename using an ofstream
    std::ofstream of(filename.c_str(), std::ios::out | std::ios::binary); // Write the file in binary mode
    of.write(&data[0], data.size()); // Slap the data onto the disk
    of.close(); // Close the file
    printf("Done handling connection\n");
    close(sfd); // Close the socket connection
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
        handleRequest(connfd); // Pass the new socket file descriptor to the handleRequest function, it will do everything from here
    }

    return 0;
}