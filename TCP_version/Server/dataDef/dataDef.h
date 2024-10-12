#pragma once
#include<memory>
#include<windows.h>
#include<map>
#include<string>
using namespace std;
#define _UDP_PORT 7099
#define _TCP_PORT 10099
#define str_ip   "192.168.14.215"
#define _TCP_IP ("192.168.202.215")
typedef int PackType;
#define _DEF_NAME_LENGTH    (100)
#define _DEF_CONTENT_LENGTH (2048)
#define _DEF_PROTOCOL_COUNT (40)
//id name最大长度
#define  _MAX_FILE_PATH (512)

//定义协议头
#define _DEF_PROTOCOL_BASE (1000)
//上线请求
#define _DEF_UDP_ONLINE_RQ (_DEF_PROTOCOL_BASE + 1)
//上线回复
#define _DEF_UDP_ONLINE_RS (_DEF_PROTOCOL_BASE + 2)
//聊天请求
#define _DEF_UDP_CHAT_RQ (_DEF_PROTOCOL_BASE + 3)
//下线请求
#define _DEF_UDP_OFFLINE_RQ (_DEF_PROTOCOL_BASE + 4)

//TCP
//注册请求
#define _DEF_TCP_REGISTER_RQ (_DEF_PROTOCOL_BASE + 5)
//注册回复
#define _DEF_TCP_REGISTER_RS (_DEF_PROTOCOL_BASE + 6)
//注册结果
#define register_success (0)
#define register_false_tel_used (1)
#define register_false_name_used (2)
#define register_false_sql_select_error (3)
#define register_false_sql_update_error (4)

//登录请求
#define _DEF_TCP_LOGIN_RQ (_DEF_PROTOCOL_BASE + 7)
//登录回复
#define _DEF_TCP_LOGIN_RS (_DEF_PROTOCOL_BASE + 8)
//登录结果
#define login_success (0)
#define login_false_no_tel (1)
#define login_false_password_wrong (2)
#define login_false_sql_select_error (3)
#define login_false_illegal (4)

//Service发送的好友信息
#define _DEF_TCP_FRIEND_INFO (_DEF_PROTOCOL_BASE + 9)
//在线状态
#define status_online (0)
#define status_offline (1)
//聊天请求
#define _DEF_TCP_CHAT_RQ (_DEF_PROTOCOL_BASE + 10)
//聊天回复
#define _DEF_TCP_CHAT_RS (_DEF_PROTOCOL_BASE + 11)
#define chat_send_success (0)
#define chat_send_false (1)

