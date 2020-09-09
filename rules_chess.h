/* CHESS 
 * Author 	: Rakesh Malik
 * Date started : 29.09.2010
 * Subject 	: Functions for rules of chess
 * Comment	: Used only by chess.c
 */

typedef struct pieces
{
	unsigned char c : 2;				/* color of piece */
	unsigned char p : 3;				/* piece */
	unsigned char n : 4;				/* point of piece */
}pieces;

enum piece{K=1,Q,B,Kn,R,P};				/* Piece definitions
							 * 0 = no piece
							 * K = King
							 * Q = Queen
							 * B = Bishop
							 * Kn= Knight
							 * R = Rook
							 * P = Pawn */
enum color{w,b,n};					/* color definitons 
							 * w = white
							 * b = black
							 * n = no color */

pieces board[8][8]={
{{w,R,5},	{w,Kn,3},	{w,B,3},	{w,Q,9},	{w,K,10},	{w,B,3},	{w,Kn,3},	{w,R,5}},
{{w,P,1},	{w,P,1},        {w,P,1},        {w,P,1},        {w,P,1},        {w,P,1},        {w,P,1},        {w,P,1}},
{{n,0,0},	{n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0}},
{{n,0,0},       {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0}},
{{n,0,0},       {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0}},
{{n,0,0},       {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0},        {n,0,0}},
{{b,P,1},       {b,P,1},        {b,P,1},        {b,P,1},        {b,P,1},        {b,P,1},        {b,P,1},        {b,P,1}},
{{b,R,5},       {b,Kn,3},       {b,B,3},        {b,Q,9},        {b,K,10},        {b,B,3},	{b,Kn,3},	{b,R,5}}};
							/* Initial board condition */

