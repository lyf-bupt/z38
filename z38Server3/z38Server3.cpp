#include "stdafx.h"
#include "project.h"

char recvBuf[100];
char recvBuf1[49];
short buff[49];
SOCKET sockConn;

DWORD WINAPI readFileHandleFun(LPVOID lp)
{
	printf("server3 enter readFileHandleFun ,ready to readFileHandle\n");

	BOOL  fileRes = TRUE;
	//char fileName[] = "..\\test\\t1\\8file\\t1_qf";
	//char shareName[] = "..\\test\\t1\\8file\\t1_gxbl";
	char fileName[] = "..\\test\\t8\\8file\\t8_qf";
	char shareName[] = "..\test\\t8\\8file\\t8_gxbl";
	char str1[256], str2[256];
	memset(str1, 0, sizeof(str1));
	memset(str2, 0, sizeof(str2));
	for (int i = 0; i < Thread_Number; i++)
	{
		sprintf(str1, "%s%d.cnf", fileName, i + 1);

		fileRes = ReadCNFFile(i, lc[i][i], *sol[i][i], str1);			//Check Not Unsat  /读切分文件 将切分文件中的字句放入求解器
		if (!fileRes) return 0;
	}

	for (int i = 0; i < Thread_Number; i++)
	{
		for (int j = 0; j < Thread_Number; j++)
		{
			if (i == j) continue;
			sprintf(str2, "%s%d%d.txt", shareName, (i + 1), (1 + j));
			fileRes = fileRes && ReadShareFile(i, lc[i][j], *sol[i][j], j, str2);
		}
		if (!fileRes) return 0;
	}


	GetShareNumberAmongAll();

	SetEvent(readFileHandle);
	printf("server3 enter readFileHandleFun end ,SetEvent(readFileHandle)\n");
}





DWORD WINAPI recvFun(LPVOID lp)
{


	printf("server3 enter recvFun ,ready to recv\n");

	char recvStopBuf[100];
	char StopServer[] = "stopAllServer";

	memset(recvStopBuf, 0, sizeof(recvStopBuf));
	if (recv(sockConn, recvStopBuf, sizeof(recvStopBuf), 0) < 0) {
		printf("接受失败");
		//exit(0);				//斗胆加一下！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
	}
	else //server验算结果（sat/unsat)接收成功
	{

		printf("\n recvStopBuf= %s\n", recvStopBuf);


		if (strcmp(StopServer, recvStopBuf) == 0) {
			printf("这里要setevent 停止\n");
			for (int i = 0; i < Thread_Number; i++) SetEvent(existHandle[i]);
			SetEvent(recvHandle);
		}
		return 0;
	}//else




}


