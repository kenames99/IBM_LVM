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
 * Module: scanner.h
 */

/*
 * Change History:
 *
 */

/*
 * Functions: LIST * ScanCommandLine
 *
 * Description: Defines the interface to the scanner.  The scanner takes
 *              the command line passed to it and breaks it into tokens.
 *              These tokens are then appended to a linked list of tokens
 *              created by the scanner.  When the entire command line has
 *              been processed, the linked list is returned to the caller.
 *
 * Notes: The basic Token structure used by the scanner, screener, and parser
 *        is defined here, as well as the MaxIdentifierLength, which limits
 *        the size of identifiers.
 *
 */

/* This module defines the interface to the scanner, which is the front end of
   the syntactic analyzer used by the LVM command.  The scanner is an FSA whose
   states and actions are as indicated in the table below:

                                                                Token Type
   State   Characters that trigger a           State to         Output To
   Name           Transition                   Transition to    Screener
   ----------------------------------------------------------------------------

   Start  --
          ' '                               -> SingleSpace
          '\t'                              -> SingleTab
          ',',':','+',';','(',')','[',']',
               ,'{','}'                     -> AcceptSeparator
          '0' .. '9'                        -> IsNumber
          'A' .. 'Z', 'a' ..'z'             -> IsString
          EOF                               -> EndState
                                            -> Error;

   IsString --
          'A' .. 'Z', 'a' ..'z', '0' .. '9' -> IsString
                                            -> AcceptString;

   IsNumber --
          '0' .. '9'                        -> IsNumber
                                            -> AcceptNumber;

   SingleSpace --
          ' '                               -> IsMultiSpace
                                            -> AcceptSingleSpace;

   IsMultiSpace --
          ' '                               -> IsMultiSpace
                                            -> AcceptMultiSpace;

   IsMultiTab --
          ' '                               -> IsMultiTab
                                            -> AcceptMultiTab;

   AcceptSingleSpace --
                                            -> Start            => "Space";

   AcceptMultiSpace --
                                            -> Start            => "MultiSpace";

   AcceptSingleTab --
                                            -> Start            => "Tab";

   AcceptMultiTab --
                                            -> Start            => "MultiTab";

   AcceptSeparator --
                                            -> Start            => "Separator";

   AcceptString --
                                            -> Start            => "String";

   AcceptNumber --
                                            -> Start            => "Number";

   EndState --
                                                                => "EOF"

   ----------------------------------------------------------------------------

   The scanner maintains a buffer.  Each time a character is used in a
   transition, it is placed into the buffer.  The buffer is cleared each
   time a transition to the Start state is made.  When the scanner reaches
   a state where it outputs a value (as indicated in the table), the output
   consists of two parts: the contents of the buffer, and a characterization
   of the contents of the buffer.  In the table above, only the characterization
   is shown in the output column.  In those cases where output occurs on a
   transition to the start state, the output takes place before the transition
   to the start state.  Each of the items "output" by the scanner is appended
   to a linked list, which is returned to the caller when scanning has been
   completed.  Thus, the scanner returns a linked list of tokens.                 */


#ifndef LVM_CLI_SCANNER_H

#define LVM_CLI_SCANNER_H 1

#include "list.h"


/* This defines the maximum length that any identifier may be.  Longer
   identifiers will be truncated to this length */
#define MaxIdentifierLength 32

/* Character value of the LVM command line separtors */
#define  SemiColonChar      ';'
#define  ColonChar          ':'
#define  CommaChar          ','
#define  SpaceChar          ' '
#define  TabChar            '\t'
#define  OpenParenChar      '('
#define  CloseParenChar     ')'
#define  OpenBracketChar    '['
#define  CloseBracketChar   ']'
#define  OpenBraceChar      '{'
#define  CloseBraceChar     '}'
#define  EqualSignChar      '='
#define  QuoteChar          '"'

