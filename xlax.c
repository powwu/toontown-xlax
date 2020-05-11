/*
 * Copyright � 1992 by Frank Adelstein.  All Rights Reserved.
 * Version 2.0 modifications Copyright � 2007 by Thomas A. Fine.
 *   All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the authors name not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/*
 * Neat program to send typed text to multiple windows.
 *
 * by Frank Adelstein
 *
 */

#include "xlax.h"
#include "vroot.h"

XtAppContext
  Appcon;

Widget
  Toplevel, Popup;

GC
  drawGC,
  xorGC;

Windows_t
  Windows[50];

XKeyEvent tmpevents[MAXEVENTS];
int tmpeventindex;
int tmpwindex;
  
int 
  WindowIndex;

char *prefix="xlax:";
char findname[16];

void DoNothing() {
  /* stupid, but used to override button click default actions */
  return;
}

int ErrorHandler(mydisp, myerr) 
Display *mydisp;
XErrorEvent *myerr;

{
  char msg[80];
  XGetErrorText(mydisp, myerr->error_code, msg, 80);
  (void) fprintf(stderr, "%s\n", msg);
  if (myerr->error_code == BadWindow) {
    fprintf(stderr, "Removing window %d\n", myerr->resourceid);
    Remove_Window(myerr->resourceid);
    return 0;
  } else {
    (void) fprintf(stderr, 
               "Fatal!!  We got errors: error [%d] request [%d] minor [%d]\n",
               myerr->error_code, myerr->request_code, myerr->minor_code); 
    exit(1); 
  } 
}

int main (argc, argv)
int    argc;
char **argv;

{
  int i, find=0;
  void DoNothing();
  static XtActionsRec TextActions[] = {
    {"DoNothing", DoNothing},
    {NULL, NULL}
  };

  /* xlax [-find] [-prefix name] */

  /* initialize toolkit */
  Toplevel = XtAppInitialize (&Appcon, TITLE, NULL, 0, &argc, argv,
			      NULL, NULL, 0);

  XtAppAddActions(Appcon, TextActions, XtNumber(TextActions));
  XSetErrorHandler(ErrorHandler);

  if (argc > 1) {
    for (i=1; i<argc; ++i) {
      if (strcmp(argv[i],"-find") == 0) {
	find=1;
      } else if (strcmp(argv[i],"-prefix") == 0) {
	if (i+1>=argc) {
	  fprintf(stderr,"usage: %s [-prefix string] [-find]\n",argv[0]);
	  exit(0);
	}
	prefix=argv[++i];
      } else {
	fprintf(stderr,"usage: %s [-prefix string] [-find]\n",argv[0]);
	exit(0);
      }
    }
  }

  /* do all the dirty work */
  SetupInterface(find);

  /* sit back and process events */
  XtAppMainLoop(Appcon);
  exit(0);
}

