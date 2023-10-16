#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <termios.h>
#include <fcntl.h>
#include <map>
#define ROW 10
#define COLUMN 50 
#define LOG_LENGTH 15

struct Node{
	int x , y; 
	Node( int _x , int _y ) : x( _x ) , y( _y ) {}; 
	Node(){} ; 
} frog ; 

pthread_mutex_t mutex;
std::map <char, int> mp;
int dir[4][2] = {0,1,0,-1,1,0,-1,0};
char map[ROW+10][COLUMN] ; 
int end = 0;
// Determine a keyboard is hit or not. If yes, return 1. If not, return 0. 
int kbhit(void){
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);

	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}


void *logs_move( void *t ){

	int start_pos = rand() % (COLUMN - 1);
	long ttid = (long)t;
	int tid = (int)ttid;
	/*  Move the logs  */
	
	while(!end){
		usleep(100000);

		pthread_mutex_lock(&mutex);
		if(tid&1){
			// move right
			for(int i = 0; i < COLUMN - 1; i++ )	
				map[tid][i] = ' ' ;  
			for(int i = start_pos + 1; i < start_pos + LOG_LENGTH + 1; i++) 
				map[tid][i % (COLUMN - 1)] =  '=';
			start_pos = (start_pos + 1) % (COLUMN - 1);
			frog.y += frog.x == tid;
			if(frog.y == COLUMN - 1) {end = 2; frog.y--;}
		}
		else {
			// move left
			for(int i = 0; i < COLUMN - 1; i++ )	
				map[tid][i] = ' ' ;  
			for(int i = start_pos - 1; i < start_pos + LOG_LENGTH - 1; i++) 
				map[tid][(i + COLUMN - 1) % (COLUMN - 1)] =  '=';
			start_pos = (start_pos - 1 + COLUMN - 1) % (COLUMN - 1);
			frog.y -= frog.x == tid;
			if(frog.y == 0) {end = 2; frog.y++;}

		}
		if(kbhit()){
			char d = getchar();
			if(d == 'q' || d == 'Q')
				end = 3;
			else {
				frog.x += dir[mp[d]][0];
				frog.y += dir[mp[d]][1];
				
				if(frog.x == 0) end = 1;
				else if(frog.x == ROW + 1) frog.x -= 1;
				else if(frog.y < 0 || frog.y >= COLUMN - 1) {
					end = 2;
					frog.x -= dir[mp[d]][0];
					frog.y -= dir[mp[d]][1];
				}
				else if(map[frog.x][frog.y] == ' ') end = 2;
			}	
		}
		for(int j = 0; j < COLUMN - 1; ++j )	
			map[ROW][j] = map[0][j] = '|' ;

		for(int j = 0; j < COLUMN - 1; ++j )	
			map[0][j] = map[0][j] = '|' ;

		map[frog.x][frog.y] = '0' ; 

		printf("\033[H\033[2J");
		for(int i = 0; i <= ROW; ++i)	
			puts( map[i] );
		pthread_mutex_unlock(&mutex);

	}
	switch(end){
		case 1:puts("Youn win");break;
		case 2:puts("You lose");break;
		case 3:puts("You quit the game");break;
	}
	pthread_exit(NULL);

	
}

int main( int argc, char *argv[] ){

	// Initialize the river map and frog's starting position
	memset( map , 0, sizeof( map ) ) ;
	int i , j ; 
	for( i = 1; i < ROW; ++i ){	
		for( j = 0; j < COLUMN - 1; ++j )	
			map[i][j] = ' ' ;  
	}	

	for( j = 0; j < COLUMN - 1; ++j )	
		map[ROW][j] = map[0][j] = '|' ;

	for( j = 0; j < COLUMN - 1; ++j )	
		map[0][j] = map[0][j] = '|' ;

	frog = Node( ROW, (COLUMN-1) / 2 ) ; 
	map[frog.x][frog.y] = '0' ; 

	//Print the map into screen
	for( i = 0; i <= ROW; ++i)	
		puts( map[i] );


	/*  Create pthreads for wood move and frog control.  */

	unsigned int seed;
    seed = static_cast<unsigned int>(time(NULL));
    srand(seed);

	mp['d'] = mp['D'] = 0;
	mp['a'] = mp['A'] = 1;
	mp['w'] = mp['W'] = 3;
	mp['s'] = mp['S'] = 2;

	pthread_t threads[ROW];
	int rc;
	for(long i = 1; i < ROW; i++){
		rc = pthread_create(&threads[i], NULL, logs_move, (void*)i);
		if(rc){
			printf("ERROE: return code from pthread_create() is %d\n", rc);
			exit(1);
		}
	}
	/*  Display the output for user: win, lose or quit.  */
	pthread_exit(NULL);
	return 0;

}
