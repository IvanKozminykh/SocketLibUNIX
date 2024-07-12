#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netdb.h>

#define SOCKETLIB_API

SOCKETLIB_API int PORT;
SOCKETLIB_API int SOCK;

SOCKETLIB_API int CreateSocket()
{
    // Создание сокета
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Error initializing socket");
        return -1;
    }
    printf("Server Socket initialization is OK.\n");

    return sock;
}
SOCKETLIB_API int SendData(int sock, const char *data, int dataLen)
{
    int iResult = send(sock, data, dataLen, 0);
    if (iResult == -1)
    {
        perror("Send error");
        close(sock);
        return 1;
    }
    printf("Sent: %d bytes\n", iResult);
    return iResult;
}

SOCKETLIB_API int ReceiveData(int sock, char *buffer, int bufferLen)
{
    int iResult = recv(sock, buffer, bufferLen, 0);
    if (iResult > 0)
    {
        printf("Received: %d bytes\n", iResult);
    }
    else if (iResult == 0)
    {
        printf("Connection closed\n");
    }
    else
    {
        perror("Receive failed");
    }
    return iResult;
}


SOCKETLIB_API void ListAdapters()
{
    struct ifaddrs *ifaddr, *ifa;
    char addrBuf[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return;
    }

    int i = 0;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next, i++)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            void *addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr, addrBuf, sizeof(addrBuf));
            printf("\nAdapter %d\n", i);
            printf("    Name: %s\n", ifa->ifa_name);
            printf("    IP Address: %s\n", addrBuf);
        }
    }

    freeifaddrs(ifaddr);
}

SOCKETLIB_API struct ifaddrs *SelectAdapter()
{
    ListAdapters();

    int choice;
    printf("\nSelect an adapter by number: ");
    scanf("%d", &choice);

    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return NULL;
    }

    int i = 0;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next, i++)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET && i == choice)
        {
            return ifa;
        }
    }

    printf("Invalid selection.\n");
    freeifaddrs(ifaddr);
    return NULL;
}

SOCKETLIB_API int Server(int PORT, int sock)
{
    struct ifaddrs *selectedAdapter = SelectAdapter();

    if (selectedAdapter == NULL)
    {
        printf("No adapter selected.\n");
        return 1;
    }

    char addrBuf[INET_ADDRSTRLEN];
    void *addr = &((struct sockaddr_in *)selectedAdapter->ifa_addr)->sin_addr;
    inet_ntop(AF_INET, addr, addrBuf, sizeof(addrBuf));
    printf("Selected Adapter IP address: %s\n", addrBuf);

    // Указание параметров сервера
    struct sockaddr_in saServer;
    memset(&saServer, 0, sizeof(saServer));

    saServer.sin_family = AF_INET;
    saServer.sin_port = htons(PORT);
    inet_pton(AF_INET, addrBuf, &saServer.sin_addr);

    // Привязка сокета
    int erStat = bind(sock, (struct sockaddr *)&saServer, sizeof(saServer));

    if (erStat != 0)
    {
        perror("Error binding socket to server info");
        close(sock);
        return 1;
    }
    printf("Binding socket to Server info is OK.\n");

    // Включение прослушивания сокета
    erStat = listen(sock, SOMAXCONN);

    if (erStat != 0)
    {
        perror("Can't start to listen");
        close(sock);
        return 1;
    }
    printf("Listening...\n");

    // Подтверждение подключения
    struct sockaddr_in ClientInfo;
    memset(&ClientInfo, 0, sizeof(ClientInfo));
    socklen_t client_size = sizeof(ClientInfo);

    int ClientConn = accept(sock, (struct sockaddr *)&ClientInfo, &client_size);

    if (ClientConn == -1)
    {
        perror("Client detected, but can't connect to client");
        close(sock);
        close(ClientConn);
        return 1;
    }
    printf("Connection to client established successfully.\n");

    // Непрерывное взаимодействие
    char recvBuffer[24];
    char sendBuffer[24] = "Yes";
    int recvResult;
    while (1)
    {
        recvResult = ReceiveData(ClientConn, recvBuffer, sizeof(recvBuffer));

        if (recvResult <= 0)
        {
            perror("Connection closed or error occurred");
            break;
        }

        printf("Received from Client: %s\n", recvBuffer);
        SendData(ClientConn, sendBuffer, sizeof(sendBuffer));
    }

    close(ClientConn);
    close(sock);
    return 0;
}

SOCKETLIB_API int Client(int PORT, int sock)
{
    /*
    char serverIP[46]="172.30.203.179";
    // Подключение к серверу
    struct sockaddr_in ServerInfo;
    memset(&ServerInfo, 0, sizeof(ServerInfo));

    ServerInfo.sin_family = AF_INET;
    ServerInfo.sin_port = htons(PORT);
    inet_pton(AF_INET, serverIP, &ServerInfo.sin_addr);
    */

    char hostname[]="WS-315A33";
    char* IPBuffer;
    struct hostent *host_entery;

    host_entery=gethostbyname(hostname);
    if(host_entery==NULL)
    {
        printf("Gethostbyname is error");
        return -1;
    }

    IPBuffer=inet_ntoa(*((struct in_addr *)host_entery->h_addr_list[0]));
    if(IPBuffer==NULL)
    {
        printf("inet_ntoa is error");
        return -1;
    }

    struct sockaddr_in ServerInfo;
    memset(&ServerInfo, 0, sizeof(ServerInfo));
    ServerInfo.sin_family=AF_INET;
    ServerInfo.sin_port=htons(PORT);
    ServerInfo.sin_addr.s_addr=inet_addr(IPBuffer);

    int erStat = connect(sock, (struct sockaddr *)&ServerInfo, sizeof(ServerInfo));

    if (erStat != 0)
    {
        perror("Connection to server failed");
        close(sock);
        return 1;
    }
    printf("Connection established SUCCESSFULLY.\n");

    // Непрерывное взаимодействие
    char sendBuffer[24] = "Okay";
    char recvBuffer[24];
    int recvResult;
    while (1)
    {
        SendData(sock, sendBuffer, sizeof(sendBuffer));
        recvResult = ReceiveData(sock, recvBuffer, sizeof(recvBuffer));
        if (recvResult <= 0)
        {
            perror("Connection closed or error occurred");
            break;
        }

        printf("Received from Server: %s\n", recvBuffer);
        sleep(5); // Задержка для демонстрации непрерывного обмена данными
    }
    close(sock);
    return 0;
}

