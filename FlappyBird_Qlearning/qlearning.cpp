#include <windows.h>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <time.h>

using namespace std;
#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))
#define RangeRand(x,y)((rand()%((y)-(x)+1))+(x))  //取得[x,y]的随机整数

#define GAMESIZE_W    800  // 游戏宽度
#define GAMESIZE_H    600  // 游戏高度
#define MS_PIPEHEIGHT       420
#define MS_GROUNDHEIGHT     112

#define MS_STAGEMOVESPEED 150   // 移动速度
#define MS_PIPECREATETIME 1.7   // 管子创建时间间隔
#define MS_PIPEGAPHEIGHT  100   // 管子间隙高度
#define MS_PIPECREATEYMINRESERVED  0.f
#define MS_PIPECREATEYMAXRESERVED  100.f
#define MS_PIPECREATEYMIN (GAMESIZE_H - MS_PIPEHEIGHT - (MS_PIPEGAPHEIGHT / 2.f) + MS_PIPECREATEYMINRESERVED) // 管子间隙中心最小Y值
#define MS_PIPECREATEYMAX (MS_PIPEHEIGHT + (MS_PIPEGAPHEIGHT / 2.f) - MS_PIPECREATEYMAXRESERVED)              // 管子间隙中心最大Y值

#define BUFSIZE 50
#define XGRID 6 // X方向离散化每格像素数
#define YGRID 6 // Y方向离散化每格像素数
#define GYGRID 40  // Y方向离地距离离散化每格像素数 

#define PIPE_GAP (MS_STAGEMOVESPEED*MS_PIPECREATETIME) // 管子间隔
#define MAX_X_OFFSET PIPE_GAP // X方向最大偏移量
#define MIN_X_OFFSET 0   // X方向最小偏移量

#define MAX_Y_OFFSET (0) // Y方向最大偏移量
#define MIN_Y_OFFSET (-MS_PIPEGAPHEIGHT)// Y方向最小偏移量

#define MAX_GY_OFFSET 40  // 距地距离最大偏移量
#define MIN_GY_OFFSET 0  // 距地距离最小偏移量

#define X_BIN (int((MAX_X_OFFSET-MIN_X_OFFSET)/XGRID))
#define Y_BIN (int((MAX_Y_OFFSET-MIN_Y_OFFSET)/YGRID) + 2)
#define GY_BIN (int((MAX_GY_OFFSET-MIN_GY_OFFSET)/GYGRID) + 1)
#define BIRD_STATE_NO 2 // 鸟的生命状态数 
#define ACTION_NO 2

typedef struct StateSpace{
	float h_dis; // horizontal distance between bird center and pipe center, which is pipeCenter.x - bird.x
	float v_dis; // vertical distance between bird center and pipe center, which is pipeCenter.y - bird.y
	float g_dis; // distance between bird center and ground
	int life;    // bird died, then life = 0, else life = 1
}State;

typedef struct StateSpaceBins{
	int h_bin; // horizontal distance discretization bins
	int v_bin; // vertical distance discretization bins
	int g_bin; // distance between bird center and ground discretization bins
	int l_bin; // life state bins
	StateSpaceBins(int h,int v,int g,int l):h_bin(h),v_bin(v),g_bin(g),l_bin(l){}
}StateBins;

typedef int ActionBins;

int ind_x(int x)
{
	return max(0,min(X_BIN-1,(x-MIN_X_OFFSET)/XGRID));
}

int ind_y(int y)
{
	if (y > MAX_Y_OFFSET)
		return Y_BIN - 1;
	else if (y < MIN_Y_OFFSET)
		return 0;
	return max(1,min(Y_BIN-2,(y-MIN_Y_OFFSET)/YGRID));
}

int ind_gy(int gy)
{
	if (gy < GYGRID)
		return 0;
	return max(1,min(GY_BIN-1,(gy-MIN_GY_OFFSET)/GYGRID));
}

float ***** createQ(const StateBins & sBin,const ActionBins & aBin)
{
	int xsize=sBin.h_bin, ysize=sBin.v_bin, gsize= sBin.g_bin, bstate = sBin.l_bin, zsize=aBin;
	float ***** Q = new float ****[xsize];
	for (int i = 0; i < xsize; ++i){
		Q[i] = new float ***[ysize];
		for (int j = 0; j < ysize ; ++j){
			Q[i][j] = new float **[gsize];
			for (int m = 0; m < gsize; ++m){
				Q[i][j][m] = new float *[bstate];
				for (int k = 0; k < bstate; ++k)
					Q[i][j][m][k] = new float [zsize];
			}
		}
	}
	return Q;
}
void releaseQ(float *****Q, const StateBins & sBin,const ActionBins & aBin)
{
	int xsize=sBin.h_bin, ysize=sBin.v_bin, gsize= sBin.g_bin, bstate = sBin.l_bin, zsize=aBin;
	for (int i = 0; i < xsize; ++i){
		for (int j = 0; j < ysize ; ++j){
			for (int m = 0; m < gsize; ++m){
				for (int k = 0; k < bstate; ++k){
					delete Q[i][j][m][k];
				}
				delete Q[i][j][m];
			}
			delete Q[i][j];
		}
		delete Q[i];
	}
	delete Q;
}

