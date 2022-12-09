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
        char buffer[1024];
        int bytesRead = read(sfd, buffer, 1024);
        if(bytesRead == 0){
            break;
        }
        for(int i = 0; i < bytesRead; i++){
            incomingData.push_back(buffer[i]);
        }
    }
    printf("Read %d bytes total\n", incomingData.size());
    // Parse the filename from the request
    std::string filename;
    char* filenameStart = strstr(&incomingData[0], "filename=");
    char* filenameEnd = strstr(filenameStart, "\r\n\r\n");
    filename = std::string(filenameStart + 9, filenameEnd - filenameStart - 9);
    printf("Filename: %s\n", filename.c_str());
    // Parse the data from the request into a new vector
    std::vector<char> data;
    for(char* i = filenameEnd + 4; i != &incomingData[incomingData.size() - 1]; i++){
        data.push_back(*i);
    }
    incomingData.clear(); // May as well clear it now, since it isnt needed anymore
    // Write the data to disk using the filename:
    std::ofstream of(filename.c_str(), std::ios::out | std::ios::binary);
    of.write(&data[0], data.size());
    of.close();
    printf("Done handling connection\n");
}


int main(){
    // Create a socket listening for a connection on port 8080
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    // Bind the socket to the port and listen for connections
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 128);

    // Accept connections and handle them
    while(1){
        int connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        handleRequest(connfd);
    }

    return 0;
}