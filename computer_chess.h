/* CHESS 
 * Author 	: Rakesh Malik & Dibyayan Chakroborty
 * Date started : 30.01.2011
 * Subject	: Chess functions for player computer
 * Comment	: Used only by chess.c
 */

typedef struct move_struct					/* Structure for move */ 
{
	unsigned char x1 : 3;					/* source x */
	unsigned char y1 : 3;					/* source y */
	unsigned char x2 : 3;					/* destination x */
	unsigned char y2 : 3;					/* destination y */
}
move_struct;

typedef struct move_struct_list					/* Structure of a node of linked-list for storing valid moves */
{
	struct move_struct move;				/* move structure */
	struct move_struct_list *next;				/* pointer to next node */
	int i;							/* information ( can be used to store various information ) */
}
move_struct_list;
void entry(move_struct_list **list,int x1,int y1,int x2,int y2);	/* inserts a given move in linked-list */
void delete_move(move_struct_list **list,int x1,int y1,int x2,int y2);	/* deletes a given move from linked-list if present */

typedef struct point						/* Structure for a point */
{
	unsigned char x : 4;					
	unsigned char y : 4;
}
coordinate;

int getmove(int color,int *x1,int *y1,int *x2,int *y2);	/* Generates a move in x1y1-x2y2 for given color */

void copy_board(struct pieces [8][8],struct pieces [8][8]);	/* copies whole board from src to dest */

int strength(int c,int x,int y,int flag);			/* Calculates strength given on xy from whole board for given color 
								 * flag = 0 means preciousness of pieces are considered
								 * 	= 1 means point of pieces are considered */
int strength_from(int color,int x,int y);			/* Calculates total strength given on whole board from xy for given color */

int pos_value[8][8];						/* positional values of xy*/
int guard[8][8];						/* total value guarded on board from xy */
int attack[8][8];						/* total value attacked on board from xy */
void update_positional_value();					/* Updates posional value,guard value, attack value in whole board */

struct attack_list_struct					/* list of pieces that attacked a particular cell */
{
	struct point a[2][16];					/* list of attack from cells aquired by white and black */
	char n[2];						/* number of attackers of white and black */
}
attack_lists[8][8];						/* attack list of whole board */
int is_valuable_position(char color,char x,char y);		/* Returns 1 if xy is valuable position for given color else returns 0 */
void create_attack_list(int color,int x,int y,int depth,struct attack_list_struct *attack_list,int lb1,int lb2,int simple);
								/* Creates attack list for gine color in xy
								 * depth,lb1,lb2 are to sent 0
								 * attack_list is to be sent attack_lists[y][x] genarally
								 * simple = 1 means recursion is avoided/0 when recursion needed */
void exclude_threatened_pieces(int color,int x,int y,struct attack_list_struct *main_list,struct attack_list_struct list,int depth);
								/* used by create_attack_list() */
int gain(int color,int x,int y,struct attack_list_struct attack_list);


/* -----------------Function Definitions------------------- */

