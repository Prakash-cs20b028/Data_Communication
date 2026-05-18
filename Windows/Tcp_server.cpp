#include<winsock.h>
#include<iostream>  

using namespace std;

int main()
{
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0)  //initialize Winsock
    {
        cout << "Failed to initialize Winsock." << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //create a TCP socket
    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Failed to create socket." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) //bind the socket to an address and port
    {
        cout << "Failed to bind socket." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) //listen for incoming connections
    {
        cout << "Failed to listen on socket." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port 8080..." << endl;

    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL); //accept an incoming connection
        if (clientSocket == INVALID_SOCKET)
        {
            cout << "Failed to accept connection." << endl;
            continue; //continue accepting other connections
        }

        cout << "Client connected!" << endl;

        // Here you can add code to handle communication with the client
        while (true)
        {
            char buffer[1024];
            int recvResult = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); //receive message from the client
            if (recvResult > 0)
            {
                buffer[recvResult] = '\0'; //null-terminate the received message
                cout << "Received from client: " << buffer << endl;
            }
            else if (recvResult == 0)
            {
                cout << "Client disconnected." << endl;
                break; //client disconnected
            }
            else
            {
                cout << "Failed to receive message." << endl;
                break; //error occurred
            }
        }
        closesocket(clientSocket); //close the client socket after handling communication
    }

    closesocket(serverSocket); //close the server socket
    WSACleanup(); //clean up Winsock
    return 0;
}