void
SetupInterface(dofind)
int dofind;
{
  int cnt;

  Arg args[15];

  Widget
    Frame,
    thing, find, quit, sendit, paste,
    view, draw,
    popfoo, poplabel, popstring, popclear, popcancel, popok;

  Display
    *display;

  XtTranslations text_trans;
  static String text_trans_str =
    "<Btn1Down>: DoNothing() \n\
     <Btn2Down>: DoNothing() \n\
     <Btn3Down>: DoNothing() \n\
     <Key>Return: insert-string(\"^M\") \n\
     <Key>BackSpace: insert-string(\"^H\") \n\
     <Key>Delete: insert-string(\"^?\")";

  /* create a frame widget to hold things */
  cnt = 0;
  Frame = XtCreateManagedWidget ("Frame", formWidgetClass, 
				 Toplevel, args, cnt);

  XtAddEventHandler (Frame, KeyPressMask, False, keyboardCB, NULL);
  XtAddEventHandler (Frame, KeyReleaseMask, False, keyboardCB, NULL); // Toontown patch

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNlabel, "Add Windows");            cnt++;

  thing = XtCreateManagedWidget ("addbutton", 
				 commandWidgetClass,
				 Frame, args, cnt);
  XtAddCallback (thing, XtNcallback, selectCB, NULL);

  strcpy(findname,"Find ");
  strncpy(findname+5,prefix,11); // Changed from strlcpy in toontown patch to provide greater compatibility
  if (strlen(prefix)<6) {
    for (cnt=5+strlen(prefix); cnt<11; ++cnt) {
      findname[cnt]=' ';
    }
    findname[cnt]='\0';
  }
  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNfromVert, thing);                  cnt++;
  XtSetArg (args[cnt], XtNlabel, findname);            cnt++;

  find = XtCreateManagedWidget ("findbutton", 
				 commandWidgetClass,
				 Frame, args, cnt);
  XtAddCallback (find, XtNcallback, findCB, NULL);

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNfromVert, find);                  cnt++;
  XtSetArg (args[cnt], XtNlabel, "Send String");            cnt++;

  sendit = XtCreateManagedWidget ("senditbutton", 
				  commandWidgetClass,
				  Frame, args, cnt);
  XtAddCallback (sendit, XtNcallback, senditCB, NULL);

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNlabel, "Paste");            cnt++;
  XtSetArg (args[cnt], XtNfromVert, sendit);                cnt++;

  paste = XtCreateManagedWidget ("pastebutton", 
				  commandWidgetClass,
				  Frame, args, cnt);
  XtAddCallback (paste, XtNcallback, pasteCB, NULL);

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNlabel, "Kill Window");            cnt++;
  XtSetArg (args[cnt], XtNfromVert, paste);                cnt++;

  thing = XtCreateManagedWidget ("killbutton", 
				 commandWidgetClass,
				 Frame, args, cnt);
  XtAddCallback (thing, XtNcallback, killCB, NULL);

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNfromVert, thing);                 cnt++;
  XtSetArg (args[cnt], XtNlabel, "Quit");                   cnt++;

  quit = XtCreateManagedWidget ("quitbutton", 
				 commandWidgetClass,
				 Frame, args, cnt);
  XtAddCallback (quit, XtNcallback, quitCB, NULL);

  /* create a viewport widget and a frame widget to hold things */
  cnt = 0;
  XtSetArg (args[cnt], XtNfromHoriz, find);                   cnt++;
  XtSetArg (args[cnt], XtNborder, 2);                         cnt++;
  XtSetArg(args[cnt], XtNwidth,  400);                        cnt++;
  XtSetArg(args[cnt], XtNheight,  150);                       cnt++;
  XtSetArg(args[cnt], XtNallowHoriz, True);  cnt++;
  XtSetArg(args[cnt], XtNallowVert,  True);  cnt++;
  view = XtCreateManagedWidget("View", viewportWidgetClass,
                               Frame, args, cnt);

  /* I don't understand why I need to do this, but...*/
  cnt = 0;
  XtSetArg(args[cnt], XtNwidth,  10);                         cnt++;
  XtSetArg(args[cnt], XtNheight, 10);                         cnt++;
  XtSetValues(XtNameToWidget(Toplevel, "*clip"), args, cnt);

  cnt = 0;

  XtSetArg (args[cnt], XtNborder, 2);                         cnt++;
  XtSetArg(args[cnt], XtNwidth,  200);                        cnt++;
  XtSetArg(args[cnt], XtNheight,  150);                       cnt++;
  draw = XtCreateWidget ("drawingarea", formWidgetClass,
			 view, args, cnt);

/* popup for entering sendstrings */

  cnt = 0;
  XtSetArg (args[cnt], XtNtitle, "Set send string");     cnt++;
  XtSetArg (args[cnt], XtNiconName, "Set send string");  cnt++;
  Popup = XtCreatePopupShell("sendstring", topLevelShellWidgetClass,
			     Toplevel, args, cnt);
  

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNwidth, 200);                      cnt++;
  XtSetArg(args[cnt], XtNheight,  150);                     cnt++;
  popfoo = XtCreateManagedWidget ("popfoo", formWidgetClass,
				  Popup, args, cnt);

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNborderWidth, 0);                  cnt++;
  XtSetArg (args[cnt], XtNwidth, 200);                      cnt++;
  poplabel = XtCreateManagedWidget ("poplabel", labelWidgetClass,
				  popfoo, args, cnt);

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNwidth, 200);                      cnt++;
  XtSetArg (args[cnt], XtNfromVert, poplabel);              cnt++;
  XtSetArg (args[cnt], XtNeditType, XawtextEdit);           cnt++;
  popstring = XtCreateManagedWidget ("sendtext", asciiTextWidgetClass,
				  popfoo, args, cnt);
  text_trans = XtParseTranslationTable(text_trans_str);
  XtOverrideTranslations(popstring, text_trans);
  XtAddEventHandler (popstring, KeyPressMask, False, stringinputCB, NULL);
  XtAddEventHandler (popstring, KeyReleaseMask, False, stringinputCB, NULL); // Toontown patch

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNfromVert, popstring);             cnt++;
  XtSetArg (args[cnt], XtNlabel, "Clear");                  cnt++;
  popclear = XtCreateManagedWidget ("clearbutton", commandWidgetClass,
				    popfoo, args, cnt);
  XtAddCallback (popclear, XtNcallback, clearCB, NULL);

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNfromVert, popstring);             cnt++;
  XtSetArg (args[cnt], XtNfromHoriz, popclear);             cnt++;
  XtSetArg (args[cnt], XtNlabel, "Cancel");                 cnt++;
  popcancel = XtCreateManagedWidget ("cancelbutton", commandWidgetClass,
				    popfoo, args, cnt);
  XtAddCallback (popcancel, XtNcallback, cancelCB, NULL);

  cnt = 0;
  XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
  XtSetArg (args[cnt], XtNfromVert, popstring);             cnt++;
  XtSetArg (args[cnt], XtNfromHoriz, popcancel);            cnt++;
  XtSetArg (args[cnt], XtNlabel, "OK");                     cnt++;
  popok = XtCreateManagedWidget ("okbutton", commandWidgetClass,
				    popfoo, args, cnt);
  XtAddCallback (popok, XtNcallback, okCB, NULL);

  XtRealizeWidget (Toplevel);
  XtRealizeWidget (Popup);

  /* set up keyboard mappings */
  display = XtDisplay(find);
  BuildCharToKeyMap(display);

  if (dofind) {
    findCB(find,NULL,NULL);
  }

  return;

}


