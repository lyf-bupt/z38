#include "stdafx.h"
#include "project.h"


#define Thread_Number  8
#define Share_Variable_Number  4000
#define Clause_Number          20000
#define Variable_Number_Per_Unit  4000
#define Push_Unit_Limit   20000
#define Literals_In_Clasue  20 
#define SPLIT_SPACE_NUM  6 
#define SPLIT_SPACE_SEG  6


CRITICAL_SECTION cs[Thread_Number][Thread_Number + 2];
HANDLE hHandle[Thread_Number][Thread_Number];       // first is itself, the second represents others
HANDLE existHandle[Thread_Number];
HANDLE recvHandle;
HANDLE readFileHandle;
boolean  haveSat, haveUnSat;
BOOL   AllUnitResults[Thread_Number];

context lc[Thread_Number][Thread_Number];
solver *sol[Thread_Number][Thread_Number];
clock_t  startT, endT, clientRecvSatT, clientRecvtotalT, clientRecvUnSatT;


int shareVariableValue[Thread_Number][Thread_Number][Thread_Number][Share_Variable_Number];
short shareVariableAssignment[Thread_Number][Thread_Number][Thread_Number][Share_Variable_Number];
int  shareVariableNum[Thread_Number][Thread_Number][Thread_Number];
int  VariableInUnit[Thread_Number][Thread_Number][Variable_Number_Per_Unit];
int  ClauseInUnit[Thread_Number][Thread_Number][Clause_Number];
int  VariableNumInUnit[Thread_Number][Thread_Number];
short HighToLowVR[Thread_Number][Thread_Number];

int shareNumberAmongAll[Thread_Number][2 * Thread_Number], shareUniNum[Thread_Number][2 * Thread_Number], AllShareNumIndex[Thread_Number][2 * Thread_Number];  //layer
int shareVariableValueAmongAll[Thread_Number][Thread_Number][Share_Variable_Number], AllShareVariableValueIndex[Thread_Number][Thread_Number][Share_Variable_Number]; //layer number
short  CopyVariableAllUnitsAssignment[Thread_Number][Thread_Number][Variable_Number_Per_Unit];
short  VariableAllUnitsAssignment[Thread_Number][Variable_Number_Per_Unit];

int  ClauseNumInSystem, VariableNumInSystem, optLimit;
int  stack[Thread_Number], stackStart[Thread_Number];
short ConfirmedVariableAllUnitsAssignment[Thread_Number][Variable_Number_Per_Unit];


ClauseDes  AllClausesInSystem[Clause_Number];
expr* variableExpInSystem[Thread_Number][Thread_Number][Variable_Number_Per_Unit];
expr* revExp[Thread_Number];
_int64   pushNum[Thread_Number];
unsigned int SpaceSplit[Thread_Number];
int TestSplitSeg[Thread_Number];
int TempCount = 3;

char col[100][30];					//last 新加的！！！！！！！！！！！！！！！！！！！！！！！！！！！！！//Start


bool ConfirmedVar(short buff[]) {



	//short buff[] = { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,1,0,0,0,0,1,1,1 };
	int m = 0, n = 0, j = 0, confirm = 0;
	int i;
	for (i = 0; i<48; i++)
		//cout << buff[i] << " ";
		printf("%d  ", buff[i]);
	printf("\n");


	for (j = 0; j < 48; j++)
	{
		if (j % 6 == 0) {
			n = 0;
			++m;

			//cout << "开头 m= " << m - 1 << " ";
			//cout << "开头 n= " << n + 1 << endl;
			ConfirmedVariableAllUnitsAssignment[m - 1][n + 1] = buff[j];
			confirm++;

		}
		else {
			n++;
			//cout << "平时 m= " << m - 1 << " ";
			//cout << "平时 n= " << n + 1 << endl;
			ConfirmedVariableAllUnitsAssignment[m - 1][n + 1] = buff[j];
			confirm++;
		}


		printf("ConfirmedVariableAllUnitsAssignment[%d][%d] = %d", m - 1, n + 1, ConfirmedVariableAllUnitsAssignment[m - 1][n + 1]);
		printf("\n");

		//cout << a[i] << " ";
		//cout << endl;


	}

	if (confirm > 0) {
		return 0;
	}
	else {
		return -1;
	}

	printf("\n");


}

/**
int main() {

short buff[] = { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,1,0,0,0,0,1,1,1 };
bool isTrue = ConfirmedVar(buff);
if (isTrue == 0) {
printf("has confirmed");
}

}
*/



bool ConfirmedVar1(short buff[]) {

	char row[100];
	char col[100][30];
	int x = 0;

	FILE *fp = fopen("C:\\Users\\test\\Desktop\\a.txt", "r");
	//将文件第一行赋值给数组col   col[0]=1  col[1]=5 
	while (fgets(row, 100, fp) != NULL) {				//buf=1 2												
		if (sscanf(row, "%s", col[x]) == 1)
		{
			printf("%s\n", col[x]);
			x++;
		};
	}

	//short buff[] = { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,1,0,0,0,0,1,1,1 };
	int m = 0, n, j = 0, confirm = 0;
	int i;
	for (i = 0; i<48; i++)
		//cout << buff[i] << " ";
		printf("%d  ", buff[i]);
	printf("\n");


	for (j = 0; j < 48; j++)
	{
		if (j % 6 == 0) {
			n = 0;
			++m;

			if (buff[j] == 0) {
				ConfirmedVariableAllUnitsAssignment[m - 1][atoi(col[n])] = 1;
				confirm++;
			}
			else if (buff[j] == 1) {
				ConfirmedVariableAllUnitsAssignment[m - 1][atoi(col[n])] = -1;
				confirm++;
			}
			else {
				printf("Fail to confirm\n ");
			}


		}
		else {
			n++;
			if (buff[j] == 0) {
				ConfirmedVariableAllUnitsAssignment[m - 1][atoi(col[n])] = 1;
				confirm++;
			}
			else if (buff[j] == 1) {
				ConfirmedVariableAllUnitsAssignment[m - 1][atoi(col[n])] = -1;
				confirm++;
			}
			else {
				printf("Fail to confirm\n ");
			}

		}


		printf("ConfirmedVariableAllUnitsAssignment[%d][%d] = %d", m - 1, atoi(col[n]), ConfirmedVariableAllUnitsAssignment[m - 1][atoi(col[n])]);
		printf("\n");


	}

	if (confirm > 0) {
		return 0;
	}
	else {
		return -1;
	}

	printf("\n");


}


