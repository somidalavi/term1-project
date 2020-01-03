#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <wingdi.h>
#include <unistd.h>
#include <ctype.h>


#define MAX_T_SIZE 16
#define MAX_ROW 4
#define MAX_COL 4
#define MAX_TOT_SIZE 20000
#define MAX_FILE_PATH 250
#define INF_DIST 1000000
#define WALLC '#'
#define PACMANC '0'
#define NORMALC '1'
#define PRIZEC '*'

const int CYCLELENGTH = 10;
const int PACMANH = 50;
const int PACMANW = 50;
const int n = 4,m = 4;
const int totalSize = 16;
const int nCell[4] = {1,4,-1,-4};
const char comtochar[4] = {'r','d','l','u'};
const int dx[4] = {1,0,-1,0};
const int dy[4] = {0,1,0,-1};

int dp[1 << MAX_T_SIZE][MAX_T_SIZE];
int mopt[1 << MAX_T_SIZE][MAX_T_SIZE];
struct {
	int pR,pC;
	int pX,pY;
	int curCycle;
	int state[MAX_ROW][MAX_COL];
	int coms[MAX_TOT_SIZE];
	int comssize,curcom;
} game;


int getnum(int i,int j) {return i * 4 + j;}
int getrow(int c){return c / m;}
int getcol(int c) {return c % m;}

int getpath(int pt[],int st,int en,int opt[][MAX_T_SIZE],int dist[][MAX_T_SIZE],int *pts){
	if (dist[st][en] == INF_DIST){
		printf("Life is nothing but a seductive illusion\n");
		fflush(stdout);
		printf("%d\n",1/0);
	}
	if (dist[st][en] == 1 ) {
		for (int c = 0; c < 4;c++){
			if (en - st == nCell[c]){
				pt[(*pts)++] = c;
				break;
			}
		}
		return 0;
	}
	getpath(pt,st,opt[st][en],opt,dist,pts);
	getpath(pt,opt[st][en],en,opt,dist,pts);
	return 0;
}



void preproc(){

	// reading the game table
	char fpath[MAX_FILE_PATH];
	gets(fpath);
	FILE *tfile = fopen(fpath,"r");
	char currow[MAX_COL + 10];
	for (int i = 0;i < n;i++){
		fgets(currow,10,tfile);
		for (int j = 0;j < m;j++){
			switch(currow[j]){
				case WALLC:
					game.state[i][j] = -1;
					break;
				case PRIZEC:
					game.state[i][j] = 1;
					break;
				case NORMALC:
					game.state[i][j] = 0;
					break;
				case PACMANC:
					game.state[i][j] = 0;
					game.pR = i;
					game.pC = j;
					break;
			}
				//printf("%d ",game.state[i][j]);
		}
	}
	fclose(tfile);
	// end of input

	// seting up floyd_warshall
	int dist[MAX_T_SIZE][MAX_T_SIZE] = {};
	int opt[MAX_T_SIZE][MAX_T_SIZE] = {};

	for (int i = 0;i < n;i++){
		for (int j = 0;j < m;j++){
			int curnum = getnum(i,j);
			for (int c = 0;c < 4;c++){
				int newnum = curnum + nCell[c];
				int ci = getrow(newnum),cj = getcol(newnum);
				if (newnum >= 0 && newnum < 16 && game.state[i][j] != -1 && game.state[ci][cj] != -1 && abs(ci - i) + abs(cj - j) == 1){
					dist[curnum][newnum] = 1;
					dist[newnum][curnum] = 1;
				}
			}
		}
	}
	for (int i = 0;i < totalSize;i++){
		for (int j = 0;j < totalSize;j++){
			opt[i][j] = -1;
			if (i == j) continue;
			if (dist[i][j] == 0) dist[i][j] = INF_DIST;

		}
	}
	// floyd_warshall set up
	// floyd_warshall
	int ndist;
	for (int k = 0;k < totalSize;k++){
		for (int j =0;j < totalSize;j++){
			for (int i = 0;i < totalSize;i++){
				ndist = dist[i][k] + dist[k][j];
				if (ndist < dist[i][j]){
					opt[i][j] = k;
					dist[i][j] = ndist;
				}
			}
		}
	}

	// end of floyd_warshall
	// bitmask DP TSP
	int mMask = 0;
	for (int i = 0;i < n;i++){
		for (int j = 0;j < m;j++){
			if (game.state[i][j] == 1){
				mMask |= (1 << getnum(i,j));
			}
		}
	}
	int EvenMoreMMask = mMask | (1 << getnum(game.pR,game.pC)); // Perfect naming

	int tt = (1 << totalSize);
	for (int i = 0;i < totalSize;i++)
		dp[0][i] = 0;
	for (int mask = 1;mask < tt ;mask++){
		if ((mask | mMask) > mMask) continue;
		for (int i = 0;i < totalSize;i++){
			if ((mask & (1 << i)) > 0) continue;
			dp[mask][i] = INF_DIST;
			mopt[mask][i] = -1;
			for (int j = 0;j < totalSize;j++){
				if ( (mask ^ (1 << j)) > mask) continue;
				ndist = dp[mask ^ (1 << j)][j] + dist[i][j];
				if (ndist < dp[mask][i]){
					mopt[mask][i] = j;
					dp[mask][i] = ndist;
				}
			}
			//printf("%d %d %d\n",mask,i,dp[mask][i]);
		}
	}


	// end of bitmask dp TSP
	// calculating final path
	int curMask = mMask;
	int curP = getnum(game.pR,game.pC),newP;
	while(curMask){
		getpath(game.coms,curP,mopt[curMask][curP],opt,dist,&game.comssize);
		newP = mopt[curMask][curP];
		curMask ^= (1 << mopt[curMask][curP]);
		curP = newP;
	}
	// final path calculated
}
void update(){
	if (game.curcom >= game.comssize) return ;
	game.curCycle++;
	game.pX += dx[game.coms[game.curcom]] * PACMANW / CYCLELENGTH;
	game.pY += dy[game.coms[game.curcom]] * PACMANH / CYCLELENGTH;
	if (game.curCycle == CYCLELENGTH){
		
		int curnum = getnum(game.pR,game.pC);
		curnum += nCell[game.coms[game.curcom]];
		game.pR = getrow(curnum);
		game.pC = getcol(curnum);
		game.curcom++;
		game.curCycle = 0;
		if (game.state[game.pR][game.pC] == 1) game.state[game.pR][game.pC] = 0;
	}
}

