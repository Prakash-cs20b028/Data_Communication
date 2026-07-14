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
        cout << "Send to the server: ";

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
            if(strcmp(buffer, "exit") == 0)
            {
                cout << "Server ended the connection." << endl;
                running = false;
                break;
            }
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
            if (running)
                cout << "Receive error." << endl;

            running = false;
            break;
        }
    }
}

int main()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); // af_inet-> ipv4 or ipv6, sock_stream-> tcp, 0-> default protocol
    if (clientSocket < 0)
    {
        cout << "Failed to create socket." << endl;
        return 1;
    }

    // Prepares the address of the server.
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
    {
        cout << "Invalid server address." << endl;
        close(clientSocket);
        return 1;
    }

    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        cout << "Failed to connect to server." << endl;
        close(clientSocket);
        return 1;
    }

    cout << "Connected to server!" << endl;


    thread t1(sendMessageThread, std::ref(running), clientSocket);
    thread t2(receiveMessageThread, std::ref(running), clientSocket);

    if (t1.joinable())
        t1.join();

    running = false;

    shutdown(clientSocket, SHUT_RDWR);

    if (t2.joinable())
        t2.join();

    close(clientSocket);

    return 0;
}
