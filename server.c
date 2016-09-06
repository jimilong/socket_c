#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h> // socket
#include <sys/types.h>  // 基本数据类型
#include <unistd.h> // read write
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> // open close
#include <sys/shm.h>
#include <arpa/inet.h>

#include <signal.h>

#define PORT 8888
#define SERV "127.0.0.1"
#define QUEUE 20
#define BUFF_SIZE 1024

#define NET_PACKET_DATA_SIZE 1024   
#define NET_PACKET_SIZE (sizeof(NetPacketHeader) + NET_PACKET_DATA_SIZE) * 10
#define ERR_BADPARAM 0
#define SOCKET_ERROR -1


/// 网络数据包包头  
struct NetPacketHeader  
{  
    unsigned short      nDataLen;  ///< 数据包大小，包含封包头和封包数据大小  
    //unsigned short      wOpcode;    ///< 操作码  
};  
  
/// 网络数据包  
struct NetPacket  
{  
    //struct NetPacketHeader     Header;                         ///< 包头  
    unsigned short      nDataLen;  ///< 数据包大小，包含封包头和封包数据大小  
    unsigned char       Data[NET_PACKET_DATA_SIZE];     ///< 数据  
};

typedef struct NetPacketHeader PACKAGE_HEAD;
typedef struct NetPacket Packet;

///获取定长网络数据
int RecvSize( int sock_client, char* strData, int iLen )
{
    if( strData == NULL )
    return ERR_BADPARAM;

    char *p = strData;
    int len = iLen;
    int ret = 0;
    int returnlen = 0;
    while( len > 0)
    {
        ret = recv( sock_client, p+(iLen-len), iLen-returnlen, 0 );
        if ( ret == SOCKET_ERROR || ret == 0 )
        {
            return ret;
        }

        len -= ret;
        returnlen += ret;
    }

    return returnlen;
}

int sockfd;
void handle_signal(int sign); // 退出信号处理
void http_send(int sock,char *content); // http 发送相应报文

int main(int argc, char **argv) 
{
    /* Check command line args */
    if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
    }

    int port = atoi(argv[1]);

    signal(SIGINT,handle_signal);

    int count = 0; // 计数
    // 定义 socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    // 定义 sockaddr_in
    struct sockaddr_in skaddr;
    skaddr.sin_family = AF_INET; // ipv4
    skaddr.sin_port   = htons(port);
    skaddr.sin_addr.s_addr = inet_addr(SERV);
    // bind，绑定 socket 和 sockaddr_in
    if( bind(sockfd,(struct sockaddr *)&skaddr,sizeof(skaddr)) == -1 ){
            perror("bind error");
            exit(1);
    }

    // listen，开始添加端口
    if( listen(sockfd,QUEUE) == -1 ){
            perror("listen error");
            exit(1);
    }

    // 客户端信息
    //char buff[BUFF_SIZE];
    struct sockaddr_in claddr;
    socklen_t length = sizeof(claddr);
    ////
    char PackageHead[1024];
    char PackageContext[1024*20];

    fd_set read_set,ready_set;
    FD_ZERO(&read_set);
    FD_SET(sockfd, &read_set);
    int len;
    PACKAGE_HEAD *pPackageHead;
    int i = 1;
    while(1)
    {
        ready_set = read_set;
        select(sockfd + 1, &ready_set, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &ready_set)) {
            int sock_client = accept(sockfd,(struct sockaddr *)&claddr, &length);
            printf("%d\n",++count);
            if( sock_client <0 ){
                    perror("accept error");
                    exit(1);
            }

            if (fork() == 0) {
                close(sockfd);
                while(1) {
                    memset(PackageHead,0,sizeof(PACKAGE_HEAD));
                    len = RecvSize(sock_client, (char*)PackageHead,sizeof(PACKAGE_HEAD));
                    printf("header_client_len:%d\n", len);
                    if( len == SOCKET_ERROR )
                    {
                        break;
                    }
                    if(len == 0)
                    {
                        break;
                    }
                    pPackageHead = (PACKAGE_HEAD *)PackageHead;
                    printf("data_client_len:%d\n", pPackageHead->nDataLen);
                    memset(PackageContext,0,sizeof(PackageContext));
                    if(pPackageHead->nDataLen>0)
                    {
                        len = RecvSize(sock_client, (char*)PackageContext,pPackageHead->nDataLen);
                    }
                    printf("data from client %d\n", i);
                    printf("revc_client-data:%s-%d\n", PackageContext, i);
                    /////////
                    Packet packet;
                    memset(&packet,0,sizeof(Packet));
                    char *pData = "hello socket client!";
                    memcpy(packet.Data, pData, NET_PACKET_DATA_SIZE);
                    packet.nDataLen = NET_PACKET_DATA_SIZE;
                    send(sock_client, (char *)&packet, sizeof(packet), 0);
                    printf("send success\n");
                }
                close(sock_client);
                exit(0);
            }
            
            i++;
            close(sock_client);
        }
    }
    fputs("Bye Cleey",stdout);
    close(sockfd);
    return 0;
}