int ConfirmedVar2(char *topfileName, short buff[]) {  // 给confirm的第二个参数取了绝对值

	printf("topfileName= %s\n", topfileName); //

	char row[100];
	//char col[100][30];								//last 隐藏了这！！！！！！！！！！！！！！！
	int x = 0;

	FILE *fp = fopen(topfileName, "r");
	//将文件第一行赋值给数组col   col[0]=1  col[1]=5 
	while (fgets(row, 100, fp) != NULL) {				//buf=1 2												
		if (sscanf(row, "%s", col[x]) == 1)
		{
			printf("%s\n", col[x]);
			x++;
		};
	}

	//short buff[] = { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,1,0,0,0,0,1,1,1 };
	int m = 0, n, j = 0, confirm = 0;
	int i;
	for (i = 0; i<48; i++)
		//cout << buff[i] << " ";
		printf("%d  ", buff[i]);
	printf("\n");


	for (j = 0; j < 48; j++)
	{
		if (j % 6 == 0) {
			n = 0;
			++m;

			if (buff[j] == 0) {
				ConfirmedVariableAllUnitsAssignment[m - 1][abs(atoi(col[n]))] = 1;
				confirm++;
			}
			else if (buff[j] == 1) {
				ConfirmedVariableAllUnitsAssignment[m - 1][abs(atoi(col[n]))] = -1;
				confirm++;
			}
			else {
				printf("Fail to confirm\n ");
			}


		}
		else {
			n++;
			if (buff[j] == 0) {
				ConfirmedVariableAllUnitsAssignment[m - 1][abs(atoi(col[n]))] = 1;
				confirm++;
			}
			else if (buff[j] == 1) {
				ConfirmedVariableAllUnitsAssignment[m - 1][abs(atoi(col[n]))] = -1;
				confirm++;
			}
			else {
				printf("Fail to confirm\n ");
			}

		}


		printf("ConfirmedVariableAllUnitsAssignment[%d][%d] = %d", m - 1, atoi(col[n]), ConfirmedVariableAllUnitsAssignment[m - 1][abs(atoi(col[n]))]);
		printf("\n");


	}

	if (confirm > 0) {
		return 0;
	}
	else {
		return -1;
	}

	printf("\n");


}

/**
int main() {

short buff[] = { 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,1,0,0,0,0,1,1,1 };
char fileName[] = "C:\\Users\\test\\Desktop\\a.txt"; //
bool isTrue = ConfirmedVar(fileName, buff); //
if (isTrue == 0) {
printf("has confirmed");
}

}
*/


//===========================老师的方法

void threadStart() {

	for (int i = 0; i < Thread_Number; i++)
		SetEvent(hHandle[i][i]);
	Sleep(0);
	startT = clock();
}


BOOL IsInUnit(int layer, int unit, int variableValue)
{
	return VariableInUnit[layer][unit][variableValue];
}

BOOL ClauseIsInUnit(int layer, int unit, int clauseNo)
{
	return ClauseInUnit[layer][unit][clauseNo];
}

BOOL IsInShare(int layer, int stack, int variableValue)
{
	return shareVariableValueAmongAll[layer][stack][variableValue];
}

BOOL IsInShare1(int layer, int id, int op, int variableValue)
{
	return shareVariableValue[layer][id][op][variableValue];
}


BOOL PutInUnit(int layer, int unit, int variableValue)
{
	return VariableInUnit[layer][unit][variableValue] ? FALSE : (VariableInUnit[layer][unit][variableValue] = 1);
}

BOOL PutClauseInUnit(int layer, int unit, int clauseNo)
{
	return ClauseInUnit[layer][unit][clauseNo] ? FALSE : (ClauseInUnit[layer][unit][clauseNo] = 1);
}

BOOL AddNewShare(int layer, int unit, int newVariable)
{
	if (!PutInUnit(layer, unit, newVariable)) return FALSE;
	BOOL is = FALSE;
	for (int i = 0; i < Thread_Number; i++)
	{
		if (i == unit) continue;
		if (!IsInUnit(layer, i, newVariable)) continue;

		EnterCriticalSection(&cs[layer][Thread_Number]);
		shareVariableValue[layer][unit][i][newVariable] = 1;
		shareVariableValue[layer][i][unit][newVariable] = 1;
		shareVariableNum[layer][unit][i] = max(shareVariableNum[layer][unit][i], newVariable);
		shareVariableNum[layer][i][unit] = shareVariableNum[layer][unit][i];
		LeaveCriticalSection(&cs[layer][Thread_Number]);

		is = TRUE;
	}
	return is;
}