/*
 * Routine to let user select a window using the mouse
 * (taken from dsimple.c from xwd)
 */

Window Select_Window(dpy)
     Display *dpy;
{
  int status;
  Cursor cursor;
  XEvent event;
  Window target_win = None, root = RootWindow(dpy,DefaultScreen(dpy));
  int buttons = 0;

  /* Make the target cursor */
  cursor = XCreateFontCursor(dpy, XC_crosshair);

  /* Grab the pointer using target cursor, letting it room all over */
  status = XGrabPointer(dpy, root, False,
                        ButtonPressMask|ButtonReleaseMask, GrabModeSync,
                        GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) {
    fprintf(stderr, "Can't grab the mouse.");
    exit (2);
  }

  /* Let the user select a window... */
  while ((target_win == None) || (buttons != 0)) {
    /* allow one more event */
    XAllowEvents(dpy, SyncPointer, CurrentTime);
    XWindowEvent(dpy, root, ButtonPressMask|ButtonReleaseMask, &event);
    switch (event.type) {
    case ButtonPress:
      if (target_win == None) {
        target_win = event.xbutton.subwindow; /* window selected */
        if (target_win == None)  {
	  fprintf(stderr, "target win = None\n");
	  target_win = root;
	}
      }
      buttons++;
      break;
    case ButtonRelease:
      if (buttons > 0) /* there may have been some down before we started */
        buttons--;
       break;
    }
  }

  XUngrabPointer(dpy, CurrentTime);      /* Done with pointer */

  return(target_win);
}

void findCB (w,client_data, call_data)
Widget w;
caddr_t  client_data;
caddr_t  call_data;
{
  Display *dpy = XtDisplay(w);
  Window root = RootWindow(dpy,DefaultScreen(dpy));

  int cnt;
  Arg args[15];
  
  cnt = 0;
  XtSetArg (args[cnt], XtNlabel, "Searching...");            cnt++;
  XtSetValues(w, args, cnt);

  Find_Xlax (dpy, root);

  cnt = 0;
  XtSetArg (args[cnt], XtNlabel, findname);            cnt++;
  XtSetValues(w, args, cnt);

}


void Find_Xlax (dpy, top)
Display *dpy;
Window top;
{
  Window *children, dummy;
  unsigned int nchildren;
  int i, x;
  Window w=0;
  XClassHint class_hint;

  if (XGetClassHint(dpy, top, &class_hint)) {
    if (strncmp(class_hint.res_name,prefix,strlen(prefix))==0) {
      /* found one, see if we have it yet.  If not, parse sendstring and add */
      for (x = 0; x < WindowIndex; x++) {
	  if (Windows[x].wind == top) {
	      break;
	  }
      }
      if (x>=WindowIndex) { /* not found, add it */
	Add_Window(dpy,top,class_hint.res_name);
	/* fprintf(stderr, "Adding window %d\n", top); */
      }
      /* and don't bother searching children? */
      return;
    }
  }

  if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
    return;
  for (i=0; i<nchildren; ++i) {
    Find_Xlax(dpy,children[i]);
  }
}

