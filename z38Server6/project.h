
#ifndef __FUNCTION_H__
#define __FUNCTION_H__
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include"z3++.h"
#include<time.h>
#include<math.h>
#include <bitset>//使用bitset标准库好了
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "winsock2.h"
#include<cstdlib>
#pragma comment(lib,"ws2_32.lib")//引用库文件
#include<vector>
#include<string>
#include<Windows.h>
#include<cstring>
#include <fstream>
#include <memory.h>


using namespace std;
using namespace z3;



#define Thread_Number  8
#define Share_Variable_Number  4000
#define Clause_Number          20000
#define Variable_Number_Per_Unit  4000
#define Push_Unit_Limit   20000
#define Literals_In_Clasue  20 
#define SPLIT_SPACE_NUM  6 
#define SPLIT_SPACE_SEG  6
extern CRITICAL_SECTION cs[Thread_Number][Thread_Number + 2];
extern HANDLE hHandle[Thread_Number][Thread_Number];       // first is itself, the second represents others
extern HANDLE existHandle[Thread_Number];
extern HANDLE recvHandle;
extern HANDLE readFileHandle;
extern boolean  haveSat, haveUnSat;
extern BOOL   AllUnitResults[Thread_Number];
extern context lc[Thread_Number][Thread_Number];
extern solver *sol[Thread_Number][Thread_Number];
extern clock_t  startT, endT, clientRecvSatT, clientRecvtotalT, clientRecvUnSatT;
extern int shareVariableValue[Thread_Number][Thread_Number][Thread_Number][Share_Variable_Number];
extern short shareVariableAssignment[Thread_Number][Thread_Number][Thread_Number][Share_Variable_Number];
extern int  shareVariableNum[Thread_Number][Thread_Number][Thread_Number];
extern int  VariableInUnit[Thread_Number][Thread_Number][Variable_Number_Per_Unit];
extern int  ClauseInUnit[Thread_Number][Thread_Number][Clause_Number];
extern int  VariableNumInUnit[Thread_Number][Thread_Number];
extern short HighToLowVR[Thread_Number][Thread_Number];
extern int shareNumberAmongAll[Thread_Number][2 * Thread_Number], shareUniNum[Thread_Number][2 * Thread_Number], AllShareNumIndex[Thread_Number][2 * Thread_Number];  //layer
extern int shareVariableValueAmongAll[Thread_Number][Thread_Number][Share_Variable_Number], AllShareVariableValueIndex[Thread_Number][Thread_Number][Share_Variable_Number]; //layer number
extern short  CopyVariableAllUnitsAssignment[Thread_Number][Thread_Number][Variable_Number_Per_Unit];
extern short  VariableAllUnitsAssignment[Thread_Number][Variable_Number_Per_Unit];
extern int  ClauseNumInSystem, VariableNumInSystem, optLimit;
extern int  stack[Thread_Number], stackStart[Thread_Number];
extern short ConfirmedVariableAllUnitsAssignment[Thread_Number][Variable_Number_Per_Unit];
struct ClauseDes
{
	int clauseNo;
	char clauseNoStr[16];
	BYTE literalNum;
	int  literals[Literals_In_Clasue];
	expr* clauseExp[Thread_Number][Thread_Number];
};
extern ClauseDes  AllClausesInSystem[Clause_Number];
extern expr* variableExpInSystem[Thread_Number][Thread_Number][Variable_Number_Per_Unit];
extern expr* revExp[Thread_Number];
extern _int64   pushNum[Thread_Number];
extern unsigned int SpaceSplit[Thread_Number];
extern int TestSplitSeg[Thread_Number];
extern int TempCount;


bool ConfirmedVar(short buff[]);
bool ConfirmedVar1(short buff[]);
int ConfirmedVar2(char *topfileName, short buff[]);
void clauseVariableifConflict(int causeNo);

//=====================================
void threadStart();
BOOL IsInUnit(int layer, int unit, int variableValue);
BOOL ClauseIsInUnit(int layer, int unit, int clauseNo);
BOOL IsInShare(int layer, int stack, int variableValue);
BOOL IsInShare1(int layer, int id, int op, int variableValue);
BOOL PutInUnit(int layer, int unit, int variableValue);
BOOL PutClauseInUnit(int layer, int unit, int clauseNo);
BOOL AddNewShare(int layer, int unit, int newVariable);
bool ReadCNFFile(int id, context &c, solver &s, char *fileName);
bool ReadShareFile(int id, context &c, solver &s, int op, char *shareName);
BOOL IsSharedAmongAll(int layer, int value, int stack);
void GetShareNumberAmongAll4(int layer, int stack);
void GetShareNumberAmongAll2(int layer, int deep, int stack);
void GetShareNumberAmongAll();
BOOL GetClause1(int layer, int id, int op, ClauseDes &des, int &addNum);
void GetShareModelFromLastChecking(int id, int op, int layer, context &c, solver &s);
void ImposeSpaceOnLastChecking(int id, int op, int layer, context &c, solver &s);
BOOL ConfirmAssignments(int layer, int causeNo, BOOL isStartPart, BOOL &isConflictWithSplit);
BOOL GetNexSpaceSeg(int layer, int causeNo, BOOL isStartPart, BOOL &isConflictWithSplit);
BOOL GetNexSpaceSeg1(int layer, int causeNo, BOOL isStartPart, BOOL &isConflictWithSplit);
void ReverseModelByUncore(int layer, int causeNo);
short VerifLastFromLower(int id, int op, int layer, solver &s, int idC, int stac, BOOL &isConflictWithSplit);
BOOL IsTestedAll(int layer);
check_result StartCheck(int myId, BOOL &isConflictWithSplit);
DWORD WINAPI ThreadProcc(void* lp);


#endif
#pragma once




#pragma once