//添加好友请求
#define _DEF_TCP_ADD_FRIEND_RQ (_DEF_PROTOCOL_BASE + 12)
//添加好友回复
#define _DEF_TCP_ADD_FRIEND_RS (_DEF_PROTOCOL_BASE + 13)
//删除好友请求 发给服务器
#define _DEF_TCP_DELETE_FRIEND_RQ (_DEF_PROTOCOL_BASE + 17)
//删除好友操作回复 来自服务器
#define _DEF_TCP_DELETE_FRIEND_RS (_DEF_PROTOCOL_BASE + 18)
#define delete_friend_success (0)
#define delete_friend_false (1)
//朋友删除你的消息
#define _DEF_TCP_FRIEND_DELETE_YOU_MSG (_DEF_PROTOCOL_BASE + 19)
//添加好友的结果
#define add_friend_success (0)
#define add_friend_false_offline (2)
#define add_friend_false_no_exist (1)
#define add_friend_false_disagree (3)
//下线请求
#define _DEF_TCP_OFFLINE_RQ (_DEF_PROTOCOL_BASE + 14)
//服务器->客户端 的登录验证
#define _DEF_TCP_VALIDATE_LOGINED_RQ (_DEF_PROTOCOL_BASE + 15)
//客户端->服务器 的登录验证回复
#define _DEF_TCP_VALIDATE_LOGINED_RS (_DEF_PROTOCOL_BASE + 16)
//发送客户端它的群聊信息
#define _DEF_TCP_SEND_CLIENT_GROUP_INFO (_DEF_PROTOCOL_BASE + 20)
#define group_offline (0)
#define group_online (1)
//发送客户端它的群聊的所有成员信息
#define _DEF_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO (_DEF_PROTOCOL_BASE + 21)
//群聊聊天消息转发
#define _DEF_TCP_GROUP_CHAT_MSG (_DEF_PROTOCOL_BASE + 22)
//接收服务器的文件消息
#define _DEF_TCP_RECV_FILE_MSG (_DEF_PROTOCOL_BASE + 23)
#define file_friend (0)
#define file_group (1)
//服务器发送文件消息请求
#define _DEF_TCP_SEND_FILE_RQ (_DEF_PROTOCOL_BASE + 24)
//发送文件结果
#define _DEF_TCP_SEND_FILE_RES (_DEF_PROTOCOL_BASE + 25)
#define send_file_succeed (0)
#define send_file_failed_ser_recv (1)
#define send_file_failed_ser_send (2)
#define send_file_failed_fri_offline (3)
#define send_file_failed_fri_refuse (4)
#define send_file_failed_gro_offline (5)
//接收服务器的文件消息的回复
#define _DEF_TCP_RECV_FILE_RES (_DEF_PROTOCOL_BASE + 26)
#define user_recv_file_yes (0)
#define user_recv_file_no (1)

//请求协议包