/* String value of the LVM reserved words, in uppercase */
#define  InstallStr        "/INSTALL"
#define  AllStr            "ALL"
#define  BestFitStr        "BESTFIT"
#define  BootDOSStr        "BOOTDOS"
#define  BootOS2Str        "BOOTOS2"
#define  BootableStr       "BOOTABLE"
#define  BootmgrStr        "/BOOTMGR"
#define  CRIStr            "CRI"
#define  CRStr             "CR"
#define  CompatibilityStr  "COMPATIBILITY"
#define  CreateStr         "/CREATE"
#define  DeleteStr         "/DELETE"
#define  DriveLetterStr    "/DRIVELETTER"
#define  DriveStr          "DRIVE"
#define  ExistingStr       "EXISTING"
#define  ExpandStr         "/EXPAND"
#define  FSStr             "FS"
#define  FileStr           "/FILE"
#define  FirstFitStr       "FIRSTFIT"
#define  FreespaceStr      "FREESPACE"
#define  FromEndStr        "FROMEND"
#define  FromLargestStr    "FROMLARGEST"
#define  FromSmallestStr   "FROMSMALLEST"
#define  FromStartStr      "FROMSTART"
#define  HideStr           "/HIDE"
#define  LVMStr            "LVM"
#define  LastFitStr        "LASTFIT"
#define  LogicalStr        "LOGICAL"
#define  NewMBRStr         "/NEWMBR"
#define  NewStr            "NEW"
#define  NoBootStr         "NOBOOT"
#define  NonBootableStr    "NONBOOTABLE"
#define  NotBootableStr    "NOTBOOTABLE"
#define  PartitionStr      "PARTITION"
#define  PrimaryStr        "PRIMARY"
#define  QueryStr          "/QUERY"
#define  RBStr             "RB"
#define  RediscoverPRMStr  "/REDISCOVERPRM"
#define  SIStr             "/SI"
#define  SetNameStr        "/SETNAME"
#define  SetStartableStr   "/SETSTARTABLE"
#define  SizeStr           "SIZE"
#define  SlashSizeStr      "/SIZE"
#define  StartLogStr       "/STARTLOG"
#define  UnusableStr       "UNUSABLE"
#define  UnusedStr         "UNUSED"
#define  VolumeStr         "VOLUME"
#define  VolumesStr        "VOLUMES"

/* Forward slash needs to be treated specially when */
/* it is embedded in a string that starts with a    */
/* special install option (i.e. FS, SIZE, or CRI)   */
/* and is followed by "size" */
#define  ForwardSlashChar  '/'


/*********************************************************************/
/*                                                                   */
/*   Function Name: ScanCommandLine                                  */
/*                                                                   */
/*   Descriptive Name: Generates a list of tokens derived from the   */
/*                     string passed to it.                          */
/*                                                                   */
/*   Input: pSTRING CommandLine - The string to be scanned and       */
/*                               tokenized.                          */
/*                                                                   */
/*   Output: If Success : The function return value will be a        */
/*                        pointer to a LIST containing the tokens    */
/*                        generated.                                 */
/*                                                                   */
/*           If Failure : The function return value will be NULL     */
/*                        and an error message will be displayed to  */
/*                        the user using the ReportError function    */
/*                        described in ERROR.H.                      */
/*                                                                   */
/*   Error Handling: If an error occurs, all memory allocated by this*/
/*                   function is freed, an error message is displayed*/
/*                   using ReportError, and the function return value*/
/*                   will be NULL.                                   */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
extern LIST ScanCommandLine(pSTRING pCommandLine);


/*********************************************************************/
/*                                                                   */
/*   Function Name: DeleteTokenList                                  */
/*                                                                   */
/*   Descriptive Name: Deletes the list of tokens created by the     */
/*                     ScanCommandLine function.  This function must */
/*                     be used to eliminate the list or memory leaks */
/*                     will result.                                  */
/*                                                                   */
/*   Input: LIST * TokenList - The list of tokens to eliminate.      */
/*                                                                   */
/*   Output: If Success : The function return value will be TRUE.    */
/*                        *TokenList will be NULL.                   */
/*                                                                   */
/*           If Failure : The function return value will be FALSE.   */
/*                        *TokenList will be unchanged.              */
/*                                                                   */
/*   Error Handling: If an error occurs and is detected, processing  */
/*                   will continue as far as possible, i.e. as much  */
/*                   of the list will be freed as is possible.       */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function is intended to be called as part of any    */
/*          exit paths in the program.                               */
/*                                                                   */
/*********************************************************************/
extern BOOLEAN DeleteTokenList( LIST * pTokenList );

#endif