///////////////////////////////////



typedef struct doc_type{
        char *key;
        char *value;
}HTTP_CONTENT_TYPE;

HTTP_CONTENT_TYPE http_content_type[] = {
        { "html","text/html" },
        { "gif" ,"image/gif" },
        { "jpeg","image/jpeg" }
};

int sockfd;
char *http_res_tmpl = "HTTP/1.1 200 OK\r\n"
        "Server: Cleey's Server V1.0\r\n"
    "Accept-Ranges: bytes\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Content-Type: %s\r\n\r\n";

void handle_signal(int sign); // 退出信号处理
void http_send(int sock,char *content); // http 发送相应报文

int main111(int argc, char **argv) 
{

    /* Check command line args */
    if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
    }

    int port = atoi(argv[1]);

    signal(SIGINT,handle_signal);

    int count = 0; // 计数
    // 定义 socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    // 定义 sockaddr_in
    struct sockaddr_in skaddr;
    skaddr.sin_family = AF_INET; // ipv4
    skaddr.sin_port   = htons(port);
    skaddr.sin_addr.s_addr = inet_addr(SERV);
    // bind，绑定 socket 和 sockaddr_in
    if( bind(sockfd,(struct sockaddr *)&skaddr,sizeof(skaddr)) == -1 ){
            perror("bind error");
            exit(1);
    }

    // listen，开始添加端口
    if( listen(sockfd,QUEUE) == -1 ){
            perror("listen error");
            exit(1);
    }

    // 客户端信息
    char buff[BUFF_SIZE];
    struct sockaddr_in claddr;
    socklen_t length = sizeof(claddr);


    while(1){
            int sock_client = accept(sockfd,(struct sockaddr *)&claddr, &length);
            printf("%d\n",++count);
            if( sock_client <0 ){
                    perror("accept error");
                    exit(1);
            }
            memset(buff,0,sizeof(buff));
            recv(sock_client,buff,sizeof(buff),0);
            fputs(buff,stdout);
            //send(sock_client,buff,len,0);
            http_send(sock_client,"Hello World!");
            close(sock_client);
    }
    fputs("Bye Cleey",stdout);
    close(sockfd);
    return 0;
}

void http_send(int sock_client,char *content){
        char HTTP_HEADER[BUFF_SIZE],HTTP_INFO[BUFF_SIZE];
        int len = strlen(content);
        sprintf(HTTP_HEADER,http_res_tmpl,len,"text/html");
        len = sprintf(HTTP_INFO,"%s%s",HTTP_HEADER,content);

        send(sock_client,HTTP_INFO,len,0);
}

void handle_signal(int sign){
        fputs("\nSIGNAL INTERRUPT \nBye Cleey! \nSAFE EXIT\n",stdout);
        close(sockfd);
        exit(0);
}