bool ReadCNFFile(int id, context &c, solver &s, char *fileName)
{

	FILE *fp = fopen(fileName, "r");
	if ((!fp) || (id < 0)) return FALSE;
	char cnf[1000], valueS[256], *retVal;
	int i = 1, ctr = 0, clauseSize = 0, clauseNum = 0;
	BOOL isFirstchar = TRUE;
	expr clauseP(c);
	std::string strT;

	while (!feof(fp))
	{
		retVal = fgets(cnf, 999, fp);

		if (retVal && (strlen(cnf) > 0))
		{
			ctr = 0; clauseSize = 0; strT = "(or"; isFirstchar = TRUE;
			for (int i = 0; i < strlen(cnf); i++)
			{
				if (isFirstchar)
				{
					if (cnf[i] == ' ') continue;
					isFirstchar = FALSE;
				}
				if (((cnf[i] == ' ') && (i > 0)) || (i == strlen(cnf) - 1) || (cnf[i] == '\n'))
				{
					if (i == strlen(cnf) - 1) valueS[ctr++] = cnf[i];
					valueS[ctr] = 0;
					ctr = 0;

					if (atoi(valueS) < 0)
					{
						if (AllClausesInSystem[ClauseNumInSystem].literalNum == 0)
						{
							clauseP = !c.bool_const(&valueS[1]);
							for (int j = 0; j < Thread_Number; j++)
							{
								for (int k = 0; k < Thread_Number; k++)
								{
									AllClausesInSystem[ClauseNumInSystem].clauseExp[j][k] = new expr(lc[j][k]);
									*AllClausesInSystem[ClauseNumInSystem].clauseExp[j][k] = !(lc[j][k].bool_const(&valueS[1]));
									if (variableExpInSystem[j][atoi(&valueS[1])] == NULL)
									{
										variableExpInSystem[j][k][atoi(&valueS[1])] = new expr(lc[j][k]);
										*variableExpInSystem[j][k][atoi(&valueS[1])] = (lc[j][k].bool_const(&valueS[1]));
									}
								}//for k
							}//for j
						}
						else
						{
							clauseP = clauseP || (!c.bool_const(&valueS[1]));
							for (int j = 0; j < Thread_Number; j++)
							{
								for (int k = 0; k < Thread_Number; k++)
								{
									*AllClausesInSystem[ClauseNumInSystem].clauseExp[j][k] = (*AllClausesInSystem[ClauseNumInSystem].clauseExp[j][k]) || (!(lc[j][k].bool_const(&valueS[1])));
									if (variableExpInSystem[j][k][atoi(&valueS[1])] == NULL)
									{
										variableExpInSystem[j][k][atoi(&valueS[1])] = new expr(lc[j][k]);
										*variableExpInSystem[j][k][atoi(&valueS[1])] = (lc[j][k].bool_const(&valueS[1]));
									}
								}//for k
							}//for j
						}
						AllClausesInSystem[ClauseNumInSystem].literals[AllClausesInSystem[ClauseNumInSystem].literalNum++] = atoi(&valueS[0]);
						if (VariableNumInSystem < atoi(&valueS[1])) VariableNumInSystem = atoi(&valueS[1]);
						for (int j = 0; j < Thread_Number; j++)
						{
							VariableInUnit[j][id][atoi(&valueS[1])] = 1;
							if (VariableNumInUnit[j][id] < atoi(&valueS[1])) VariableNumInUnit[j][id] = atoi(&valueS[1]);
						}
					}
					else if (atoi(valueS) > 0)
					{
						if (AllClausesInSystem[ClauseNumInSystem].literalNum == 0)
						{
							clauseP = c.bool_const(&valueS[0]);
							for (int j = 0; j < Thread_Number; j++)
							{
								for (int k = 0; k < Thread_Number; k++)
								{
									AllClausesInSystem[ClauseNumInSystem].clauseExp[j][k] = new expr(lc[j][k]);
									*AllClausesInSystem[ClauseNumInSystem].clauseExp[j][k] = (lc[j][k].bool_const(&valueS[0]));
									if (variableExpInSystem[j][k][atoi(&valueS[0])] == 0)
									{
										variableExpInSystem[j][k][atoi(&valueS[0])] = new expr(lc[j][k]);
										*variableExpInSystem[j][k][atoi(&valueS[0])] = (lc[j][k].bool_const(&valueS[0]));
									}
								}//for k
							}//for j
						}
						else
						{
							clauseP = clauseP || c.bool_const(&valueS[0]);
							for (int j = 0; j < Thread_Number; j++)
							{
								for (int k = 0; k < Thread_Number; k++)
								{
									*AllClausesInSystem[ClauseNumInSystem].clauseExp[j][k] = (*AllClausesInSystem[ClauseNumInSystem].clauseExp[j][k]) || ((lc[j][k].bool_const(&valueS[0])));
									if (variableExpInSystem[j][k][atoi(&valueS[0])] == NULL)
									{
										variableExpInSystem[j][k][atoi(&valueS[0])] = new expr(lc[j][k]);
										*variableExpInSystem[j][k][atoi(&valueS[0])] = (lc[j][k].bool_const(&valueS[0]));
									}
								}
							}//for
						}

						AllClausesInSystem[ClauseNumInSystem].literals[AllClausesInSystem[ClauseNumInSystem].literalNum++] = atoi(&valueS[0]);
						if (VariableNumInSystem < atoi(&valueS[0])) VariableNumInSystem = atoi(&valueS[0]);
						for (int j = 0; j < Thread_Number; j++)
						{
							VariableInUnit[j][id][atoi(&valueS[0])] = 1;
							if (VariableNumInUnit[j][id] < atoi(&valueS[0])) VariableNumInUnit[j][id] = atoi(&valueS[0]);
						}
					}
				}
				else
					valueS[ctr++] = cnf[i];

			}//for
			 // std::cout << AllClausesInSystem[ClauseNumInSystem].literalNum << " string = " << strT << "\n";
			if (AllClausesInSystem[ClauseNumInSystem].literalNum > 0) {
				AllClausesInSystem[ClauseNumInSystem].clauseNo = ClauseNumInSystem;
				sprintf(AllClausesInSystem[ClauseNumInSystem].clauseNoStr, "(%d )", ClauseNumInSystem);

				//std::cout << clauseP << " string = "<< strT << "\n";
				s.add(clauseP, AllClausesInSystem[ClauseNumInSystem].clauseNoStr);
				for (int j = 0; j < Thread_Number; j++)
				{
					if (j != id)
						(*sol[j][id]).add(to_expr(lc[j][id], Z3_translate(c, clauseP, lc[j][id])), AllClausesInSystem[ClauseNumInSystem].clauseNoStr);
					ClauseInUnit[j][id][ClauseNumInSystem] = 1;
				}


				ClauseNumInSystem++;
			}
			//			if (clauseSize > 0) { char p[32]; itoa(clauseNum, p, 10);  s.add(clauseP, p); memcpy( &clauseChar[id][clauseNum][0], &cnf[0], (((256) < (strlen(cnf))) ? (256) : (strlen(cnf))) ); clauseNum++;	};
		}//if
	}//while


	static int eachClauseNum = 0;
	if (ClauseNumInSystem - eachClauseNum > 1000) optLimit = 0;
	else if (ClauseNumInSystem - eachClauseNum > 800) optLimit = 20;
	else optLimit = 10;

	optLimit = 0;

	eachClauseNum = ClauseNumInSystem;


	std::cout << "clauseNum=" << ClauseNumInSystem << '\n';
	fclose(fp);

	// test
	/*	clock_t sT = clock();
	if (s.check() == unsat)
	{
	std::cout << "unsat" << "\n";
	int i = 0;
	}
	clock_t eT = clock();
	double duration = (double)(eT - sT);
	std::cout << "sat time=" << duration << "\n";
	*/
	//test


	return TRUE;
}