int getmove(int c,int *X1,int *Y1,int *X2,int *Y2)
{
	int x,y,x1,y1,x2,y2,_x1,_y1,_x2,_y2,i,j,k,flag;
	pieces copy1[8][8],copy2[8][8];
	move_struct_list *list=NULL,*temp,*temp1;

	update_positional_value();
	for(i=0;i<8;i++)
		for(j=0;j<8;j++)
			create_attack_list(c,i,j,0,&attack_lists[j][i],0,0,false);
	*player_computer=2;
	
	// Creating valid move list 
	for(x1=0;x1<8;x1++)
		for(y1=0;y1<8;y1++)
			if(board[y1][x1].c==c)
				for(x2=0;x2<8;x2++)
					for(y2=0;y2<8;y2++)
						if(valid(x1,y1,x2,y2))
							entry(&list,x1,y1,x2,y2);
	
	*player_computer=2;
	int novm=0;	//Number of valid moves
	for(temp=list;temp;temp=temp->next) novm++;
	double percentagecompleted=2.0;
	
	for(temp=list;temp;temp=temp->next)
	{
		x1=temp->move.x1,y1=temp->move.y1,x2=temp->move.x2,y2=temp->move.y2;
		
		//1// if move causes opponant to be mate all other moves are discarded
		copy_board(copy1,board);
		copy_piece(&board[y2][x2],&board[y1][x1]);
		delete_piece(&board[y1][x1]);
		if(mate(!c))
			for(temp1=list;temp1;temp1=temp1->next)
				if(x1!=temp1->move.x1 || y1!=temp1->move.y1 || x2!=temp1->move.x2 || y2!=temp1->move.y2)
					delete_move(&list,temp1->move.x1,temp1->move.y1,temp1->move.x2,temp1->move.y2);
		copy_board(board,copy1);

		//2// if move causes mate in depth 1 it is discarded
		copy_board(copy1,board);
		copy_piece(&board[y2][x2],&board[y1][x1]);
		delete_piece(&board[y1][x1]);
		for(_x1=0;_x1<8;_x1++)
			for(_y1=0;_y1<8;_y1++)
				if(board[_y1][_x1].c==!c)
					for(_x2=0;_x2<8;_x2++)
						for(_y2=0;_y2<8;_y2++)
							if(valid(_x1,_y1,_x2,_y2))
							{
								copy_board(copy2,board);
								copy_piece(&board[_y2][_x2],&board[_y1][_x1]);
								delete_piece(&board[_y1][_x1]);
								if(mate(c))
									delete_move(&list,temp->move.x1,temp->move.y1,
											temp->move.x2,temp->move.y2);
								copy_board(board,copy2);
							}
		copy_board(board,copy1);
		
		percentagecompleted+=(double)8/(double)novm;
		*player_computer=(int)percentagecompleted;
	}	
	
	//3// move is discarded if it causes more than minimum number of threats		
	int threats,minthreats;
	for(minthreats=100,temp=list;temp;temp=temp->next)
	{		
		x1=temp->move.x1,y1=temp->move.y1,x2=temp->move.x2,y2=temp->move.y2;
		temp->i=0;
		copy_board(copy1,board);ng valid move list 
		copy_piece(&board[y2][x2],&board[y1][x1]);
		delete_piece(&board[y1][x1]);
		threats=0;
		for(x=0;x<8;x++)
			for(y=0;y<8;y++)
				create_attack_list(c,x,y,0,&attack_lists[y][x],0,0,false);
		for(x=0;x<8;x++)
			for(y=0;y<8;y++)
				if(gain(c,x,y,attack_lists[y][x])<0)
					temp->i++;
		for(_x1=0;_x1<8 && temp->i<=minthreats;_x1++)
		{
			for(_y1=0;_y1<8 && temp->i<=minthreats;_y1++)
			{
				if(board[_y1][_x1].c==!c)
					for(_x2=0;_x2<8 && temp->i<=minthreats;_x2++)
						for(_y2=0;_y2<8 && temp->i<=minthreats;_y2++)
							if(valid(_x1,_y1,_x2,_y2))
							{
								threats=0;
								copy_board(copy2,board);
								copy_piece(&board[_y2][_x2],&board[_y1][_x1]);
								delete_piece(&board[_y1][_x1]);
								for(x=0;x<8;x++)
									for(y=0;y<8;y++)
										create_attack_list(c,x,y,0,&attack_lists[y][x],0,0,false);
								if(gain(!c,_x2,_y2,attack_lists[_y2][_x2])>=0)
								{
									for(x=0;x<8;x++)
										for(y=0;y<8;y++)
										{
											flag=0;
											if(gain(c,x,y,attack_lists[y][x])>=0)
												flag=1;
											if(board[y][x].c==c)
												for(i=0;i<8 && flag==0;i++)
													for(j=0;j<8 && flag==0;j++)
														if(valid(x,y,i,j)
															&& gain(c,i,j,attack_lists[j][i])>=0)
															flag=1;
											if(flag==0 && can_be_guarded(c,x,y))
												flag=1;
											if(flag==0) threats++;
										}
								}
								copy_board(board,copy2);
								if(threats>temp->i)
									temp->i=threats;
							}
				percentagecompleted+=(double)80/(double)(64*novm);
				*player_computer=(int)percentagecompleted;
			}
		}
		copy_board(board,copy1);
		if(temp->i<minthreats)
			minthreats=temp->i;
		
		
	}
	for(temp=list;temp;temp=temp->next)
		if(temp->i>minthreats)
			delete_move(&list,temp->move.x1,temp->move.y1,temp->move.x2,temp->move.y2);
	
	for(novm=0,temp=list;temp;temp=temp->next) novm++;
	*player_computer=90;
	
	// Move selection
	int ot=0;
	int max_ot=0;
	int eat=0;						/* Number of pieces that can be taken by current move */
	int t[16];
	int t1[16];
	move_struct_list *temp_eat;
	temp1=list;
	for(temp=list;temp;temp=temp->next)
	{
		ot=0;
		for(x=0;x<8;x++)
			for(y=0;y<8;y++)
				create_attack_list(c,x,y,0,&attack_lists[y][x],0,0,false);
		x1=temp->move.x1,y1=temp->move.y1,x2=temp->move.x2,y2=temp->move.y2;
		if(board[y2][x2].c==!c && gain(!c,x2,y2,attack_lists[y2][x2])<0)
			if(board[y2][x2].n>eat)
			{
				eat=board[y2][x2].n;
				temp_eat=temp;
			}
		copy_board(copy1,board);
		copy_piece(&board[y2][x2],&board[y1][x1]);
		delete_piece(&board[y1][x1]);
		for(x=0;x<8;x++)
			for(y=0;y<8;y++)
				create_attack_list(c,x,y,0,&attack_lists[y][x],0,0,false);
		for(x=0;x<8;x++)
			for(y=0;y<8;y++)
				if(board[y][x].c==!c && gain(!c,x,y,attack_lists[y][x])<0)
					t1[ot++]=board[y][x].n;
		copy_board(board,copy1);
		if(ot>max_ot)
		{
			max_ot=ot;
			for(i=0;i<ot;i++) t[i]=t1[i];
			temp1=temp;
		}
		
		*player_computer+=8/novm;
	}
	if(eat>0)
	{
		flag=0;
		for(i=0;i<max_ot;i++)
			if(t[i]>eat)
				flag=1;
		if(flag==0) temp1=temp_eat;
	}
		
	/* If move selection Fails Select Random */
	*player_computer=99;
	
	if(temp1==NULL)
	{
		int old_rank,new_rank,best_rank=-99999999;
		for(temp=list;temp;temp=temp->next)
		{
			x1=temp->move.x1,y1=temp->move.y1,x2=temp->move.x2,y2=temp->move.y2;
			update_positional_value();
			old_rank=attack[y1][x1]+guard[y1][x1]+pos_value[y1][x1];
			copy_board(copy1,board);
			movepiece(x1,y1,x2,y2,1);
			update_positional_value();
			new_rank=attack[y2][x2]+guard[y2][x2]+pos_value[y2][x2];
			copy_board(board,copy1);
			if(new_rank-old_rank>best_rank)
			{
				temp1=temp;
				best_rank=new_rank-old_rank;
			}
		}
	}
	
	/* Return */	
	*X1=temp1->move.x1,*Y1=temp1->move.y1,*X2=temp1->move.x2,*Y2=temp1->move.y2;
}

