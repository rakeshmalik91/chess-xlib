/* Subject	: Handiling button,textbox etc. in xlib
 * Author	: Rakesh Malik
 * date	started	: 03.03.2011
 */

#ifndef _WINDOW_CONTENTS_H
#define _WINDOW_CONTENTS_H

#include<X11/Xlib.h>
#include<string.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdio.h>
#include<sys/types.h>
#include<dirent.h>

int key_pressed(Display *display,XEvent event,char *key);
					// returns 1 if a key is pressed else returns o

int button_press_area(XEvent event,int x,int y,int w,int h);
					// returns 1 if event is buttonpress and it is pressed in given area else returns 0

char *file_starting_with(char *fpath);	// Returns a filepath starting with 'fpath' if exist
int get_filename(char *fpath,Display *display,Window win,GC gc,char *caption,char *buttonlebel);
					// opens a window and scans a filepath and returns in 'fpath'

typedef struct button_struct
{
	Display *display;		// display
	Window win;			// Parent window
	GC gc;				// GC
	int x;				// Starting x
	int y;				// starting y
	int height;			// width
	int width;			// height
	char *lebel;			// button lebel string
	XColor button;			// button color
	XColor shade;			// button shade color
	XColor text;			// lebel text color
	XColor buttondown;		// button color when button is pressed
}Button;
Button* create_button(Display *display,Window win,GC gc,int x,int y,int width,int height,XColor button,XColor shade,XColor text,XColor buttondown,char *lebel);
					// Initializes a button structure and prints
int button_pressed(XEvent event,Button *b);
					// Prints button press event and returns 1 if button is pressed else returns 0
void free_buttons(int n,...);		// Frees all allocated space for given 'n' buttons

typedef struct textbox_struct
{
	Display *display;		// display
	Window parent;			// parent window
	Window win;			// textbox window
	GC gc;				// gc
	int x;				// starting x
	int y;				// starting y
	int height;			// width
	int width;			// height
	char text[1000];		// text (can be initialized)
	int len;			// current text length
	int cur;			// current cursor position
	int start;			// text start position for printing ( used for scrolling )
	XColor boxc;			// textbox color
	XColor borderc;			// textbox borer color
	XColor textc;			// text color
}TextBox;
TextBox *create_textbox(Display *display,Window parent,GC gc,int x,int y,int width,int height,XColor boxc,XColor borderc,XColor textc,char *text);
					// Initializes a textbox structure and prints
char *textbox_input(XEvent event,TextBox *t);
					// Takes input in the textbox
void free_textbox(int n,...);		// Frees all allocated space for 'n' given textboxes

typedef struct optionbuton_struct
{
	Display *display;		// display
	Window win;			// parent windiow
	GC gc;				// gc
	int x;				// starting x
	int y;				// starting y
	int state;			// current flag state
	char *lebel;			// button lebel
}OptionButton;
OptionButton *create_optionbutton(Display *display,Window win,GC gc,int x,int y,int state,char *lebel);
					// Initializes a option-button structure and prints
int optionbutton_pressed(XEvent event,OptionButton *o);
					// Prints or removes tick on screen and returns 1 if option-button is pressed else returns 0
void free_optionbutton(int n,...);	// Frees all allocated space for 'n' given option-button


/* Function Definitions */
int key_pressed(Display *display,XEvent event,char *key)
{
	if((event.type==KeyPress || event.type==KeyRelease) && event.xkey.keycode==XKeysymToKeycode(display,XStringToKeysym(key))) return 1;
	else return 0;
}

int button_press_area(XEvent event,int x,int y,int w,int h)
{
	if(event.xbutton.x>=x && event.xbutton.x<=x+w && event.xbutton.y>=y && event.xbutton.y<=y+h) return 1;
	else return 0;
}