void
selectCB (w, client_data, call_data)
Widget w;
caddr_t  client_data;
caddr_t  call_data;

{
  Display *dpy;
  Window target;
  int x, y;

  int cnt;
  Arg args[15];
  
  cnt = 0;
  XtSetArg (args[cnt], XtNlabel, "Adding...");            cnt++;
  XtSetValues(w, args, cnt);
  do {
    XtAppProcessEvent (Appcon, XtIMAll);
  } while (XtAppPending(Appcon));


  dpy = XtDisplay(w);
  while (1) {
    /* keep selecting windows until the xlax window is selected */
    target = Select_Window(dpy);

    /* stupid undocumented kludge */
    target = XmuClientWindow (dpy, target);

    if (target == XmuClientWindow(dpy, XtWindow(Toplevel))) {
       /* we just clicked on ourself...let's avoid an endless loop */
       cnt = 0;
       XtSetArg (args[cnt], XtNlabel, "Add Windows");            cnt++;
       XtSetValues(w, args, cnt);
       return;
    }
    /* just return if we've already got that window yet */
    for (x = 0; x < WindowIndex; x++) {
        if (Windows[x].wind == target) {
            cnt = 0;
            XtSetArg (args[cnt], XtNlabel, "Add Windows");            cnt++;
            XtSetValues(w, args, cnt);
	    return;
	}
    }
    Add_Window(dpy,target,NULL);
  }  	/* end while (1) */  
}

void Add_Window(dpy,w,hint)
Display *dpy;
Window w;
char *hint;
{
  char *name;
  Widget frame;
  int cnt;
  Arg args[15];
  int x, y;

    XFlush(dpy);
    x = XFetchName(dpy, w, &name);

    /* add a toggle button */
    frame = XtNameToWidget (Toplevel, "*drawingarea");

    cnt = 0;
    XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
    XtSetArg (args[cnt], XtNlabel, name);                     cnt++;
    XtSetArg (args[cnt], XtNfromVert, 
	      (WindowIndex)?Windows[WindowIndex - 1].button:NULL);  cnt++;
    XtSetArg (args[cnt], XtNtop, XtChainTop);                 cnt++;
    XtSetArg (args[cnt], XtNbottom, XtChainTop);              cnt++;
    XtSetArg (args[cnt], XtNleft, XtChainLeft);               cnt++;
    XtSetArg (args[cnt], XtNright, XtChainLeft);              cnt++;
    XtSetArg (args[cnt], XtNwidth, 190);                      cnt++;
    XtSetArg (args[cnt], XtNheight, 20);                      cnt++;
    XtSetArg (args[cnt], XtNstate, True);                     cnt++;
    Windows[WindowIndex].button = XtCreateManagedWidget ("togglebutton", 
						         toggleWidgetClass,
						         frame, args, cnt);
    /* add a toggle button */
    frame = XtNameToWidget (Toplevel, "*drawingarea");

    cnt = 0;
    XtSetArg (args[cnt], XtNborder, 2);                       cnt++;
    XtSetArg (args[cnt], XtNlabel, "<click to set>"); cnt++;
    XtSetArg (args[cnt], XtNfromVert, 
	      (WindowIndex)?Windows[WindowIndex - 1].text:NULL);  cnt++;
    XtSetArg (args[cnt], XtNfromHoriz, 
	      Windows[WindowIndex].button);                   cnt++;
    XtSetArg (args[cnt], XtNtop, XtChainTop);                 cnt++;
    XtSetArg (args[cnt], XtNbottom, XtChainTop);              cnt++;
    XtSetArg (args[cnt], XtNleft, XtChainLeft);               cnt++;
    XtSetArg (args[cnt], XtNright, XtChainLeft);              cnt++;
    XtSetArg (args[cnt], XtNwidth, 190);                      cnt++;
    XtSetArg (args[cnt], XtNheight, 20);                      cnt++;
    Windows[WindowIndex].text = XtCreateManagedWidget ("toggletext", 
						       labelWidgetClass,
						       frame, args, cnt);

    XtAddCallback (Windows[WindowIndex].button, XtNcallback, 
		   toggleCB, (XtPointer *) WindowIndex);

    XtAddEventHandler (Windows[WindowIndex].text, ButtonPressMask,
		       False, togglestringCB, (XtPointer *) WindowIndex);

    XtUnmanageChild(frame);
    XtManageChild(frame);

    Windows[WindowIndex].wind = w;
    Windows[WindowIndex].eventindex = 0;
    if (hint) {
      /* only preset the string if it is not zero length */
      if (*(hint+strlen(prefix))) {
        presetString(dpy,WindowIndex,hint+strlen(prefix));
      }
    }
    Windows[WindowIndex++].active = 1;
}