int strength(int c,int x,int y,int flag)
{
	int str=0;
	char i,j,k,l;
	struct pieces copy[8][8];
	int tempx,tempy;
	copy_board(copy,board);
	board[y][x].c=!c;
	for(i=0;i<8;i++)
		for(j=0;j<8;j++)
			if(board[j][i].c==c && valid(i,j,x,y))
				str+=((flag==1)?10:0)+board[j][i].n;				
	copy_board(board,copy);
	return str;
}
void create_attack_list(int c,int x,int y,int depth,struct attack_list_struct *attack_list,int lb1,int lb2,int simple)
{
	char i,j,k,l;
	struct pieces copy[8][8];
	int tempx,tempy;
	copy_board(copy,board);
	attack_list->n[c]=lb1;
	attack_list->n[!c]=lb2;
	for(i=0;i<8;i++)
		for(j=0;j<8;j++)
		{
			board[y][x].c=!c;
			if(board[j][i].c==c && valid(i,j,x,y))
			{
				attack_list->a[c][attack_list->n[c]].x=i;
				attack_list->a[c][attack_list->n[c]++].y=j;
			}
			board[y][x].c=c;
			if(board[j][i].c==!c && valid(i,j,x,y))
			{
				attack_list->a[!c][attack_list->n[!c]].x=i;
				attack_list->a[!c][attack_list->n[!c]++].y=j;
			}
			copy_board(board,copy);
		}
	//Sorting
	for(k=lb1;k<attack_list->n[c];k++)
		for(l=lb1;l<attack_list->n[c]-k-1;l++)
			if(board[attack_list->a[c][l].y][attack_list->a[c][l].x].n > 
					board[attack_list->a[c][l+1].y][attack_list->a[c][l+1].x].n)
			{
				tempx=attack_list->a[c][l+1].x;
				attack_list->a[c][l+1].x=attack_list->a[c][l].x;
				attack_list->a[c][l].x=tempx;
				tempy=attack_list->a[c][l+1].y;
				attack_list->a[c][l+1].y=attack_list->a[c][l].y;
				attack_list->a[c][l].y=tempy;
			}
	for(k=lb2;k<attack_list->n[!c];k++)
		for(l=lb2;l<attack_list->n[!c]-k-1;l++)
			if(board[attack_list->a[!c][l].y][attack_list->a[!c][l].x].n > 
					board[attack_list->a[!c][l+1].y][attack_list->a[!c][l+1].x].n)
			{
				tempx=attack_list->a[!c][l+1].x;
				attack_list->a[!c][l+1].x=attack_list->a[!c][l].x;
				attack_list->a[!c][l].x=tempx;
				tempy=attack_list->a[!c][l+1].y;
				attack_list->a[!c][l+1].y=attack_list->a[!c][l].y;
				attack_list->a[!c][l].y=tempy;
			}
	if(simple) return;
	//Recursion
	copy_board(copy,board);
	for(i=lb1;i<attack_list->n[c];i++)
		delete_piece(&board[attack_list->a[c][i].y][attack_list->a[c][i].x]);
	for(i=lb2;i<attack_list->n[!c];i++)
		delete_piece(&board[attack_list->a[!c][i].y][attack_list->a[!c][i].x]);
	int depthlimit=0,bishops=0,rooks=0,queens=0;
	for(i=0;i<8;i++)
		for(j=0;j<8;j++)
			if(board[i][j].c==c)
				if(board[i][j].p==B) bishops++;
				else if(board[i][j].p==R) rooks++;
				else if(board[i][j].p==Q) queens++;
	depthlimit=queens+((rooks>=bishops)?rooks:(bishops>=1)?1:0);
	if(depth<depthlimit) create_attack_list(c,x,y,depth+1,attack_list,attack_list->n[c],attack_list->n[!c],false);
	copy_board(board,copy);	
	//excluding threatened pieces
	struct attack_list_struct temp;
	if(depth==0 && board[y][x].c==c)
	{
		temp.n[c]=attack_list->n[c];
		for(i=0;i<temp.n[c];i++)
			temp.a[c][i]=attack_list->a[c][i];
		temp.n[!c]=attack_list->n[!c];
		for(i=0;i<temp.n[!c];i++)
			temp.a[!c][i]=attack_list->a[!c][i];
		exclude_threatened_pieces(c,x,y,attack_list,temp,0);
	}
}

