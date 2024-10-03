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
 * Module: utility.h
 */

/*
 * Change History:
 *
 */

/*
 * Functions: BOOLEAN IsEof
 *            BOOLEAN IsComma
 *            BOOLEAN IsColon
 *            BOOLEAN IsSemiColon
 *            BOOLEAN IsWhiteSpace
 *            BOOLEAN SkipOptionalWhiteSpace
 *
 * Description:  This module defines the utility functions available in
 *               UTILITY.C.  These functions are used by multiple modules
 *               in LVM.EXE.
 *
 */

#ifndef LVM_CLI_UTILITY_H

#define LVM_CLI_UTILITY_H

#include "gbltypes.h"

extern BOOLEAN IsEof(LIST Tokens);
extern BOOLEAN IsComma(LIST Tokens);
extern BOOLEAN IsColon(LIST Tokens);
extern BOOLEAN IsSemiColon(LIST Tokens);
extern BOOLEAN IsOpenParen(LIST Tokens);
extern BOOLEAN IsCloseParen(LIST Tokens);
extern BOOLEAN IsOpenBracket(LIST Tokens);
extern BOOLEAN IsCloseBracket(LIST Tokens);
extern BOOLEAN IsOpenBrace(LIST Tokens);
extern BOOLEAN IsCloseBrace(LIST Tokens);
extern BOOLEAN IsWhitespace(LIST Tokens);
extern BOOLEAN SkipOptionalWhitespace(LIST Tokens);

#endif