int get_filename(char *fpath,Display *display,Window win,GC gc,char *caption,char *buttonlebel)
{
	int flag=0,screen_num = DefaultScreen(display);
	Window parent_window=RootWindow(display,screen_num);
	Colormap cm=DefaultColormap(display,screen_num);
	XColor fore,back,button,shade,text,textbox;
	XAllocNamedColor(display,cm,"dark green",&fore,&fore);
	XAllocNamedColor(display,cm,"light green",&back,&back);
	XAllocNamedColor(display,cm,"green",&button,&button);
	XAllocNamedColor(display,cm,"gray20",&shade,&shade);
	XAllocNamedColor(display,cm,"black",&text,&text);
	XAllocNamedColor(display,cm,"white",&textbox,&textbox);

	Window subwin=XCreateSimpleWindow(display,win,100,100,400,200,2,back.pixel,fore.pixel);
	XMapWindow(display,subwin);
	
//	XFontStruct *font=XLoadQueryFont(display,"*-helvetica-*-18-*");
//	XSetFont(display,gc,font->fid);
	XSetForeground(display,gc,text.pixel);
	XDrawString(display,subwin,gc,10,20,caption,10);
	
	TextBox *t=create_textbox(display,subwin,gc,10,50,350,30,textbox,text,text,fpath);
	Button *ok=create_button(display,subwin,gc,50,100,100,30,button,shade,text,button,buttonlebel);
	Button *cancel=create_button(display,subwin,gc,200,100,100,30,button,shade,text,button,"Cancel");
	
	XEvent event;
	XSelectInput(display,subwin,KeyPressMask | ButtonPressMask | ButtonReleaseMask);
	while(1)
	{
		XNextEvent(display,&event);
		strcpy(fpath,textbox_input(event,t));
		if(button_pressed(event,cancel) || key_pressed(display,event,"Escape"))
			break;
		else if(button_pressed(event,ok) || key_pressed(display,event,"Return"))
		{
			flag=1;
			break;
		}
		else if(key_pressed(display,event,"Tab"))
		{
			file_starting_with(t->text);
			t->len=t->cur=strlen(t->text);
		}
	}
	XDestroyWindow(display,subwin);
	free_buttons(2,ok,cancel);
	free_textbox(1,t);
	return flag;
}

char *file_starting_with(char *fpath)
{
	char dirpath[1000];					// parent directory path
	char fname[256];					// file name
	strcpy(dirpath,fpath);
	int i=strlen(dirpath)-1;
	while(i>=0 & dirpath[i]!='/') dirpath[i--]='\0';
	strcpy(fname,fpath+i+1);
	if(strlen(fname)==0) return fpath;
	if(strlen(dirpath)==0) strcpy(dirpath,"./");
	DIR *dir;						// parent directory stream
	if((dir=opendir(dirpath))==NULL) return fpath;
	struct dirent *de;					// parent directory entry
	int len=strlen(fname);
	char c=0;
	char add[256]="\0";					// addition with given file name
	while(1)
	{
		if((de=readdir(dir))==NULL) break;
		if(strlen(de->d_name)>len && strncmp(fname,de->d_name,len)==0)
			if(strlen(add)==0) 
				strcpy(add,de->d_name+len);
			else 
			{
				for(i=0;add[i]!='\0';i++)
					if(add[i]!=(de->d_name+len)[i])
					{
						add[i]='\0';
						break;
					}
				if(strlen(add)==0) break;
			}
	}
	return strcat(fpath,add);
}

Button* create_button(Display *display,Window win,GC gc,int x,int y,int width,int height,XColor button,XColor shade,XColor text,XColor buttondown,char *lebel)
{
	Button *b=(Button*)malloc(sizeof(Button));
	b->display=display;
	b->win=win;
	b->gc=gc;
	b->x=x;
	b->y=y;
	b->height=height;
	b->width=width;
	b->lebel=(char*)calloc(strlen(lebel)+1,sizeof(char));
	strcpy(b->lebel,lebel);
	b->button=button;
	b->shade=shade;
	b->text=text;
	b->buttondown=buttondown;
		
	XSetForeground(display,gc,shade.pixel);
	XFillRectangle(display,win,gc,x+5,y+5,width,height);
	XSetForeground(display,gc,button.pixel);
	XFillRectangle(display,win,gc,x,y,width,height);
	XSetForeground(display,gc,text.pixel);
	XDrawString(display,win,gc,x+5,y+height/2+5,lebel,strlen(lebel));
	return b;
}