void exclude_threatened_pieces(int c,int x,int y,struct attack_list_struct *main_list,struct attack_list_struct list,int depth)
{
	if(depth>=list.n[c] || list.n[c]==0 || list.n[!c]==0) return;
	pieces copy[8][8];
	int i,j,k,l,threat,current_gain;
	coordinate p;
	struct attack_list_struct temp_lists[8][8];
	for(i=0;i<list.n[!c];i++)
	{
		if(depth>=main_list->n[c]) return;
		current_gain=board[list.a[c][depth].y][list.a[c][depth].x].n;
		copy_board(copy,board);
		copy_piece(&board[y][x],&board[list.a[c][depth].y][list.a[c][depth].x]);
		delete_piece(&board[list.a[!c][i].y][list.a[!c][i].x]);
		delete_piece(&board[list.a[c][depth].y][list.a[c][depth].x]);
		for(j=0;j<8;j++)
			for(k=0;k<8;k++)
				create_attack_list(c,j,k,0,&temp_lists[k][j],0,0,true);
		threat=false;
		for(j=list.a[c][depth].x,k=list.a[c][depth].y+1;!threat&&k<8;k++)
		{
			if(j==x && k==y) continue;
			if(gain(c,j,k,temp_lists[k][j])<0 && board[k][j].n>current_gain) threat=true;
			if(board[k][j].c!=n) break;
		}
		for(j=list.a[c][depth].x,k=list.a[c][depth].y-1;!threat&&k>=0;k--)
		{
			if(j==x && k==y) continue;
			if(gain(c,j,k,temp_lists[k][j])<0 && board[k][j].n>current_gain) threat=true;
			if(board[k][j].c!=n) break;
		}
		for(j=list.a[c][depth].x-1,k=list.a[c][depth].y;!threat&&j>=0;j--)
		{
			if(j==x && k==y) continue;
			if(gain(c,j,k,temp_lists[k][j])<0 && board[k][j].n>current_gain) threat=true;
			if(board[k][j].c!=n) break;
		}
		for(j=list.a[c][depth].x+1,k=list.a[c][depth].y;!threat&&j<8;j++)
		{
			if(j==x && k==y) continue;
			if(gain(c,j,k,temp_lists[k][j])<0 && board[k][j].n>current_gain) threat=true;
			if(board[k][j].c!=n) break;
		}
		for(j=list.a[c][depth].x+1,k=list.a[c][depth].y+1;!threat&&j<8&&k<8;j++,k++)
		{
			if(j==x && k==y) continue;
			if(gain(c,j,k,temp_lists[k][j])<0 && board[k][j].n>current_gain) threat=true;
			if(board[k][j].c!=n) break;
		}
		for(j=list.a[c][depth].x-1,k=list.a[c][depth].y+1;!threat&&j>=0&&k<8;j--,k++)
		{
			if(j==x && k==y) continue;
			if(gain(c,j,k,temp_lists[k][j])<0 && board[k][j].n>current_gain) threat=true;
			if(board[k][j].c!=n) break;
		}
		for(j=list.a[c][depth].x+1,k=list.a[c][depth].y-1;!threat&&j<8&&k>=0;j++,k--)
		{
			if(j==x && k==y) continue;
			if(gain(c,j,k,temp_lists[k][j])<0 && board[k][j].n>current_gain) threat=true;
			if(board[k][j].c!=n) break;
		}
		for(j=list.a[c][depth].x-1,k=list.a[c][depth].y-1;!threat&&j>=0&&k>=0;j--,k--)
		{
			if(j==x && k==y) continue;
			if(gain(c,j,k,temp_lists[k][j])<0 && board[k][j].n>current_gain) threat=true;
			if(board[k][j].c!=n) break;
		}
		copy_board(board,copy);
		if(threat)
		{
			main_list->n[c]--;
			memmove(main_list->a[c]+depth,main_list->a[c]+depth+1,(16-depth-1)*sizeof(coordinate));
		}
		else if(depth<3)
			exclude_threatened_pieces(c,x,y,main_list,temp_lists[x][y],depth+1);
	}
}


int gain(int c,int x,int y,struct attack_list_struct attack_list)
{
	int i,gain=0;
	if(attack_list.n[!c]==0)
		gain=0;
	else if(board[y][x].c==c && attack_list.n[c]<attack_list.n[!c])
	{
		gain-=board[y][x].n;
		for(i=0;i<attack_list.n[c];i++)
			gain-=board[attack_list.a[c][i].y][attack_list.a[c][i].x].n;
		for(i=0;i<attack_list.n[c];i++)
			gain+=board[attack_list.a[!c][i].y][attack_list.a[!c][i].x].n;
	}
	else if(board[y][x].c==c && attack_list.n[c]==attack_list.n[!c])
	{
		gain-=board[y][x].n;
		for(i=0;i<attack_list.n[c]-1;i++)
			gain-=board[attack_list.a[c][i].y][attack_list.a[c][i].x].n;
		for(i=0;i<attack_list.n[!c];i++)
			gain+=board[attack_list.a[!c][i].y][attack_list.a[!c][i].x].n;
	}
	else if(board[y][x].c==c && attack_list.n[c]>attack_list.n[!c])
	{
		for(i=0;i<attack_list.n[!c];i++)
			gain+=board[attack_list.a[!c][i].y][attack_list.a[!c][i].x].n;
		gain-=board[y][x].n;
		for(i=0;i<attack_list.n[!c]-1;i++)
			gain-=board[attack_list.a[c][i].y][attack_list.a[c][i].x].n;		
	}
	else gain=0;
	return gain;
}

