#include"inet.h"

using namespace std;
char* GetIpList()
{
    set<long>* iplist = new set<long>;
    char name[100] = "";
    gethostname(name,sizeof(name));
    struct hostent *remoteHost = gethostbyname(name);
    int i = 0;
    unsigned long ip = 0;
    while(remoteHost && remoteHost->h_addr_list[i])
    {
        ip = *(u_long*)remoteHost->h_addr_list[i++];
        iplist->insert(ip);
    }
    cout<<endl<<remoteHost<<endl;
    return (char*)iplist;
}
char* INet::m_psetLongIpList = nullptr;
//INet.cpp:4:13: Return type of out-of-line definition of 'INet::GetIpList' differs from that in the declaration
//inet.h:39:22: previous definition is here



