/*
 * Copyright 1992 by Frank Adelstein.  All Rights Reserved.
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


/************/
/* INCLUDES */
/************/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xmu/WinUtil.h>

#include <X11/cursorfont.h>
#define XK_MISCELLANY  1
#define XK_LATIN1  1
#include <X11/keysymdef.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <signal.h>

/***********/
/* DEFINES */
/***********/

#define TITLE     "xlax"
#define CLASS     "Xlax"
#define MAXEVENTS 150

/*************/
/* types     */
/*************/

typedef struct {
  Window wind;
  int    active;
  XKeyEvent events[MAXEVENTS];
  int    eventindex;
  Widget button;
  Widget text;
} Windows_t;

/*************/
/* GLOBALS   */
/*************/

void SetupInterface();
void selectCB();
void findCB();
void killCB();
void keyboardCB();
void toggleCB();
void quitCB();
void pasteCB();
void senditCB();
void togglestringCB();
void stringinputCB();
void okCB();
void clearCB();
void cancelCB();
void Find_Xlax();
void Add_Window();
void presetString();
KeyCode CharToKeycodeMod();
int Remove_Window();
void BuildCharToKeyMap();

int kcmap[256];
int modmap[256];