//上线请求：协议头,ip,name
struct _STRU_ONLINE {
    _STRU_ONLINE() :type(_DEF_UDP_ONLINE_RQ)
        //,ip(0)
    {
        memset(name, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    //long ip;
    char name[_DEF_NAME_LENGTH];
};

//聊天请求：协议头,ip,message
struct _STRU_CHAT_RQ {
    _STRU_CHAT_RQ() :type(_DEF_UDP_CHAT_RQ)
        //,ip(0)
    {
        memset(content, 0, _DEF_CONTENT_LENGTH);
    }
    PackType type;
    //long ip;
    char content[_DEF_CONTENT_LENGTH];
};
//下线请求：协议头,ip,name

struct _STRU_OFFLINE_RQ {
    _STRU_OFFLINE_RQ() :type(_DEF_UDP_OFFLINE_RQ)//,ip(0)
    {
    }
    PackType type;
    //long ip;
};




//TCP数据报结构体

//注册请求---type 电话 昵称 密码
struct _STRU_TCP_REGISTER_RQ {
    _STRU_TCP_REGISTER_RQ
    () :type(_DEF_TCP_REGISTER_RQ)
    {
        memset(tel, 0, _DEF_NAME_LENGTH);
        memset(name, 0, _DEF_NAME_LENGTH);
        memset(password, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    char tel[_DEF_NAME_LENGTH];
    char name[_DEF_NAME_LENGTH];
    char password[_DEF_NAME_LENGTH];
};

//注册回复
struct _STRU_TCP_REGISTER_RS {
    _STRU_TCP_REGISTER_RS
    ()
        :type(_DEF_TCP_REGISTER_RS)
        , m_nRegister(register_false_name_used)//默认失败
    {}
    PackType type;
    int m_nRegister; //注册结果---成功(1) 失败1电话重复(1) 失败2昵称重复(2)

};
//登录请求
struct _STRU_TCP_LOGIN_RQ {
    _STRU_TCP_LOGIN_RQ
    () :type(_DEF_TCP_LOGIN_RQ),userId(-1),bOnlineStatus(false)
    {
        memset(tel, 0, _DEF_NAME_LENGTH);
        memset(password, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int userId;
    char tel[_DEF_NAME_LENGTH];
    char password[_DEF_NAME_LENGTH];
    bool bOnlineStatus;
};
//登录回复
struct _STRU_TCP_LOGIN_RS {
    _STRU_TCP_LOGIN_RS
    () :type(_DEF_TCP_LOGIN_RS), m_nLogin(login_false_no_tel),userId(0),sock(0)
    {

    }
    PackType type;
    int userId;
    int m_nLogin; 
    long sock;
    //#define login_success (0)#define login_false_no_tel (1)
    //#define login_false_password_wrong (2)

};
//Service发送的好友信息---type id name iconid feeling 在线状态
struct _STRU_TCP_FRIEND_INFO {
    _STRU_TCP_FRIEND_INFO
    () :type(_DEF_TCP_FRIEND_INFO), id(0), iconId(0), status(status_offline)
    {
        memset(name, 0, _DEF_NAME_LENGTH);
        memset(feeling, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int id;
    int iconId;
    int status;
    char name[_DEF_NAME_LENGTH];
    char feeling[_DEF_NAME_LENGTH];
    map<int,string>hisGroupMap;
};
//聊天请求---type 聊天内容content 服务器转发对象toId 服务器转发自谁fromId
struct _STRU_TCP_CHAT_RQ {
    _STRU_TCP_CHAT_RQ
    () :type(_DEF_TCP_CHAT_RQ), fromId(0), toId(0)
    {
        memset(content, 0, _DEF_CONTENT_LENGTH);
    }
    PackType type;
    int fromId;
    int toId;
    char content[_DEF_CONTENT_LENGTH];
};
//聊天回复---type 聊天结果（聊天失败时收到聊天回复）
struct _STRU_TCP_CHAT_RS {
    _STRU_TCP_CHAT_RS
    () :type(_DEF_TCP_CHAT_RS), m_nChatRes(chat_send_false)
        , fromId(-1)
    {

    }
    PackType type;
    int fromId;
    int m_nChatRes;
};
//添加好友请求---type 昵称name(只支持根据昵称添加好友) 自己的id 自己的name
struct _STRU_TCP_ADD_FRIEND_RQ {
    _STRU_TCP_ADD_FRIEND_RQ
    () :type(_DEF_TCP_ADD_FRIEND_RQ), fromId(0)
    {
        memset(fromName, 0, _DEF_NAME_LENGTH);
        memset(toName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int fromId;
    char fromName[_DEF_NAME_LENGTH];
    char toName[_DEF_NAME_LENGTH]; //加谁为好友
};
//添加好友回复---type 添加结果 自己id 好友f_id 好友昵称f_name
struct _STRU_TCP_ADD_FRIEND_RS {
    _STRU_TCP_ADD_FRIEND_RS
    () :type(_DEF_TCP_ADD_FRIEND_RS), m_nAddRes(add_friend_false_no_exist)
        , fromId(0), friendId(0)
    {
        memset(friendName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int fromId;
    int friendId;
    char friendName[_DEF_NAME_LENGTH];
    int m_nAddRes;
};
//删除好友请求
struct _STRU_TCP_DELETE_FRIEND_RQ {
    _STRU_TCP_DELETE_FRIEND_RQ()
        :type(_DEF_TCP_DELETE_FRIEND_RQ), userId(-1)
    {
        memset(friendName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int userId;
    char friendName[_DEF_NAME_LENGTH];
};
//删除好友回复
struct _STRU_TCP_DELETE_FRIEND_RS {
    _STRU_TCP_DELETE_FRIEND_RS()
        :type(_DEF_TCP_DELETE_FRIEND_RS), friendId(-1), userId(-1), deleteRes(delete_friend_false)
    {
        memset(friendName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int userId;
    int friendId;
    char friendName[_DEF_NAME_LENGTH];
    int deleteRes;
};

//朋友删除你的msg
struct _STRU_TCP_FRIEND_DELETE_YOU_MSG {
    _STRU_TCP_FRIEND_DELETE_YOU_MSG() :friendId(-1), type(_DEF_TCP_FRIEND_DELETE_YOU_MSG) {}
    PackType type;
    int friendId;
};

//发送客户端他加入的一个群信息
struct _STRU_TCP_SEND_CLIENT_GROUP_INFO {
    _STRU_TCP_SEND_CLIENT_GROUP_INFO():type(_DEF_TCP_SEND_CLIENT_GROUP_INFO)
        ,groupId(-1),groupIconid(1),groupStatus(group_offline)
    {
        memset(groupName,0,_DEF_NAME_LENGTH);
    }
    PackType type;
    int groupId;
    int groupIconid;
    char groupName[_DEF_NAME_LENGTH];
    int groupStatus;
};

//发送客户端他加入的群的一个成员的信息
struct _STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO {
    _STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO() :type(_DEF_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO)
        , groupId(-1), groupIconid(1), memberId(-1), memberIconid(0), memberStatus(status_offline), groupStatus(group_offline)
    {
        memset(groupName, 0, _DEF_NAME_LENGTH);
        memset(memberName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int groupId;
    int groupIconid;
    int memberId;
    int memberIconid;
    int memberStatus;
    int groupStatus;
    char groupName[_DEF_NAME_LENGTH];
    char memberName[_DEF_NAME_LENGTH];
};



//客户端1发送文件的请求
struct _STRU_TCP_CLIENT_SEND_FILE_RQ
{
    _STRU_TCP_CLIENT_SEND_FILE_RQ()
        :type(_DEF_TCP_SEND_FILE_RQ), toFriOrGop(file_friend), id(-1), userId(-1)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
    }
    PackType type;
    int toFriOrGop;
    int id;
    int userId;
    char szFileId[_MAX_FILE_PATH];
};

//客户端2接收文件的消息
struct _STRU_TCP_CLIENT_RECV_FILE_MSG {
    _STRU_TCP_CLIENT_RECV_FILE_MSG()
        :type(_DEF_TCP_RECV_FILE_MSG), fromFriOrGop(file_friend), id(-1) 
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
    }
    PackType type;
    int fromFriOrGop;
    int id;
    char szFileId[_MAX_FILE_PATH];
};

//客户端2接收文件的回复
struct _STRU_TCP_CLIENT_RECV_FILE_RES {
    _STRU_TCP_CLIENT_RECV_FILE_RES()
        :type(_DEF_TCP_RECV_FILE_RES), fromFriOrGop(file_friend), id(-1),userId(-1)
        ,userRecvRes(user_recv_file_no)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
    }
    PackType type;
    int fromFriOrGop;
    int id;
    int userId;
    int userRecvRes;
    char szFileId[_MAX_FILE_PATH];
};

//客户端1发送文件结果 服务器已转发 转发失败
struct _STRU_TCP_SEND_FILE_RES
{
    _STRU_TCP_SEND_FILE_RES()
        :type(_DEF_TCP_SEND_FILE_RES), toFriOrGop(file_friend), id(-1), sendRes(send_file_failed_ser_recv)
    {}
    PackType type;
    int toFriOrGop;
    int id;
    int sendRes;
};



//群聊聊天消息
struct _STRU_TCP_GROUP_CHAT_MSG {
    _STRU_TCP_GROUP_CHAT_MSG()
        :type(_DEF_TCP_GROUP_CHAT_MSG), memberId(-1), groupId(-1)
    {
        memset(content, 0, _DEF_CONTENT_LENGTH);
    }
    PackType type;
    int memberId;
    int groupId;
    char content[_DEF_CONTENT_LENGTH];
};

//下线请求----type 自己的id
struct _STRU_TCP_OFFLINE_RQ {
    _STRU_TCP_OFFLINE_RQ
    () :type(_DEF_TCP_OFFLINE_RQ), id(0)
    {

    }
    PackType type;
    int id;
};
//验证登录请求
struct _STRU_TCP_VALIDATE_RQ {
    _STRU_TCP_VALIDATE_RQ
    () :type(_DEF_TCP_VALIDATE_LOGINED_RQ), id(0)
    {

    }
    PackType type;
    int id;
};
//验证登录回复
struct _STRU_TCP_VALIDATE_RS {
    _STRU_TCP_VALIDATE_RS
    () :type(_DEF_TCP_VALIDATE_LOGINED_RS), id(0), status(false)
    {

    }
    PackType type;
    int id;
    bool status;
};
//用户信息表t_user: (id int(自增 主键)查找快,tel,name,password,iconid,feeling)
//好友关系表t_friend:(id1,id2,) 为了快速采用双向存储

//发送文件相关的：



//文件请求
#define _DEF_PROTOCOL_FILE_INFO_RQ (_DEF_PROTOCOL_BASE + 100)
//文件块
#define _DEF_PROTOCOL_FILE_BLOCK_RQ (_DEF_PROTOCOL_BASE + 101)
//一个文件服务器接收成功时发送文件id给Kernel 记录在map中
#define _DEF_PROTOCOL_FILE_RECV_ACC (_DEF_PROTOCOL_BASE + 102)
//一个文件服务器转发成功时发送文件id给Kernel 记录在map中
#define _DEF_PROTOCOL_FILE_RESEND_ACC (_DEF_PROTOCOL_BASE + 103)

//id name最大长度
#define  _MAX_FILE_PATH (512)
//文件传输块的最大长度
#define  _MAX_FILE_CONTENT_SIZE (8 * 1024)

//协议结构体
//文件信息请求：协议头 文件名 文件大小 文件的唯一标识id
struct STRU_FILE_INFO_RQ {
    STRU_FILE_INFO_RQ() :nType(_DEF_PROTOCOL_FILE_INFO_RQ), szFileSize(0)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
        memset(szFileName, 0, _MAX_FILE_PATH);
    }
    int nType;
    char szFileId[_MAX_FILE_PATH];
    char szFileName[_MAX_FILE_PATH];
    long long szFileSize; // long long可以发送
};
//文件块
struct STRU_FILE_BLOCK_RQ {
    STRU_FILE_BLOCK_RQ() :nType(_DEF_PROTOCOL_FILE_BLOCK_RQ), nBlockSize(0)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
        memset(szFileContent, 0, _MAX_FILE_CONTENT_SIZE);
    }
    int nType;
    char szFileId[_MAX_FILE_PATH];
    char szFileContent[_MAX_FILE_CONTENT_SIZE];
    int nBlockSize;
};

//本地保存的文件信息:文件标识 文件名 文件路径 当前位置 总大小 文件指针 
struct STRU_FILE_INFO {
    STRU_FILE_INFO() :nPos(0), nFileSize(0), pFile(nullptr)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
        memset(szFileName, 0, _MAX_FILE_PATH);
        memset(szFilePath, 0, _MAX_FILE_PATH);
    }
    char szFileId[_MAX_FILE_PATH];
    char szFileName[_MAX_FILE_PATH];
    char szFilePath[_MAX_FILE_PATH];
    long long nPos; //当前在什么位置
    long long nFileSize; //文件总大小
    FILE* pFile; //文件指针
};

//一个文件写入成功时发送文件id给Kernel 记录在map中
struct STRU_FILE_RECV_ACC_MSG {
    STRU_FILE_RECV_ACC_MSG()
    :type(_DEF_PROTOCOL_FILE_RECV_ACC)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
    }
    int type;
    char szFileId[_MAX_FILE_PATH];
};

//一个文件转发成功时发送文件id给Kernel 记录在map中
struct STRU_FILE_RESEND_ACC_MSG {
    STRU_FILE_RESEND_ACC_MSG()
        :type(_DEF_PROTOCOL_FILE_RESEND_ACC)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
    }
    int type;
    char szFileId[_MAX_FILE_PATH];
};