int strength_from(int c,int x1,int y1)
{
	int i,j,str=0;
	for(i=0;i<8;i++)if(i!=x1)
	{
		if(board[y1][i].c==c) str+=5*strength(c,i,y1,1);
		else str+=strength(c,i,y1,1);
		if((i==3||i==4) && (y1==3||y1==4)) str+=5*strength(c,i,y1,1);
	}
	for(i=0;i<8;i++) if(i!=y1)
	{
		if(board[i][x1].c==c) str+=5*strength(c,x1,i,1);
		else str+=strength(c,x1,i,1);
		if((x1==3||x1==4) && (i==3||i==4)) str+=5*strength(c,x1,i,1);
	}
	for(i=x1+1,j=y1+1;i<8 && j<8;i++,j++)
	{
		if(board[j][i].c==c) str+=5*strength(c,i,j,1);
		else str+=strength(c,i,j,1);
		if((i==3||i==4) && (j==3||j==4)) str+=5*strength(c,i,j,1);
	}
	for(i=x1+1,j=y1-1;i<8 && j>=0;i++,j--)
	{
		if(board[j][i].c==c) str+=5*strength(c,i,j,1);
		else str+=strength(c,i,j,1);
		if((i==3||i==4) && (j==3||j==4)) str+=5*strength(c,i,j,1);
	}
	for(i=x1-1,j=y1+1;i>=0 && j<8;i--,j++)
	{
		if(board[j][i].c==c) str+=5*strength(c,i,j,1);
		else str+=strength(c,i,j,1);
		if((i==3||i==4) && (j==3||j==4)) str+=5*strength(c,i,j,1);
	}
	for(i=x1-1,j=y1-1;i>=0 && j>=0;i--,j--)
	{
		if(board[j][i].c==c) str+=5*strength(c,i,j,1);
		else str+=strength(c,i,j,1);
		if((i==3||i==4) && (j==3||j==4)) str+=5*strength(c,i,j,1);
	}
	return str;
}

int can_be_guarded(int c,int x,int y)
{
	int x1,y1,x2,y2,flag=0;
	pieces copy[8][8];
	for(x1=0;x1<8 && flag==0;x1++)
		for(y1=0;y1<8 && flag==0;y1++)
			if(board[y1][x1].c==c)
				for(x2=0;x2<8 && flag==0;x2++)
					for(y2=0;y2<8 && flag==0;y2++)
						if(valid(x1,y1,x2,y2))
						{
							copy_board(copy,board);
							copy_piece(&board[y2][x2],&board[y1][x1]);
							delete_piece(&board[y1][x1]);
							create_attack_list(c,x,y,0,&attack_lists[y][x],0,0,false);
							if(gain(c,x,y,attack_lists[y][x])>=0)
								flag=1;
							copy_board(board,copy);
						}
	return flag;
}

void entry(move_struct_list** list,int x1,int y1,int x2,int y2)
{
	move_struct_list *temp;
	temp=(move_struct_list*)malloc(sizeof(move_struct_list));
	temp->move.x1=x1;
	temp->move.y1=y1;
	temp->move.x2=x2;
	temp->move.y2=y2;
	temp->next=*list;
	*list=temp;
}

void delete_move(move_struct_list **list,int x1,int y1,int x2,int y2)
{
	move_struct_list *temp;
	for(;*list;list=&(*list)->next)
		if((*list)->move.x1==x1 && (*list)->move.y1==y1 && (*list)->move.x2==x2 && (*list)->move.y2==y2)
		{
			temp=*list;
			*list=(*list)->next;
			free(temp);
			return;
		}
}

