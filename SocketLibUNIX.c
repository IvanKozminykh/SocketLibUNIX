#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern int PORT;
extern int SOCK;

int CreateSock()
{
    struct sockaddr
    {
        unsigned sa_family;
        char sa_data[14];
    };

    struct sockaddr_in
    {
        short sin_family;
        unsigned sin_port;
        struct in_addr sin_addr;
        char sin_zero[8];
    };



    int sock;
    struct sockaddr_in;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock==-1)
    {
        perror("Socket is error\n");
        return -1;
    }

}

int SendData(int sock, const char* data, int dataLen)
{
    int iResult=send(sock, data, dataLen, 0);
    if(iResult==-1)
    {
        printf("Send error\n");
        close(sock);
        return -1;
    }
    else
    {
        printf("Send: %d\n", iResult);
    }
}

int ReciveData(int sock, char* buffer, int bufferLen)
{
    int iResult=recv(sock, buffer, bufferLen, 0);
    if(iResult>0)
    {
        printf("Recived: %d\n", iResult);
    }
    else if(iResult==0)
    {
        printf("Connection closed\n");
    }
    else
    {
        printf("Recived failed\n");
    }
}

int Server(int PORT, int sock)
{
    char hostname[256];
    char ipname[256];

    if(gethostname(hostname, sizeof(hostname))==-1)
    {
        perror("GetHostName is error\n");
    }
    else
    {
        printf("Hostname: %s\n", hostname);
    }

    struct hostent* HostInfo;

    HostInfo=gethostbyname(hostname);

    inet_ntop(AF_INET, HostInfo->h_addr_list[0], ipname, sizeof(ipname));
    printf("IP: %s\n", ipname);


    struct sockaddr_in saServer;

    saServer.sin_family=AF_INET;
    saServer.sin_port=htons(PORT);
    saServer.sin_addr.s_addr=inet_addr(ipname);

    int erStat=bind(sock, (struct sockaddr*)&saServer, sizeof(saServer));

    if(erStat!=0)
    {
        printf("Error socket binding to server info \n");
        close(sock);
        return -1;
    }
    else
    {
        printf("Binding socket to server is good\n");
    }
    
    erStat=listen(sock, SOMAXCONN);

    if(erStat!=0)
    {
        printf("Cant start to listen\n");
    }
    else
    {
        printf("Listening...\n");
    }

    struct sockaddr_in ClientInfo;

    memset(&ClientInfo, 0, sizeof(ClientInfo));

    int ClientInfoLen=sizeof(ClientInfo);

    //zeromemory(&ClientInfo, sizeof(ClientInfo));

    int ClientConn=accept(sock, (struct sockaddr*)&ClientInfo, &ClientInfoLen);

    if(ClientConn<0)
    {
        printf("Accept is failed\n");
        close(sock);
        close(ClientConn);
        return -1;
    }
    else
    {
        printf("Accept is good\n");
    }

    char recvBuffer[1024];
    char sendBuffer[1024]="Yes";
    int recvResult;
    while(1)
    {
        recvResult=ReciveData(ClientConn, recvBuffer, sizeof(recvBuffer));
        if(recvResult<=0)
        {
            printf("Connection closed or error occured\n");
            break;
        }
        recvBuffer[recvResult]='\0';
        printf("Recived from client: %s\n", recvBuffer);
        SendData(ClientConn, sendBuffer, sizeof(sendBuffer));
    }
    close(ClientConn);
    close(sock);
    return 0;
}

int Client(int PORT, int sock)
{
    char hostname[256];
    char ipname[256];

    if(gethostname(hostname, sizeof(hostname))==-1)
    {
        perror("GetHostName is error\n");
    }
    else
    {
        printf("Hostname: %s\n", hostname);
    }

    struct hostent* HostInfo;

    HostInfo=gethostbyname(hostname);

    inet_ntop(AF_INET, HostInfo->h_addr_list[0], ipname, sizeof(ipname));
    printf("IP: %s\n", ipname);


    struct  sockaddr_in ServerInfo;

    //ZeroMemory(&ServerInfo, sizeof(ServerInfo));

    ServerInfo.sin_family=AF_INET;
    ServerInfo.sin_port=htons(PORT);
    ServerInfo.sin_addr.s_addr=inet_addr(ipname);

    int erStat=connect(sock, (struct sockaddr*)&ServerInfo, sizeof(ServerInfo));

    if (erStat!=0)
    {
        printf("Connection to server is failed\n");
        close(sock);
        return -1;
    }
    else
    {
        printf("Connection established SUCCESSFULLY\n");
    }

    char sendBuffer[1024]="Okay";
    char recvBuffer[1024];
    int recvResult;
    while(1)
    {
        SendData(sock, sendBuffer, sizeof(sendBuffer));
        recvResult=ReciveData(sock, recvBuffer, sizeof(recvBuffer));
        if(recvResult<=0)
        {
            printf("Connection closed or error occured\n");
            break;
        }

        recvBuffer[recvResult]='\0';
        printf("Recived from Server: %s\n", recvBuffer);
        sleep(10);
    }
    close(sock);
    return 0;



}