int Remove_Window(w)
Window w;
{
  Widget  frame;
  int x, length, removed=0;

  Arg     args[10];
  int     cnt;

  /* check to see if window is in our list */
  for (x = 0; x < WindowIndex; x++) {
    if (Windows[x].wind == w) {
      /* remove it from our list then */
      XtDestroyWidget(Windows[x].button);
      XtDestroyWidget(Windows[x].text);
      frame = XtNameToWidget (Toplevel, "*drawingarea");
      if (x+1 < WindowIndex) {
        cnt = 0;
        XtSetArg(args[cnt], XtNfromVert, 
                 (x) ? Windows[x-1].button : NULL); cnt++;
        XtSetValues(Windows[x+1].button, args, cnt);
        cnt = 0;
        XtSetArg(args[cnt], XtNfromVert, 
                 (x) ? Windows[x-1].text : NULL);   cnt++;
        XtSetValues(Windows[x+1].text, args, cnt);

        length = sizeof(Windows_t) * (WindowIndex-x-1);
        bcopy(&Windows[x+1], &Windows[x], length);
      }
      WindowIndex--;
      XtUnmanageChild(frame);
      XtManageChild(frame);
      removed=1;
      break;
    }
  }
  return(removed);
}

void
killCB (w, client_data, call_data)
Widget w;
caddr_t  client_data;
caddr_t  call_data;

{
  Display *dpy;
  Window  target;
  Widget  frame;
  int     x, length;

  Arg     args[10];
  int     cnt;

  dpy = XtDisplay(w);
  /* get the selection */
  target = Select_Window(XtDisplay(w));

  /* check to see if it is us, in one of the client control toggles */
  /* doesn't work ...
  for (x = 0; x < WindowIndex; x++) {
fprintf(stderr,"t=%d, butt=%d, text=%d\n", target,
	       XtWindow(Windows[x].button), XtWindow(Windows[x].text));
    if (target == XtWindow(Windows[x].button)) {
      Remove_Window(Windows[x].wind);
      return;
    }
    if (target == XtWindow(Windows[x].text)) {
      Remove_Window(Windows[x].wind);
      return;
    }
  }
  */

  /* tell me what that really means */
  target = XmuClientWindow(dpy, target);

  if (target == XmuClientWindow(dpy, XtWindow(Toplevel))) {
    /* it's us.. nothing else to do */
    return;
  } 

  /* otherwise see if that window is in our list */
  Remove_Window(target);
}

void
keyboardCB (w, client_data, event)
Widget w;
caddr_t  client_data;
XKeyEvent *event;

{
  int x;

  /* send the keys to every active window that's been selected */
  for (x = 0; x < WindowIndex; x++)
    {
      if (Windows[x].active == 1)
	{
	  event->window = Windows[x].wind;
	  XSendEvent(XtDisplay(w), Windows[x].wind, True, KeyPressMask, (XEvent *) event);
    XSendEvent(XtDisplay(w), Windows[x].wind, True, KeyReleaseMask, (XEvent *) event); // Toontown patch
	}
    }
  return;
}

void presetString (dpy,wi,str)
Display *dpy;
int wi;
char *str;
{
  int i;
  long m;
  Window root = RootWindow(dpy,DefaultScreen(dpy));

  Arg     args[10];
  int     cnt;

  if (strlen(str) >= MAXEVENTS) {
    fprintf(stderr,"Warning: preset string %s will be truncated!\n",str);
  }
  for (i=0; i<strlen(str)&&i<MAXEVENTS; ++i) {
    /* skip if we don't have a keycode mapping for this character */
    if (kcmap[str[i]] < 0) {
      fprintf(stderr,"No key mapping for %c!\n", str[i]);
      continue;
    }
    Windows[wi].events[i].type=2;
    Windows[wi].events[i].serial=1;
    Windows[wi].events[i].send_event=0;
    Windows[wi].events[i].display=dpy;
    Windows[wi].events[i].window=Windows[wi].wind;
    Windows[wi].events[i].root=root;
    Windows[wi].events[i].subwindow=0;
    Windows[wi].events[i].x=0;
    Windows[wi].events[i].y=0;
    Windows[wi].events[i].x_root=0;
    Windows[wi].events[i].y_root=0;
    Windows[wi].events[i].same_screen=1;
    Windows[wi].events[i].keycode=kcmap[str[i]];
    Windows[wi].events[i].state=modmap[str[i]];
  }
  Windows[wi].eventindex=i;
  cnt = 0;
  XtSetArg(args[cnt], XtNlabel, str);    cnt++;
  XtSetValues(Windows[wi].text, args, cnt);
}

