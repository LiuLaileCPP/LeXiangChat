#pragma once
#include<Winsock2.h>
#include<WS2tcpip.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<list>
#include<map>
#include<list>
#include<set>
#include<errno.h>

#define RECV_BUF_SIZE 1024
#define SEND_BUF_SIZE 1024
#define _DEF_MSG_SIZE_ 4096
#define SERVER_IP "192.168.71.215"
#define SERVER_PORT 8080
#define CLIENT_PORT 10080
#define _DEF_WORK_TYPE_SIZE_ 1000
#define _DEF_MAX_ONLINE_CLIENTS_ 1000

//群聊的最大数量
#define _DEF_MAX_GROUP_SIZE 1000
#define _DEF_PROTO_SIZE_ 1000

//数据包协议类型
#define _DEF_REG_RQ_ 1
#define _DEF_REG_RS_ 2
#define _DEF_LOG_RQ_ 3
#define _DEF_LOG_RS_ 4
#define _DEF_ADD_FRIEND_RQ_ 6
#define _DEF_ADD_FRIEND_RS_ 7
#define _DEF_DELETE_FRIEND_NOTICE_ 8
#define _DEF_OFFLINE_NOTICE_ 9
#define _DEF_ALIVE_MSG_ 10
#define _DEF_FRI_INFO_ 11
#define _DEF_GRO_INFO_ 12
#define _DEF_GROMEM_INFO_ 13
#define _DEF_CHAT_MSG_ 14
#define _DEF_CHAT_GRO_MSG_ 23
#define _DEF_ADD_FRI_RQ_ 15
#define _DEF_ADD_FRI_RS_ 16
#define _DEF_DEL_FRI_RQ_ 17
#define _DEF_QUIT_GRO_RQ_ 18
#define _DEF_ADD_FRI_TO_GRO_ 19
#define _DEF_DEL_A_GRO_MEM_ 20
#define _DEF_CREATE_GRO_RQ_ 21
#define _DEF_CREATE_GRO_RS_ 22

#define _DEF_SERVER_ERR_ 402
#define _DEF_CLIENT_NO_ 997
#define TYPE_ALIVE 999
#define TYPE_RECORD_ADDR 998

//#define _DEF_REG_RQ_ 1
//#define _DEF_REG_RQ_ 1
//#define _DEF_REG_RQ_ 1
//#define _DEF_REG_RQ_ 1



//inet 的sendbuf和recvbuf
struct recvBuf {
    recvBuf()
    {
        type = data1 = data2 = data3 = data4 = no = -1;
        res1 = res2 = res3 = res4 = false;
        memset(buf1, 0, RECV_BUF_SIZE);
        memset(buf2, 0, RECV_BUF_SIZE);
        memset(buf3, 0, RECV_BUF_SIZE);
        memset(buf4, 0, RECV_BUF_SIZE);
    }
    int type;
    int data1;
    int data2;
    int data3;
    int data4;
    int no;
    bool res1;
    bool res2;
    bool res3;
    bool res4;
    char buf1[RECV_BUF_SIZE];
    char buf2[RECV_BUF_SIZE];
    char buf3[RECV_BUF_SIZE];
    char buf4[RECV_BUF_SIZE * 8];
};

struct sendBuf {
    sendBuf()
    {
        type = data1 = data2 = data3 = data4 = no = -1;
        res1 = res2 = res3 = res4 = false;
        memset(buf1, 0, RECV_BUF_SIZE);
        memset(buf2, 0, RECV_BUF_SIZE);
        memset(buf3, 0, RECV_BUF_SIZE);
        memset(buf4, 0, RECV_BUF_SIZE);
    }
    int type;
    int data1;
    int data2;
    int data3;
    int data4;
    int no;
    bool res1;
    bool res2;
    bool res3;
    bool res4;
    char buf1[RECV_BUF_SIZE];
    char buf2[RECV_BUF_SIZE];
    char buf3[RECV_BUF_SIZE];
    char buf4[RECV_BUF_SIZE * 8];
};

struct friMsg {

    friMsg() :type(_DEF_CHAT_MSG_), fromId(-1), toId(-1), res(false)
    {
        memset(fromName, 0, 20);
        memset(buf, 0, _DEF_MSG_SIZE_);
    }
    int type;
    int fromId;
    int toId;
    bool res;
    char fromName[20];
    char buf[_DEF_MSG_SIZE_];
};


struct groMsg
{
    groMsg() :type(_DEF_CHAT_GRO_MSG_), fromId(-1), toId(-1), gId(-1), res(false)
    {
        memset(fromName, 0, 20);
        memset(buf, 0, _DEF_MSG_SIZE_);
    }
    int type;
    int fromId;
    int toId;
    int gId;
    bool res;
    char fromName[20];
    char buf[_DEF_MSG_SIZE_];
};
