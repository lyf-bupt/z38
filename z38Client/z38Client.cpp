#include "stdafx.h"
#include "project.h"

using namespace std;

SOCKET sockClient[8];
HANDLE exitHandle[8];

DWORD WINAPI Fun(LPVOID lp)
{
	int serverId = *(int*)(lp);
	char recvBuf[8][100];

	printf("enter Fun,serverId=%d \n", serverId);

	//memset(recvBuf, 0, sizeof(recvBuf));

	//接收server的计算结果									//接受完一个结果就判断，不要等到接受完全部结果
	if (recv(sockClient[serverId], recvBuf[serverId], sizeof(recvBuf[serverId]), 0) <= 0) {
		printf("接受失败 id=%d", serverId);
	}
	else //server验算结果（sat/unsat)接收成功
	{

		printf("recvBuf[%d]=%s\n", serverId, recvBuf[serverId]);

		char result[] = "sat";
		char stopAllServer[] = "stopAllServer";

		if (strcmp(result, recvBuf[serverId]) == 0) {	//if server  sat
			printf("here is sat,send to serverto stop\n");

			//sat print out time
			clientRecvSatT = clock();
			double duration = (double)(clientRecvSatT - startT);
			std::cout << "One sat.The whole calculate duration = " << duration << "\n";

			//sat .then send to other server to stop calculate
			for (int i = 0; i < 8; i++) {
				if (i != serverId) {
					printf("here is sat,send to server%d to stop：%s\n", i, stopAllServer);
					if (send(sockClient[i], stopAllServer, sizeof(stopAllServer), 0) < 0) {  //之前之所以报错的原因是 这里一直是sockClient1
						printf("fail to send 'stopAllServer'\n");
					}
					else {
						printf("here is sat,send to server%d to stop,send succeed：%s\n", i, stopAllServer);
					}
				}//if

			}//for

			 //close socket
			 //closesocket(sockClient[0]);
			 //closesocket(sockClient[1]);	//为什么关闭socket之后server端继续运行了一波？？？？？？？因为server没关socket
			for (int i = 0; i < 8; i++) {
				SetEvent(exitHandle[i]);		
			}
					//表示 sat了 client可以结束了 主线程不用等待了
											//return 0;
		}//if sat
		else {
			printf("\n receive from server ,server unsat\n");
			//sat print out time
			clientRecvUnSatT = clock();
			double duration = (double)(clientRecvUnSatT - startT);
			printf("server%d unsat  ", serverId);
			std::cout << "The calculate duration = " << duration << "\n";
			SetEvent(exitHandle[serverId]);
			printf("server%d has setEvent \n",serverId);
			//closesocket(sockClient[serverId]);
		}
	}//else

	 //close socket
	 //closesocket(sockClient[0]);
	 //closesocket(sockClient[1]);

	return 0;

}



