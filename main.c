#include <pspuser.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>

// constants
#define MAX 3
#define TURN_1 1
#define TURN_2 2
#define TURN_E 0
#define TURN_FULL 3
#define BLINK_MAX 30

// PSP_MODULE_INFO is required
PSP_MODULE_INFO("gato", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);




int exit_callback(int arg1, int arg2, void *common) {
    sceKernelExitGame();
    return 0;
}

int callback_thread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int setup_callbacks(void) {
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, 0);
    if(thid >= 0)
        sceKernelStartThread(thid, 0, 0);
    return thid;
}

void initGame(int *board, int *x, int *y, int *turn, int *win){
	int i;

	for(i = 0; i < MAX * MAX; i++){
		board[i] = TURN_E;
	}
	
	*x = 0;
	*y = 0;
	*turn = TURN_1;
	*win = TURN_E;
}

int checkLine(int *board, int start, int inc){
	int s = board[start];
	int l = 0;
	int i, b = start;

	if(s != TURN_E){
		for(i = 0; i < MAX - 1; i++){
			b += inc;
			if(board[b] == s)
				l++;
		}
	}

	if(l == MAX - 1)
		return s;
	else
		return TURN_E;
}

int checkBoard(int *board){
	int win = TURN_E;

	int i;

	// horizontal
	for(i = 0; i < MAX && win == TURN_E; i++){
		win = checkLine(board, i * MAX, 1);
	}
	// vertical
	for(i = 0; i < MAX && win == TURN_E; i++){
		win = checkLine(board, i, MAX);
	}
	// diagonal	
	if(win == TURN_E)
		win = checkLine(board, 0, MAX + 1);
	if(win == TURN_E)
		win = checkLine(board, MAX - 1, MAX - 1);

	if(win == TURN_E){
		for(i = 0; i < MAX*MAX && board[i] != TURN_E; i++);
		
		if(i == MAX*MAX)
			win = TURN_FULL;
	}

	return win;
}

int main(void)  {
    // Use above functions to make exiting possible
    setup_callbacks();

	SceCtrlData pad, padTemp;

	int x, y;

	int i, j;

	int board[MAX * MAX];
	int b;
	int turn;
	int win;

	int blinkTimer = 0;

	initGame(board, &x, &y, &turn, &win);

    // print on a debug screen on a loop
    pspDebugScreenInit();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    
    while(1) {
        pspDebugScreenSetXY(0, 5);

        pspDebugScreenEnableBackColor(1);
        pspDebugScreenSetBackColor(0x00440054);
       	pspDebugScreenSetTextColor(0x00ffffff);

		pspDebugScreenPrintf("            ");
		if(win == TURN_1)
			pspDebugScreenPrintf("player 1 won");
		else if(win == TURN_2)
			pspDebugScreenPrintf("player 2 won");
		else if(win == TURN_FULL)
			pspDebugScreenPrintf("  tie");
		else
			pspDebugScreenPrintf("            ");
		pspDebugScreenPrintf("\n");
		
       	pspDebugScreenPrintf("      ");
        pspDebugScreenPrintf("gato :3\n");

		for(j = 0; j < MAX; j++){
			pspDebugScreenPrintf("      ");
			
	        for(i = 0; i < MAX; i++){
	        	if(x == i && y == j && blinkTimer > BLINK_MAX / 2){
        	       	pspDebugScreenSetTextColor(0x00ff00ff);
	        		if(turn == TURN_1)
	        			pspDebugScreenPrintf("X");
	        		else if(turn == TURN_2)
	        			pspDebugScreenPrintf("O");
	        		pspDebugScreenSetTextColor(0x00ffffff);
        		}
	        	else{
	      			b = board[i + (j * MAX)];
	        		if(b == TURN_1)
	        			pspDebugScreenPrintf("X");
	        		else if(b == TURN_2)
	        			pspDebugScreenPrintf("O");
	        		else if(b == TURN_E)
	        			pspDebugScreenPrintf(" ");
        		}

	        	if(i == MAX - 1)
	        		pspDebugScreenPrintf("\n");
	        	else
	        		pspDebugScreenPrintf("|");
	        }

			pspDebugScreenPrintf("      ");
	        if(j != MAX - 1){
	        	for(i = 0; i < MAX; i++){
	        		if(i == MAX - 1)
	        			pspDebugScreenPrintf("-");
        			else
	        			pspDebugScreenPrintf("--");
       			}
        	}
       		pspDebugScreenPrintf("\n");
		}
        
        // get pad data
        sceCtrlReadBufferPositive(&pad, 1);

        if(pad.Buttons & PSP_CTRL_UP && !padTemp.Buttons){
        	y--;
        	if (y < 0)
        		y = MAX - 1;
        }
        else if(pad.Buttons & PSP_CTRL_DOWN && !padTemp.Buttons){
        	y++;
        	if (y >= MAX)
        		y = 0;
        }
        else if(pad.Buttons & PSP_CTRL_LEFT && !padTemp.Buttons){
        	x--;
        	if (x < 0)
        		x = MAX - 1;
        }
        else if(pad.Buttons & PSP_CTRL_RIGHT && !padTemp.Buttons){
        	x++;	
        	if (x >= MAX)
        		x = 0;
        }
        
        if(pad.Buttons & PSP_CTRL_CROSS && !padTemp.Buttons){
        	if(win == TURN_E){
	        	if(board[x + (y * MAX)] == TURN_E){
	        		board[x + (y * MAX)] = turn;

	        		if(turn == TURN_1)
	        			turn = TURN_2;
	       			else
	       				turn = TURN_1;
	        	}
        	}
        	else{
        		initGame(board, &x, &y, &turn, &win);
        	}
        }

        padTemp = pad;

        win = checkBoard(board);
        
       	pspDebugScreenPrintf("            ");
       	if(win == TURN_E){
       		pspDebugScreenPrintf("(press X to play)\n");
       		
       		blinkTimer++;
       		if(blinkTimer >= BLINK_MAX){
       			blinkTimer = 0;
       		}
     	}
       	else{
       		pspDebugScreenSetTextColor(0x00ff00ff);
        	pspDebugScreenPrintf("(press X to restart)\n");
        	pspDebugScreenSetTextColor(0x00ffffff);
       	}

        sceDisplayWaitVblankStart();
    }

    return 0;
}