bool ReadShareFile(int id, context &c, solver &s, int op, char *shareName)
{
	for (int layer = 0; layer < Thread_Number; layer++)
	{
		shareVariableNum[layer][id][op] = min(VariableNumInUnit[layer][id], VariableNumInUnit[layer][op]);
		shareVariableNum[layer][op][id] = shareVariableNum[layer][id][op];
		for (int i = 0; i <= shareVariableNum[layer][id][op]; i++)
		{
			shareVariableValue[layer][id][op][i] = VariableInUnit[layer][id][i] & VariableInUnit[layer][op][i];
			shareVariableValue[layer][op][id][i] = shareVariableValue[layer][id][op][i];
		}
	}
	return 1;
}



BOOL IsSharedAmongAll(int layer, int value, int stack)
{
	return shareVariableValueAmongAll[layer][stack][value];
}


void GetShareNumberAmongAll4(int layer, int stack)
{
	for (int y = 1; y <= VariableNumInSystem; y++)
	{
		if (IsInShare(layer, stack, y)) continue;
		for (int i = 0; i < Thread_Number; i++)
		{
			//		if (i == ((layer + Thread_Number - 1) % Thread_Number)) continue;
			for (int next = 0; next < Thread_Number; next++)
			{
				if ((next == i)) continue;
				//			if ((next == i) || (next == ((layer + Thread_Number - 1) % Thread_Number))) continue;
				shareVariableValueAmongAll[layer][stack][y] |= shareVariableValue[layer][i][next][y];
				if (shareVariableValueAmongAll[layer][stack][y]) break;
			}
			if (shareVariableValueAmongAll[layer][stack][y]) break;
		}
		if (shareVariableValueAmongAll[layer][stack][y])
		{
			if (shareNumberAmongAll[layer][stack] < y) shareNumberAmongAll[layer][stack] = y;
			AllShareVariableValueIndex[layer][stack][AllShareNumIndex[layer][stack]++] = y;
		}
	}
}


void GetShareNumberAmongAll2(int layer, int deep, int stack)
{
	//	for (int next = layer; next <= deep + layer; next++)
	//	{
	int i = (layer + (Thread_Number - 1)) % Thread_Number;
	//		if (i == layer) continue;

	for (int threadN = 0; threadN < Thread_Number; threadN++)
	{
		for (int k = 1; k <= shareVariableNum[threadN][layer][i]; k++)
		{
			//		memset(AllShareNumIndex, 0, sizeof(AllShareNumIndex));  //layer
			//		memset(AllShareVariableValueIndex, 0, sizeof(AllShareVariableValueIndex));  //layer
			if (IsInShare(layer, stack, k)) continue;

			BOOL isNot = FALSE; int next = 0, isCompute = shareVariableValue[threadN][i][layer][k];
			for (next = layer; next <= deep + layer; next++)
			{
				int x = (next + (Thread_Number - 1)) % Thread_Number;
				if (x == i)  continue;

				isCompute &= shareVariableValue[threadN][i][x][k];
				if (!isCompute) break;
			}

			if (isCompute) {
				shareVariableValueAmongAll[layer][stack][k] = 1;
				if (shareNumberAmongAll[layer][stack] < k) shareNumberAmongAll[layer][stack] = k;
				AllShareVariableValueIndex[layer][stack][AllShareNumIndex[layer][stack]++] = k;
			};
		}//for k
	}
	//	}//for i
}

void GetShareNumberAmongAll()
{
	/*	for (int layer = 0; layer < Thread_Number; layer++)
	{
	for (int i = 1; i < Thread_Number; i++)
	{
	GetShareNumberAmongAll2((layer + 1) % Thread_Number, Thread_Number - i, i - 1);
	if ((shareNumberAmongAll[(layer + 1) % Thread_Number][i - 1] != 0))
	shareUniNum[(layer + 1) % Thread_Number][i - 1] = Thread_Number - i - 1;
	}
	}
	*/
	//	for (int layer = 0; layer < Thread_Number; layer++)
	//	{
	//		GetShareNumberAmongAll3((layer + 1) % Thread_Number, Thread_Number - 1, Thread_Number - 1);
	//		shareUniNum[(layer + 1) % Thread_Number][Thread_Number - 1] = Thread_Number - 2;
	/*		for (int i = 2; i < Thread_Number; i++)
	{
	GetShareNumberAmongAll3((layer + 1) % Thread_Number, i, i - 3 + Thread_Number);
	shareUniNum[(layer + 1) % Thread_Number][Thread_Number - 3 + i] = i-1;
	}
	*/
	//	}


	for (int layer = 0; layer < Thread_Number; layer++)
	{
		GetShareNumberAmongAll4(layer, Thread_Number - 1);
		shareUniNum[(layer + 1) % Thread_Number][Thread_Number - 1] = Thread_Number - 2;

		//		if (layer != 0)
		//		{
		//			memcpy(&shareVariableValueAmongAll[layer][Thread_Number - 1][0], &shareVariableValueAmongAll[0][Thread_Number - 1][0],sizeof(shareVariableValueAmongAll[0][Thread_Number - 1]));
		//			shareNumberAmongAll[layer][Thread_Number - 1] = shareNumberAmongAll[0][Thread_Number - 1];
		//			AllShareNumIndex[layer][Thread_Number - 1] = AllShareNumIndex[0][Thread_Number - 1];
		//			memcpy(&AllShareVariableValueIndex[layer][Thread_Number - 1][0], &AllShareVariableValueIndex[0][Thread_Number - 1][0], sizeof(AllShareVariableValueIndex[0][Thread_Number - 1]));
		//		}
	}
}