int button_pressed(XEvent event,Button *b)
{
	if(event.type==ButtonPress && event.xbutton.x>=b->x && event.xbutton.y>=b->y && event.xbutton.x<=b->x+b->width && event.xbutton.y<=b->y+b->height)
	{
		XSetForeground(b->display,b->gc,b->shade.pixel);
		XFillRectangle(b->display,b->win,b->gc,b->x,b->y,b->width,b->height);			
		XSetForeground(b->display,b->gc,b->buttondown.pixel);
		XFillRectangle(b->display,b->win,b->gc,b->x+5,b->y+5,b->width,b->height);			
		XSetForeground(b->display,b->gc,b->text.pixel);
		XDrawString(b->display,b->win,b->gc,b->x+5+5,b->y+b->height/2+5,b->lebel,strlen(b->lebel));
		XWindowEvent(b->display,b->win,ButtonReleaseMask,&event);
		XSetForeground(b->display,b->gc,b->shade.pixel);
		XFillRectangle(b->display,b->win,b->gc,b->x+5,b->y+5,b->width,b->height);			
		XSetForeground(b->display,b->gc,b->button.pixel);
		XFillRectangle(b->display,b->win,b->gc,b->x,b->y,b->width,b->height);			
		XSetForeground(b->display,b->gc,b->text.pixel);
		XDrawString(b->display,b->win,b->gc,b->x+5,b->y+b->height/2+5,b->lebel,strlen(b->lebel));
		if(event.xbutton.x>=b->x && event.xbutton.y>=b->y && event.xbutton.x<=b->x+b->width && event.xbutton.y<=b->y+b->height) return 1;
		else return 0;
	}
	else return 0;
}

void free_buttons(int n,...)			/* void free_buttons(int n,Button *b1,Button *b2,...); */
{
	va_list list;
	va_start(list,n);
	Button *b;
	while(n>0)
	{
		b=va_arg(list,Button*);
		free(b->lebel);			//SIG_ABRT
		free(b);
		n--;
	}
	va_end(list);
}

TextBox *create_textbox(Display *display,Window parent,GC gc,int x,int y,int width,int height,XColor boxc,XColor borderc,XColor textc,char *text)
{
	TextBox *t=(TextBox*)malloc(sizeof(TextBox));
	t->display=display;
	t->parent=parent;
	t->gc=gc;
	t->x=x;
	t->y=y;
	t->height=height;
	t->width=width;
	t->boxc=boxc;
	t->win=XCreateSimpleWindow(display,parent,x,y,width,height,2,borderc.pixel,boxc.pixel);
	XMapWindow(display,t->win);
	t->borderc=borderc;
	t->textc=textc;
	strcpy(t->text,text);
	t->cur=t->len=strlen(text);
	t->start=0;

	XSetForeground(display,gc,textc.pixel);
	char s[1000];
	int i=strlen(t->text);
	strcpy(s,t->text);
	s[i]='|';
	s[i+1]='\0';
	XDrawString(t->display,t->win,t->gc,5,20,s,strlen(s));
	return t;
}

