/****************************************************************************
*                                                                           *
* about_window.h -- Lunapaint,                                              *
*    http://developer.berlios.de/projects/lunapaintami/                     *
* Copyright (C) 2006, 2007, Hogne Titlestad <hogga@sub-ether.org>           *
* Copyright (C) 2009-2011 LunaPaint Development Team                        *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License for more details.                              *
*                                                                           *
* You should have received a copy of the GNU General Public License         *
* along with this program; if not, write to the Free Software Foundation,   *
* Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.            *
*                                                                           *
****************************************************************************/

#ifndef _about_window_h_
#define _about_window_h_

#include <stdio.h>
#include <stdlib.h>

#ifndef __AROS__
#include "aros/aros.h"
#endif

#include <exec/types.h>
#include <libraries/mui.h>
#include <libraries/asl.h>
#include <cybergraphx/cybergraphics.h>

#include <dos/filehandler.h>

#include <proto/asl.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/muimaster.h>
#include <proto/cybergraphics.h>
#include <clib/alib_protos.h>
#include <string.h>

#include "locale.h"

#include "config.h"
#include "parts.h"

extern Object *aboutWindow, *aboutWindowOk;

void Init_AboutWindow ( );


#endif
