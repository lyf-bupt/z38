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

	//����server�ļ�����									//������һ��������жϣ���Ҫ�ȵ�������ȫ�����
	if (recv(sockClient[serverId], recvBuf[serverId], sizeof(recvBuf[serverId]), 0) <= 0) {
		printf("����ʧ�� id=%d", serverId);
	}
	else //server��������sat/unsat)���ճɹ�
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
					printf("here is sat,send to server%d to stop��%s\n", i, stopAllServer);
					if (send(sockClient[i], stopAllServer, sizeof(stopAllServer), 0) < 0) {  //֮ǰ֮���Ա����ԭ���� ����һֱ��sockClient1
						printf("fail to send 'stopAllServer'\n");
					}
					else {
						printf("here is sat,send to server%d to stop,send succeed��%s\n", i, stopAllServer);
					}
				}//if

			}//for

			 //close socket
			 //closesocket(sockClient[0]);
			 //closesocket(sockClient[1]);	//Ϊʲô�ر�socket֮��server�˼���������һ������������������Ϊserverû��socket
			for (int i = 0; i < 8; i++) {
				SetEvent(exitHandle[i]);		
			}
					//��ʾ sat�� client���Խ����� ���̲߳��õȴ���
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
	//�����׽���
	WSADATA wsaData;
	char recvbuff[1024];
	memset(recvbuff, 0, sizeof(recvbuff));

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("��ʼ��Winsockʧ��");
		return 0;
	}

	/**
	//=============�����һ������=================================================

	//�˿ں� IP
	SOCKADDR_IN addrSrv[8];

	addrSrv[0].sin_family = AF_INET;
	addrSrv[0].sin_port = htons(1111);
	addrSrv[0].sin_addr.S_un.S_addr = inet_addr("10.108.165.220");				//��									//10.108.167.253 ѧ����ip��ַ

																				
	addrSrv[1].sin_family = AF_INET;
	addrSrv[1].sin_port = htons(2222);
	addrSrv[1].sin_addr.S_un.S_addr = inet_addr("10.108.167.191");				//�ұʼǱ�
	
	addrSrv[2].sin_family = AF_INET;
	addrSrv[2].sin_port = htons(3333);
	addrSrv[2].sin_addr.S_un.S_addr = inet_addr("10.108.165.220");			//��

	addrSrv[3].sin_family = AF_INET;
	addrSrv[3].sin_port = htons(4444);
	addrSrv[3].sin_addr.S_un.S_addr = inet_addr("10.108.165.220");				//��
	
	addrSrv[4].sin_family = AF_INET;
	addrSrv[4].sin_port = htons(5555);
	addrSrv[4].sin_addr.S_un.S_addr = inet_addr("10.108.167.253");				//��

	addrSrv[5].sin_family = AF_INET;
	addrSrv[5].sin_port = htons(6666);
	addrSrv[5].sin_addr.S_un.S_addr = inet_addr("10.108.167.253");			//��

	addrSrv[6].sin_family = AF_INET;
	addrSrv[6].sin_port = htons(7777);
	addrSrv[6].sin_addr.S_un.S_addr = inet_addr("10.108.167.191");			//�ұʼǱ�

	addrSrv[7].sin_family = AF_INET;
	addrSrv[7].sin_port = htons(8888);
	addrSrv[7].sin_addr.S_un.S_addr = inet_addr("10.108.167.191");		//�� �ʼǱ�
	*/


	//�˿ں� IP
	SOCKADDR_IN addrSrv[8];

	addrSrv[0].sin_family = AF_INET;
	addrSrv[0].sin_port = htons(6660);
	addrSrv[0].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");												//10.108.167.253 ѧ����ip��ַ


	addrSrv[1].sin_family = AF_INET;
	addrSrv[1].sin_port = htons(6661);
	addrSrv[1].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");			//��	
	
	addrSrv[2].sin_family = AF_INET;
	addrSrv[2].sin_port = htons(6662);
	addrSrv[2].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");			//��
	
	addrSrv[3].sin_family = AF_INET;
	addrSrv[3].sin_port = htons(6663);
	addrSrv[3].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");				//��
	
	addrSrv[4].sin_family = AF_INET;
	addrSrv[4].sin_port = htons(6664);
	addrSrv[4].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");				//��

	addrSrv[5].sin_family = AF_INET;
	addrSrv[5].sin_port = htons(6665);
	addrSrv[5].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");			//��
	
	addrSrv[6].sin_family = AF_INET;
	addrSrv[6].sin_port = htons(6666);
	addrSrv[6].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");			//�ұʼǱ�
	
	addrSrv[7].sin_family = AF_INET;
	addrSrv[7].sin_port = htons(6667);
	addrSrv[7].sin_addr.S_un.S_addr = inet_addr("127.0.0.1");		//�� �ʼǱ�
	
	


	for (int i = 0; i < 8; i++) {

		//�����׽���
		sockClient[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (SOCKET_ERROR == sockClient[0]) {
			printf("Socket() error:%d", WSAGetLastError());
			return 0;
		}

		//�������������������
		if (connect(sockClient[i], (struct  sockaddr*)&addrSrv[i], sizeof(addrSrv[i])) == INVALID_SOCKET) {
			printf("server%d����ʧ��:%d",i, WSAGetLastError());
			return 0;
		}
	}

	//==============�����һ,���� end============================================


	// ===============��data��������=====================
	int i, j = 0;
	string pre_str;//string��
	string end_str;//string��
	string str;

	char buf[8][49];

	for (i = 0; i < 8; i++) {

		string totalStr;
		//����ǰ�������� 000
		bitset<3> t;
		t = i;
		pre_str = t.to_string();

		//���ɺ��������� 000
		for (j = 0; j < 8; j++) {

			bitset<3> t;
			t = j;
			end_str = t.to_string();

			//�ϳ� 000000
			str = pre_str + end_str;
			cout << " " << str;
			totalStr = totalStr + str;

		}//for j

		cout << ",,,," << totalStr << endl;
		//��totalStr����char����
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

	//������һ�����ŷ���Ҫconfirm��ֵ
	for (int i = 0; i < 8; i++) {
		printf("buf[%d]= %s\n", i, buf[i]);
		send(sockClient[i], buf[i], sizeof(buf[i]), 0);//֮ǰ֮���Ա����ԭ���� ����һֱ��sockClient1
	}



	printf("���߳̽�������\n ");
	//�����¼�  
	for (int i = 0; i < 8; i++) {
		exitHandle[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	HANDLE hThread[8];
	//�������߳�
	int ID[8];
	for (int i = 0; i < 8; i++) {
		ID[i] = i;
		hThread[i] = CreateThread(NULL, 0, Fun, &ID[i], 0, NULL);

	}
	//Sleep(300000);

	//�ȴ�ȫ���߳����н���
	int cn = 0;
	while (true)
	{
		printf("ûwait����\n ");
		DWORD hr = WaitForMultipleObjects((DWORD)8, exitHandle, false, INFINITE);
		if (hr == WAIT_FAILED) {
			break;
			printf("hr == WAIT_FAILED\n ");
		}
		cn++;
		printf("cn=%d\n ", cn);
		if (cn >= 8)	//�����̶߳�ִ�����
		{
			//unsat print out time
			clientRecvtotalT = clock();
			double duration = (double)(clientRecvtotalT - startT);
			std::cout << "The whole calculate duration = " << duration << "\n";

			printf("���߳�Ҫ������\n ");
			break;//  ����whileѭ��
		}
	}

	//�ر��׽���
	for (int i = 0; i < 8; i++) {
		closesocket(sockClient[i]);	
	}
	

	WSACleanup();//�ͷų�ʼ��Ws2_32.dll���������Դ��
	system("pause");//����Ļ����
	return 0;
}







