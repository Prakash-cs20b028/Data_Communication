#include<winsock.h>
#include<iostream>
#include<string>
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

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //create a TCP socket
    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Failed to create socket." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // Replace with actual server IP if needed

    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) //connect to the server
    {
        cout << "Failed to connect to server." << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server!" << endl;
    string input;
    while(true)
    {
        cout << "Enter a message to send to the server (or 'exit' to quit): ";
        getline(cin, input);
        if (input == "exit")
            break;

        int sendResult = send(clientSocket, input.c_str(), input.length(), 0); //send message to the server
        if (sendResult == SOCKET_ERROR)
        {
            cout << "Failed to send message." << endl;
            break;
        }
    }
    
    closesocket(clientSocket);
    WSACleanup();
    return 0;

}
