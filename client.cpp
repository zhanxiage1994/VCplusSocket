#include "winsock2.h"  
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <fstream>
#include <opencv2\opencv.hpp>

using namespace std;  

//#define SERVER
#define CLIENT
#define IP_ADDR  "127.0.0.1"

class ClientSock
{
public:
	ClientSock();
	~ClientSock();
	SOCKET getClient() {return cClient;};

private:
	static const int PORT = 10086;
	int             retVal;        
	SOCKET          cClient;          
	SOCKADDR_IN     addrServ;;      //server address  
};

ClientSock::ClientSock()
{
	WSADATA wsd;           
 
	if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)  
	{ 
		cout << "WSAStartup failed!" << endl;  
	}  

	cClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);      
	if(INVALID_SOCKET == cClient)  
	{  
		cout << "socket failed!" << endl;  
		WSACleanup();
	}  
  
	addrServ.sin_family = AF_INET;  
	addrServ.sin_port = htons(PORT);  
	addrServ.sin_addr.s_addr = inet_addr(IP_ADDR);        

	retVal = connect(cClient, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));  
	if(SOCKET_ERROR == retVal)  
	{     
		cout << "connect failed!" << endl;  
		closesocket(cClient);  
		WSACleanup();         
	}  
}

ClientSock::~ClientSock()
{
	//closesocket(cClient);  
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
	ClientSock cSock;
	SockData sockData(cSock.getClient());
	//cSock.recvData(string("haha.bmp"));
	//char* answer = "copythat!";
	//cSock.sendData(int(strlen(answer)+1));
	//cSock.sendData(answer,strlen(answer)+1);
	sockData.sendData(string("haha.bmp"));
	cout<<"send over!"<<endl;
	int iTemp = sockData.recvData();
	char* recvstr = new char[iTemp];
	sockData.recvData(recvstr,iTemp);
	cout<<recvstr<<endl;
	delete[] recvstr;


	system("pause");
	return 0;  
}  
