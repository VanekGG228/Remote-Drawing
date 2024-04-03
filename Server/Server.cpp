#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <stdio.h>
#include <queue>

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

struct Point {
    char x;
    char y;
};

int main() {
    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("Ошибка при инициализации Winsock: %d\n", iResult);
        return 1;
    }

    SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Ошибка при создании сокета: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    u_long mode = 1; 
    ioctlsocket(ListenSocket, FIONBIO, &mode);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serverAddr.sin_port = htons(27015); 

    iResult = bind(ListenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        printf("error bind: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }


    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("error listen: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::vector<SOCKET> clients;
    std::queue<std::vector<char>> que;
    while (true) {
        SOCKET ClientSocket; 
        sockaddr_in clientAddr;  
        int clientAddrSize = sizeof(clientAddr); 

        ClientSocket = accept(ListenSocket, (sockaddr*)&clientAddr, &clientAddrSize); 
        if (ClientSocket != INVALID_SOCKET) { 
       
            clients.push_back(ClientSocket);
        }
       

    
        for (int i = 0; i < clients.size(); i++) {

            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(clients[i], &readSet);
 
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;

            int result = select(0, &readSet, nullptr, nullptr, &timeout);
            if (result == SOCKET_ERROR) {

                printf("Ошибка при вызове select(): %d\n", WSAGetLastError());
               
            }
            else if (result > 0) {
                std::vector<char> point(20);
                int recvResult = recv(clients[i], point.data(), point.size(), 0);
                if (recvResult > 0) {
                    int x = 0;
                    for (int i = 0; i < point.size(); i++) {
                        if (point[i] == ' ') x++; 
                        if(x>4) std::cout << "IT ISNT NORMAL";
                        std::cout << (point[i]) << std::endl;
                    }

                    que.push(point);

                }
                else {
                    printf("Error: %d\n", WSAGetLastError());
                    closesocket(clients[i]);
                    clients.erase(clients.begin() + i);
                }
            }
        }
     
        
        if (!que.empty()) {
            std::vector<char> point = que.front();
            bool c = true;
            for (int i = 0; i < clients.size(); i++) {

                int sendResult = send(clients[i], point.data(), point.size(), 0);
                if (sendResult == SOCKET_ERROR) {
                    c = false;
                }
            }
            if (c) {
                que.pop();
                std::cout << "sended" << std::endl;
            }
        }
       

        
    }

    // Закрытие сокетов и очистка ресурсов
    for (int i = 0; i < clients.size(); i++) {
        closesocket(clients[i]);
    }

    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}