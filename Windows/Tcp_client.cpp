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

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Failed to create socket." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        cout << "Failed to connect to server." << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server!" << endl;

    atomic<bool> running(true);
    thread receiveThread([&]() {
        while (running)
        {
            char buffer[1024];
            int recvResult = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (recvResult > 0)
            {
                buffer[recvResult] = '\0';
                cout << "Server: " << buffer << endl;
            }
            else if (recvResult == 0)
            {
                cout << "Server closed the connection." << endl;
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
        string input;
        cout << "Enter a message to send to the server (or 'exit' to quit): ";
        if (!getline(cin, input))
            break;

        if (input == "exit")
        {
            send(clientSocket, input.c_str(), (int)input.length(), 0);
            running = false;
            break;
        }

        int sendResult = send(clientSocket, input.c_str(), (int)input.length(), 0);
        if (sendResult == SOCKET_ERROR)
        {
            cout << "Failed to send message." << endl;
            running = false;
            break;
        }
    }

    running = false;
    shutdown(clientSocket, SD_BOTH);
    if (receiveThread.joinable())
        receiveThread.join();

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