char killed[2][15]={{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
							/* Taken pieces of white and black */
char num[2]={0,0};					/* Number of taken pieces of white and black*/

int point[2]={0,0};					/* Total point of white and black */

int valid(char x1,char y1,char x2,char y2);		/* Returns 1 if the given move is valid */
/* following functions are parts of valid() */
int king_valid(char x1,char y1,char x2,char y2);
int queen_valid(char x1,char y1,char x2,char y2);
int bishop_valid(char x1,char y1,char x2,char y2);
int knight_valid(char x1,char y1,char x2,char y2);
int rook_valid(char x1,char y1,char x2,char y2);
int pawn_valid(char x1,char y1,char x2,char y2);

int under_attack(char x,char y,char color);		/* Returns 1 if the given position is in attack for given color */

void newboard();					/* Initializes new game in the board */
void copy_board(struct pieces dest[8][8],struct pieces src[8][8]);
							/* copies whole 'src' board in 'dest' bosrd */

void movepiece(char x1,char y1,char x2,char y2,int fakemove);
							/* Moves piece at X1Y1 to X2Y2 if possible 
							 * 'fakemove' = true when auxilary board is used else it is false*/
void copy_piece(pieces *dest,pieces *src);		/* Copies a piece structure to another */
void delete_piece(pieces *p);				/* Nullifies a piece structure */

int king_in_check(char color);				/* Returns 1 king of given color is in ckeck else returns 0*/
int mate(char color);					/* Returns 1 if a given color is stalemate else returns 0*/
int turn_is_check(char x1,char y1,char x2,char y2);	/* Returns 1 if a turn leads to check else returns 0*/

int castle(char x1,char y1,char x2,char y2);		/* Return 1 if given move is a castelling else returns 0*/
/* following functions are parts of castle() */
int king_side_castle_white(char x1,char y1,char x2,char y2);
int queen_side_castle_white(char x1,char y1,char x2,char y2);
int king_side_castle_black(char x1,char y1,char x2,char y2);
int queen_side_castle_black(char x1,char y1,char x2,char y2);


int b_r1=0,b_r2=0,b_k=0,w_r1=0,w_r2=0,w_k=0;
int last_x1=-1,last_y1=-1,last_x2=-1,last_y2=-1;

void copy_board(struct pieces dest[8][8],struct pieces src[8][8])
{
	int i,j,str=0;
	for(i=0;i<8;i++)
		for(j=0;j<8;j++)
		{
			dest[i][j].c=src[i][j].c;	
			dest[i][j].p=src[i][j].p;	
			dest[i][j].n=src[i][j].n;	
		}
}

void newboard()
{
	int i,j;
	for(i=0;i<2;i++)
		for(j=0;j<16;j++)
			killed[i][j]=0;

	for(j=0;j<8;j++)
		for(i=2;i<=5;i++)
			board[i][j].c=n, board[i][j].p=0, board[i][j].n=0;

	board[0][0].c=w, board[0][0].p=R, board[0][0].n=5;
	board[0][1].c=w, board[0][1].p=Kn, board[0][1].n=3;
	board[0][2].c=w, board[0][2].p=B, board[0][2].n=3;
	board[0][3].c=w, board[0][3].p=Q, board[0][3].n=9;
	board[0][4].c=w, board[0][4].p=K, board[0][4].n=10;
	board[0][5].c=w, board[0][5].p=B, board[0][5].n=3;
	board[0][6].c=w, board[0][6].p=Kn, board[0][6].n=3;
	board[0][7].c=w, board[0][7].p=R, board[0][7].n=5;
	
	board[1][0].c=w, board[1][0].p=P, board[1][0].n=1;
	board[1][1].c=w, board[1][1].p=P, board[1][1].n=1;
	board[1][2].c=w, board[1][2].p=P, board[1][2].n=1;
	board[1][3].c=w, board[1][3].p=P, board[1][3].n=1;
	board[1][4].c=w, board[1][4].p=P, board[1][4].n=1;
	board[1][5].c=w, board[1][5].p=P, board[1][5].n=1;
	board[1][6].c=w, board[1][6].p=P, board[1][6].n=1;
	board[1][7].c=w, board[1][7].p=P, board[1][7].n=1;
	
	board[6][0].c=b, board[6][0].p=P, board[6][0].n=1;
	board[6][1].c=b, board[6][1].p=P, board[6][1].n=1;
	board[6][2].c=b, board[6][2].p=P, board[6][2].n=1;
	board[6][3].c=b, board[6][3].p=P, board[6][3].n=1;
	board[6][4].c=b, board[6][4].p=P, board[6][4].n=1;
	board[6][5].c=b, board[6][5].p=P, board[6][5].n=1;
	board[6][6].c=b, board[6][6].p=P, board[6][6].n=1;
	board[6][7].c=b, board[6][7].p=P, board[6][7].n=1;
	
	board[7][0].c=b, board[7][0].p=R, board[7][0].n=5;
	board[7][1].c=b, board[7][1].p=Kn, board[7][1].n=3;
	board[7][2].c=b, board[7][2].p=B, board[7][2].n=3;
	board[7][3].c=b, board[7][3].p=Q, board[7][3].n=9;
	board[7][4].c=b, board[7][4].p=K, board[7][4].n=10;
	board[7][5].c=b, board[7][5].p=B, board[7][5].n=3;
	board[7][6].c=b, board[7][6].p=Kn, board[7][6].n=3;
	board[7][7].c=b, board[7][7].p=R, board[7][7].n=5;
	last_x1=last_y1=last_x2=last_y2=-1;
	w_k=w_r1=w_r2=b_k=b_r1=b_r2=0;
}

int valid(char x1,char y1,char x2,char y2)
{
	if(x1<0 || y1<0 || x2<0 || y2<0 || x1>=8 || y1>=8 || x2>=8 || y2>=8) return 0;
	if(board[y1][x1].c==n) return 0;
	/* if in next move king in is check */
	pieces temp={0,0,0};
	char flag=1,c=board[y1][x1].c;
	copy_piece(&temp,&board[y2][x2]);
	copy_piece(&board[y2][x2],&board[y1][x1]);
	delete_piece(&board[y1][x1]);
	if(king_in_check(c)) flag=0;
	copy_piece(&board[y1][x1],&board[y2][x2]);
	copy_piece(&board[y2][x2],&temp);
	if(flag==0) return 0;
	/* else */
	switch(board[y1][x1].p)
	{
		case K:
			if(king_valid(x1,y1,x2,y2)) return 1;
			break;
		case Q:
			if(queen_valid(x1,y1,x2,y2)) return 1;
			break;
		case B:
			if(bishop_valid(x1,y1,x2,y2)) return 1;
			break;
		case Kn:
			if(knight_valid(x1,y1,x2,y2)) return 1;
			break;
		case R:
			if(rook_valid(x1,y1,x2,y2)) return 1;
			break;
		case P:
			if(pawn_valid(x1,y1,x2,y2)) return 1;
			break;
	}
	return 0;
}

int king_in_check(char color)
{
	char x,y,flag=0;
	for(y=0;y<8;y++)
	{
		for(x=0;x<8;x++)
			if(board[y][x].p==K && board[y][x].c==color)
			{
				flag=1;
				break;
			}
		if(flag==1) break;
	}
	if(under_attack(x,y,color)) return 1;
	else return 0;
}

int king_valid(char x1,char y1,char x2,char y2)
{
	if(board[y1][x1].c==board[y2][x2].c) return 0;
	if(	(queen_side_castle_white(x1,y1,x2,y2) && w_k==0 && w_r1==0) || 
		(king_side_castle_white(x1,y1,x2,y2) && w_k==0 && w_r2==0) || 
		(queen_side_castle_black(x1,y1,x2,y2) && b_k==0 && b_r1==0) ||
		(king_side_castle_black(x1,y1,x2,y2) && b_k==0 && b_r2==0) ||
		((	(y2==y1+1 && x2==x1)	||
			(y2==y1-1 && x2==x1)	||
			(y2==y1 && x2==x1+1)	||
			(y2==y1 && x2==x1-1)	||
			(y2==y1+1 && x2==x1+1)	||
			(y2==y1-1 && x2==x1+1)	||
			(y2==y1+1 && x2==x1-1)	||
			(y2==y1-1 && x2==x1-1)	)
		&& !under_attack(x2,y2,board[y1][x1].c)) )
		return 1;
	else return 0;
}

int queen_valid(char x1,char y1,char x2,char y2)
{
	char j,k;
	if(board[y1][x1].c==board[y2][x2].c) return 0;
	if(	(y2==y1 && x2!=x1)	||
		(y2!=y1 && x2==x1)	||
		(y2-y1==x2-x1)		||
		(y2-y1==x1-x2)		)
	{
		if(y2==y1 && x2>x1) for(j=x1+1;j<x2;j++) if(board[y1][j].c!=n) return 0;
		if(y2==y1 && x2<x1) for(j=x1-1;j>x2;j--) if(board[y1][j].c!=n) return 0;
		if(y2>y1 && x2==x1) for(j=y1+1;j<y2;j++) if(board[j][x1].c!=n) return 0;
		if(y2<y1 && x2==x1) for(j=y1-1;j>y2;j--) if(board[j][x1].c!=n) return 0;
		if(y2>y1 && x2>x1) for(k=y1+1,j=x1+1;k<y2,j<x2;k++,j++) if(board[k][j].c!=n) return 0;
		if(y2<y1 && x2>x1) for(k=y1-1,j=x1+1;k>y2,j<x2;k--,j++) if(board[k][j].c!=n) return 0;
		if(y2>y1 && x2<x1) for(k=y1+1,j=x1-1;k<y2,j>x2;k++,j--) if(board[k][j].c!=n) return 0;
		if(y2<y1 && x2<x1) for(k=y1-1,j=x1-1;k>y2,j>x2;k--,j--) if(board[k][j].c!=n) return 0;
		return 1;
	}
	return 0;
}

int bishop_valid(char x1,char y1,char x2,char y2)
{
	char j,k;
	if(board[y1][x1].c==board[y2][x2].c) return 0;
	if(	(y2-y1==x2-x1)		||
		(y2-y1==x1-x2)		)
	{
		if(y2>y1 && x2>x1) for(k=y1+1,j=x1+1;k<y2,j<x2;k++,j++) if(board[k][j].c!=n) return 0;
		if(y2<y1 && x2>x1) for(k=y1-1,j=x1+1;k>y2,j<x2;k--,j++) if(board[k][j].c!=n) return 0;
		if(y2>y1 && x2<x1) for(k=y1+1,j=x1-1;k<y2,j>x2;k++,j--) if(board[k][j].c!=n) return 0;
		if(y2<y1 && x2<x1) for(k=y1-1,j=x1-1;k>y2,j>x2;k--,j--) if(board[k][j].c!=n) return 0;
		return 1;
	}
	return 0;
}

int knight_valid(char x1,char y1,char x2,char y2)
{
	if(board[y1][x1].c==board[y2][x2].c) return 0;
	if(	(x2==x1+1 && y2==y1+2)	||
		(x2==x1+1 && y2==y1-2)	||
		(x2==x1-1 && y2==y1+2)	||
		(x2==x1-1 && y2==y1-2)	||
		(x2==x1+2 && y2==y1+1)	||
		(x2==x1+2 && y2==y1-1)	||
		(x2==x1-2 && y2==y1+1)	||
		(x2==x1-2 && y2==y1-1)	)
		return 1;
	return 0;
}

int rook_valid(char x1,char y1,char x2,char y2)
{
	char j;
	if(board[y1][x1].c==board[y2][x2].c) return 0;
	if(	(y2==y1 && x2!=x1)	||
		(y2!=y1 && x2==x1)	)
	{
		if(y2==y1 && x2>x1) for(j=x1+1;j<x2;j++) if(board[y1][j].c!=n) return 0;
		if(y2==y1 && x2<x1) for(j=x1-1;j>x2;j--) if(board[y1][j].c!=n) return 0;
		if(y2>y1 && x2==x1) for(j=y1+1;j<y2;j++) if(board[j][x1].c!=n) return 0;
		if(y2<y1 && x2==x1) for(j=y1-1;j>y2;j--) if(board[j][x1].c!=n) return 0;
		return 1;
	}
	return 0;
}

int pawn_valid(char x1,char y1,char x2,char y2)
{
	if(board[y1][x1].c==board[y2][x2].c) return 0;
	if(board[y1][x1].c==w)
	{
		if(y2==y1+1 && x2==x1 && board[y2][x2].c==n) return 1;
		if(y1==1 && y2==3 && x2==x1 && board[2][x2].c==n && board[3][x2].c==n) return 1;
		if(y2==y1+1 && (x2==x1-1 || x2==x1+1) && board[y2][x2].c==b) return 1;
	}
	else
	{
		if(y2==y1-1 && x2==x1 && board[y2][x2].c==n) return 1;
		if(y1==6 && y2==4 && x2==x1 && board[5][x2].c==n && board[4][x2].c==n) return 1;
		if(y2==y1-1 && (x2==x1-1 || x2==x1+1) && board[y2][x2].c==w) return 1;
	}
	/* Enpassent */
	if(board[y1][x1].c==w && y1==4 &&
			((last_x1==x1-1 && last_y1==6 && last_x2==x1-1 && last_y2==4 && y2==5 && x2==x1-1 && x1-1>=0) ||
			(last_x1==x1+1 && last_y1==6 && last_x2==x1+1 && last_y2==4 && y2==5 && x2==x1+1 && x1+1<8)))
		return 1;
	if(board[y1][x1].c==b && y1==3 &&
			((last_x1==x1-1 && last_y1==1 && last_x2==x1-1 && last_y2==3 && y2==2 && x2==x1-1 && x1-1>=0) ||
			(last_x1==x1+1 && last_y1==1 && last_x2==x1+1 && last_y2==3 && y2==2 && x2==x1+1 && x1+1<8)))
		return 1;
	return 0;
}

int under_attack(char x,char y,char c)
{
	char i,j,k;
	/* Attacked by king */
	char kingx[8]={+1,+1,+0,+0,-1,-1,-1,+1};
	char kingy[8]={+1,+0,+1,-1,+0,-1,+1,-1};
	for(i=0;i<8;i++)
		if(y+kingy[i]>=0 && y+kingy[i]<8 && x+kingx[i]>=0 && x+kingx[i]<8 &&
				board[y+kingy[i]][x+kingx[i]].c==!c && board[y+kingy[i]][x+kingx[i]].p==K)
			return 1;
	/* Attacked by knight */
	char knightx[8]={+1,+1,+2,+2,-1,-1,-2,-2};
	char knighty[8]={+2,-2,+1,-1,+2,-2,+1,-1};
	for(i=0;i<8;i++)
		if(y+knighty[i]>=0 && y+knighty[i]<8 && x+knightx[i]>=0 && x+knightx[i]<8 &&
				board[y+knighty[i]][x+knightx[i]].c==!c && board[y+knighty[i]][x+knightx[i]].p==Kn)
			return 1;
	/* Attacked by pawn */
	if(c==w)
	{
		if(board[y+1][x+1].p==P && board[y+1][x+1].c==b) return 1;
		if(board[y+1][x-1].p==P && board[y+1][x-1].c==b) return 1;
	}
	else
	{
		if(board[y-1][x+1].p==P && board[y-1][x+1].c==w) return 1;
		if(board[y-1][x-1].p==P && board[y-1][x-1].c==w) return 1;
	}
	/* Attacked vertically by rook or queen */
	for(j=x+1;j<8;j++) 
		if(board[y][j].c!=n)
		{
			if(board[y][j].c==!c && (board[y][j].p==R || board[y][j].p==Q)) return 1;
			break;
		}
	for(j=x-1;j>=0;j--)
		if(board[y][j].c!=n)
		{
			if(board[y][j].c==!c && (board[y][j].p==R || board[y][j].p==Q)) return 1;
			break;
		}
	/* Attacked horizontally by rook or queen */
	for(j=y+1;j<8;j++)
		if(board[j][x].c!=n)
		{
			if(board[j][x].c==!c && (board[j][x].p==R || board[j][x].p==Q)) return 1;
			break;
		}
	for(j=y-1;j>=0;j--)
		if(board[j][x].c!=n)
		{
	       		if(board[j][x].c==!c && (board[j][x].p==R || board[j][x].p==Q)) return 1;
			break;
		}
	/* Attacked diagonally by bishop or queen */
	for(k=y+1,j=x+1;k<8 && j<8;k++,j++)
		if(board[k][j].c!=n)
		{
	       		if(board[k][j].c==!c && (board[k][j].p==B || board[k][j].p==Q)) return 1;
			break;
		}
	for(k=y-1,j=x+1;k>=0 && j<8;k--,j++)
		if(board[k][j].c!=n)
		{
	       		if(board[k][j].c==!c && (board[k][j].p==B || board[k][j].p==Q)) return 1;
			break;
		}
	for(k=y+1,j=x-1;k<8 && j>=0;k++,j--)
		if(board[k][j].c!=n)
		{
	       		if(board[k][j].c==!c && (board[k][j].p==B || board[k][j].p==Q)) return 1;
			break;
		}
	for(k=y-1,j=x-1;k>=0 && j>=0;k--,j--)
		if(board[k][j].c!=n)
		{
	       		if(board[k][j].c==!c && (board[k][j].p==B || board[k][j].p==Q)) return 1;
			break;
		}
	return 0;	
}

int castle(char x1,char y1,char x2,char y2)
{
	if(	queen_side_castle_white(x1,y1,x2,y2) ||
		king_side_castle_white(x1,y1,x2,y2) ||
		queen_side_castle_black(x1,y1,x2,y2) ||
		king_side_castle_black(x1,y1,x2,y2) )
		return 1;
	return 0;
}

int queen_side_castle_white(char x1,char y1,char x2,char y2)
{
	if(board[0][4].c==w && board[0][0].c==w &&
			board[0][0].p==R && board[0][1].p==0 && board[0][2].p==0 && board[0][3].p==0 &&
			!under_attack(0,0,w) && !under_attack(1,0,w) && !under_attack(2,0,w) && !under_attack(3,0,w) &&
			x1==4 && y1==0 && x2==2 && y2==0)
		return 1;
	else return 0;
}
int king_side_castle_white(char x1,char y1,char x2,char y2)
{
	if(board[0][4].c==w && board[0][7].c==w &&
			board[0][7].p==R && board[0][6].p==0 && board[0][5].p==0 && 
			!under_attack(7,0,w) && !under_attack(6,0,w) && !under_attack(5,0,w) &&
			x1==4 && y1==0 && x2==6 && y2==0)
		return 1;
	else return 0;
}
int queen_side_castle_black(char x1,char y1,char x2,char y2)
{
	if(board[7][4].c==b && board[7][0].c==b &&
			board[7][0].p==R && board[7][1].p==0 && board[7][2].p==0 && board[7][3].p==0 &&
			!under_attack(0,7,b) && !under_attack(1,7,b) && !under_attack(2,7,b) && !under_attack(3,7,b) &&
			x1==4 && y1==7 && x2==2 && y2==7)
		return 1;
	else return 0;
}
int king_side_castle_black(char x1,char y1,char x2,char y2)
{
	if(board[7][4].c==b && board[7][7].c==b &&
			board[7][7].p==R && board[7][6].p==0 && board[7][5].p==0 && 
			!under_attack(7,7,b) && !under_attack(6,7,b) && !under_attack(5,7,b) &&
			x1==4 && y1==7 && x2==6 && y2==7)
		return 1;
	else return 0;
}

void movepiece(char x1,char y1,char x2,char y2,int fakemove)
{
	if(!fakemove)
	{
		if(x1==0 && y1==7) b_r1=1;
		if(x1==0 && y1==0) w_r1=1;
		if(x1==7 && y1==7) b_r2=1;
		if(x1==7 && y1==0) w_r2=1;
		if(board[y1][x1].p==K && board[y1][x1].c==w) w_k=1;
		if(board[y1][x1].p==K && board[y1][x1].c==b) b_k=1;
	}
	/* Castelling */
	if(queen_side_castle_white(x1,y1,x2,y2))
	{
		copy_piece(&board[0][3],&board[0][0]);
		delete_piece(&board[0][0]);
	}
	else if(king_side_castle_white(x1,y1,x2,y2))
	{
		copy_piece(&board[0][5],&board[0][7]);
		delete_piece(&board[0][7]);
	}
	else if(queen_side_castle_black(x1,y1,x2,y2))
	{
		copy_piece(&board[7][3],&board[7][0]);
		delete_piece(&board[7][0]);
	}
	else if(king_side_castle_black(x1,y1,x2,y2))
	{
		copy_piece(&board[7][5],&board[7][7]);
		delete_piece(&board[7][7]);
	}
	/* Enpassent */
	if(board[y1][x1].c==w && y1==4 &&
			((last_x1==x1-1 && last_y1==6 && last_x2==x1-1 && last_y2==4 && y2==5 && x2==x1-1 && x1-1>=0) ||
			(last_x1==x1+1 && last_y1==6 && last_x2==x1+1 && last_y2==4 && y2==5 && x2==x1+1 && x1+1<8)))
	{
		copy_piece(&board[y2][x2],&board[y1][x1]);
		delete_piece(&board[y2-1][x2]);
		point[w]++;
		killed[b][(num[b])++]=P;
	}
	if(board[y1][x1].c==b && y1==3 &&
			((last_x1==x1-1 && last_y1==1 && last_x2==x1-1 && last_y2==3 && y2==2 && x2==x1-1 && x1-1>=0) ||
			(last_x1==x1+1 && last_y1==1 && last_x2==x1+1 && last_y2==3 && y2==2 && x2==x1+1 && x1+1<8)))
	{
		copy_piece(&board[y2][x2],&board[y1][x1]);
		delete_piece(&board[y2+1][x2]);
		point[b]++;
		killed[w][(num[w])++]=P;
	}
	
	if(board[y2][x2].c==!board[y1][x1].c)
	{
		switch(board[y2][x2].p)
		{
			case P:
				point[board[y1][x1].c]+=1;
				break;
			case Kn:
			case B:
				point[board[y1][x1].c]+=3;
				break;
			case R:
				point[board[y1][x1].c]+=5;
				break;
			case Q:
				point[board[y1][x1].c]+=9;
				break;
		}
		killed[board[y2][x2].c][(num[board[y2][x2].c])++]=board[y2][x2].p;
	}
	copy_piece(&board[y2][x2],&board[y1][x1]);
	delete_piece(&board[y1][x1]);
	if(!fakemove) last_x1=x1,last_y1=y1,last_x2=x2,last_y2=y2;
}

void delete_piece(pieces *a)
{
	a->c=n;
	a->p=0;
	a->n=0;
}
void copy_piece(pieces *a,pieces *b)
{
	a->c=b->c;
	a->p=b->p;
	a->n=b->n;
}

int turn_is_check(char x1,char y1,char x2,char y2)
{
	pieces temp={0,0,0};
	char flag=0,c=board[y1][x1].c;
	copy_piece(&temp,&board[y2][x2]);
	copy_piece(&board[y2][x2],&board[y1][x1]);
	delete_piece(&board[y1][x1]);
	if(king_in_check(c)) flag=1;
	copy_piece(&board[y1][x1],&board[y2][x2]);
	copy_piece(&board[y2][x2],&temp);
	return flag;
}

int mate(char color)
{
	char x,y,x1,y1;
	for(y=0;y<8;y++)
		for(x=0;x<8;x++)
			if(board[y][x].c==color)
				for(y1=0;y1<8;y1++)
					for(x1=0;x1<8;x1++)
						if(valid(x,y,x1,y1))
							return 0;
	return 1;
}
