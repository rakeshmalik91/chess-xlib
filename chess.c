/* CHESS 
 * Author 	: Rakesh Malik
 * Date started : 17.01.2011
 * Subject 	: Single/Double player CHESS game using XLib Graphical interface
 */

enum logic {false,true};

/* Shared variables */
enum player_computer_state {IDLE=0,REQUESTED=1,COMPLETED=100};
int *player_computer,*X1_return,*Y1_return,*X2_return,*Y2_return;

/* Header file inclusion */
#include<stdio.h>
#include<ctype.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<signal.h>
#include"window_contents.h"
#include"rules_chess.h"
#include"computer_chess.h"
#include"24bit_BMP_with_54byte_header.h"
#include"printf.h"

/* Definition of the constants */
#define WINDOW_WIDTH 1200						// Window width 
#define WINDOW_HEIGHT 800						// Window height 
#define X0 50								// Window co-ordinate 
#define Y0 50			

#define L 70								// Square length 
#define BW 20								// Border width 

/* Definition of the variables */
int turn=w,turns=0;
int X1=-1,Y1=-1,X2=-1,Y2=-1,picked=0;

struct BMP_24b_54B image[2][7],image_mask[7];				// Bitmap images
struct pixel
{
	unsigned long value;
	unsigned char transparent;
}
pixmap[2][7][L][L];							// Pixel value array of Bitmap images

struct tm total_time={0,0,0,0,0,0,0,0,0},				// Total time
	  white_time={0,0,0,0,0,0,0,0,0},				// Total time for only white player
	  black_time={0,0,0,0,0,0,0,0,0},				// Total time for only black player
	  turn_time={0,0,0,0,0,0,0,0,0},				// Time for current turn
	  remaining_time={0,0,0,0,0,0,0,0,0};				// Remaining time in timed game
time_t time1,time2;							// Used for calculation of one second
char hmax=0,mmax=40;							// Time for timed game 

enum player {HUMAN,COMPUTER};
char player[2]={HUMAN,HUMAN};						// Flags indicate the contrroller of white and black player

char playing=false;							// Flag indicates whether a game is running or not
char TimedGame=true;							// Flag indicates whether a timed game is being played or not
char RecordGame=false;							// Flag indicates whether the game is being recorded on not

FILE *rec;								// Recording file stream
char recordfilename[50]="\0";						// Recording file name

int child_pid;								// process ID computer player in other process

int mainmenu(Display*,Window,GC);					// Launches the mainmenu window
int endgame(Display*,Window,GC,char*);					// Launches the endgame window when checkmate/stalemate/timeout occurs
int move_piece_for_human(Display*,Window,GC);				// moves piece after a turn only for human player and does related works
int save(Display*,Window,GC);						// Launches save game window
int load(Display*,Window,GC);						// Launches load game window
void load_record_file(Display*,Window,GC);				// launches load record file window
void about_chess(Display *display,Window win, GC gc);			// Launches about chess window

void loadimages(Display *display);					// Loads all the needed images in memory
void print_piece_on_board(Display*,Window,GC,int,int,int,int);		// Prints a piece from board on screen
void printpiece(Display*,Window,GC,int,int,int,int);			// Prints any given piece



