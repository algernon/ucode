/* 
* ucode — a daemon-like implementation of Ctrl+Shift+U-based
*  Unicode character input in X
*
* Copyright (c) 2014, Ruslan Kabatsayev <b7.10110111@gmail.com>
* 
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 3.0 of the License, or (at your option) any later version.
* 
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
* 
* You should have received a copy of the GNU Lesser General Public
* License along with this library.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iconv.h>
#include <xdo.h>

xdo_t* xdo=0;
Display* dpy=0;

void type(const uint32_t charcode)
{
    char string[]={(char)(charcode&0xff),(char)((charcode>>8)&0xff),(char)((charcode>>16)&0xff),(char)((charcode>>24)&0xff)};
    char utf8string[16]={0};
    size_t inbytes=sizeof string;
    size_t outbytes=sizeof utf8string;
    char* ins=string;
    char* outs=utf8string;
    iconv_t cd=iconv_open("utf8","utf32");
    if(iconv(cd,&ins,&inbytes,&outs,&outbytes)==-1)
        perror("iconv");
    else
        xdo_type(xdo,CURRENTWINDOW,utf8string,12000);
}

void enterUnicode()
{
    XGrabKeyboard(dpy, DefaultRootWindow(dpy), False, GrabModeAsync, GrabModeAsync,CurrentTime);

    XEvent ev;
    Bool controlPressed=True, shiftPressed=True;
    uint32_t charcode=0;
    int i=0;
    while(True)
    {
        XNextEvent(dpy, &ev);
        if(ev.type == KeyPress)
        {
            KeySym key=XLookupKeysym(&ev.xkey,0);
            if(key==XK_Escape)
            {
                XUngrabKeyboard(dpy, CurrentTime);
                return;
            }
            if(i==8)
                continue;
            if(XK_0 <= key && key <= XK_9)
            {
                charcode<<=4;
                charcode+=key-XK_0;
                ++i;
            }
            else if(XK_A <= key && key <= XK_F)
            {
                charcode<<=4;
                charcode+=key-XK_A+10;
                ++i;
            }
            else if(XK_a <= key && key <= XK_f)
            {
                charcode<<=4;
                charcode+=key-XK_a+10;
                ++i;
            }
        }
        else if(ev.type == KeyRelease)
        {
            KeySym key=XLookupKeysym(&ev.xkey,0);
            if(key==XK_Shift_L||key==XK_Shift_R)
                shiftPressed=False;
            if(key==XK_Control_L||key==XK_Control_R)
                controlPressed=False;
        }
        if(!(controlPressed||shiftPressed))
            break;
    }
    XUngrabKeyboard(dpy, CurrentTime);
    XSync(dpy,False);

    type(charcode);
}

int main(void)
{
    dpy = XOpenDisplay(0);

    if(!dpy)
    {
        fprintf(stderr,"Couldn't open X display\n");
        return 1;
    }

    xdo=xdo_new_with_opened_display(dpy,NULL,False);
    if(!xdo)
    {
        fprintf(stderr,"Couldn't initialize libxdo\n");
        return 1;
    }

    int grabResult=0;
    grabResult=XGrabKey(dpy,
                           XKeysymToKeycode(dpy,XK_U),
                           ControlMask | ShiftMask,
                           DefaultRootWindow(dpy),
                           False,
                           GrabModeAsync,
                           GrabModeAsync);
    grabResult=XGrabKey(dpy,
                           XKeysymToKeycode(dpy,XK_U),
                           ControlMask | ShiftMask | Mod2Mask,
                           DefaultRootWindow(dpy),
                           False,
                           GrabModeAsync,
                           GrabModeAsync);
    if(!grabResult)
    {
        fprintf(stderr,"Failed to grab Ctrl+Shift+U: %d\n",grabResult);
        return 1;
    }

    XSelectInput(dpy, DefaultRootWindow(dpy), KeyPressMask);

    XEvent ev;
    while(True)
    {
        XNextEvent(dpy, &ev);
        if(ev.type == KeyPress)
            enterUnicode();
    }
}
