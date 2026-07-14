#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

atomic<bool> running(true);
void sendMessageThread(atomic<bool>& running, int clientSocket)
{
    while (running)
    {
        string input;
        cout << "Send to the client: ";

        if (!getline(cin, input))
        {
            running = false;
            break;
        }

        int sendResult = send(clientSocket, input.c_str(), input.length(), 0);
        if (sendResult < 0)
        {
            cout << "Failed to send message." << endl;
            running = false;
            break;
        }

        if (input == "exit")
        {
            running = false;
            break;
        }
    }
}
void receiveMessageThread(atomic<bool>& running, int clientSocket)
{
    while (running)
    {
        char buffer[1024];
        int recvResult = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (recvResult > 0)
        {
            buffer[recvResult] = '\0';
            cout << "Client: " << buffer << endl;

            if (strcmp(buffer, "exit") == 0)
            {
                cout << "Client ended the connection." << endl;
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
            if (running)
                cout << "Receive error." << endl;

            running = false;
            break;
        }
    }
}
int main()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        cout << "Failed to create socket." << endl;
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        cout << "Failed to bind socket." << endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 1) < 0)
    {
        cout << "Failed to listen." << endl;
        close(serverSocket);
        return 1;
    }

    cout << "Server is listening on port 8080..." << endl;

    sockaddr_in clientAddress{};
    socklen_t clientAddressSize = sizeof(clientAddress);

    int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);
    if (clientSocket < 0)
    {
        cout << "Failed to accept client." << endl;
        close(serverSocket);
        return 1;
    }

    cout << "Client connected!" << endl;

    thread t1(sendMessageThread, std::ref(running), clientSocket);
    thread t2(receiveMessageThread, std::ref(running), clientSocket);

    if(t1.joinable())
        t1.join();  

    running = false;

    shutdown(clientSocket, SHUT_RDWR);

    if (t2.joinable())
        t2.join();

    close(clientSocket);
    close(serverSocket);

    return 0;
}