void BuildCharToKeyMap (disp)
Display *disp;
{
  int mincode, maxcode;
  int i, m, len;
  XKeyEvent evt;
  char out[32];
  int states[4] = { 0, ShiftMask, ControlMask, ShiftMask|ControlMask };

  evt.type=2;
  evt.serial=1;
  evt.send_event=0;
  evt.display=disp;
  evt.window=0;
  evt.root=0;
  evt.subwindow=0;
  evt.x=0;
  evt.y=0;
  evt.x_root=0;
  evt.y_root=0;
  evt.same_screen=1;

  XDisplayKeycodes(disp, &mincode, &maxcode);

  for (i=0; i<256; ++i) {
    kcmap[i]=-1;
    modmap[i]=-1;
  }

  for (i=mincode; i<maxcode; ++i) {
    evt.keycode=i;
    for (m=3; m>=0; --m) {
      evt.state=states[m];
      len=XLookupString(&evt, out, 32, NULL, NULL);
      if (len == 1) {
        kcmap[out[0]] = i;
        modmap[out[0]] = states[m];
      }
    }
  }
}

KeyCode CharToKeycodeMod(disp, c, m)
Display *disp;
unsigned char c;
long *m;
{
  KeySym ks, ksr;
  KeyCode kc;
  XKeyEvent evt;
  long mr, len;
  char str[2], out[32];

  str[0]=c;
  str[1]='\0';

  evt.type=2;
  evt.serial=1;
  evt.send_event=0;
  evt.display=disp;
  /* the next two values are not "correct" but seem to work */
  evt.window=0;
  evt.root=0;
  evt.subwindow=0;
  evt.x=0;
  evt.y=0;
  evt.x_root=0;
  evt.y_root=0;
  evt.same_screen=1;

  if (c < 32) {
    str[0]+=64;
    /* Assume keysym is same as character.  This seems to be true, but
       it seems like the wrong approach.  It does work better than the
       older broken method */
    ks=c;
    kc=XKeysymToKeycode(disp,ks);
    evt.keycode=kc;
    evt.state=0;
    len=XLookupString(&evt, out, 32, NULL, NULL);
    if (len == 1 && out[0] == str[0]) {
      *m = ControlMask;
      return(kc);
    }
    evt.keycode=kc;
    evt.state=ShiftMask;
    len=XLookupString(&evt, out, 32, NULL, NULL);
    if (len == 1 && out[0] == str[0]) {
      *m = ShiftMask | ControlMask;
      return(kc);
    }
    fprintf(stderr, "Didn't resolve keycode! (c=%d, ks=%d,kc=%d\n",c,ks,kc);
    *m = ControlMask;
    return(0);
  } else if (c>127) {
    fprintf(stderr, "Didn't resolve keycode! (c=%d, ks=%d,kc=%d\n",c,ks,kc);
    *m = 0;
    return(0);
  } else {
    ks=c;
    kc=XKeysymToKeycode(disp,ks);
    evt.keycode=kc;
    evt.state=0;
    len=XLookupString(&evt, out, 32, NULL, NULL);
    if (len == 1 && out[0] == c) {
      *m = 0;
      return(kc);
    }
    evt.keycode=kc;
    evt.state=ShiftMask;
    len=XLookupString(&evt, out, 32, NULL, NULL);
    if (len == 1 && out[0] == c) {
      *m = ShiftMask;
      return(kc);
    }
    fprintf(stderr, "Didn't resolve keycode! (c=%d, ks=%d,kc=%d\n",c,ks,kc);
    *m = 0;
    return(0);
  }
}

