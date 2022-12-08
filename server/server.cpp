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

#define SERV_PORT 8080



int main(){
    // Create a socket listening for a connection on port 8080
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 1);

    // Accept a connection
    int connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
    
    // Read the message from the client
    // The message will be a HTTP POST request in the following format:
    /*
    POST / HTTP/1.1
    Host:[HOSTNAME]
    Content-Type: application/octet-stream
    Content-Disposition: attachment; filename=[FILENAME]

    [DATA]
    */
    // Read the request into a std::vector<char>

    std::vector<char> incomingData;
    while(1){
        char buffer[1024];
        int bytesRead = read(connfd, buffer, 1024);
        printf("Read %d bytes\n", bytesRead);
        if(bytesRead == 0){
            break;
        }
        for(int i = 0; i < bytesRead; i++){
            incomingData.push_back(buffer[i]);
        }
    }
    printf("Read %d bytes total\n", incomingData.size());
    for(auto i = incomingData.begin(); i != incomingData.end(); i++){
        printf("%c", *i);
    }
    printf("\n");


    // Parse the filename from the request
    std::string filename;

    char* filenameStart = strstr(&incomingData[0], "filename=");
    // Read until the next newline
    char* filenameEnd = strstr(filenameStart, "\r\n");
    filename = std::string(filenameStart + 9, filenameEnd - filenameStart - 9);
    printf("Filename: %s\n", filename.c_str());

    // Parse the data from the request

    // Find the start of the data (marked by the end of the filename)
    // Read until the end of the request

    std::vector<char> data;
    for(char* i = filenameEnd + 2; i != &incomingData[incomingData.size() - 1]; i++){
        data.push_back(*i);
    }
    // Write the data to disk using the filename
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666);
    write(fd, &data[0], data.size());
    close(fd);


    return 0;
}