BOOL GetClause1(int layer, int id, int op, ClauseDes &des, int &addNum)
{
	if (ClauseIsInUnit(layer, id, des.clauseNo)) return FALSE;
	for (int i = 0; i < des.literalNum; i++)
	{
		if (abs(des.literals[i]) < 4)
			int ij = 0;
		if (AddNewShare(layer, id, abs(des.literals[i]))) {
			addNum++;
		}
	}
	return TRUE;
}



void GetShareModelFromLastChecking(int id, int op, int layer, context &c, solver &s)
{
	std::string strT;
	char strOther[256];
	model md = s.get_model();

	for (int i = 0; i < AllShareNumIndex[layer][Thread_Number - 1]; i++)
	{
		if (!IsInUnit(layer, id, AllShareVariableValueIndex[layer][Thread_Number - 1][i])) continue;
		strT = md.eval(*variableExpInSystem[layer][id][AllShareVariableValueIndex[layer][Thread_Number - 1][i]]).decl().name().str();
		if (strT == "false")
		{
			VariableAllUnitsAssignment[layer][AllShareVariableValueIndex[layer][Thread_Number - 1][i]] = 1;
		}
		else if (strT == "true")
		{
			VariableAllUnitsAssignment[layer][AllShareVariableValueIndex[layer][Thread_Number - 1][i]] = -1;
		}
		else
		{
			//		VariableAllUnitsAssignment[layer][AllShareVariableValueIndex[layer][Thread_Number - 1][i]] = -1;
			VariableAllUnitsAssignment[layer][AllShareVariableValueIndex[layer][Thread_Number - 1][i]] = 0;
		}
	}//for

	 //	std::cout << "Id= " << id << " op " << op << "extract model count = " << count << " uncount = " << unc << "stack" << stack << "share number" << shareNumberAmongAll[layer][Thread_Number - 1] << '\n';
}


void ImposeSpaceOnLastChecking(int id, int op, int layer, context &c, solver &s)
{
	/*	for (int i = 0; i < SPLIT_SPACE_NUM; i++)
	{
	if( SpaceSplit[layer]&(1<<i) )
	VariableAllUnitsAssignment[layer][AllShareVariableValueIndex[layer][Thread_Number - 1][i]] = -1;
	else
	VariableAllUnitsAssignment[layer][AllShareVariableValueIndex[layer][Thread_Number - 1][i]] = 1;
	}
	*/

	memcpy(&VariableAllUnitsAssignment[layer][0], &ConfirmedVariableAllUnitsAssignment[layer][0], sizeof(ConfirmedVariableAllUnitsAssignment[layer]));
}



void clauseVariableifConflict(int causeNo) {

	int conflictnum = 0, pos = 0;
	for (int i = 0; i < AllClausesInSystem[causeNo].literalNum; i++)	//对某子句的全部变量进行循环
	{
		pos = abs(AllClausesInSystem[causeNo].literals[i]);				//获得子句中某变量的变量号
		if (pos == abs(atoi(col[0])) || pos == abs(atoi(col[1])) || pos == abs(atoi(col[2]))) {						//若变量号为被赋值的变量号 conflictnum+1

			printf("\npos=%d,in ConfirmAssignments,abs(atoi(col[0]))=%d,abs(atoi(col[1]))=%d,abs(atoi(col[2]))=%d!!!!!!!!\n", pos, abs(atoi(col[0])), abs(atoi(col[1])), abs(atoi(col[2])));
			conflictnum++;
			printf("conflict=%d\n", conflictnum);
		}
	}

	if (conflictnum == AllClausesInSystem[causeNo].literalNum) {		//若子句中全部变量都属于000对应的变量号（都被提前赋值）关socket，告诉c关对这个server的socket
		printf("\nconflictnum=%d,AllClausesInSystem[causeNo].literalNum=%d,setEvent!!!!!!!!\n", conflictnum, AllClausesInSystem[causeNo].literalNum);
		for (int i = 0; i < Thread_Number; i++) SetEvent(existHandle[i]);
		SetEvent(recvHandle);
	}
	else {
		printf("\n conflictnum=%d,AllClausesInSystem[causeNo].literalNum=%d,continue,not setEvent!!!!\n", conflictnum, AllClausesInSystem[causeNo].literalNum);
	}

	//return 0;
}






BOOL ConfirmAssignments(int layer, int causeNo, BOOL isStartPart, BOOL &isConflictWithSplit)
{
	int confirmedCount = 0, pos = 0;
	BOOL is = FALSE, rootI = TRUE, oneStep = TRUE;
	for (int i = 0; i < AllClausesInSystem[causeNo].literalNum; i++)
	{
		pos = abs(AllClausesInSystem[causeNo].literals[i]);
		if (ConfirmedVariableAllUnitsAssignment[layer][pos] != 0) confirmedCount++;
	}
	//printf("\nconfirmedCount=%d,AllClausesInSystem[causeNo].literalNum=%d,tell close this socket!!!!!!!!\n", confirmedCount, AllClausesInSystem[causeNo].literalNum);
	if (AllClausesInSystem[causeNo].literalNum == confirmedCount)			//若不满足的子句中的变量全部被赋值
	{
		clauseVariableifConflict(causeNo);
		/**
		//这是我写的last！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
		int conflictnum = 0;
		for (int i = 0; i < AllClausesInSystem[causeNo].literalNum; i++)	//对某子句的全部变量进行循环
		{
		pos = abs(AllClausesInSystem[causeNo].literals[i]);				//获得子句中某变量的变量号
		if (pos == abs(atoi(col[0])) || pos == abs(atoi(col[1])) || pos == abs(atoi(col[2]))) {						//若变量号为被赋值的变量号 conflictnum+1

		printf("\npos=%d,in ConfirmAssignments,abs(atoi(col[0]))=%d,abs(atoi(col[1]))=%d,abs(atoi(col[2]))=%d!!!!!!!!\n", pos, abs(atoi(col[0])), abs(atoi(col[1])), abs(atoi(col[2])));
		conflictnum++;
		printf("conflict=%d\n", conflictnum);
		}
		}

		if (conflictnum == AllClausesInSystem[causeNo].literalNum) {		//若子句中全部变量都属于000对应的变量号（都被提前赋值）关socket，告诉c关对这个server的socket
		printf("\nconflictnum=%d,AllClausesInSystem[causeNo].literalNum=%d,tell close this socket!!!!!!!!\n", conflictnum, AllClausesInSystem[causeNo].literalNum);
		}
		else {
		printf("\n conflictnum=%d,AllClausesInSystem[causeNo].literalNum=%d,continue,not close this socket!!!!\n", conflictnum, AllClausesInSystem[causeNo].literalNum);
		}

		//这是我写的last！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
		*/
		isConflictWithSplit = TRUE;
		return TRUE;
	}

	if ((AllClausesInSystem[causeNo].literalNum - confirmedCount) != 1) return FALSE;

	for (int i = 0; i < AllClausesInSystem[causeNo].literalNum; i++)
	{
		pos = abs(AllClausesInSystem[causeNo].literals[i]);
		if (ConfirmedVariableAllUnitsAssignment[layer][pos] == 0)
		{
			if (AllClausesInSystem[causeNo].literals[i] > 0) ConfirmedVariableAllUnitsAssignment[layer][pos] = -1;
			else ConfirmedVariableAllUnitsAssignment[layer][pos] = 1;
			TempCount++;
			return TRUE;
		}
	}
	return FALSE;
}


