/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Module: getkey.h
 */

/*
 * Change History:
 *
 */


#define TABCHAR         9
#define CTRL_Z          26
#define ESCAPE          27
#define SPACE           32
#define ENTER           '\r'


/*     key code definitions */

#define RIGHT_CURSOR    256
#define LEFT_CURSOR     257
#define UP_CURSOR       258
#define DOWN_CURSOR     259

#define LINE_BEGINNING  260
#define LINE_END        261
#define UPPAGE          262
#define DOWNPAGE        263
#define TOP_OF_FILE     264
#define END_OF_FILE     265
#define INS_CHAR        266
#define DEL_CHAR        267

#define HELP            269
#define F1              270
#define F2              271
#define F3              272
#define F4              273
#define F5              274
#define F6              275
#define F7              276
#define F8              277
#define F9              278
#define F10             279
#define F11             280
#define F12             281

#define UNASSIGNED_KEY   -1


/*
 * Special keys are either 00 or E0 followed by the scan code of the key
 * on the next getch. DBCS first char can be E0 also, but with a scan code of
 * 00, which is not a special key. So E0,00 means that this is the first
 * DBCS char and the routine returns the E0 and discards the 00. The next
 * DBCS char is returned normally on the next GetKeystroke.
 *
 */

PUBLIC
int
GetKeystroke ( void );