char *textbox_input(XEvent event,TextBox *t)
{
	if(event.type==KeyPress)
	{
		if(key_pressed(t->display,event,"BackSpace"))
		{	
			if(t->cur>0)
			{
				memmove(t->text+t->cur-1,t->text+t->cur,1000-t->cur-1);
				t->len--;
				t->cur--;
			}
		}
		else if(key_pressed(t->display,event,"Left"))
		{
			if(t->cur>0)
				t->cur--;
		}
		else if(key_pressed(t->display,event,"Right"))
		{
			if(t->cur<t->len)
				t->cur++;
		}
		else if(key_pressed(t->display,event,"End"))
			t->cur=t->len;
		else if(key_pressed(t->display,event,"Home"))
			t->cur=0;
		else if(key_pressed(t->display,event,"Delete"))
		{
			if(t->cur<t->len)
			{
				memmove(t->text+t->cur,t->text+t->cur+1,1000-t->cur-1);
				t->len--;
			}
		}
		else if(t->cur<1000)
		{
			KeySym keysym;
			char c;
			XLookupString(&event.xkey,&c,1,&keysym,NULL);
			if(isprint(c))
			{
				memmove(t->text+t->cur+1,t->text+t->cur,1000-t->cur-1);
				t->text[t->cur++]=c;
				t->len++;
			}
		}
		XClearWindow(t->display,t->win);
		XSetForeground(t->display,t->gc,t->textc.pixel);
		char s[1000];
		int i=strlen(t->text);
		if(t->cur>t->width*7/50) t->start=t->cur-t->width*7/50;
		else t->start=0;
		sprintf(s,"%*.*s|%s",t->cur-t->start,t->cur-t->start,t->text+t->start,t->text+t->cur);
		XDrawString(t->display,t->win,t->gc,5,20,s,strlen(s));
	}
	return t->text;
}

void free_textbox(int n,...)
{
	va_list list;
	va_start(list,n);
	TextBox *t;
	while(n>0)
	{
		t=va_arg(list,TextBox*);
		free(t);
		n--;
	}
	va_end(list);
}

OptionButton *create_optionbutton(Display *display,Window win,GC gc,int x,int y,int state,char *lebel)
{
	OptionButton *o=(OptionButton*)malloc(sizeof(OptionButton));
	o->display=display;
	o->win=win;
	o->gc=gc;
	o->x=x;
	o->y=y;
	o->lebel=(char*)calloc(strlen(lebel)+1,sizeof(char*));
	strcpy(o->lebel,lebel);
	o->state=state;

	int scrno=DefaultScreen(display);
	XSetForeground(display,gc,WhitePixel(display,scrno));
	XFillRectangle(display,win,gc,x,y,15,15);
	XSetForeground(display,gc,BlackPixel(display,scrno));
	XDrawRectangle(display,win,gc,x,y,15,15);
	XDrawString(display,win,gc,x+25,y+10,lebel,strlen(lebel));	
	if(state==true)
	{
		Colormap cm=DefaultColormap(display,scrno);
		XColor red;
		XAllocNamedColor(display,cm,"red",&red,&red);
		XSetForeground(display,gc,red.pixel);
		XDrawLine(display,win,gc,x,y+7,x+5,y+15);
		XDrawLine(display,win,gc,x+5,y+15,x+15,y);
	}
	return o;
}

int optionbutton_pressed(XEvent event,OptionButton *o)
{
	if(event.type==ButtonPress && button_press_area(event,o->x,o->y,15,15))
	{
		o->state=!o->state;
		int scrno=DefaultScreen(o->display);
		XSetForeground(o->display,o->gc,WhitePixel(o->display,scrno));
		XFillRectangle(o->display,o->win,o->gc,o->x,o->y,15,15);
		XSetForeground(o->display,o->gc,BlackPixel(o->display,scrno));
		XDrawRectangle(o->display,o->win,o->gc,o->x,o->y,15,15);
		XDrawString(o->display,o->win,o->gc,o->x+25,o->y+10,o->lebel,strlen(o->lebel));	
		if(o->state==true)
		{
			Colormap cm=DefaultColormap(o->display,scrno);
			XColor red;
			XAllocNamedColor(o->display,cm,"red",&red,&red);
			XSetForeground(o->display,o->gc,red.pixel);
			XDrawLine(o->display,o->win,o->gc,o->x,o->y+7,o->x+5,o->y+15);
			XDrawLine(o->display,o->win,o->gc,o->x+5,o->y+15,o->x+15,o->y);
		}
		return 1;
	}
	return 0;
}

void free_optionbuttons(int n,...)
{
	va_list list;
	va_start(list,n);
	OptionButton *o;
	while(n>0)
	{
		o=va_arg(list,OptionButton*);
		free(o->lebel);
		free(o);
		n--;
	}
	va_end(list);
}

#endif
