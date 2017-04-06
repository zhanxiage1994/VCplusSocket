#include "winsock2.h"  
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <fstream>
#include <opencv2\opencv.hpp>

using namespace std;  

#define SERVER
//#define CLIENT
#define IP_ADDR  "127.0.0.1"

class ServerSock
{
public:
	ServerSock();
	~ServerSock();
	SOCKET getClient() {return sClient;};

private:
	static const int PORT = 10086;
	int             retVal;        
	SOCKET          sServer;       
	SOCKET          sClient;        
	SOCKADDR_IN     addrServ;;    
};

ServerSock::ServerSock()
{
	WSADATA wsd;           
	
	if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)  
	{  
		cout << "WSAStartup failed!" << endl;  
	}  
	  
	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);      
	if(INVALID_SOCKET == sServer)  
	{  
		cout << "socket failed!" << endl;  
		WSACleanup();
	}  

	addrServ.sin_family = AF_INET;  
	addrServ.sin_port = htons(PORT);  
	addrServ.sin_addr.s_addr = INADDR_ANY;        

	retVal = bind(sServer, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));  
	if(SOCKET_ERROR == retVal)  
	{     
		cout << "bind failed!" << endl;  
		closesocket(sServer); 
		WSACleanup();        
	}  
	//开始监听   
	cout<<"start listen!"<<endl;
	retVal = listen(sServer, 1);  
	if(SOCKET_ERROR == retVal)  
	{  
		cout << "listen failed!" << endl;         
		closesocket(sServer);  
		WSACleanup();         
	}  
	//接受客户端请求  
	sockaddr_in addrClient;  
	int addrClientlen = sizeof(addrClient);  
	sClient = accept(sServer,(sockaddr FAR*)&addrClient, &addrClientlen);  
	cout<<"accept!"<<endl;
	if(INVALID_SOCKET == sClient)  
	{  
		cout << "accept failed!" << endl;         
		closesocket(sServer);  
		WSACleanup();          
	} 
}

ServerSock::~ServerSock()
{
	closesocket(sServer);  
	//closesocket(sClient);  
	WSACleanup();			
}

class SockData
{
public:
	SockData(SOCKET _client) { client = _client;};
	~SockData()  { closesocket(client); };
	bool sendData(const char sData);
	bool sendData(int sData);
	bool sendData(char* sData,int Len = BUFSIZE);
	bool sendData(const string sfileRoad);
	int recvData();
	bool recvData(char* buf, int Len = BUFSIZE);
	bool recvData(const string rfileRoad);

private:
	static const int BUFSIZE = 1024;		
	int             retVal;       
	SOCKET          client;      
};


bool SockData::sendData(const char sData)
{
	retVal = send(client,&sData,sizeof(char),0);
	if(retVal==SOCKET_ERROR)
	{
		cout<<"Send Into error: "<<GetLastError()<<endl;
		cout<<"Fail char: "<<sData<<endl;
		return false;
	}
	return true;
}

bool SockData::sendData(int sData)
{
	stringstream strTemp;strTemp.clear();strTemp.str("");
	strTemp << sData;
	const string sTemp = strTemp.str();
	const char* cTemp = sTemp.c_str();
	if(! sendData(char('0'+strlen(cTemp))))
	{
		cout<<"In trouble"<<endl;
	}
	retVal = send(client,cTemp,sizeof(char) * strlen(cTemp),0);
	if(retVal==SOCKET_ERROR)
	{
		cout<<"Send Into error: "<<GetLastError()<<endl;
		cout<<"Fail int: "<<sData<<endl;
		return false;
	}
	return true;
}

bool SockData::sendData(char* sData,int Len)
{
	retVal=send(client,sData,sizeof(char) * Len,0);
	if(retVal==SOCKET_ERROR)
	{
		cout<<"Send Into error: "<<GetLastError()<<endl;
		return false;
	}
	return true;
}

bool SockData::sendData(const string sfileRoad)
{
	ifstream in(sfileRoad,fstream::in|fstream::binary);
	if(!in.is_open())
	{
		cout<<"fail open file."<<endl;
		return false;
	}
	in.seekg (0, ios::end);  
	int length = in.tellg();  
	in.seekg (0, ios::beg); 

	sendData(length);
	char* buf = new char[BUFSIZE]; 
	ZeroMemory(buf, BUFSIZE);

	while(!in.eof())
	{
		in.read(buf,BUFSIZE);
		retVal=send(client,buf,sizeof(char) * BUFSIZE,0);
		if(retVal==SOCKET_ERROR)
		{
			cout<<"Send Into error: "<<GetLastError()<<endl;
			return false;
		}
	}
	in.close();
	delete[] buf;

	return true;
}

int SockData::recvData()
{
	char intBit;
	retVal = recv(client,&intBit,sizeof(char),0);
	if(retVal == SOCKET_ERROR)
	{
		cout<<"Recv Into error: "<<GetLastError()<<endl;
		cout<<"Fail intBit!"<<endl;
		return false;
	}
	int Bit = atoi(&intBit);
	char* cInt = new char[Bit];
	retVal = recv(client,cInt,sizeof(char) * Bit,0);
	if(retVal == SOCKET_ERROR)
	{
		cout<<"Recv Into error: "<<GetLastError()<<endl;
		cout<<"Fail intBit!"<<endl;
		return false;
	}
	int rData = atoi(cInt);
	delete[] cInt;

	return rData;
}

bool SockData::recvData(char* buf, int Len)
{
	ZeroMemory(buf,Len);
	if(Len <= BUFSIZE)
	{
		retVal = recv(client,buf,sizeof(char) * Len,0);
		if(retVal == SOCKET_ERROR)
		{
			cout<<"Recv Into error: "<<GetLastError()<<endl;
			cout<<"Fail char*!"<<endl;
			return false;
		}
	}
	else
	{
		int rbufReal = 0, rbufNeed = Len;
		while (rbufReal < Len)
		{
			retVal = recv(client,buf,sizeof(char) * BUFSIZE,0);
			if(retVal == SOCKET_ERROR)
			{
				cout<<"Recv Into error: "<<GetLastError()<<endl;
				cout<<"Fail char*!"<<endl;
				return false;
			}
			rbufReal += retVal;
			rbufNeed = Len - rbufReal;
		}
	}

	return true;
}

bool SockData::recvData(const string rfileRoad)
{
	int length = recvData();
	char* buf = new char[BUFSIZE];
	ZeroMemory(buf, BUFSIZE);
	ofstream out(rfileRoad,fstream::out|fstream::binary);
	if(!out.is_open())
	{
		cout<<"don't create file."<<endl;
		return false;
	}
	int rbufReal = 0, rbufNeed = length;
	while (rbufReal < length)
	{
		if(recvData(buf))
		{
			rbufReal += retVal;
			rbufNeed = length - rbufReal;
			out.write(buf,sizeof(char)*retVal);
		}
	}	
	out.close();
	delete[] buf;

	return true;
}



int main()  
{  
	ServerSock serSock;
	SockData sockData(serSock.getClient());
	
	//sSock.sendData(string("Sys.bmp"));
	//cout<<"send over!"<<endl;
	//int iTemp = sSock.recvData();
	//char* recvstr = new char[iTemp];
	//sSock.recvData(recvstr,iTemp);
	//cout<<recvstr<<endl;
	//delete[] recvstr;

	sockData.recvData(string("Sys1.bmp"));
	char* answer = "copy that !";
	sockData.sendData(int(strlen(answer) + 1));
	sockData.sendData(answer,strlen(answer) + 1);

	system("pause");
	return 0;  
}  