bool initQ(float *****Q, const string &filepath,const StateBins & sBin,const ActionBins & aBin)
{
	int xsize=sBin.h_bin, ysize=sBin.v_bin, gsize= sBin.g_bin, bstate = sBin.l_bin, zsize=aBin;
	char filename[100];
	sprintf(filename,"%s\\Q%d%d%d%d%d.txt",filepath.c_str(),xsize,ysize,gsize,bstate,zsize);
	fstream inputfile(filename,std::fstream::in);
	
	if (inputfile.fail()){
		cout << "open file error\n" << endl;
		inputfile.close();
		for (int i = 0; i < xsize; ++i)
			for (int j = 0; j < ysize ; ++j)
				for (int n = 0; n < gsize; ++n)
					for (int k = 0; k < bstate; ++k)
						for (int m = 0; m < zsize; ++m)
							Q[i][j][n][k][m] = 0; 
	}
	else{
		cout << "loading data ..." << flush ;
		for (int i = 0; i < xsize; ++i){
			cout << "." << flush ;
			for (int j = 0; j < ysize ; ++j)
				for (int n = 0; n < gsize; ++n)
					for (int k = 0; k < bstate; ++k)
						for (int m = 0; m < zsize; ++m)
							inputfile >> Q[i][j][n][k][m];
		}
		inputfile.close();
		cout << endl << "load data completed!" << endl;
	}
	return true;
}

int findMax(float *****Q,float *value, const StateBins & sBin,const ActionBins & aBin)
{
	int kk=0;
	*value = -1000000000;
	for (int k = 0; k < aBin; ++k){
		if (*value < Q[sBin.h_bin][sBin.v_bin][sBin.g_bin][sBin.l_bin][k]){
			*value = Q[sBin.h_bin][sBin.v_bin][sBin.g_bin][sBin.l_bin][k];
			kk = k;
		}
	}
	return kk;
}

bool writeQ(float *****Q, const string &filepath,const StateBins & sBin,const ActionBins & aBin)
{
	int xsize=sBin.h_bin, ysize=sBin.v_bin, gsize= sBin.g_bin, bstate = sBin.l_bin, zsize=aBin;
	char filename[500];
	sprintf(filename,"copy %s\\Q%d%d%d%d%d.txt Q%d%d%d%d%d_backup.txt",filepath.c_str(),xsize,ysize,gsize,bstate,zsize,xsize,ysize,gsize,bstate,zsize);
	system(filename);
	sprintf(filename,"%s\\Q%d%d%d%d%d.txt",filepath.c_str(),xsize,ysize,gsize,bstate,zsize);
	fstream outputfile(filename,std::fstream::out);
	if (outputfile.fail()){
		cout << "open file error\n" << endl;
		return false;
	}
	cout << "saving data ... " << flush ;

	for (int i = 0; i < xsize; ++i){
		cout << "." << flush ;
		for (int j = 0; j < ysize ; ++j){
			for (int n = 0; n < gsize; ++n){
				for (int k = 0; k < bstate; ++k){
					for (int m = 0; m < zsize; ++m){
						outputfile << left << setw(12) << Q[i][j][n][k][m];									
					}
				}
			}
			outputfile << endl;
		}
		outputfile << endl;
	}
	outputfile << endl;
	outputfile.close();
	cout << endl << "save data completed!" << endl;
	return true;
}