int main()
{
	//初始化
	printf("server3 ready\n");
	WSADATA wsaData;
	int port = 6663;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("初始化失败");
		return 0;
	}

	//创建用于监听的套接字,即服务端的套接字
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port);
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//bind
	int retVal = bind(sockSrv, (LPSOCKADDR)&addrSrv, sizeof(SOCKADDR_IN));
	if (retVal == SOCKET_ERROR) {
		printf("连接失败:%d\n", WSAGetLastError());
		return 0;
	}
	//listen
	if (listen(sockSrv, 10) == SOCKET_ERROR) {
		printf("监听失败:%d", WSAGetLastError());
		return 0;
	}

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	while (1)
	{
		//等待客户请求到来
		sockConn = accept(sockSrv, (SOCKADDR *)&addrClient, &len);
		if (sockConn == SOCKET_ERROR) {
			printf("等待请求失败:%d", WSAGetLastError());
			break;
		}


		memset(shareVariableNum, 0, sizeof(shareVariableNum));
		memset(shareVariableAssignment, 0, sizeof(shareVariableAssignment));
		memset(shareVariableValue, 0, sizeof(shareVariableValue));
		memset(shareVariableValueAmongAll, 0, sizeof(shareVariableValueAmongAll));
		memset(shareVariableValueAmongAll, 0, sizeof(shareVariableValueAmongAll));
		memset(shareNumberAmongAll, 0, sizeof(shareNumberAmongAll));
		memset(shareUniNum, 0, sizeof(shareUniNum));
		memset(AllUnitResults, 0, sizeof(AllUnitResults));
		memset(VariableInUnit, 0, sizeof(VariableInUnit));
		memset(VariableNumInUnit, 0, sizeof(VariableNumInUnit));
		memset(hHandle, 0, sizeof(hHandle));
		memset(stack, 0, sizeof(stack));
		memset(stackStart, 0, sizeof(stackStart));
		memset(VariableAllUnitsAssignment, 0, sizeof(VariableAllUnitsAssignment));
		memset(ConfirmedVariableAllUnitsAssignment, 0, sizeof(ConfirmedVariableAllUnitsAssignment));
		memset(CopyVariableAllUnitsAssignment, 0, sizeof(CopyVariableAllUnitsAssignment));
		memset(&AllClausesInSystem[0], 0, Clause_Number * sizeof(ClauseDes));
		memset(variableExpInSystem, 0, Thread_Number*Variable_Number_Per_Unit * sizeof(expr*));
		memset(AllShareNumIndex, 0, sizeof(AllShareNumIndex));  //layer
		memset(AllShareVariableValueIndex, 0, sizeof(AllShareVariableValueIndex));  //layer
		memset(ClauseInUnit, 0, sizeof(ClauseInUnit));
		memset(revExp, 0, Thread_Number * sizeof(expr*));
		memset(pushNum, 0, sizeof(pushNum));
		memset(SpaceSplit, 0, sizeof(SpaceSplit));
		memset(TestSplitSeg, 0, sizeof(TestSplitSeg));


		for (int i = 0; i < Thread_Number; i++)
		{
			existHandle[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			for (int k = 0; k < Thread_Number; k++)
			{
				hHandle[i][k] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				sol[i][k] = new solver(lc[i][k]);
				params p(lc[i][k]);
				p.set("unsat_core", true);
				sol[i][k]->set(p);
				InitializeCriticalSection(&cs[i][k]);
			}

			revExp[i] = new expr(lc[i][0]);
			//SpaceSplit[i] = i << 11;
			InitializeCriticalSection(&cs[i][Thread_Number]);


		}//for
		
		////新学期新加的一个线程*********************
		readFileHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		HANDLE readFileHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readFileHandleFun, 0, 0, NULL);
		////*****************************************




		//printf("客户端的IP是:[%s]\n", inet_pton(addrClient.sin_addr));
		//收到数据
		recv(sockConn, recvBuf1, sizeof(recvBuf1), 0);
		cout << recvBuf1;
		printf("char buff end\n");

		//将buf转为short类型
		for (int j = 0; j < 48; j++) {
			buff[j] = recvBuf1[j] - '0';
			//cout << "Shh " << buffs[i][j];
			printf("buff][%d] = %d", j, buff[j]);
			printf("\n short buf end\n");
		}


		//毛冉周六晚上新加的 start===========================================================
		/**
		memset(shareVariableNum, 0, sizeof(shareVariableNum));
		memset(shareVariableAssignment, 0, sizeof(shareVariableAssignment));
		memset(shareVariableValue, 0, sizeof(shareVariableValue));
		memset(shareVariableValueAmongAll, 0, sizeof(shareVariableValueAmongAll));
		memset(shareVariableValueAmongAll, 0, sizeof(shareVariableValueAmongAll));
		memset(shareNumberAmongAll, 0, sizeof(shareNumberAmongAll));
		memset(shareUniNum, 0, sizeof(shareUniNum));
		memset(AllUnitResults, 0, sizeof(AllUnitResults));
		memset(VariableInUnit, 0, sizeof(VariableInUnit));
		memset(VariableNumInUnit, 0, sizeof(VariableNumInUnit));
		memset(hHandle, 0, sizeof(hHandle));
		memset(stack, 0, sizeof(stack));
		memset(stackStart, 0, sizeof(stackStart));
		memset(VariableAllUnitsAssignment, 0, sizeof(VariableAllUnitsAssignment));
		memset(ConfirmedVariableAllUnitsAssignment, 0, sizeof(ConfirmedVariableAllUnitsAssignment));
		memset(CopyVariableAllUnitsAssignment, 0, sizeof(CopyVariableAllUnitsAssignment));
		memset(&AllClausesInSystem[0], 0, Clause_Number * sizeof(ClauseDes));
		memset(variableExpInSystem, 0, Thread_Number*Variable_Number_Per_Unit * sizeof(expr*));
		memset(AllShareNumIndex, 0, sizeof(AllShareNumIndex));  //layer
		memset(AllShareVariableValueIndex, 0, sizeof(AllShareVariableValueIndex));  //layer
		memset(ClauseInUnit, 0, sizeof(ClauseInUnit));
		memset(revExp, 0, Thread_Number * sizeof(expr*));
		memset(pushNum, 0, sizeof(pushNum));
		memset(SpaceSplit, 0, sizeof(SpaceSplit));
		memset(TestSplitSeg, 0, sizeof(TestSplitSeg));
		
		for (int i = 0; i < Thread_Number; i++)
		{
			existHandle[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			for (int k = 0; k < Thread_Number; k++)
			{
				hHandle[i][k] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				sol[i][k] = new solver(lc[i][k]);
				params p(lc[i][k]);
				p.set("unsat_core", true);
				sol[i][k]->set(p);
				InitializeCriticalSection(&cs[i][k]);
			}

			revExp[i] = new expr(lc[i][0]);
			//SpaceSplit[i] = i << 11;
			InitializeCriticalSection(&cs[i][Thread_Number]);


		}//for
		*/

		 //这里是现在的confirm！！！！！
		 //char topfileName[] = "..\\test\\t8\\t8_top100.txt";
		char topfileName[] = "..\\test\\t8\\t8_top100.txt";
		int isTrue = ConfirmedVar2(topfileName, buff);
		bool confirm = false;
		if (isTrue == 0) {
			confirm = true;
			printf("has confirmed");
			//printf(ConfirmedVariableAllUnitsAssignment[][]);
		}


		haveSat = FALSE, haveUnSat = FALSE;

		////新学期加上的
		while (true)
		{
			printf("没wait到setHandle（readFileHandle）呢，所以就卡这里不往下走了\n ");
			DWORD hr = WaitForSingleObject(readFileHandle, INFINITE);
			if (hr == WAIT_FAILED) {
				break;
				printf("hr == WAIT_FAILED\n ");
			}
			else
			{
				printf("文件已经读写完毕，继续往下走\n ");
				break;//  跳出while循环
			}
		}



		/**新学期去掉的
		BOOL	fileRes = TRUE;
		//char fileName[] = "..\\test\\t1\\8file\\t1_qf";
		//char shareName[] = "..\\test\\t1\\8file\\t1_gxbl";
		char fileName[] = "..\\test\\t8\\8file\\t8_qf";
		char shareName[] = "..\test\\t8\\8file\\t8_gxbl";
		char str1[256], str2[256];
		memset(str1, 0, sizeof(str1));
		memset(str2, 0, sizeof(str2));
		for (int i = 0; i < Thread_Number; i++)
		{
		sprintf(str1, "%s%d.cnf", fileName, i + 1);

		fileRes = ReadCNFFile(i, lc[i][i], *sol[i][i], str1);			//Check Not Unsat  /读切分文件 将切分文件中的字句放入求解器
		if (!fileRes) return 0;
		}

		for (int i = 0; i < Thread_Number; i++)
		{
		for (int j = 0; j < Thread_Number; j++)
		{
		if (i == j) continue;
		sprintf(str2, "%s%d%d.txt", shareName, (i + 1), (1 + j));
		fileRes = fileRes&&ReadShareFile(i, lc[i][j], *sol[i][j], j, str2);
		}
		if (!fileRes) return 0;
		}


		GetShareNumberAmongAll();
		新学期去掉的*/

		HANDLE hThread[Thread_Number];
		int ID[Thread_Number];
		for (int i = 0; i < Thread_Number; i++)
		{
			ID[i] = i;
			hThread[i] = CreateThread(NULL, 0, ThreadProcc, (void*)&ID[i], 0, NULL);
		}

		//如果已经被confirm了，setEvent 开始计算
		if (confirm == true) {
			threadStart();
		}



		//周二新加的一个线程***********************
		recvHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		HANDLE recvThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recvFun, 0, 0, NULL);
		//*****************************************

		//ConfirmAssignments


		int cn = 0;
		int result;//这个是我加的，用来判断结果是sat还是unsat，并返回给server			//Start

		while (true)
		{
			DWORD hr = WaitForMultipleObjects((DWORD)Thread_Number, existHandle, false, INFINITE);//Start
			if (hr == WAIT_FAILED) break;
			cn++;

			endT = clock();
			double duration = (double)(endT - startT);
			std::cout << "complete time = " << duration << " myId " << hr << "\n";

			if (cn >= Thread_Number)
			{
				BOOL checkedRes = TRUE;
				for (int i = 0; i < Thread_Number; i++) checkedRes &= AllUnitResults[i];


				if (checkedRes) {

					result = 1; //这个是我加的，用来判断结果是sat还是unsat，并返回给server
					printf("\nserver check result = %d \n", result);
					std::cout << "Check TRUE Sat" << '\n';
				}
				else {

					result = 0; //这个是我加的，用来判断结果是sat还是unsat，并返回给server
					printf("\nserver check result = %d \n", result);
					std::cout << "Check Not Unsat" << '\n';
				}
				SetEvent(recvHandle);//先计算完毕的话就停止recv*********
				break;
			}
		}//while




		 //这是周二早上新加的***************************************
		 //	   WaitForSingleObject(existHandle, INFINITE);
		int cn2 = 0;
		while (true)
		{
			printf("没wait到呢\n ");
			DWORD hr = WaitForSingleObject(recvHandle, INFINITE);
			if (hr == WAIT_FAILED) {
				break;
				printf("hr == WAIT_FAILED\n ");
			}
			cn2++;
			printf("cn2=%d\n ", cn2);
			if (cn2 >= 1)	//set（recvHandle）一次
			{
				printf("计算完毕，不用再recv了\n ");
				break;//  跳出while循环
			}
		}
		//*************************************************************




		///output results

		for (int i = 0; i < Thread_Number; i++)
		{
			for (int k = 0; k < Thread_Number; k++)
				CloseHandle(hHandle[i][k]);
			//		   CloseHandle(hHandle[i][0]);
		}

		Sleep(1000);
		//EventDedcutionResourceModel();
		for (int i = 0; i < Thread_Number; i++)
			for (int j = 0; j < Thread_Number; j++)
				DeleteCriticalSection(&cs[i][j]);

		std::cout << "done\n";


		//毛冉周六晚上新加的 end =================================================================




		//毛冉周二加的输出接口 start =================================================================
		//将验证结果发给client
		if (result == 0) {  //unsat

			char sendbuf1[] = "unsat";
			printf("return to client :unsat\n");
			int Send = send(sockConn, sendbuf1, sizeof(sendbuf1), 0);
			if (Send == SOCKET_ERROR) {
				printf("验证结果发送失败");
				break;
			}
			//closesocket(sockConn);
			//closesocket(sockSrv);
			//return 0;

		}//unsat
		else if (result == 1) {  //sat

			char sendbuf1[] = "sat";
			printf("return to client :sat\n");
			int Send = send(sockConn, sendbuf1, sizeof(sendbuf1), 0);
			if (Send == SOCKET_ERROR) {
				printf("验证结果发送失败");
				break;
			}
			//closesocket(sockConn);
			//closesocket(sockSrv);

		}//else if, sat
		else {
			printf("not sat/unsat.....exception?");
		}


		//毛冉周二加的输出接口 end =================================================================

		closesocket(sockConn);

	}

	closesocket(sockSrv);
	printf("end\n");
	WSACleanup();
	system("pause");
	return 0;
}





/**
//sat recv stopallcalculate
char recvStopBuf[100];
if (recv(sockConn, recvStopBuf, sizeof(recvBuf), 0) <= 0) {
printf("接受失败");
}
else //server验算结果（sat/unsat)接收成功
{

printf("\n recvStopBuf= %s\n", recvStopBuf);

char StopServer[1024] = "stopAllServer";
if (strcmp(StopServer, recvStopBuf) == 0) {
printf("这里要setevent 停止\n");
}
}//else
*/