BOOL GetNexSpaceSeg(int layer, int causeNo, BOOL isStartPart, BOOL &isConflictWithSplit)
{
	int pos;
	BOOL is = FALSE, rootI = TRUE, oneStep = TRUE;
	for (int i = 0; i < AllClausesInSystem[causeNo].literalNum; i++)
	{
		pos = abs(AllClausesInSystem[causeNo].literals[i]);
		if (pos > SPLIT_SPACE_NUM) { rootI = FALSE; oneStep = FALSE; continue; }
		//		space[pos] = 1;
		if (pos > SPLIT_SPACE_NUM - SPLIT_SPACE_SEG)
		{
			//	SpaceSplit[layer] = SpaceSplit[layer] | 0x1fffff;
			//	TestSplitSeg[layer] = 1;
			//	break;
			isConflictWithSplit = TRUE;
			continue;
		}

		rootI = FALSE; is = TRUE; isConflictWithSplit = TRUE;
		pos = 1 << (((pos - 1) / SPLIT_SPACE_SEG) * SPLIT_SPACE_SEG);
		pos = 1;
		if (TestSplitSeg[layer] == 0) TestSplitSeg[layer] = pos;
		else if (pos < TestSplitSeg[layer]) TestSplitSeg[layer] = pos;
	}

	if (rootI)
	{
		SpaceSplit[layer] = SpaceSplit[layer] | 0x1;
		return TRUE;
	}

	if ((!oneStep) && is)
	{
		if (!isStartPart)
		{
			TestSplitSeg[layer] = 0;
			return FALSE;
		}
	}

	return is;
}

BOOL GetNexSpaceSeg1(int layer, int causeNo, BOOL isStartPart, BOOL &isConflictWithSplit)
{
	int pos;
	BOOL is = FALSE, rootI = TRUE, oneStep = TRUE;
	for (int i = 0; i < AllClausesInSystem[causeNo].literalNum; i++)
	{
		pos = abs(AllClausesInSystem[causeNo].literals[i]);
		if (pos > SPLIT_SPACE_NUM) { rootI = FALSE; oneStep = FALSE; continue; }
		//		space[pos] = 1;
		if (pos > SPLIT_SPACE_NUM - SPLIT_SPACE_SEG)
		{
			//	SpaceSplit[layer] = SpaceSplit[layer] | 0x1fffff;
			//	TestSplitSeg[layer] = 1;
			//	break;
			isConflictWithSplit = TRUE;
			continue;
		}

		rootI = FALSE; is = TRUE; isConflictWithSplit = TRUE;
		//		pos = 1 << (((pos - 1) / SPLIT_SPACE_SEG) * SPLIT_SPACE_SEG);
		pos = 1 << pos;

		if (TestSplitSeg[layer] == 0) TestSplitSeg[layer] = pos;
		else if (pos < TestSplitSeg[layer]) TestSplitSeg[layer] = pos;
	}

	if (rootI)
	{
		SpaceSplit[layer] = SpaceSplit[layer] | 0x1;
		return TRUE;
	}

	if ((!oneStep) && is)
	{
		if (!isStartPart)
		{
			//	TestSplitSeg[layer] = 0;
			//	return FALSE;
		}		//Start
	}

	if (is && (TestSplitSeg[layer] > 1))
		int ij = 0;

	return is;
}

void ReverseModelByUncore(int layer, int causeNo)
{
	memset(&VariableAllUnitsAssignment[layer][0], 0, sizeof(VariableAllUnitsAssignment[0]));
	/*	for (int i = 0; i < AllClausesInSystem[causeNo].literalNum; i++)
	{
	if (AllClausesInSystem[causeNo].literals[i] > 0)
	{
	VariableAllUnitsAssignment[layer][AllClausesInSystem[causeNo].literals[i]] = -1;
	}
	else
	{
	VariableAllUnitsAssignment[layer][abs(AllClausesInSystem[causeNo].literals[i])] = 1;
	}
	}
	*/

}