int main()
{
	//加载套接字
	WSADATA wsaData;
	char recvbuff[1024];
	memset(recvbuff, 0, sizeof(recvbuff));

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("初始化Winsock失败");
		return 0;
	}

	/**
	//=============计算机一，二号=================================================

	//端口号 IP
	SOCKADDR_IN addrSrv[8];

	addrSrv[0].sin_family = AF_INET;
	addrSrv[0].sin_port = htons(1111);
	addrSrv[0].sin_addr.S_un.S_addr = inet_addr("10.108.165.220");				//董									//10.108.167.253 学长的ip地址

																				
	addrSrv[1].sin_family = AF_INET;
	addrSrv[1].sin_port = htons(2222);
	addrSrv[1].sin_addr.S_un.S_addr = inet_addr("10.108.167.191");				//我笔记本
	
	addrSrv[2].sin_family = AF_INET;
	addrSrv[2].sin_port = htons(3333);
	addrSrv[2].sin_addr.S_un.S_addr = inet_addr("10.108.165.220");			//董

	addrSrv[3].sin_family = AF_INET;
	addrSrv[3].sin_port = htons(4444);
	addrSrv[3].sin_addr.S_un.S_addr = inet_addr("10.108.165.220");				//董
	
	addrSrv[4].sin_family = AF_INET;
	addrSrv[4].sin_port = htons(5555);
	addrSrv[4].sin_addr.S_un.S_addr = inet_addr("10.108.167.253");				//周

	addrSrv[5].sin_family = AF_INET;
	addrSrv[5].sin_port = htons(6666);
	addrSrv[5].sin_addr.S_un.S_addr = inet_addr("10.108.167.253");			//周

	addrSrv[6].sin_family = AF_INET;
	addrSrv[6].sin_port = htons(7777);
	addrSrv[6].sin_addr.S_un.S_addr = inet_addr("10.108.167.191");			//我笔记本

	addrSrv[7].sin_family = AF_INET;
	addrSrv[7].sin_port = htons(8888);
	addrSrv[7].sin_addr.S_un.S_addr = inet_addr("10.108.167.191");		//我 笔记本
	*/


	//端口号 IP
	SOCKADDR_IN addrSrv[8];

	addrSrv[0].sin_family = AF_INET;
	addrSrv[0].sin_port = htons(6660);
	addrSrv[0].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");												//10.108.167.253 学长的ip地址


	addrSrv[1].sin_family = AF_INET;
	addrSrv[1].sin_port = htons(6661);
	addrSrv[1].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");			//周	
	
	addrSrv[2].sin_family = AF_INET;
	addrSrv[2].sin_port = htons(6662);
	addrSrv[2].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");			//周
	
	addrSrv[3].sin_family = AF_INET;
	addrSrv[3].sin_port = htons(6663);
	addrSrv[3].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");				//董
	
	addrSrv[4].sin_family = AF_INET;
	addrSrv[4].sin_port = htons(6664);
	addrSrv[4].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");				//董

	addrSrv[5].sin_family = AF_INET;
	addrSrv[5].sin_port = htons(6665);
	addrSrv[5].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");			//董
	
	addrSrv[6].sin_family = AF_INET;
	addrSrv[6].sin_port = htons(6666);
	addrSrv[6].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");			//我笔记本
	
	addrSrv[7].sin_family = AF_INET;
	addrSrv[7].sin_port = htons(6667);
	addrSrv[7].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");		//我 笔记本
	
	


	for (int i = 0; i < 8; i++) {

		//创建套接字
		sockClient[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (SOCKET_ERROR == sockClient[0]) {
			printf("Socket() error:%d", WSAGetLastError());
			return 0;
		}

		//向服务器发出连接请求
		if (connect(sockClient[i], (struct  sockaddr*)&addrSrv[i], sizeof(addrSrv[i])) == INVALID_SOCKET) {
			printf("server%d连接失败:%d",i, WSAGetLastError());
			return 0;
		}
	}

	//==============计算机一,二号 end============================================


	// ===============将data放入数组=====================
	int i, j = 0;
	string pre_str;//string类
	string end_str;//string类
	string str;

	char buf[8][49];

	for (i = 0; i < 8; i++) {

		string totalStr;
		//生成前三个数字 000
		bitset<3> t;
		t = i;
		pre_str = t.to_string();

		//生成后三个数字 000
		for (j = 0; j < 8; j++) {

			bitset<3> t;
			t = j;
			end_str = t.to_string();

			//合成 000000
			str = pre_str + end_str;
			cout << " " << str;
			totalStr = totalStr + str;

		}//for j

		cout << ",,,," << totalStr << endl;
		//将totalStr放入char数组
		int length = totalStr.copy(buf[i], 48);
		buf[i][length] = '\0';


		//for (int j = 0;j < 48;j++) {
		//	cout << " " << i << j << buf[i][j];
		//printf("\n");
		//}
		//cout << "pause" << endl;
	}//for i

	 
	for (int i = 0; i < 8; i++) {
		printf("buf[%d]= %s\n", i, buf[i]);
	}

	//================end===============================

	//向计算机一，二号发送要confirm的值
	for (int i = 0; i < 8; i++) {
		printf("buf[%d]= %s\n", i, buf[i]);
		send(sockClient[i], buf[i], sizeof(buf[i]), 0);//之前之所以报错的原因是 这里一直是sockClient1
	}



	printf("用线程接收数据\n ");
	//创建事件  
	for (int i = 0; i < 8; i++) {
		exitHandle[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	HANDLE hThread[8];
	//创建两线程
	int ID[8];
	for (int i = 0; i < 8; i++) {
		ID[i] = i;
		hThread[i] = CreateThread(NULL, 0, Fun, &ID[i], 0, NULL);

	}
	//Sleep(300000);

	//等待全部线程运行结束
	int cn = 0;
	while (true)
	{
		printf("没wait到呢\n ");
		DWORD hr = WaitForMultipleObjects((DWORD)8, exitHandle, false, INFINITE);
		if (hr == WAIT_FAILED) {
			break;
			printf("hr == WAIT_FAILED\n ");
		}
		cn++;
		printf("cn=%d\n ", cn);
		if (cn >= 8)	//两个线程都执行完毕
		{
			//unsat print out time
			clientRecvtotalT = clock();
			double duration = (double)(clientRecvtotalT - startT);
			std::cout << "The whole calculate duration = " << duration << "\n";

			printf("主线程要结束了\n ");
			break;//  跳出while循环
		}
	}

	//关闭套接字
	for (int i = 0; i < 8; i++) {
		closesocket(sockClient[i]);	
	}
	

	WSACleanup();//释放初始化Ws2_32.dll所分配的资源。
	system("pause");//让屏幕暂留
	return 0;
}







