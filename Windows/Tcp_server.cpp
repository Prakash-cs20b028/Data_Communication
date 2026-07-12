#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>

using namespace std;

int main()
{
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        cout << "Failed to initialize Winsock." << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        cout << "Failed to bind socket." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "Failed to listen on socket." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port 8080..." << endl;

    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET)
        {
            cout << "Failed to accept connection." << endl;
            continue;
        }

        cout << "Client connected!" << endl;

        atomic<bool> running(true);
        thread receiveThread([&]() {
            while (running)
            {
                char buffer[1024];
                int recvResult = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (recvResult > 0)
                {
                    buffer[recvResult] = '\0';
                    cout << "Client: " << buffer << endl;
                    if (string(buffer) == "exit")
                    {
                        cout << "Client requested to close the connection." << endl;
                        running = false;
                        break;
                    }
                }
                else if (recvResult == 0)
                {
                    cout << "Client disconnected." << endl;
                    running = false;
                    break;
                }
                else
                {
                    cout << "Receive error." << endl;
                    running = false;
                    break;
                }
            }
        });

        while (running)
        {
            string response;
            cout << "Enter a message to send to the client (or 'exit' to quit): ";
            if (!getline(cin, response))
                break;

            int sendResult = send(clientSocket, response.c_str(), (int)response.length(), 0);
            if (sendResult == SOCKET_ERROR)
            {
                cout << "Failed to send message." << endl;
                running = false;
                break;
            }

            if (response == "exit")
            {
                running = false;
                break;
            }
        }

        running = false;
        shutdown(clientSocket, SD_BOTH);
        if (receiveThread.joinable())
            receiveThread.join();

        closesocket(clientSocket);
        cout << "Connection closed. Waiting for next client..." << endl;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