short VerifLastFromLower(int id, int op, int layer, solver &s, int idC, int stac, BOOL &isConflictWithSplit)
{
	int size = 0, pos = 0, VerN = 0, count = 0;
	char strOther[256];
	int addCount = 0;

	EnterCriticalSection(&cs[layer][id]);
	s.push();
	for (int i = 0; i < AllShareNumIndex[layer][Thread_Number - 1]; i++)
	{
		if (!IsInUnit(layer, id, AllShareVariableValueIndex[layer][Thread_Number - 1][i])) continue;

		if (VariableAllUnitsAssignment[layer][AllShareVariableValueIndex[layer][Thread_Number - 1][i]] >= 1)
		{
			s.add(!*variableExpInSystem[layer][id][AllShareVariableValueIndex[layer][Thread_Number - 1][i]]);
		}
		else if (VariableAllUnitsAssignment[layer][AllShareVariableValueIndex[layer][Thread_Number - 1][i]] <= -1)
		{
			s.add(*variableExpInSystem[layer][id][AllShareVariableValueIndex[layer][Thread_Number - 1][i]]);
		}
	}

	//	if (stac >= 7)
	//std::cout << "Big  Id= " << id << " op = " << op << " layer = " << layer << " size= " << size << " svN= " << shareNumberAmongAll[layer][Thread_Number - 1] << " share ID op Number = " << shareVariableNum[op][id] << "stack"<<stac<<"ttt"<<ttt<<'\n';


	check_result res = s.check();

	if (res == unsat)
	{
		expr_vector eV = s.unsat_core();
		size = eV.size();
		s.pop();
		LeaveCriticalSection(&cs[layer][id]);
		std::string strT; int causeNo = 0; BOOL isSplit = FALSE;

		if (size >= 1)
		{
			if (id == idC)
				std::cout << "myId un core size = " << size << '\n';
			if ((size > 100) && (idC == id))
			{
				//    isConflictWithSplit = TRUE;
				TestSplitSeg[layer] = 1;
				return -1;
			}
			//		short involvedSpaceNo[SPLIT_SPACE_NUM+1];
			//		memset( &involvedSpaceNo[0], 0, sizeof(involvedSpaceNo) );
			for (unsigned int j = 0; j < size; j++)
			{
				EnterCriticalSection(&cs[layer][id]);
				std::string exStr = Z3_ast_to_string(lc[layer][id], eV[j]);
				LeaveCriticalSection(&cs[layer][id]);
				//				std::cout << "exStr = " << exStr << " unClause= " << *AllClausesInSystem[atoi(&exStr.c_str()[2])].clauseExp[op] << " No= " << AllClausesInSystem[atoi(&exStr.c_str()[2])].clauseNoStr << " id= " << id << " op = " << op << '\n';


				int wU = idC; causeNo = atoi(&exStr.c_str()[2]);
				//		if (id==layer)
				//		   isSplit = GetNexSpaceSeg1(layer, causeNo, idC == id, isConflictWithSplit);
				//		else
				//			   isSplit = GetNexSpaceSeg( layer, causeNo, idC == id, isConflictWithSplit);
				//		if (size == 1)
				isSplit = ConfirmAssignments(layer, causeNo, idC == id, isConflictWithSplit);

				for (int stU = 0; (stU < Thread_Number) && (idC != id) && (!isSplit); stU++)
				{
					//						if ((stU != idC) && (stU != op)) continue;
					//					if ((wU != op) ) { wU = (wU + 1) % Thread_Number;  continue; }
					//						if ((wU == idC) && (id!=layer) ) { wU = (wU + 1) % Thread_Number;  continue; }
					EnterCriticalSection(&cs[layer][Thread_Number]);
					//						if ( (stU == idC) && WillExpandShare(wU, AllClausesInSystem[atoi(&exStr.c_str()[2])])) { LeaveCriticalSection(&cs[Thread_Number]); wU = (wU + 1) % Thread_Number; continue; }
					if (!ClauseIsInUnit(layer, wU, causeNo))
					{
						GetClause1(layer, wU, id, AllClausesInSystem[causeNo], addCount);
						PutClauseInUnit(layer, wU, causeNo);
						LeaveCriticalSection(&cs[layer][Thread_Number]);

						EnterCriticalSection(&cs[layer][wU]);
						sol[layer][wU]->add(*AllClausesInSystem[causeNo].clauseExp[layer][wU], AllClausesInSystem[causeNo].clauseNoStr);
						LeaveCriticalSection(&cs[layer][wU]);
						VerN = 1;
					}
					else LeaveCriticalSection(&cs[layer][Thread_Number]);
					if (wU == op) break;
					wU = (wU + 1) % Thread_Number;
				}//for stu 

				 //   if(VerN==1)
				 //			std::cout << "verify other is not, After myId = " << id << " idC " << idC << " size= " << size << "  cause No = " << causeNo << "ver No  = " << VerN<< "\n\n";
			}//for j

			 //	if (involvedSpaceNo[22] && involvedSpaceNo[23] && involvedSpaceNo[24])
			 //		int ij = 0;

			if (addCount > 0)
			{
				EnterCriticalSection(&cs[layer][Thread_Number]);
				GetShareNumberAmongAll();
				/*				for (int i = 0; i < Thread_Number; i++) {
				if (shareUniNum[(idC + 1) % Thread_Number][i] != 0) {
				stackStart[(idC + 1) % Thread_Number] = i;
				break;
				}
				}*/
				LeaveCriticalSection(&cs[layer][Thread_Number]);
			}

		}//size

		if (addCount > 0)
			return -2;
		else if ((VerN != 1) && (size > 0))
		{
			/*		if ( (TestSplitSeg[layer] == 0) && (id!=idC) )
			{
			std::cout << "no uncore move  myid = " << id << '\n';
			isConflictWithSplit = TRUE;
			TestSplitSeg[layer] = 1;
			}
			*/
			if ((causeNo > 0) && (id == layer))
			{
				//		ReverseModelByUncore(layer, causeNo);
			}

			return -5;
		}
		else if (size>0) return -3;
		else return -1;
	}//if unsat

	GetShareModelFromLastChecking(id, op, layer, lc[layer][id], s);
	s.pop();
	LeaveCriticalSection(&cs[layer][id]);

	//	ImposeSpaceOnLastChecking(id, op, layer, lc[layer][id], s);

	return 1;
}


BOOL IsTestedAll(int layer)
{
	return (SpaceSplit[layer] & 0x1) == 0x1;
}

unsigned int IncreaseTestSpace(int layer)
{
	SpaceSplit[layer] = SpaceSplit[layer] + TestSplitSeg[layer];
	TestSplitSeg[layer] = 0;
	for (int j = 0; j < SPLIT_SPACE_NUM; j++)
	{
		if (SpaceSplit[layer] & (1 << j))
			ConfirmedVariableAllUnitsAssignment[layer][j + 1] = -1;
		else
			ConfirmedVariableAllUnitsAssignment[layer][j + 1] = 1;
	}
	return SpaceSplit[layer] & (0x1);
}