KeyCode _old_CharToKeycodeMod(disp, c, m)
Display *disp;
unsigned char c;
long *m;
{
  KeySym ks, ksr;
  KeyCode kc;
  XKeyEvent evt;
  long mr, len;
  char str[2], out[32];

  str[0]=c;
  str[1]='\0';

  evt.type=2;
  evt.serial=1;
  evt.send_event=0;
  evt.display=disp;
  /* the next two values are not "correct" but seem to work */
  evt.window=0;
  evt.root=0;
  evt.subwindow=0;
  evt.x=0;
  evt.y=0;
  evt.x_root=0;
  evt.y_root=0;
  evt.same_screen=1;

  if (c < 32) {
    str[0]+=64;
    /* This was wrong.  This function converts from the string keysym
       name, not the character string.  So "A" works because the name
       and character are the same, but ":" doesn't, because the name
       is "colon".  XStringToKeysym can only look up "colon" */
    ks=XStringToKeysym(str);
    kc=XKeysymToKeycode(disp,ks);
    evt.keycode=kc;
    evt.state=0;
    len=XLookupString(&evt, out, 32, NULL, NULL);
    if (len == 1 && out[0] == str[0]) {
      *m = ControlMask;
      return(kc);
    }
    evt.keycode=kc;
    evt.state=ShiftMask;
    len=XLookupString(&evt, out, 32, NULL, NULL);
    if (len == 1 && out[0] == str[0]) {
      *m = ShiftMask | ControlMask;
      return(kc);
    }
    fprintf(stderr, "Didn't resolve keycode! (c=%d, ks=%d,kc=%d\n",c,ks,kc);
    *m = ControlMask;
    return(0);
  } else {
    ks=XStringToKeysym(str);
    kc=XKeysymToKeycode(disp,ks);
    evt.keycode=kc;
    evt.state=0;
    len=XLookupString(&evt, out, 32, NULL, NULL);
    if (len == 1 && out[0] == c) {
      *m = 0;
      return(kc);
    }
    evt.keycode=kc;
    evt.state=ShiftMask;
    len=XLookupString(&evt, out, 32, NULL, NULL);
    if (len == 1 && out[0] == c) {
      *m = ShiftMask;
      return(kc);
    }
    fprintf(stderr, "Didn't resolve keycode! (c=%d, ks=%d,kc=%d\n",c,ks,kc);
    *m = 0;
    return(0);
  }
}

void
cancelCB (w, client_data, call_data)
Widget w;
caddr_t  client_data;
caddr_t  call_data;
{
  XtSetKeyboardFocus(Popup,None);
  XtPopdown(Popup);
}

void
okCB (w, client_data, call_data)
Widget w;
caddr_t  client_data;
caddr_t  call_data;
{
  int i, len, offset;
  char text[500];
  Arg     args[10];
  int     cnt;

  bcopy (tmpevents, Windows[tmpwindex].events, tmpeventindex * (sizeof (XKeyEvent)));
  Windows[tmpwindex].eventindex=tmpeventindex;

  offset=0;
  for (i=0; i<Windows[tmpwindex].eventindex; ++i) {
    len=XLookupString(&Windows[tmpwindex].events[i],text+offset,500,NULL,NULL);
    offset += len;
    text[offset]='\0';
  }
  cnt=0;
  XtSetArg(args[cnt], XtNlabel, text);   cnt++;
  XtSetValues(Windows[tmpwindex].text,args,cnt);
  XtSetKeyboardFocus(Popup,None);
  XtPopdown(Popup);
}

void
clearCB (w, client_data, call_data)
Widget w;
caddr_t  client_data;
caddr_t  call_data;
{
  Arg args[10];
  int cnt;
  Widget tmpw;

  tmpw = XtNameToWidget (Toplevel, "*sendtext");

  tmpeventindex=0;
  cnt=0;
  XtSetArg(args[cnt], XtNstring, "");   cnt++;
  XtSetValues(tmpw,args,cnt);
}


void
stringinputCB (w, client_data, event)
Widget w;
caddr_t  client_data;
XKeyEvent *event;

{
  if (tmpeventindex == MAXEVENTS) {
    /* if we've reached the max, just beep and return */
    XBell(XtDisplay(w), 0);
    return;
  }

  /* save each key event for that window and bump up the counter */
  bcopy (event, &tmpevents[tmpeventindex], sizeof (XKeyEvent));

  tmpevents[tmpeventindex].window = Windows[tmpwindex].wind;
  tmpeventindex++;
  
}

void
togglestringCB (w, index, event)
Widget w;
int  index;
XButtonEvent *event;