int main()
{
	int i,j,k,l,temp_value;						// used as temporary variables

	/* Allocating shared memory player computer */
	int shmid;
	if((shmid=shmget(1000,sizeof(int)*5,IPC_CREAT|0666))==-1)
	{
		perror("shmget() error");
		return;
	}
	if((player_computer=(int*)shmat(shmid,NULL,0))==(int*)(-1))
	{
		perror("shmat() error");
		return;
	}
	X1_return=player_computer+1;
	Y1_return=player_computer+2;
	X2_return=player_computer+3;
	Y2_return=player_computer+4;
	*player_computer=IDLE;


	/* Creating display */
	Display *display=XOpenDisplay(NULL);
	int screen_num = DefaultScreen(display);
	
	/* Loading piece images */
	loadimages(display);
	
	/* Creating colormaps */
	Colormap cm=DefaultColormap(display,screen_num);
	XColor darkred,lightred,red,brown,darkgreen,lightgreen,green,blue,black,white,grey1,grey2,slategray,lemon;
	XAllocNamedColor(display,cm,"rgb:af/1f/00",&darkred,&darkred);
	XAllocNamedColor(display,cm,"rgb:ff/8f/8f",&lightred,&lightred);
	XAllocNamedColor(display,cm,"red",&red,&red);
	XAllocNamedColor(display,cm,"brown",&brown,&brown);
	XAllocNamedColor(display,cm,"dark green",&darkgreen,&darkgreen);
	XAllocNamedColor(display,cm,"light green",&lightgreen,&lightgreen);
	XAllocNamedColor(display,cm,"green",&green,&green);
	XAllocNamedColor(display,cm,"blue",&blue,&blue);
	XAllocNamedColor(display,cm,"black",&black,&black);
	XAllocNamedColor(display,cm,"white",&white,&white);
	XAllocNamedColor(display,cm,"gray80",&grey1,&grey1);
	XAllocNamedColor(display,cm,"gray20",&grey2,&grey2);
	XAllocNamedColor(display,cm,"slate gray",&slategray,&slategray);
	XAllocNamedColor(display,cm,"lemon chiffon",&lemon,&lemon);

	/* Creating window */
	Window win,parent_window=RootWindow(display,screen_num);
	win=XCreateSimpleWindow(display,parent_window,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,2,black.pixel,black.pixel);
	XMapWindow(display,win);
	XTextProperty xtp;
	char *window_name="Chess";
	int rc=XStringListToTextProperty(&window_name,1,&xtp);
	XSetWMName(display,win,&xtp);

	/* Creating backup pixmap */
	Pixmap backup=XCreatePixmap(display,win,L,L,DefaultDepth(display,screen_num));

	/* Creating GC */
	XGCValues values;
	GC gc=XCreateGC(display,win,0,&values);
	XSetBackground(display,gc,blue.pixel);
	XSetLineAttributes(display,gc,2,LineSolid,CapNotLast,JoinMiter);
	XSetFillStyle(display,gc,FillSolid);
		
	/* Time */
	time1=time(&time1);
	localtime(&time1);

	/* Event */	
	XSelectInput(display,win,SubstructureRedirectMask | ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | 
			Button1MotionMask);
	XEvent event;
	XNextEvent(display,&event);

	/* Font */
	XFontStruct *font;
	//font=XLoadQueryFont(display,"*-helvetica-*-18-*");
	//XSetFont(display,gc,font->fid);
	
	int backupx,backupy;
	char s[20];
	Button *mainmenubutton;
	
	while(1)
	{	
		if(!playing)
			event.type=mainmenu(display,win,gc);
		time2=time(&time2);
		localtime(&time2);
		if((i=difftime(time2,time1))>=1)
		{
			/* Total time */
			(total_time.tm_sec)++;
			if(total_time.tm_sec>=60)
				(total_time.tm_min)++,total_time.tm_sec=0;
			if(total_time.tm_min>=60)
				(total_time.tm_hour)++,total_time.tm_min=0;
			/* Time for White */
			if(turn==w)
			{
				(white_time.tm_sec)++;
				if(white_time.tm_sec>=60)
					(white_time.tm_min)++,white_time.tm_sec=0;
				if(white_time.tm_min>=60)
					(white_time.tm_hour)++,white_time.tm_min=0;
			}
			/* Time for Black */
			else
			{
				(black_time.tm_sec)++;
				if(black_time.tm_sec>=60)
					(black_time.tm_min)++,black_time.tm_sec=0;
				if(black_time.tm_min>=60)
					(black_time.tm_hour)++,black_time.tm_min=0;
			}
			/* Total time */
			(turn_time.tm_sec)++;
			if(turn_time.tm_sec>=60)
				(turn_time.tm_min)++,turn_time.tm_sec=0;
			if(turn_time.tm_min>=60)
				(turn_time.tm_hour)++,turn_time.tm_min=0;
			/* Remaining time */
			if(TimedGame)
			{
				(remaining_time.tm_sec)--;
				if(remaining_time.tm_sec<=0)
					(remaining_time.tm_min)--,remaining_time.tm_sec=59;
				if(remaining_time.tm_min<=0)
					(remaining_time.tm_hour)--,remaining_time.tm_min=59;
			}

			XSetForeground(display,gc,black.pixel);
			XFillRectangle(display,win,gc,900,100,200,30);
			XFillRectangle(display,win,gc,900,130,200,30);
			XFillRectangle(display,win,gc,900,160,200,30);
			XFillRectangle(display,win,gc,900,190,200,30);
			XFillRectangle(display,win,gc,900,220,200,30);
			XSetForeground(display,gc,red.pixel);
			sprintf(s,"%dh %dm %ds",total_time.tm_hour,total_time.tm_min,total_time.tm_sec);
			XDrawString(display,win,gc,900,115,s,strlen(s));
			if(TimedGame)
			{
				sprintf(s,"%dh %dm %ds",remaining_time.tm_hour,remaining_time.tm_min,remaining_time.tm_sec);
				XDrawString(display,win,gc,900,245,s,strlen(s));
			}
			sprintf(s,"%dh %dm %ds",turn_time.tm_hour,turn_time.tm_min,turn_time.tm_sec);
			XDrawString(display,win,gc,900,145,s,strlen(s));
			XSetForeground(display,gc,lemon.pixel);
			sprintf(s,"%dh %dm %ds",white_time.tm_hour,white_time.tm_min,white_time.tm_sec);
			XDrawString(display,win,gc,900,175,s,strlen(s));
			XSetForeground(display,gc,slategray.pixel);
			sprintf(s,"%dh %dm %ds",black_time.tm_hour,black_time.tm_min,black_time.tm_sec);
			XDrawString(display,win,gc,900,205,s,strlen(s));
			
			time1=time(&time1);
			localtime(&time1);

			/* Checking for CHECK */
			if(king_in_check(turn))
			{
				XSetForeground(display,gc,black.pixel);
				XFillRectangle(display,win,gc,X0,Y0+L*8+85,200,50);
				XSetForeground(display,gc,(turn==w)?lemon.pixel:slategray.pixel);
				XDrawString(display,win,gc,X0,Y0+L*8+BW+100,
						(turn==w)?"White king in check":"Black king in check",19);
			}
			/* CHECKMATE & STALEMATE */
			if(mate(turn))
			{
				if(king_in_check(turn)) 
					event.type=endgame(display,win,gc,(turn==w)?"White Checkmate":"Black Checkmate");
				else 
					event.type=endgame(display,win,gc,(turn==w)?"White Stalemate":"Black Stalemate");
			}
			/* Checking Time out */
			if(TimedGame && remaining_time.tm_hour==0 && remaining_time.tm_min==0 && remaining_time.tm_sec==0)
				event.type=endgame(display,win,gc,(point[w]>point[b])?"Time out : White wins":
						(point[w]<point[b])?"Time out : Black wins":"Time out : Match tie");
		}
		
		if(event.type==MapRequest || event.type==Expose)
		{
			XSetForeground(display,gc,black.pixel);
			XFillRectangle(display,win,gc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
			/* Printing board */
			XSetForeground(display,gc,brown.pixel);
			XFillRectangle(display,win,gc,X0-BW,Y0-BW,8*L+2*BW,BW);
			XFillRectangle(display,win,gc,X0-BW,Y0+8*L,8*L+2*BW,BW);
			XFillRectangle(display,win,gc,X0-BW,Y0,BW,8*L);
			XFillRectangle(display,win,gc,X0+8*L,Y0,BW,8*L);
			for(i=0;i<8;i++)
				for(j=0;j<8;j++)
				{
					if((i+j)%2==1)
						XSetForeground(display,gc,grey2.pixel);
					else
						XSetForeground(display,gc,grey1.pixel);
					XFillRectangle(display,win,gc,X0+i*L,Y0+j*L,L,L);
				}
			XSync(display,0);
			
			/* Printing pieces */
			for(i=0;i<8;i++)
				for(j=0;j<8;j++)
					if(board[j][i].c!=n && (!picked || X1!=i || Y1!=j || X2!=-1))
						print_piece_on_board(display,win,gc,i,j,X0+i*L,Y0+j*L);
			
			XSync(display,0);
			/* Printing Text */
			XSetForeground(display,gc,white.pixel);
			XDrawString(display,win,gc,X0,Y0+L*8+BW+20,"Turn for ",9);
			XDrawString(display,win,gc,X0+75,Y0+L*8+BW+20,(turn==w)?"White":"Black",5);
			for(i=0;i<8;i++)
			{
				sprintf(s,"%c",i+'A');
				XDrawString(display,win,gc,X0+i*L+L/2,20,s,1);
			}
			for(i=0;i<8;i++)
			{
				sprintf(s,"%d",i+1);
				XDrawString(display,win,gc,0,Y0+i*L+L/2,s,1);
			}
			XSetForeground(display,gc,blue.pixel);
			XDrawString(display,win,gc,650,115,"Total time :",12);
			if(TimedGame) XDrawString(display,win,gc,650,245,"Remaining time :",16);
			XDrawString(display,win,gc,650,145,"Time for current turn :",23);
			XSetForeground(display,gc,lemon.pixel);
			XDrawString(display,win,gc,650,175,"Total time for white :",22);
			XSetForeground(display,gc,slategray.pixel);
			XDrawString(display,win,gc,650,205,"Total time for black :",22);
			event.type=0;
			
			/* Printing taken pieces */
			XSetForeground(display,gc,blue.pixel);
			XDrawString(display,win,gc,650,300,"Pieces taken :",14);
			for(i=0;i<6 && i<num[w];i++) printpiece(display,win,gc,w,killed[w][i],650+i*L,320);
			for(i=6;i<12 && i<num[w];i++) printpiece(display,win,gc,w,killed[w][i],630+(i-6)*L,350);
			for(i=12;i<15 && i<num[w];i++) printpiece(display,win,gc,w,killed[w][i],700+(i-12)*L,380);
			for(i=0;i<6 && i<num[b];i++) printpiece(display,win,gc,b,killed[b][i],650+i*L,480);
			for(i=6;i<12 && i<num[b];i++) printpiece(display,win,gc,b,killed[b][i],630+(i-6)*L,510);
			for(i=12;i<15 && i<num[b];i++) printpiece(display,win,gc,b,killed[b][i],720+(i-12)*L,540);
				
			/* Mainmenu button */
			mainmenubutton=create_button(display,win,gc,900,600,100,30,green,grey2,black,green,"Pause");
		}
		
		/* Printing Computer State */
		//if(*player_computer!=temp_value)
		{
			if(player[turn]==COMPUTER)
			{
				XSetForeground(display,gc,red.pixel);
				XFillRectangle(display,win,gc,50,700,*player_computer*4,5);
				XSetForeground(display,gc,grey2.pixel);
				XFillRectangle(display,win,gc,50+*player_computer*4,700,400-(*player_computer*4),5);
			}
			else
			{
				XSetForeground(display,gc,grey2.pixel);
				XFillRectangle(display,win,gc,50,700,400,5);
			}
		}
		temp_value=*player_computer;
		
		/* if player is given from computer to human while calculating */
		if(player[turn]!=COMPUTER && *player_computer!=IDLE)		
			*player_computer=IDLE;
			
		/* Input */
		if(XCheckMaskEvent(display,SubstructureRedirectMask | ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | Button1MotionMask,&event)==0)
			if(*player_computer!=COMPLETED)
				continue;

		/* Pause Game */
		if(button_pressed(event,mainmenubutton) || (key_pressed(display,event,"Escape")))
			mainmenu(display,win,gc);
				
		/* ---------------Turn for Human---------- */
		if(player[turn]==HUMAN && event.type==ButtonPress && button_press_area(event,X0,Y0,8*L,8*L))
		{
			X1=(event.xbutton.x-X0)/L;
			Y1=(event.xbutton.y-Y0)/L;
			if(board[Y1][X1].c==turn)
			{	
				picked=1;
				if((X1+Y1)%2==0) XSetForeground(display,gc,grey1.pixel);
				else XSetForeground(display,gc,grey2.pixel);
				XFillRectangle(display,win,gc,X0+X1*L,Y0+Y1*L,L,L);
				/* Highlighting */
				XSetForeground(display,gc,blue.pixel);
				XDrawRectangle(display,win,gc,X0+X1*L+2,Y0+Y1*L+2,L-4,L-4);
				for(i=0;i<8;i++)
					for(j=0;j<8;j++)
						if(valid(X1,Y1,i,j))
						{
							if((i+j)%2==0) 
								if(board[j][i].c==!turn) XSetForeground(display,gc,lightred.pixel);
								else XSetForeground(display,gc,lightgreen.pixel);
							else 
								if(board[j][i].c==!turn) XSetForeground(display,gc,darkred.pixel);
								else XSetForeground(display,gc,darkgreen.pixel);
							XFillRectangle(display,win,gc,X0+i*L,Y0+j*L,L,L);
							if(board[j][i].c==!turn) XSetForeground(display,gc,red.pixel);
							else XSetForeground(display,gc,green.pixel);
							XDrawRectangle(display,win,gc,X0+i*L+2,Y0+j*L+2,L-4,L-4);
							if(board[j][i].c!=n)
								print_piece_on_board(display,win,gc,i,j,X0+i*L,Y0+j*L);
						}

				/* Making First Backup */
				XCopyArea(display,win,backup,gc,event.xbutton.x-L/2,event.xbutton.y-L/2,L,L,0,0);
				backupx=event.xbutton.x-L/2;
				backupy=event.xbutton.y-L/2;
			}
		}
		if(player[turn]==HUMAN && event.type==MotionNotify && picked)
		{
			/* Printing backup */
			XCopyArea(display,backup,win,gc,0,0,L,L,backupx,backupy);
			
			/* Making Backup & Reprinting piece */
			XCopyArea(display,win,backup,gc,event.xbutton.x-L/2,event.xbutton.y-L/2,L,L,0,0);
			print_piece_on_board(display,win,gc,X1,Y1,event.xbutton.x-L/2,event.xbutton.y-L/2);
			backupx=event.xbutton.x-L/2;
			backupy=event.xbutton.y-L/2;
		}
		if(player[turn]==HUMAN && event.type==ButtonRelease && picked)
		{
			picked=0;
			/* Removing Messege */
			XSetForeground(display,gc,black.pixel);
			XFillRectangle(display,win,gc,X0,Y0+L*8+85,300,50);
			
			/* Printing backup */
			XCopyArea(display,backup,win,gc,0,0,L,L,backupx,backupy);
			X2=(event.xbutton.x-X0)/L;
			Y2=(event.xbutton.y-Y0)/L;

			/* Removing Highlight */
			for(i=0;i<8;i++)
				for(j=0;j<8;j++)
					if(valid(X1,Y1,i,j) || (X1==i && Y1==j))
					{
						if((i+j)%2==0) XSetForeground(display,gc,grey1.pixel);
						else XSetForeground(display,gc,grey2.pixel);
						XFillRectangle(display,win,gc,X0+i*L,Y0+j*L,L,L);
						if(board[j][i].c!=n && (i!=X1 || j!=Y1))
							print_piece_on_board(display,win,gc,i,j,X0+i*L,Y0+j*L);
					}
			
			/* Reprinting piece */
			if(board[Y1][X1].c==turn && valid(X1,Y1,X2,Y2))
			{
				/* Normal move */
				if(board[Y2][X2].c!=n)
				{
					if((X2+Y2)%2==0) XSetForeground(display,gc,grey1.pixel);
					else XSetForeground(display,gc,grey2.pixel);
					XFillRectangle(display,win,gc,X0+X2*L,Y0+Y2*L,L,L);
				}
				/* Enpassent */
				if(board[Y1][X1].c==w && Y1==4 &&
					((last_x1==X1-1 && last_y1==6 && last_x2==X1-1 && last_y2==4 && Y2==5 && X2==X1-1 && X1-1>=0) ||
					(last_x1==X1+1 && last_y1==6 && last_x2==X1+1 && last_y2==4 && Y2==5 && X2==X1+1 && X1+1<8)))
				{
					if((Y2-1+X2)%2==0) XSetForeground(display,gc,grey1.pixel);
					else XSetForeground(display,gc,grey2.pixel);
					XFillRectangle(display,win,gc,X0+X2*L,Y0+(Y2-1)*L,L,L);
				}
				if(board[Y1][X1].c==b && Y1==3 &&
					((last_x1==X1-1 && last_y1==1 && last_x2==X1-1 && last_y2==3 && Y2==2 && X2==X1-1 && X1-1>=0) ||
					(last_x1==X1+1 && last_y1==1 && last_x2==X1+1 && last_y2==3 && Y2==2 && X2==X1+1 && X1+1<8)))
				{
					if((Y2+1+X2)%2==0) XSetForeground(display,gc,grey1.pixel);
					else XSetForeground(display,gc,grey2.pixel);
					XFillRectangle(display,win,gc,X0+X2*L,Y0+(Y2+1)*L,L,L);
				}
				/* Castelling */
				if(king_side_castle_white(X1,Y1,X2,Y2))
				{
					print_piece_on_board(display,win,gc,7,0,X0+5*L,Y0+0*L);
					XSetForeground(display,gc,grey2.pixel);
					XFillRectangle(display,win,gc,X0+7*L,Y0+0*L,L,L);
					XSetForeground(display,gc,lemon.pixel);
					XDrawString(display,win,gc,X0,Y0+L*8+BW+100,"White king castled king's side",30);
				}
				if(queen_side_castle_white(X1,Y1,X2,Y2))
				{
					print_piece_on_board(display,win,gc,0,0,X0+3*L,Y0+0*L);
					XSetForeground(display,gc,grey1.pixel);
					XFillRectangle(display,win,gc,X0+0*L,Y0+0*L,L,L);
					XSetForeground(display,gc,lemon.pixel);
					XDrawString(display,win,gc,X0,Y0+L*8+BW+100,"White king castled queen's side",31);
				}
				if(king_side_castle_black(X1,Y1,X2,Y2))
				{
					print_piece_on_board(display,win,gc,7,7,X0+5*L,Y0+7*L);
					XSetForeground(display,gc,grey1.pixel);
					XFillRectangle(display,win,gc,X0+7*L,Y0+7*L,L,L);
					XSetForeground(display,gc,slategray.pixel);
					XDrawString(display,win,gc,X0,Y0+L*8+BW+100,"Black king castled king's side",30);
				}
				if(queen_side_castle_black(X1,Y1,X2,Y2))
				{
					print_piece_on_board(display,win,gc,0,7,X0+3*L,Y0+7*L);
					XSetForeground(display,gc,grey2.pixel);
					XFillRectangle(display,win,gc,X0+0*L,Y0+7*L,L,L);
					XSetForeground(display,gc,slategray.pixel);
					XDrawString(display,win,gc,X0,Y0+L*8+BW+100,"Black king castled queen's side",31);
				}
				print_piece_on_board(display,win,gc,X1,Y1,X0+X2*L,Y0+Y2*L);
				
				/* Move piece */
				event.type=move_piece_for_human(display,win,gc);

				/* Recording */
				if(RecordGame)
				{
					fprintf(rec,"%s : H : %c%d-%c%d : %s\n",
							(turn==w)?"White":"Black",
							'A'+X1,1+Y1,'A'+X2,1+Y2,
							(board[Y2][X2].p==P)?"Pawn":
							(board[Y2][X2].p==B)?"Bishop":
							(board[Y2][X2].p==Kn)?"Knight":
							(board[Y2][X2].p==K)?"King":
							(board[Y2][X2].p==R)?"Rook":
							(board[Y2][X2].p==Q)?"Queen":"");
				}
				
				/* Updating Variables */
				if(turn==w) turns++;
				turn=!turn;
				turn_time.tm_sec=turn_time.tm_min=turn_time.tm_hour=0;

			}
			else
				print_piece_on_board(display,win,gc,X1,Y1,X0+X1*L,Y0+Y1*L);
			X1=Y1=X2=Y2=-1;
			
			/* Reprinting Text */
			XSetForeground(display,gc,black.pixel);
			XFillRectangle(display,win,gc,X0+75,Y0+L*8+BW,60,30);
			XSetForeground(display,gc,white.pixel);
			XDrawString(display,win,gc,X0+75,Y0+L*8+BW+20,(turn==w)?"White":"Black",5);
			
			/* Printing taken pieces */
			XSetForeground(display,gc,blue.pixel);
			XDrawString(display,win,gc,650,300,"Pieces taken :",14);
			for(i=0;i<6 && i<num[w];i++) printpiece(display,win,gc,w,killed[w][i],650+i*L,320);
			for(i=6;i<12 && i<num[w];i++) printpiece(display,win,gc,w,killed[w][i],630+(i-6)*L,350);
			for(i=12;i<15 && i<num[w];i++) printpiece(display,win,gc,w,killed[w][i],700+(i-12)*L,380);
			for(i=0;i<6 && i<num[b];i++) printpiece(display,win,gc,b,killed[b][i],650+i*L,480);
			for(i=6;i<12 && i<num[b];i++) printpiece(display,win,gc,b,killed[b][i],630+(i-6)*L,510);
			for(i=12;i<15 && i<num[b];i++) printpiece(display,win,gc,b,killed[b][i],720+(i-12)*L,540);
		}
		
		/* -------------Turn for Computer---------------- */
		if(player[turn]==COMPUTER && *player_computer==IDLE)
		{
			/* Removing Messe3ge */
			XSetForeground(display,gc,black.pixel);
			XFillRectangle(display,win,gc,X0,Y0+L*8+85,300,50);
			
			/* if CHECKMATE & STALEMATE */
			if(!mate(turn))
			{	
				/* Generating move & moving piece */
				if(*player_computer==IDLE)
				{
					*player_computer=REQUESTED;
					child_pid=fork();
					if(child_pid==0)
					{
						getmove(turn,&X1,&Y1,&X2,&Y2);
						*X1_return=X1,*Y1_return=Y1,*X2_return=X2,*Y2_return=Y2;
						*player_computer=COMPLETED;
						_exit(1);
					}
				}
			}
		}
		if(player[turn]==COMPUTER && *player_computer==COMPLETED)
		{
			/* Reprinting Text */
			XSetForeground(display,gc,black.pixel);
			XFillRectangle(display,win,gc,X0+75,Y0+L*8+BW,60,30);
			XSetForeground(display,gc,white.pixel);
			XDrawString(display,win,gc,X0+75,Y0+L*8+BW+20,(turn==w)?"White":"Black",5);

			X1=*X1_return,Y1=*Y1_return,X2=*X2_return,Y2=*Y2_return;
			movepiece(X1,Y1,X2,Y2,0);
			if(board[Y2][X2].p==P && ((Y2==7 && board[Y2][X2].c==w) || (Y2==0 && board[Y2][X2].c==b)))
				board[Y2][X2].p=Q;
		
			/* Updating board in screen */
				/* Normal move */
			if((X1+Y1)%2==0) XSetForeground(display,gc,grey1.pixel);
			else XSetForeground(display,gc,grey2.pixel);
			XFillRectangle(display,win,gc,X0+X1*L,Y0+Y1*L,L,L);
			if((X2+Y2)%2==0) XSetForeground(display,gc,grey1.pixel);
			else XSetForeground(display,gc,grey2.pixel);
			XFillRectangle(display,win,gc,X0+X2*L,Y0+Y2*L,L,L);
			print_piece_on_board(display,win,gc,X2,Y2,X0+X2*L,Y0+Y2*L);
				/* Enpassent */
			if(board[Y1][X1].c==w && Y1==4 &&
				((last_x1==X1-1 && last_y1==6 && last_x2==X1-1 && last_y2==4 && Y2==5 && X2==X1-1 && X1-1>=0) ||
				(last_x1==X1+1 && last_y1==6 && last_x2==X1+1 && last_y2==4 && Y2==5 && X2==X1+1 && X1+1<8)))
			{
				if((Y2-1+X2)%2==0) XSetForeground(display,gc,grey1.pixel);
				else XSetForeground(display,gc,grey2.pixel);
				XFillRectangle(display,win,gc,X0+X2*L,Y0+(Y2-1)*L,L,L);
			}
			if(board[Y1][X1].c==b && Y1==3 &&
				((last_x1==X1-1 && last_y1==1 && last_x2==X1-1 && last_y2==3 && Y2==2 && X2==X1-1 && X1-1>=0) ||
				(last_x1==X1+1 && last_y1==1 && last_x2==X1+1 && last_y2==3 && Y2==2 && X2==X1+1 && X1+1<8)))
			{
				if((Y2+1+X2)%2==0) XSetForeground(display,gc,grey1.pixel);
				else XSetForeground(display,gc,grey2.pixel);
				XFillRectangle(display,win,gc,X0+X2*L,Y0+(Y2+1)*L,L,L);
			}
				/* Castelling */
			if(king_side_castle_white(X1,Y1,X2,Y2))
			{
				print_piece_on_board(display,win,gc,7,0,X0+5*L,Y0+0*L);
				XSetForeground(display,gc,grey2.pixel);
				XFillRectangle(display,win,gc,X0+7*L,Y0+0*L,L,L);
				XSetForeground(display,gc,lemon.pixel);
				XDrawString(display,win,gc,X0,Y0+L*8+BW+100,"White king castled king's side",30);
			}
			if(queen_side_castle_white(X1,Y1,X2,Y2))
			{
				print_piece_on_board(display,win,gc,0,0,X0+3*L,Y0+0*L);
				XSetForeground(display,gc,grey1.pixel);
				XFillRectangle(display,win,gc,X0+0*L,Y0+0*L,L,L);
				XSetForeground(display,gc,lemon.pixel);
				XDrawString(display,win,gc,X0,Y0+L*8+BW+100,"White king castled queen's side",31);
			}
			if(king_side_castle_black(X1,Y1,X2,Y2))
			{
				print_piece_on_board(display,win,gc,7,7,X0+5*L,Y0+7*L);
				XSetForeground(display,gc,grey1.pixel);
				XFillRectangle(display,win,gc,X0+7*L,Y0+7*L,L,L);
				XSetForeground(display,gc,slategray.pixel);
				XDrawString(display,win,gc,X0,Y0+L*8+BW+100,"Black king castled king's side",30);
			}
			if(queen_side_castle_black(X1,Y1,X2,Y2))
			{
				print_piece_on_board(display,win,gc,0,7,X0+3*L,Y0+7*L);
				XSetForeground(display,gc,grey2.pixel);
				XFillRectangle(display,win,gc,X0+0*L,Y0+7*L,L,L);
				XSetForeground(display,gc,slategray.pixel);
				XDrawString(display,win,gc,X0,Y0+L*8+BW+100,"Black king castled queen's side",31);
			}
			
			/* Recording */
			if(RecordGame)
			{
				fprintf(rec,"%s : C : %c%d-%c%d : %s\n",
						(turn==w)?"White":"Black",
						'A'+X1,1+Y1,'A'+X2,1+Y2,
						(board[Y2][X2].p==P)?"Pawn":
						(board[Y2][X2].p==B)?"Bishop":
						(board[Y2][X2].p==Kn)?"Knight":
						(board[Y2][X2].p==K)?"King":
						(board[Y2][X2].p==R)?"Rook":
						(board[Y2][X2].p==Q)?"Queen":"");
			}
				
			
			/* Updating variables */
			if(turn==w) turns++;
			turn=!turn;
			turn_time.tm_sec=turn_time.tm_min=turn_time.tm_hour=0;
			X1=Y1=X2=Y2=-1;
			*player_computer=IDLE;			
			
			/* Printing taken pieces */
			XSetForeground(display,gc,blue.pixel);
			XDrawString(display,win,gc,650,300,"Pieces taken :",14);
			for(i=0;i<6 && i<num[w];i++) printpiece(display,win,gc,w,killed[w][i],650+i*L,320);
			for(i=6;i<12 && i<num[w];i++) printpiece(display,win,gc,w,killed[w][i],630+(i-6)*L,350);
			for(i=12;i<15 && i<num[w];i++) printpiece(display,win,gc,w,killed[w][i],700+(i-12)*L,380);
			for(i=0;i<6 && i<num[b];i++) printpiece(display,win,gc,b,killed[b][i],650+i*L,480);
			for(i=6;i<12 && i<num[b];i++) printpiece(display,win,gc,b,killed[b][i],630+(i-6)*L,510);
			for(i=12;i<15 && i<num[b];i++) printpiece(display,win,gc,b,killed[b][i],720+(i-12)*L,540);
		}
		
		XSync(display,0);
	}
	XCloseDisplay(display);
}

int mainmenu(Display *display,Window win,GC gc)
{
	int screen_num = DefaultScreen(display);
	Window parent_window=RootWindow(display,screen_num);
	
	/* Creating colors */
	Colormap cm=DefaultColormap(display,screen_num);
	XColor fore,back,button,shade,text,inactivebutton,inactivetext,red;
	XAllocNamedColor(display,cm,"rgb:80/80/e0",&back,&back);
	XAllocNamedColor(display,cm,"rgb:50/50/70",&fore,&fore);
	XAllocNamedColor(display,cm,"violet",&button,&button);
	XAllocNamedColor(display,cm,"gray20",&shade,&shade);
	XAllocNamedColor(display,cm,"black",&text,&text);
	XAllocNamedColor(display,cm,"gray80",&inactivebutton,&inactivebutton);
	XAllocNamedColor(display,cm,"gray50",&inactivetext,&inactivetext);
	XAllocNamedColor(display,cm,"rgb:ff/30/30",&red,&red);
	
	/* Font */
	//XFontStruct *font=XLoadQueryFont(display,"");
	//XSetFont(display,gc,font->fid);
	
	/* Creating and mapping window */
	Window subwin=XCreateSimpleWindow(display,win,200,100,600,400,2,back.pixel,fore.pixel);
	XMapWindow(display,subwin);
	
	char rn[14]={rand()%8,rand()%8,rand()%8,rand()%8,rand()%8,rand()%8,rand()%8,rand()%8,rand()%8,rand()%8,rand()%8,rand()%8,rand()%8,rand()%8};
	char temp_string[70];
	
	XSelectInput(display,subwin,ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);
	XEvent event;
	event.type=Expose;
	Button *newgame,*continuegame,*loadgame,*savegame,*resigngame,*quit,*timeup,*timedown,*recordfilechange,*about;
	OptionButton *pblack,*pwhite,*timedgame,*recordgame;
	while(1)
	{
		/* Printing text, buttons etc. */
		XClearWindow(display,subwin);
		XSetForeground(display,gc,red.pixel);
		XDrawString(display,subwin,gc,300,20,"CHESS",5);
		print_piece_on_board(display,subwin,gc,rn[0],rn[1],350,10);
		print_piece_on_board(display,subwin,gc,rn[2],rn[3],400,0);
		print_piece_on_board(display,subwin,gc,rn[4],rn[5],430,20);
		print_piece_on_board(display,subwin,gc,rn[6],rn[7],380,30);
		print_piece_on_board(display,subwin,gc,rn[8],rn[9],410,50);
		print_piece_on_board(display,subwin,gc,rn[10],rn[11],470,40);
		print_piece_on_board(display,subwin,gc,rn[12],rn[13],300,30);
		newgame=create_button(display,subwin,gc,10,50,100,30,
				playing?inactivebutton:button,shade,playing?inactivetext:text,button,"New Game");
		continuegame=create_button(display,subwin,gc,10,100,100,30,
				playing?button:inactivebutton,shade,playing?text:inactivetext,button,"Continue Game");
		loadgame=create_button(display,subwin,gc,10,150,100,30,
				playing?inactivebutton:button,shade,playing?inactivetext:text,button,"Load Game");
		savegame=create_button(display,subwin,gc,10,200,100,30,
				playing?button:inactivebutton,shade,playing?text:inactivetext,button,"Save Game");
		resigngame=create_button(display,subwin,gc,10,250,100,30,
				playing?button:inactivebutton,shade,playing?text:inactivetext,button,"Resign Game");
		quit=create_button(display,subwin,gc,10,300,100,30,
				playing?inactivebutton:button,shade,playing?inactivetext:text,button,"Quit");
		pwhite=create_optionbutton(display,subwin,gc,200,200,player[w],"White Player Computer");
		pblack=create_optionbutton(display,subwin,gc,200,220,player[b],"Black Player Computer");
		timedgame=create_optionbutton(display,subwin,gc,200,250,TimedGame,"Play Timed Game");
		XSetForeground(display,gc,red.pixel);
		sprintf(temp_string,"%2.2d hour %2.2d minute 00 seconds",hmax,mmax);
		XDrawString(display,subwin,gc,230,280,temp_string,strlen(temp_string));
		timeup=create_button(display,subwin,gc,410,260,25,15,(!playing && TimedGame && (hmax<99||mmax<50))?button:inactivebutton,shade,
				(!playing && TimedGame && (hmax<99||mmax<50))?text:inactivetext,button,"/\\");
		timedown=create_button(display,subwin,gc,410,280,25,15,(!playing && TimedGame && (hmax>0||mmax>10))?button:inactivebutton,shade,
				(!playing && TimedGame && (hmax>0||mmax>10))?text:inactivetext,button,"\\/");			
		recordgame=create_optionbutton(display,subwin,gc,200,300,RecordGame,"Record Game");
		recordfilechange=create_button(display,subwin,gc,300,300,80,15,(RecordGame && strlen(recordfilename)>0)?button:inactivebutton,shade,
				(RecordGame && strlen(recordfilename)>0)?text:inactivetext,button,"Change File");
		XSetForeground(display,gc,red.pixel);
		XDrawString(display,subwin,gc,230,330,recordfilename,strlen(recordfilename));
		about=create_button(display,subwin,gc,500,370,80,20,button,shade,text,button,"About Chess");
		XSync(display,false);
		
		/* Event handling */
		XWindowEvent(display,subwin,ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask,&event);
		
			// "White Player Computer"
		if(optionbutton_pressed(event,pwhite))
		{
			if(*player_computer!=IDLE && turn==w)
			{
				kill(child_pid,1);
				*player_computer=IDLE;
			}
			player[w]=!player[w];
		}
		
			// "Black Player Computer"
		if(optionbutton_pressed(event,pblack))
		{
			if(*player_computer!=IDLE && turn==b)
			{
				kill(child_pid,1);
				*player_computer=IDLE;
			}
			player[b]=!player[b];
		}
		
			// "Play Timed Game"
		if(!playing && optionbutton_pressed(event,timedgame)) TimedGame=!TimedGame;
		if(!playing && TimedGame && (hmax<99||mmax<50) && button_pressed(event,timeup))
		{
			mmax+=10;
			if(mmax==60) mmax=0,hmax++;
		}
		if(!playing && TimedGame && (hmax>0||mmax>10) && button_pressed(event,timedown))
		{
			mmax-=10;
			if(mmax==-10) mmax=50,hmax--;
		}
		
			// "Record Game"
		if(optionbutton_pressed(event,recordgame))
		{
			RecordGame=!RecordGame;
			if(RecordGame==true && strlen(recordfilename)==0) load_record_file(display,subwin,gc);
		}
		if(RecordGame && button_pressed(event,recordfilechange))
			load_record_file(display,subwin,gc);
		
			// "NewGame"
		if(!playing && (button_pressed(event,newgame) || (key_pressed(display,event,"Return"))))
		{
			playing=true;
			newboard();
			remaining_time.tm_hour=hmax,remaining_time.tm_min=mmax,remaining_time.tm_sec=0;
                        total_time.tm_hour=0,total_time.tm_min=0,total_time.tm_sec=0;
                        white_time.tm_hour=0,white_time.tm_min=0,white_time.tm_sec=0;
                        black_time.tm_hour=0,black_time.tm_min=0,black_time.tm_sec=0;
                        turn_time.tm_hour=0,turn_time.tm_min=0,turn_time.tm_sec=0;
                        turn=w;
			break;
		}
		
			// "Continue Game"
		if(playing && (button_pressed(event,continuegame) || key_pressed(display,event,"Return") || key_pressed(display,event,"Escape")))
			break;
		
			// "Save Game"
		if(playing && button_pressed(event,savegame))
			if(save(display,subwin,gc)) break;
			else event.type=Expose;
			
			// "Load Game"
		if(!playing && button_pressed(event,loadgame))
			if(load(display,subwin,gc))
			{
				playing=true;
				break;
			}
			else event.type=Expose;
			
			// "Resign Game"
		if(playing && button_pressed(event,resigngame))
		{
			playing=false;
			if(*player_computer!=IDLE)
			{
				kill(child_pid,1);
				*player_computer=IDLE;
			}
		}
		
			// "Quit"
		if(!playing && (button_pressed(event,quit) || (key_pressed(display,event,"Escape"))))
			exit(0);
		
			// "About"
		if(button_pressed(event,about))
			about_chess(display,subwin,gc);
	}
	free_buttons(10,newgame,continuegame,loadgame,savegame,resigngame,quit,timeup,timedown,recordfilechange,about);
	free_optionbuttons(4,pblack,pwhite,timedgame,recordgame);
	XDestroyWindow(display,subwin);
}

void loadimages(Display *display)
{
	load_BMP_24b_54B(&image[w][K],"image/kingw.bmp");
	load_BMP_24b_54B(&image[w][Q],"image/queenw.bmp");
	load_BMP_24b_54B(&image[w][B],"image/bishopw.bmp");
	load_BMP_24b_54B(&image[w][Kn],"image/knightw.bmp");
	load_BMP_24b_54B(&image[w][R],"image/rookw.bmp");
	load_BMP_24b_54B(&image[w][P],"image/pawnw.bmp");
	load_BMP_24b_54B(&image[b][K],"image/kingb.bmp");
	load_BMP_24b_54B(&image[b][Q],"image/queenb.bmp");
	load_BMP_24b_54B(&image[b][B],"image/bishopb.bmp");
	load_BMP_24b_54B(&image[b][Kn],"image/knightb.bmp");
	load_BMP_24b_54B(&image[b][R],"image/rookb.bmp");
	load_BMP_24b_54B(&image[b][P],"image/pawnb.bmp");
	load_BMP_24b_54B(&image_mask[K],"image/kingmask.bmp");
	load_BMP_24b_54B(&image_mask[Q],"image/queenmask.bmp");
	load_BMP_24b_54B(&image_mask[B],"image/bishopmask.bmp");
	load_BMP_24b_54B(&image_mask[Kn],"image/knightmask.bmp");
	load_BMP_24b_54B(&image_mask[R],"image/rookmask.bmp");
	load_BMP_24b_54B(&image_mask[P],"image/pawnmask.bmp");
	int i,j,c,p;
	char s[14];
	Colormap cm=DefaultColormap(display,DefaultScreen(display));
	XColor color;
	for(c=0;c<2;c++)
		for(p=1;p<=6;p++)
			for(i=0;i<image[c][p].height;i++)
				for(j=0;j<image[c][p].width;j++)
					if(image_mask[p].data[i][j].red==0 && image_mask[p].data[i][j].green==0 && image_mask[p].data[i][j].blue==0)
					{
						sprintf(s,"rgb:%2.2x/%2.2x/%2.2x",image[c][p].data[i][j].red,image[c][p].data[i][j].green,image[c][p].data[i][j].blue);
						XAllocNamedColor(display,cm,s,&color,&color);
						pixmap[c][p][i][j].value=color.pixel;
					}
					else pixmap[c][p][i][j].transparent=1;
	for(p=1;p<=6;p++)
	{
		unload_BMP_24b_54B(&image_mask[p]);
		unload_BMP_24b_54B(&image[w][p]);
		unload_BMP_24b_54B(&image[b][p]);
	}
}

void print_piece_on_board(Display* display,Window win,GC gc,int i,int j,int x,int y)
{
	if(board[j][i].c==n) return;
	printpiece(display,win,gc,board[j][i].c,board[j][i].p,x,y);
}

void printpiece(Display* display,Window win,GC gc,int c,int p,int x,int y)
{
	int k,l;
	unsigned long temp=0;
	XSetForeground(display,gc,0);
	for(k=0;k<image[c][p].height;k++)
		for(l=0;l<image[c][p].width;l++)
			if(pixmap[c][p][l][k].transparent==0)
			{
				if(temp!=pixmap[c][p][l][k].value)
				{
					XSetForeground(display,gc,pixmap[c][p][l][k].value);
					temp=pixmap[c][p][l][k].value;
				}
				XDrawPoint(display,win,gc,x+k,y+L-l);
			}
}


int move_piece_for_human(Display* display,Window win,GC gc)
{
	char *col[2]={"White","Black"};
	int i,j;
	if(board[Y1][X1].c==turn && (valid(X1,Y1,X2,Y2)))
	{
		movepiece(X1,Y1,X2,Y2,0);
		if(board[Y2][X2].p==P && ((Y2==7 && board[Y2][X2].c==w) || (Y2==0 && board[Y2][X2].c==b)))
		{
			int screen_num = DefaultScreen(display);
			Window parent_window=RootWindow(display,screen_num);	
			
			Colormap cm=DefaultColormap(display,screen_num);
			XColor fore,back,button,shade,text;
			XAllocNamedColor(display,cm,"dark green",&fore,&fore);
			XAllocNamedColor(display,cm,"light green",&back,&back);
			XAllocNamedColor(display,cm,"green",&button,&button);
			XAllocNamedColor(display,cm,"gray20",&shade,&shade);
			XAllocNamedColor(display,cm,"black",&text,&text);
					
			Window subwin=XCreateSimpleWindow(display,win,700,500,400,200,2,back.pixel,fore.pixel);
			XMapWindow(display,subwin);
						
			Button *queen=create_button(display,subwin,gc,50,70,100,30,button,shade,text,button,"Queen");
			Button *rook=create_button(display,subwin,gc,170,70,100,30,button,shade,text,button,"Rook");
			Button *bishop=create_button(display,subwin,gc,50,120,100,30,button,shade,text,button,"Bishop");
			Button *knight=create_button(display,subwin,gc,170,120,100,30,button,shade,text,button,"Knight");
			
//			XFontStruct *font=XLoadQueryFont(display,"*-helvetica-*-18-*");
//			XSetFont(display,gc,font->fid);
			XEvent event;
			XSelectInput(display,subwin,KeyPressMask | ButtonPressMask | ButtonReleaseMask);
			while(1)
			{
				XWindowEvent(display,subwin,ButtonPressMask,&event);
				if(button_pressed(event,queen))
				{
					board[Y2][X2].p=Q;
					break;
				}
				if(button_pressed(event,rook))
				{
					board[Y2][X2].p=R;
					break;
				}
				if(button_pressed(event,bishop))
				{
					board[Y2][X2].p=B;
					break;
				}
				if(button_pressed(event,knight))
				{
					board[Y2][X2].p=Kn;
					break;
				}
			}
			XDestroyWindow(display,subwin);
		}
	}
}

int endgame(Display* display,Window win,GC gc,char *msg)
{
	int screen_num = DefaultScreen(display);
	Colormap cm=DefaultColormap(display,screen_num);
	XColor fore,back,button,shade,text;
	XAllocNamedColor(display,cm,"dark green",&fore,&fore);
	XAllocNamedColor(display,cm,"light green",&back,&back);
	XAllocNamedColor(display,cm,"green",&button,&button);
	XAllocNamedColor(display,cm,"gray20",&shade,&shade);
	XAllocNamedColor(display,cm,"black",&text,&text);

	Window subwin=XCreateSimpleWindow(display,win,200,100,400,150,2,back.pixel,fore.pixel);
	XMapWindow(display,subwin);
	
	XSetForeground(display,gc,back.pixel);
	XDrawString(display,subwin,gc,30,30,msg,strlen(msg));
	Button *resign=create_button(display,subwin,gc,50,70,100,30,button,shade,text,button,"Resign");
	XEvent event;
	XSelectInput(display,subwin,KeyPressMask | ButtonPressMask | ButtonReleaseMask);
	while(1)
	{	
		XWindowEvent(display,subwin,ButtonPressMask | ButtonReleaseMask,&event);
		if(button_pressed(event,resign))
		{
			playing=false;
			break;
		}
	}
	return Expose;
}

int save(Display *display,Window win,GC gc)
{
	char fpath[256]="save/";
	if(get_filename(fpath,display,win,gc,"File name :","OK")==0) return 0;
	FILE *fp=fopen(fpath,"w");
	if(fp==NULL) return 0;
	fwrite(board,sizeof(struct pieces),64,fp);
	fwrite(killed,sizeof(int),32,fp);
	fwrite(&turn,sizeof(int),1,fp);
	fwrite(&turns,sizeof(int),1,fp);
	fwrite(player,sizeof(char),2,fp);
	fwrite(&hmax,sizeof(int),1,fp);
	fwrite(&mmax,sizeof(int),1,fp);
	fwrite(&total_time,sizeof(struct tm),1,fp);
	fwrite(&turn_time,sizeof(struct tm),1,fp);
	fwrite(&white_time,sizeof(struct tm),1,fp);
	fwrite(&black_time,sizeof(struct tm),1,fp);
	fwrite(&TimedGame,sizeof(int),1,fp);
	fwrite(&remaining_time,sizeof(struct tm),1,fp);
	fclose(fp);
	return 1;
}

int load(Display *display,Window win,GC gc)
{
	char fpath[256]="save/";
	if(get_filename(fpath,display,win,gc,"File name :","OK")==0) return 0;
	FILE *fp=fopen(fpath,"r");
	if(fp==NULL) return 0;
	fread(board,sizeof(struct pieces),64,fp);
	fread(killed,sizeof(int),32,fp);
	fread(&turn,sizeof(int),1,fp);
	fread(&turns,sizeof(int),1,fp);
	fread(player,sizeof(char),2,fp);
	fread(&hmax,sizeof(int),1,fp);
	fread(&mmax,sizeof(int),1,fp);
	fread(&total_time,sizeof(struct tm),1,fp);
	fread(&turn_time,sizeof(struct tm),1,fp);
	fread(&white_time,sizeof(struct tm),1,fp);
	fread(&black_time,sizeof(struct tm),1,fp);
	fread(&TimedGame,sizeof(int),1,fp);
	fread(&remaining_time,sizeof(struct tm),1,fp);
	fclose(fp);
	return 1;
}

void load_record_file(Display *display,Window win,GC gc)
{
	char fpath[256]="record/";
	if(get_filename(fpath,display,win,gc,"File name :","OK")==0)
	{
		RecordGame=false;
		return;
	}
	FILE *fp=fopen(fpath,"a");
	if(fp!=NULL)
	{
		strcpy(recordfilename,fpath);
		rec=fp;
		fprintf(rec,"*****Output*****\n");
	}
	else RecordGame=false;
}

void about_chess(Display *display,Window win,GC gc)
{
	int screen_num = DefaultScreen(display);
	Window parent_window=RootWindow(display,screen_num);
	Colormap cm=DefaultColormap(display,screen_num);
	XColor fore,back,button,shade,text,red;
	XAllocNamedColor(display,cm,"dark green",&fore,&fore);
	XAllocNamedColor(display,cm,"light green",&back,&back);
	XAllocNamedColor(display,cm,"green",&button,&button);
	XAllocNamedColor(display,cm,"gray20",&shade,&shade);
	XAllocNamedColor(display,cm,"black",&text,&text);
	XAllocNamedColor(display,cm,"red",&red,&red);

	Window subwin=XCreateSimpleWindow(display,win,100,100,400,200,2,back.pixel,fore.pixel);
	XMapWindow(display,subwin);
	
//	XFontStruct *font=XLoadQueryFont(display,"*-helvetica-*-18-*");
//	XSetFont(display,gc,font->fid);
	XSetForeground(display,gc,red.pixel);
	XDrawString(display,subwin,gc,80,40,"Chess version 2.3",17);
	XDrawString(display,subwin,gc,50,60,"- by Rakesh Malik & Dibyayan Chakroborty",40);
	XDrawString(display,subwin,gc,80,80,"2010-'11",8);
	Button *back_button=create_button(display,subwin,gc,50,100,100,30,button,shade,text,button,"Back");
	XEvent event;
	XSelectInput(display,subwin,KeyPressMask | ButtonPressMask | ButtonReleaseMask);
	while(1)
	{
		XNextEvent(display,&event);
		if(button_pressed(event,back_button) || key_pressed(display,event,"Escape") ||  key_pressed(display,event,"Return"))
		{
			XDestroyWindow(display,subwin);
			return;
		}
	}
}