check_result StartCheck(int myId, BOOL &isConflictWithSplit)
{
	//	memset(&VariableAllUnitsAssignment[(myId+1)%Thread_Number][0], 0, sizeof(VariableAllUnitsAssignment[0]));
	ImposeSpaceOnLastChecking(myId, (myId + 1) % Thread_Number, (myId + 1) % Thread_Number, lc[(myId + 1) % Thread_Number][myId], *sol[(myId + 1) % Thread_Number][myId]);
	if (VerifLastFromLower(myId, (myId + Thread_Number - 1) % Thread_Number, (myId + 1) % Thread_Number, *sol[(myId + 1) % Thread_Number][myId], myId, Thread_Number - 1, isConflictWithSplit) <= -1)
	{
		return unsat;
	}
	else {
		return sat;
	}

}



check_result StartCheck1(int myId, BOOL &isConflictWithSplit)
{
	//	memset(&VariableAllUnitsAssignment[(myId+1)%Thread_Number][0], 0, sizeof(VariableAllUnitsAssignment[0]));
	//ImposeSpaceOnLastChecking(myId, (myId + 1) % Thread_Number, (myId + 1) % Thread_Number, lc[(myId + 1) % Thread_Number][myId], *sol[(myId + 1) % Thread_Number][myId]);
	solver &s = *sol[(myId + 1) % Thread_Number][myId];
	check_result res = s.check();

	if (res == unsat) {
		return unsat;
	}
	else {
		return sat;
	}

}

DWORD WINAPI ThreadProcc(void* lp)
{
	int myId = *((int*)lp);
	BOOL Sat = FALSE;
	try {
		DWORD hRes = 0;
		solver &s = *sol[myId][myId];
		int lastId = 0, optimalC = 0, Upper = shareUniNum[myId][stack[myId]];



		while (true)
		{
			hRes = WaitForMultipleObjects((DWORD)Thread_Number, hHandle[myId], false, INFINITE);
			if (hRes == WAIT_FAILED) break;
			int idC = hRes - WAIT_OBJECT_0;
			if (idC != myId)
			{
				short VOresult = 0; BOOL isNot = FALSE, isSplitCon = FALSE;
				int startUnit = (myId + Thread_Number - 1) % Thread_Number;

				for (int i = 0; i < Thread_Number - 1; i++)
				{
					VOresult = VerifLastFromLower((startUnit + 1) % Thread_Number, startUnit, myId, *sol[myId][(startUnit + 1) % Thread_Number], idC, Thread_Number - 1, isSplitCon);
					//						if( i>4 )
					//					std::cout << "layer " << myId << "stack " << stack[myId] << " I = " << i << " Vresult= " << VOresult << " myId =" << (startUnit + 1) % Thread_Number << " op =" << startUnit << '\n';\


					if (VOresult <= -1)
					{
						HighToLowVR[idC][myId] = -1;
						isNot = TRUE;
						break;
					}
					startUnit = (startUnit + 1) % Thread_Number;
				}//for

				if (!isNot)
				{
					printf("*********\nisNot = FALSE, SAT....idC=%d,myId=%d*****\n", idC, myId);
					HighToLowVR[idC][myId] = 1;
				}
				else {
					//printf("*********\nisNot = TRUE, UNSAT....idC=%d,myId=%d*****\n", idC, myId);
				}

				if (isSplitCon)
				{
					std::cout << "VOresult checking complete with unsat myId = " << myId << '\n'; AllUnitResults[myId] = 0; SetEvent(existHandle[myId]); continue;
				}

				SetEvent(hHandle[idC][idC]);
				continue;
			}//if (idC != myId)

			static int cct = 0;

			EnterCriticalSection(&cs[(myId + 1) % Thread_Number][myId]);
			if (HighToLowVR[myId][(myId + 1) % Thread_Number] == -1)
			{
				Sat = FALSE;
				HighToLowVR[myId][(myId + 1) % Thread_Number] = 0;
				//				if (IsTestedAll((myId + 1) % Thread_Number)) { std::cout << "checking complete with unsat myId = " << myId << '\n'; AllUnitResults[myId] = 0; SetEvent(existHandle[myId]); continue; }
				//				if (TestSplitSeg[(myId + 1) % Thread_Number]) IncreaseTestSpace((myId + 1) % Thread_Number);
			}

			if (HighToLowVR[myId][(myId + 1) % Thread_Number] == 1)
			{
				endT = clock(); 	double duration = (double)(endT - startT);
				std::cout << "my sat time= " << duration << " myId= " << myId << "\n";
				for (int i = 0; i < Thread_Number; i++)  AllUnitResults[i] = TRUE;
				for (int i = 0; i < Thread_Number; i++) SetEvent(existHandle[i]);
			}
			LeaveCriticalSection(&cs[(myId + 1) % Thread_Number][myId]);



			if (Sat) { Sleep(0); 	continue; }
			BOOL isConflictWithSplit = FALSE;
			check_result res = StartCheck(myId, isConflictWithSplit);
			//	std::cout << "step = " << TestSplitSeg[(myId + 1) % Thread_Number] << " space = " << SpaceSplit[(myId + 1) % Thread_Number] << " unsat count = " << cct << '\n';

			if (res != unsat)
			{
				Sat = TRUE;
				HighToLowVR[myId][(myId + 1) % Thread_Number] = 0;
				SetEvent(hHandle[(myId + 1) % Thread_Number][myId]);
			}
			else     // unSat
			{
				cct++; isConflictWithSplit = FALSE;
				if (!isConflictWithSplit) { std::cout << "cct checking complete with unsat myId = " << myId << " tCount = " << TempCount << '\n'; AllUnitResults[myId] = 0; SetEvent(existHandle[myId]); continue; }
				//			if (IsTestedAll((myId + 1) % Thread_Number)) { std::cout << "cct1 checking complete with unsat myId = " << myId << '\n'; AllUnitResults[myId] = 0; SetEvent(existHandle[myId]); continue; }
				//			if (TestSplitSeg[(myId + 1) % Thread_Number]) IncreaseTestSpace((myId + 1) % Thread_Number);
				SetEvent(hHandle[myId][myId]);
				printf("\n********重新计算*********\n");
			}//else



		}// while  TRUE
	}
	catch (z3::exception & ex) {
		std::cout << "myId= " << myId << " unexpected error: " << ex << "\n";
	}
	return 0L;
}