void update_positional_value()
{
	int x,y,i,j,total=0;
	char kx[8]={+1,+1,+2,+2,-1,-1,-2,-2},ky[8]={+2,-2,+1,-1,+2,-2,+1,-1};		/* Knight moves */
	for(x=0;x<8;x++)
		for(y=0;y<8;y++)
		{
			/* Positional value */
			pos_value[y][x]=0;
			switch(board[y][x].p)
			{
				case P:
					if(board[y][x].c==w) pos_value[y][x]=y-1;
					if(board[y][x].c==b) pos_value[y][x]=6-y;
					break;
				case B:
					for(i=x+1,j=y+1;i<8 && j<8 && board[j][i].c==n;i++,j++)
						if(strength(board[y][x].c,i,j,1)-strength(!board[y][x].c,i,j,1)>0)
							pos_value[y][x]++;
					for(i=x-1,j=y+1;i>=0 && j<8 && board[j][i].c==n;i--,j++)
						if(strength(board[y][x].c,i,j,1)-strength(!board[y][x].c,i,j,1)>0)
							pos_value[y][x]++;
					for(i=x+1,j=y-1;i<8 && j>=0 && board[j][i].c==n;i++,j--)
						if(strength(board[y][x].c,i,j,1)-strength(!board[y][x].c,i,j,1)>0)
							pos_value[y][x]++;
					for(i=x-1,j=y-1;i>=0 && j>=0 && board[j][i].c==n;i--,j--)
						if(strength(board[y][x].c,i,j,1)-strength(!board[y][x].c,i,j,1)>0)
							pos_value[y][x]++;
					break;
				case R:
					for(i=x+1;i<8 && board[y][i].c==n;i++) 
						if(strength(board[y][x].c,i,y,1)-strength(!board[y][x].c,i,y,1)>0)
							pos_value[y][x]++;
					for(i=x-1;i>=0 && board[y][i].c==n;i--) 
						if(strength(board[y][x].c,i,y,1)-strength(!board[y][x].c,i,y,1)>0)
							pos_value[y][x]++;
					for(i=y+1;i<8 && board[i][x].c==n;i++) 
						if(strength(board[y][x].c,x,i,1)-strength(!board[y][x].c,x,i,1)>0)
							pos_value[y][x]++;
					for(i=y-1;i>=0 && board[i][x].c==n;i--) 
						if(strength(board[y][x].c,i,y,1)-strength(!board[y][x].c,i,y,1)>0)
							pos_value[y][x]++;
					break;
				case Kn:
					for(i=0;i<8;i++)
						if(y+ky[i]>=0 && y+ky[i]<8 && x+kx[i]>=0 && x+kx[i]<8 && board[y+ky[i]][x+kx[i]].c==n
								&& strength(board[y][x].c,x+kx[i],y+ky[i],1)
								-strength(!board[y][x].c,x+kx[i],y+ky[i],1)>0)
							pos_value[y][x]++;
					break;
				case Q:
					for(i=x+1,j=y+1;i<8 && j<8 && board[j][i].c==n;i++,j++)
						if(strength(board[y][x].c,i,j,1)-strength(!board[y][x].c,i,j,1)>0)
							pos_value[y][x]++;
					for(i=x-1,j=y+1;i>=0 && j<8 && board[j][i].c==n;i--,j++)
						if(strength(board[y][x].c,i,j,1)-strength(!board[y][x].c,i,j,1)>0)
							pos_value[y][x]++;
					for(i=x+1,j=y-1;i<8 && j>=0 && board[j][i].c==n;i++,j--)
						if(strength(board[y][x].c,i,j,1)-strength(!board[y][x].c,i,j,1)>0)
							pos_value[y][x]++;
					for(i=x-1,j=y-1;i>=0 && j>=0 && board[j][i].c==n;i--,j--)
						if(strength(board[y][x].c,i,j,1)-strength(!board[y][x].c,i,j,1)>0)
							pos_value[y][x]++;
					for(i=x+1;i<8 && board[y][i].c==n;i++) 
						if(strength(board[y][x].c,i,y,1)-strength(!board[y][x].c,i,y,1)>0)
							pos_value[y][x]++;
					for(i=x-1;i>=0 && board[y][i].c==n;i--) 
						if(strength(board[y][x].c,i,y,1)-strength(!board[y][x].c,i,y,1)>0)
							pos_value[y][x]++;
					for(i=y+1;i<8 && board[i][x].c==n;i++) 
						if(strength(board[y][x].c,x,i,1)-strength(!board[y][x].c,x,i,1)>0)
							pos_value[y][x]++;
					for(i=y-1;i>=0 && board[i][x].c==n;i--) 
						if(strength(board[y][x].c,i,y,1)-strength(!board[y][x].c,i,y,1)>0)
							pos_value[y][x]++;
					break;
			}
			/* Attack */
			attack[y][x]=0;
			if(board[y][x].c!=n)
			{
				switch(board[y][x].p)
				{
					case Q:
					case R:
						for(j=x+1;j<8;j++) 
						{
							if(board[y][j].p==K && board[y][j].c==!board[y][x].c)
								attack[y][x]+=9;
							else if(board[y][j].p!=K && board[y][j].c==!board[y][x].c)
								attack[y][x]+=board[y][j].n;
							if(board[y][j].c!=n) break;
						}
						for(j=x-1;j>=0;j--)
						{
							if(board[y][j].p==K && board[y][j].c==!board[y][x].c)
								attack[y][x]+=9;
							else if(board[y][j].p!=K && board[y][j].c==!board[y][x].c)
								attack[y][x]+=board[y][j].n;
							if(board[y][j].c!=n) break;
						}
						for(i=y+1;i<8;i++) 
						{
							if(board[i][x].p==K && board[i][x].c==!board[y][x].c)
								attack[y][x]+=9;
							else if(board[i][x].p!=K && board[i][x].c==!board[y][x].c)
								attack[y][x]+=board[i][x].n;
							if(board[i][x].c!=n) break;
						}
						for(i=y-1;i>=0;i--)
						{
							if(board[i][x].p==K && board[i][x].c==!board[y][x].c)
								attack[y][x]+=9;
							else if(board[i][x].p!=K && board[i][x].c==!board[y][x].c)
								attack[y][x]+=board[i][x].n;
							if(board[i][x].c!=n) break;
						}
						if(board[y][x].p==R) break;
					case B:
						for(i=y+1,j=x+1;i<8 && j<8;i++,j++)
						{
							if(board[i][j].p==K && board[i][j].c==!board[y][x].c)
								attack[y][x]+=9;
							else if(board[i][j].p!=K && board[i][j].c==!board[y][x].c)
								attack[y][x]+=board[i][j].n;
							if(board[i][j].c!=n) break;
						}
						for(i=y-1,j=x+1;i>=0 && j<8;i--,j++)
						{
							if(board[i][j].p==K && board[i][j].c==!board[y][x].c)
								attack[y][x]+=9;
							else if(board[i][j].p!=K && board[i][j].c==!board[y][x].c)
								attack[y][x]+=board[i][j].n;
							if(board[i][j].c!=n) break;
						}
						for(i=y+1,j=x-1;i<8 && j>=0;i++,j--)
						{
							if(board[i][j].p==K && board[i][j].c==!board[y][x].c)
								attack[y][x]+=9;
							else if(board[i][j].p!=K && board[i][j].c==!board[y][x].c)
								attack[y][x]+=board[i][j].n;
							if(board[i][j].c!=n) break;
						}
						for(i=y-1,j=x-1;i>=0 && j>=0;i--,j--)
						{
							if(board[i][j].p==K && board[i][j].c==!board[y][x].c)
								attack[y][x]+=9;
							else if(board[i][j].p!=K && board[i][j].c==!board[y][x].c)
								attack[y][x]+=board[i][j].n;
							if(board[i][j].c!=n) break;
						}
						break;
					case Kn:
						for(i=0;i<8;i++)
							if(y+ky[i]>=0 && y+ky[i]<8 && x+kx[i]>=0 && x+kx[i]<8)
							{
								if(board[y+ky[i]][x+kx[i]].p==K && board[y+ky[i]][x+kx[i]].c==!board[y][x].c)
									attack[y][x]+=9;
								else if(board[y+ky[i]][x+kx[i]].p!=K && board[y+ky[i]][x+kx[i]].c==!board[y][x].c)
									attack[y][x]+=board[y+ky[i]][x+kx[i]].n;
							}
						break;
					case P:
						if(board[y][x].c==w)
						{
							if(y+1<8 && x+1<8)
							{
								if(board[y+1][x+1].p==K && board[y+1][x+1].c==b)
									attack[y][x]+=9;
								else if(board[y+1][x+1].p!=K && board[y+1][x+1].c==b)
									attack[y][x]+=board[y+1][x+1].n;
							}
							if(y+1<8 && x-1>=0)
							{
								if(board[y+1][x-1].p==K && board[y+1][x-1].c==b)
									attack[y][x]+=9;
								else if(board[y+1][x-1].p!=K && board[y+1][x-1].c==b)
									attack[y][x]+=board[y+1][x-1].n;
							}
						}
						else if(board[y][x].c==b)
						{
							if(y-1>=0 && x+1<8)
							{
								if(board[y-1][x+1].p==K && board[y-1][x+1].c==w)
									attack[y][x]+=9;
								else if(board[y-1][x+1].p!=K && board[y-1][x+1].c==w)
									attack[y][x]+=board[y-1][x+1].n;
							}
							if(y-1>=0 && x-1>=0)
							{
								if(board[y-1][x-1].p==K && board[y-1][x-1].c==w)
									attack[y][x]+=9;
								else if(board[y-1][x-1].p!=K && board[y-1][x-1].c==w)
									attack[y][x]+=board[y-1][x-1].n;
							}
						}
						break;
				}
				
			}
			/* Guard */
			guard[y][x]=0;
			if(board[y][x].c!=n)
			{
				switch(board[y][x].p)
				{
					case Q:
					case R:
						for(j=x+1;j<8;j++) 
						{
							if(board[y][j].p==Q && board[y][j].c==board[y][x].c)
								guard[y][x]+=3;
							else if(board[y][j].p!=K && board[y][j].c==board[y][x].c)
								guard[y][x]+=board[y][j].n;
							if(is_valuable_position(board[y][x].c,j,y))
								guard[y][x]++;
							if(board[y][j].c!=n) break;
						}
						for(j=x-1;j>=0;j--)
						{
							if(board[y][j].p==Q && board[y][j].c==board[y][x].c)
								guard[y][x]+=3;
							else if(board[y][j].p!=K && board[y][j].c==board[y][x].c)
								guard[y][x]+=board[y][j].n;
							if(is_valuable_position(board[y][x].c,j,y))
								guard[y][x]++;
							if(board[y][j].c!=n) break;
						}
						for(i=y+1;i<8;i++) 
						{
							if(board[i][x].p==Q && board[i][x].c==board[y][x].c)
								guard[y][x]+=3;
							else if(board[i][x].p!=K && board[i][x].c== board[y][x].c)
								guard[y][x]+=board[i][x].n;
							if(is_valuable_position(board[y][x].c,x,i))
								guard[y][x]++;
							if(board[i][x].c!=n) break;
						}
						for(i=y-1;i>=0;i--)
						{
							if(board[i][x].p==Q && board[i][x].c==board[y][x].c)
								guard[y][x]+=3;
							else if(board[i][x].p!=K && board[i][x].c==board[y][x].c)
								guard[y][x]+=board[i][x].n;
							if(is_valuable_position(board[y][x].c,x,i))
								guard[y][x]++;
							if(board[i][x].c!=n) break;
						}
						if(board[y][x].p==R) break;
					case B:
						for(i=y+1,j=x+1;i<8 && j<8;i++,j++)
						{
							if(board[i][j].p==Q && board[i][j].c==board[y][x].c)
								guard[y][x]+=3;
							else if(board[i][j].p!=K && board[i][j].c==board[y][x].c)
								guard[y][x]+=board[i][j].n;
							if(is_valuable_position(board[y][x].c,j,i))
								guard[y][x]++;
							if(board[i][j].c!=n) break;
						}
						for(i=y-1,j=x+1;i>=0 && j<8;i--,j++)
						{
							if(board[i][j].p==Q && board[i][j].c==board[y][x].c)
								guard[y][x]+=3;
							else if(board[i][j].p!=K && board[i][j].c==board[y][x].c)
								guard[y][x]+=board[i][j].n;
							if(is_valuable_position(board[y][x].c,j,i))
								guard[y][x]++;
							if(board[i][j].c!=n) break;
						}
						for(i=y+1,j=x-1;i<8 && j>=0;i++,j--)
						{
							if(board[i][j].p==Q && board[i][j].c==board[y][x].c)
								guard[y][x]+=3;
							else if(board[i][j].p!=K && board[i][j].c==board[y][x].c)
								guard[y][x]+=board[i][j].n;
							if(is_valuable_position(board[y][x].c,j,i))
								guard[y][x]++;
							if(board[i][j].c!=n) break;
						}
						for(i=y-1,j=x-1;i>=0 && j>=0;i--,j--)
						{
							if(board[i][j].p==Q && board[i][j].c==board[y][x].c)
								guard[y][x]+=3;
							else if(board[i][j].p!=K && board[i][j].c==board[y][x].c)
								guard[y][x]+=board[i][j].n;
							if(is_valuable_position(board[y][x].c,j,i))
								guard[y][x]++;
							if(board[i][j].c!=n) break;
						}
						break;
					case Kn:
						for(i=0;i<8;i++)
							if(y+ky[i]>=0 && y+ky[i]<8 && x+kx[i]>=0 && x+kx[i]<8)
							{
								if(board[y+ky[i]][x+kx[i]].p==Q && board[y+ky[i]][x+kx[i]].c==board[y][x].c)
									guard[y][x]+=3;
								else if(board[y+ky[i]][x+kx[i]].p!=K && board[y+ky[i]][x+kx[i]].c==board[y][x].c)
									guard[y][x]+=board[y+ky[i]][x+kx[i]].n;
								if(is_valuable_position(board[y][x].c,x+kx[i],y+ky[i]))
									guard[y][x]++;
							}
						break;
					case P:
						if(board[y][x].c==w)
						{
							if(y+1<8 && x+1<8)
							{
								if(board[y+1][x+1].p==Q && board[y+1][x+1].c==w)
									guard[y][x]+=3;
								else if(board[y+1][x+1].p!=K && board[y+1][x+1].c==w)
									guard[y][x]+=board[y+1][x+1].n;
								if(is_valuable_position(board[y][x].c,x+1,y+1))
									guard[y][x]++;
							}
							if(y+1<8 && x-1>=0)
							{
								if(board[y+1][x-1].p==Q && board[y+1][x-1].c==w)
									guard[y][x]+=3;
								else if(board[y+1][x-1].p!=K && board[y+1][x-1].c==w)
									guard[y][x]+=board[y+1][x-1].n;
								if(is_valuable_position(board[y][x].c,x-1,y+1))
									guard[y][x]++;
							}
						}
						else if(board[y][x].c==b)
						{
							if(y-1>=0 && x+1<8)
							{
								if(board[y-1][x+1].p==Q && board[y-1][x+1].c==b)
									guard[y][x]+=3;
								else if(board[y-1][x+1].p!=K && board[y-1][x+1].c==b)
									guard[y][x]+=board[y-1][x+1].n;
								if(is_valuable_position(board[y][x].c,x+1,y-1))
									guard[y][x]++;
							}
							if(y-1>=0 && x-1>=0)
							{
								if(board[y-1][x-1].p==Q && board[y-1][x-1].c==b)
									guard[y][x]+=3;
								else if(board[y-1][x-1].p!=K && board[y-1][x-1].c==b)
									guard[y][x]+=board[y-1][x-1].n;
								if(is_valuable_position(board[y][x].c,x-1,y-1))
									guard[y][x]++;
							}
						}
						break;
				}
			}
		}
}

int is_valuable_position(char c,char x,char y)
{
	if((x>=2 && x<=5) && (y>=3 && y<=4)) return 1;
	int i,j,k;
	char Kx[8]={+1,+1,+0,+0,-1,-1,-1,+1},Ky[8]={+1,+0,+1,-1,+0,-1,+1,-1};  /* King moves */
	for(i=0;i<8;i++)
		for(j=0;j<8;j++)
			if(board[j][i].c==c && board[j][i].p==K)
			{
				for(k=0;k<8;k++)
					if(x==i+Kx[k] && y==j+Ky[k])
						return 1;
				return 0;
			}
	return 0;
}
