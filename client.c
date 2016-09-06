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
#define SERV "0.0.0.0"
#define QUEUE 20
#define BUFF_SIZE 1024

#define NET_PACKET_DATA_SIZE 1024   
#define NET_PACKET_SIZE (sizeof(NetPacketHeader) + NET_PACKET_DATA_SIZE) * 10

/// 网络数据包包头  
struct NetPacketHeader  
{  
    unsigned short      nDataLen;  ///< 数据包大小，包含封包头和封包数据大小  
    //unsigned short      wOpcode;    ///< 操作码  
};  
  
/// 网络数据包  
struct NetPacket  
{  
    unsigned short      nDataLen;  ///< 数据包大小，包含封包头和封包数据大小  
    //unsigned short      wOpcode;    ///< 操作码                         ///< 包头  
    unsigned char       Data[NET_PACKET_DATA_SIZE];     ///< 数据  
};

typedef struct NetPacketHeader PACKAGE_HEAD;
typedef struct NetPacket Packet;


#define HELLO_WORLD_SERVER_PORT    6666 
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
#define ERR_BADPARAM 0
#define SOCKET_ERROR -1

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
 
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: ./%s ServerIPAddress\n",argv[0]);
        exit(1);
    }
 
    //设置一个socket地址结构client_addr,代表客户机internet地址, 端口
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr)); //把一段内存区的内容全部设置为0
    client_addr.sin_family = AF_INET;    //internet协议族
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
    client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
    //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    //把客户机的socket和客户机的socket地址结构联系起来
    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        exit(1);
    }
 
    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(argv[1],&server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);
    
	    //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
	    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
	    {
	        printf("Can Not Connect To %s!\n",argv[1]);
	        exit(1);
	    }
    
		Packet packet;
	    memset(&packet,0,sizeof(Packet));
	    char *pData = "hello socket server!";
	    memcpy(packet.Data, pData, NET_PACKET_DATA_SIZE);
	    packet.nDataLen = NET_PACKET_DATA_SIZE;
	    //向服务器发送buffer中的数据
	    send(client_socket, (char *)&packet, sizeof(packet), 0);
	    printf("send success\n");

	    sleep(1);
	    int len;
	    PACKAGE_HEAD *pPackageHead;
	    char PackageHead[1024];
	    char PackageContext[1024*20];
	    memset(PackageHead,0,sizeof(PACKAGE_HEAD));
	    len = RecvSize(client_socket, (char*)PackageHead,sizeof(PACKAGE_HEAD));
	    printf("header_server_len:%d\n", len);
	    if( len == SOCKET_ERROR )
	    {
	    	fputs("error",stdout);
	        return SOCKET_ERROR;
	    }
	    if(len == 0)
	    {
	    	fputs("error",stdout);
	        return -1;
	    }
	    pPackageHead = (PACKAGE_HEAD *)PackageHead;
	    printf("data_server_len:%d\n", pPackageHead->nDataLen);
	    memset(PackageContext,0,sizeof(PackageContext));
	    if(pPackageHead->nDataLen>0)
	    {
	        len = RecvSize(client_socket, (char*)PackageContext,pPackageHead->nDataLen);
	    }
	    printf("revc_server-data:%s\n", PackageContext);
	    //关闭socket
    	close(client_socket);
 
    
    return 0;
}