{
  Arg args[10];
  int cnt, i, len, offset=0;
  char text[500];
  Widget tmpw, lab;

  tmpwindex=index;
  tmpw = XtNameToWidget (Toplevel, "*sendtext");
  lab = XtNameToWidget (Toplevel, "*poplabel");

  cnt=0;
  XtSetArg(args[cnt], XtNstring, "");   cnt++;
  XtSetValues(tmpw,args,cnt);

  strcpy(text, "Replacing: ");
  offset=strlen(text);
  for (i=0; i<Windows[index].eventindex; ++i) {
    len=XLookupString(&Windows[index].events[i],text+offset,500,NULL,NULL);
    offset += len;
    text[offset]='\0';
  }
  cnt=0;
  XtSetArg(args[cnt], XtNlabel, text);   cnt++;
  XtSetValues(lab,args,cnt);
  
  tmpeventindex=0;
  XtPopup(Popup, XtGrabExclusive);
  XtSetKeyboardFocus(Popup,tmpw);
  return;
}

void
toggleCB (w, windex, state)
Widget w;
int  windex;
int  state;

{
  Windows[windex].active = state;
  return;
}

void 
pasteCB (w, client_data, call_data)
Widget w;		/* unused */
caddr_t  client_data;	/* unused */
caddr_t  call_data;	/* unused */

{
  int x, y, n;
  char *sel;
  XKeyEvent evt;
  long m;
  Display *dpy = XtDisplay(w);
  Window root = RootWindow(dpy,DefaultScreen(dpy));

  sel = XFetchBuffer(dpy, &n, 0);
  for (x = 0; x < WindowIndex; x++) {
    if (Windows[x].active == 1) {
      for (y = 0; y < strlen(sel); y++) {
	/* skip if we don't have a keycode mapping for this character */
	if (kcmap[sel[y]] < 0) {
	  fprintf(stderr,"No key mapping for %c!\n", sel[y]);
	  continue;
	}
	evt.type=2;
	evt.serial=1;
	evt.send_event=0;
	evt.display=dpy;
	evt.window=Windows[x].wind;
	evt.root=root;
	evt.subwindow=0;
	evt.x=0;
	evt.y=0;
	evt.x_root=0;
	evt.y_root=0;
	evt.same_screen=1;
	evt.keycode=kcmap[sel[y]];
	evt.state=modmap[sel[y]];

	XSendEvent(dpy, Windows[x].wind, True, 
		   KeyPressMask, (XEvent *) &evt);
  
  XSendEvent(dpy, Windows[x].wind, True, // Toontown patch
		   KeyReleaseMask, (XEvent *) &evt); // Toontown patch
  
/*
	fprintf(stderr,"Sending event %d:\n",y);
	fprintf(stderr,"  Type    %d:\n",Windows[x].events[y].type);
	fprintf(stderr,"  Serial  %d:\n",Windows[x].events[y].serial);
	fprintf(stderr,"  SendEvt %d:\n",Windows[x].events[y].send_event);
	fprintf(stderr,"  Display %d:\n",(int)Windows[x].events[y].display);
	fprintf(stderr,"  Window  %d:\n",Windows[x].events[y].window);
	fprintf(stderr,"  Root    %d:\n",Windows[x].events[y].root);
	fprintf(stderr,"  Subwin  %d:\n",Windows[x].events[y].subwindow);
	fprintf(stderr,"  x       %d:\n",Windows[x].events[y].x);
	fprintf(stderr,"  y       %d:\n",Windows[x].events[y].y);
	fprintf(stderr,"  x_root  %d:\n",Windows[x].events[y].x_root);
	fprintf(stderr,"  y_root  %d:\n",Windows[x].events[y].y_root);
	fprintf(stderr,"  state   %d:\n",Windows[x].events[y].state);
	fprintf(stderr,"  keycode %d:\n",Windows[x].events[y].keycode);
	fprintf(stderr,"  samescr %d:\n",Windows[x].events[y].same_screen);
*/
      }
    }
  }
  return;
}

void 
senditCB (w, client_data, call_data)
Widget w;		/* unused */
caddr_t  client_data;	/* unused */
caddr_t  call_data;	/* unused */

{
  int x, y;

  for (x = 0; x < WindowIndex; x++) {
    if (Windows[x].active == 1) {
      for (y = 0; y < Windows[x].eventindex; y++) {
	XSendEvent(XtDisplay(w), Windows[x].wind, True, 
		   KeyPressMask, (XEvent *) &Windows[x].events[y]);
  
	XSendEvent(XtDisplay(w), Windows[x].wind, True, // Toontown patch
		   KeyReleaseMask, (XEvent *) &Windows[x].events[y]); // Toontown patch
  
      }
    }
  }
  return;
}

void 
quitCB (w, client_data, call_data)
Widget w;		/* unused */
caddr_t  client_data;	/* unused */
caddr_t  call_data;	/* unused */

{
  exit (0);
}