int main(int argc, char **argv)
{
	float discount = 1.0;
	float learningRate = 0.7;
	StateBins Sbin(X_BIN,Y_BIN,GY_BIN,BIRD_STATE_NO);
	ActionBins Abin(ACTION_NO);
	time_t start_t=0,end_t=0;

	HANDLE hPipe;
	hPipe = CreateNamedPipe( 
		"\\\\.\\Pipe\\Test",      // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		BUFSIZE,                  // output buffer size 
		BUFSIZE,                  // input buffer size 
		0,                        // client time-out 
		NULL);                    // default security attribute 

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		printf("Error: Cannot create communication channel\n");
		return -1;
	}
	printf("Create communication channel successfully\n");
	
	if (ConnectNamedPipe(hPipe, NULL) == FALSE) // 等待客户机的连接
	{
		CloseHandle(hPipe); // 关闭管道句柄
		printf("Flappy bird connect failed\n");
		return -1;
	}
	printf("Flappy bird connect successfully\n");
	if (argc > 1)
	{
		return 0;
	}

	float *****Q=createQ(Sbin,Abin); 
	initQ(Q,".\\",Sbin,Abin);
	int trainCount=0;	
	bool pipeReadError = 0;
	while (!pipeReadError)
	{
		DWORD ReadNum=0;
		// previous status and action index
		int p_indx = ind_x(0),p_indy = RangeRand(0,GY_BIN-1), p_indgy = 1, p_bs = 1, p_a = 1; 
		State S;  // current states
		// current states indexs and action from arg{max Q(s',a)} 
		int indx = 0,indy = 0,indgy = 0,birdstate = 1, action = 0;  
		float maxQnew = 0;  // max Q(s',a)
		bool dieFlag = 0;
		mouse_event(MOUSEEVENTF_LEFTDOWN,0,   0,   0,   0);    
		mouse_event(MOUSEEVENTF_LEFTUP,0,   0,   0,   0);
		while(1)
		{
			trainCount++;
			// Step4: get the next state s' after take action a from state s
			if (ReadFile(hPipe, &S.h_dis, sizeof(float), &ReadNum, NULL) == FALSE ||
				ReadFile(hPipe, &S.v_dis, sizeof(float), &ReadNum, NULL) == FALSE ||
				ReadFile(hPipe, &S.g_dis, sizeof(float), &ReadNum, NULL) == FALSE ||
				ReadFile(hPipe, &S.life, sizeof(int), &ReadNum, NULL) == FALSE )
			{	
				pipeReadError = 1;
				CloseHandle(hPipe);
				printf("Cannot get data from flappy bird\n");
				break;
			}
			// Find the bins which the state S belongs to
			indx = ind_x(S.h_dis);
			indy = ind_y(S.v_dis);
			indgy = ind_gy(S.g_dis);
			birdstate = S.life;
			
			// Step1: find the best action a for current state s
			// In order to get a faster convergence, some state space only have one action to take
			if (indy == Y_BIN - 1 )
			{
				action = 0;  // bird is higher than pipe center, action space just have "no click" action to take
			}
			else if ( indy == 0 || indgy == 0)
			{
				action = 1;  // bird is much lower than pipe center or it's close to ground, action space just have "click" action to take
			}
			else
			{
				StateBins staBin(indx,indy,indgy,birdstate);
				action = findMax(Q,&maxQnew,staBin,Abin);	 // otherwise,take the action to maximize Q(s',a)
			}

			// Step2: update the Q(s,a)
			if ( birdstate == 1)  // alive, reward R = 1
			{	
				Q[p_indx][p_indy][p_indgy][p_bs][p_a] += learningRate*(1 + discount*maxQnew - Q[p_indx][p_indy][p_indgy][p_bs][p_a]);
			}
			else // died, reward R = -1000
			{
				Q[p_indx][p_indy][p_indgy][p_bs][p_a] += learningRate*(-1000 + discount*maxQnew - Q[p_indx][p_indy][p_indgy][p_bs][p_a]);						
				dieFlag = 1;
			}

			// Step3: take the action a
			if (action)
			{
				// Jump
				mouse_event(MOUSEEVENTF_LEFTDOWN,0,   0,   0,   0);    
				mouse_event(MOUSEEVENTF_LEFTUP,0,   0,   0,   0);
			}
			
			// Step4: update s with s'
			p_indx = indx;
			p_indy = indy;
			p_indgy = indgy;
			p_a = action;
			p_bs = birdstate;

// 			start_t = clock();
// 			printf("the running time is : %f\n", double(start_t -end_t)*1000/CLOCKS_PER_SEC);
// 			end_t = start_t;
 			cout << left << setw(10) <<S.h_dis << setw(10) << S.v_dis << setw(10) << S.g_dis << setw(10) << S.life << setw(10) <<  action << endl;
			if (dieFlag)
			{
				cout << "-----bird is dying, God save me!!-----" <<endl;
				start_t = 0;
				end_t = 0;
				if (trainCount > 500)
				{
					start_t = clock();
					writeQ(Q,".\\",Sbin,Abin);  // save Q(s,a)
					trainCount = 0;
					end_t = clock();
				}
				Sleep(max(3000 - int(double(end_t - start_t)*1000/CLOCKS_PER_SEC),1));	
				break;  // restart the game
			}
		}
	}
	writeQ(Q,".\\",Sbin,Abin);
	releaseQ(Q,Sbin,Abin);
	return 0;
}