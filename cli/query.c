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
 * Module: query.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: BOOLEAN Query_Parameters
 *
 *
 * Description:  This module implements the parser used by the LVM.EXE
 *               program.
 *
 * Notes: The basic Token structure used by the scanner, screener, and parser
 *        is defined in SCANNER.H, as well as the MaxIdentifierLength, which
 *        limits the size of identifiers.
 *
 *        This module is single threaded and assumes a single threaded
 *        program!
 *
 *        The parser is a standard recursive descent parser.
 *
 */

/* This module implements the parser, the "back end" of the syntactic analyzer
   used to parse the command line.  It accepts the following grammar:

The above grammar may be interpreted as follows:

1.  A non-terminal is a word not enclosed in either single or double quote marks.

2.  Words enclosed in double quote marks are words in the language.

3.  The grammar is divided into sections.  Each section begins with a non-terminal.
    The non-terminal for a section starts at the left most column on the line and
    is on a line by itself.  All the production rules for that non-terminal are
    listed on the lines below and are indented.  The last production rule in a
    section will end with a semi-colon.

4.  Each production rule begins with the '->' symbol.

5.  In a production rule, if there is a word enclosed in angle brackets, then
    that word may be any word that the scanner/screener has assigned that
    characterization to.
*/

/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/
#include <string.h>        /* strcmp, strncmp, strlen */
#include <stdlib.h>
#include <stdio.h>
#include "gbltypes.h"       /* BOOLEAN, CARDINAL, FALSE, TRUE */
#include "list.h"          /* LIST, NextToken, GetToken, GoToStartOfList */
#include "lvmcli.h"
#include "error.h"         /* ReportError */
#include "scanner.h"       /* Token, TokenTypes, EliminateTokenList */
#include "parser.h"
#include "names.h"
#include "debug.h"
#include "utility.h"

/*--------------------------------------------------
 * External Global Variables
 --------------------------------------------------*/

/*--------------------------------------------------
 * There are No Private Constants
 --------------------------------------------------*/

/*--------------------------------------------------
 * There are No Private Type Definitions
 --------------------------------------------------*/

/*--------------------------------------------------
 There are No Private Global Variables.
--------------------------------------------------*/

/*--------------------------------------------------
 Local Function Prototypes
--------------------------------------------------*/

/* Each of the following functions correspond to a non-terminal in the
   grammar.  Each function is responsible for parsing that part of
   the grammar which corresponds to the non-terminal the function
   represents.  Each function expects the current item in Tokens to
   be the first token that it should parse.  If it accepts the token,
   the function makes the next item in Tokens the current item.  Each
   function signals success by returning TRUE.                          */

/* The following functions also report parsing errors to the user.      */
static BOOLEAN Optional_Query_Parameters(LIST Tokens, pQueryStruct  pQueryParameters, pSTRING pCommandLine);

/* The following functions do not report parsing errors to the user.
   They do report errors accessing the Tokens LIST, though.           */


/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/



/*********************************************************************/
/*                                                                   */
/*   Function Name: Query_Parameters                                          */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Query_Parameters' found in the grammar.              */
/*                      It parses the token list using the production*/
/*                      rules for the 'Query_Parameters'                      */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the Query_Parameters line.                   */
/*                                                                   */
/*   Output: If Success : The function return value will be TRUE.    */
/*                        The current item in the Tokens LIST will be*/
/*                        the next item to parse.                    */
/*                                                                   */
/*           If Failure : The function return value will be FALSE.   */
/*                        The current item in the Tokens LIST will be*/
/*                        the unrecognized token.  The ReportError   */
/*                        function will be used to report the error  */
/*                        to the user.                               */
/*                                                                   */
/*   Error Handling:  The user will be informed of any errors via the*/
/*                    ReportError function from ERROR.H.  The current*/
/*                    item in the Tokens LIST will be the token which*/
/*                    was being parsed when the error occured.       */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes: There are two types of errors which could occur.  The    */
/*          first is a parsing error.  In this case, the current     */
/*          token does not match anything the parser expects.  The   */
/*          second type of error is an internal error.  This occurs  */
/*          when an operation on the Tokens LIST fails for some      */
/*          reason.  Both types of errors are reported to the user   */
/*          via the ReportError function in ERROR.H.                 */
/*                                                                   */
/*********************************************************************/
BOOLEAN Query_Parameters(LIST Tokens, pQueryStruct pQueryParameters, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  pSTRING  pDriveName = NULL;
  pSTRING  pDriveNumber = NULL;
  unsigned int TokenPosition;


  /* Get the current token so that we can examine it. */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error )
  {
    /* Set the OptionType field of the query structure */
    switch (CurrentToken.TokenType) {
    case All_CLI:
       pQueryParameters->OptionType = Query_All;
       break;
    case Bootable:
       pQueryParameters->OptionType = Query_Bootable;
       break;
    case Compatibility:
       pQueryParameters->OptionType = Query_Compatibility;
       break;
    case Freespace:
       pQueryParameters->OptionType = Query_Freespace;
       break;
    case Logical:
       pQueryParameters->OptionType = Query_Logical;
       break;
    case LVM:
       pQueryParameters->OptionType = Query_LVM;
       break;
    case Primary:
       pQueryParameters->OptionType = Query_Primary;
       break;
    case Unusable:
       pQueryParameters->OptionType = Query_Unusable;
       break;
    case Volumes:
       pQueryParameters->OptionType = Query_Volumes;
       break;
    default:
       pQueryParameters->OptionType = Query_Error;
       ReportParseError(Expected_Query_Option, Tokens, pCommandLine);
       break;
    } /* endswitch */

    /* Check for optional parameters */
    switch (CurrentToken.TokenType) {
    case Primary:
    case Logical:
    case Volumes:
    case Compatibility:
    case LVM:
    case All_CLI:
       NextToken(Tokens, &Error);
       if ( !Error ) {
          SkipOptionalWhitespace(Tokens);
          /* If there is a comma following the option */
          if ( IsComma(Tokens) ) {
             SkipOptionalWhitespace(Tokens);
             /* The token sequence following the comma must */
             /* satisfy the Optional Query Parameters rule. */
             if ( Optional_Query_Parameters(Tokens, pQueryParameters, pCommandLine) ) {
                ReturnValue = TRUE;
             } /* endif */
          } else {
             pQueryParameters->NameType = NoNameType;
             ReturnValue = TRUE;
          } /* endif */
       } /* endif */
       break;
    case Freespace:
    case Unusable:
    case Bootable:
       NextToken(Tokens, &Error);
       if ( !Error ) {
          SkipOptionalWhitespace(Tokens);
          /* If there is a comma following the option */
          if ( IsComma(Tokens) ) {
             SkipOptionalWhitespace(Tokens);
             GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
             if ( !Error ) {
                /* Any token following the comma must be the keword ALL */
                /* or a drive number or a drive name. */
                /* If it is ALL */
                if (CurrentToken.TokenType == All_CLI) {
                   pQueryParameters->NameType = AllNameType;
                   NextToken(Tokens, &Error);
                   ReturnValue = TRUE;
                } else {
                   /* If it is a drive number */
                   if ( Drive_Number(Tokens, &pDriveNumber) ) {
                      pQueryParameters->NameType = DriveNumberType;
                      pQueryParameters->pName = pDriveNumber;
                      ReturnValue = TRUE;
                   } else {
                      if ( Drive_Name(Tokens, SemiColon, &pDriveName) ) {
                         pQueryParameters->NameType = DriveNameType;
                         pQueryParameters->pName = pDriveName;
                         ReturnValue = TRUE;
                      } else {
                         ReportParseError(Expected_ALL_or_Drive_Number_or_Name, Tokens, pCommandLine);
                      } /* endif */
                   } /* endif */
                } /* endif */
             } /* endif */
          } else {
             pQueryParameters->NameType = NoNameType;
             ReturnValue = TRUE;
          } /* endif */
       } /* endif */
    } /* endswitch */
  }

  /* Report any errors accessing the token list as an internal error! */
  if ( Error ) {
    ReportError(Internal_Error);
  }

  return ReturnValue;

}

/*--------------------------------------------------
 * Local Functions Available
 --------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Optional_Query_Parameters                        */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Optional_Query_Parameters' found in the     */
/*                      grammar.                                     */
/*                      It parses the token list using the production*/
/*                      rules for this non-terminal.                 */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the        */
/*                          scanner/screener from the command line.  */
/*                                                                   */
/*   Output: If Success : The function return value will be TRUE.    */
/*                        The current item in the Tokens LIST will be*/
/*                        the next item to parse.                    */
/*                                                                   */
/*           If Failure : The function return value will be FALSE.   */
/*                        The current item in the Tokens LIST will be*/
/*                        the unrecognized token.  The ReportError   */
/*                        function will be used to report the error  */
/*                        to the user.                               */
/*                                                                   */
/*   Error Handling:  The user will be informed of any errors via the*/
/*                    ReportError function from ERROR.H.  The current*/
/*                    item in the Tokens LIST will be the token which*/
/*                    was being parsed when the error occured.       */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes: There are two types of errors which could occur.  The    */
/*          first is a parsing error.  In this case, the current     */
/*          token does not match anything the parser expects.  The   */
/*          second type of error is an internal error.  This occurs  */
/*          when an operation on the Tokens LIST fails for some      */
/*          reason.  Both types of errors are reported to the user   */
/*          via the ReportError function in ERROR.H.                 */
/*                                                                   */
/*********************************************************************/
static BOOLEAN Optional_Query_Parameters(LIST Tokens, pQueryStruct pQueryParameters, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  pSTRING  pDriveName = NULL;
  pSTRING  pFilesystemType = NULL;
  pSTRING  pDriveNumber = NULL;
  pSTRING  pNumber = NULL;
  unsigned int TokenPosition;


  /* Get the current token so that we can examine it. */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error )
  {
     if (CurrentToken.TokenType == All_CLI) {
        pQueryParameters->NameType = AllNameType;
        NextToken(Tokens, &Error);
        ReturnValue = TRUE;
     } else {
        /* If it is a drive number */
        if ( Drive_Number(Tokens, &pDriveNumber) ) {
           pQueryParameters->NameType = DriveNumberType;
           pQueryParameters->pName = pDriveNumber;
           ReturnValue = TRUE;
        } else {
           if ( Drive_Name(Tokens, SemiColon, &pDriveName) ) {
              pQueryParameters->NameType = DriveNameType;
              pQueryParameters->pName = pDriveName;
              ReturnValue = TRUE;
           } else {
              ReportParseError(Expected_ALL_or_Drive_Number_or_Name, Tokens, pCommandLine);
              return ReturnValue;
           } /* endif */
        } /* endif */
     } /* endif */

     /* If the next token is a comma, the following token */
     /* must be a valid filesystem type. */
     SkipOptionalWhitespace(Tokens);
     GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
     if ( !Error ) {
        if ( IsComma(Tokens) ) {
           SkipOptionalWhitespace(Tokens);
           GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
           if ( !Error) {
              /* The token following the comma must be filesystem type */
              if ( Acceptable_Name(Tokens, SemiColon, &pFilesystemType) ) {
                 pQueryParameters->pFileSystemType = pFilesystemType;
                 ReturnValue = TRUE;
              } else {
                 /* A number is also acceptable as a filesystem type */
                 if (CurrentToken.TokenType == Number) {
                    pNumber = (pSTRING) malloc(strlen(CurrentToken.pTokenText)+1);
                    if (pNumber != NULL) {
                       pQueryParameters->pFileSystemType = pNumber;
                       strcpy(pNumber, CurrentToken.pTokenText);
                       NextToken(Tokens, &Error);
                       ReturnValue = TRUE;
                    } else {
                       ReportError(Not_Enough_Memory);
                       ReturnValue = FALSE;
                       return ReturnValue;
                    } /* endif */
                 } else {
                    ReturnValue = FALSE;
                    ReportParseError(Expected_File_System_Type, Tokens, pCommandLine);
                 } /* endif */
              } /* endif */
           } /* endif */
        } else {
           /* Since we wouldn't have reached this point if the required */
           /* portion of the rule had not been satisfied above, then    */
           /* we have satisfied the optional query parameters rule even */
           /* without the optional comma and filesytem type. */
           ReturnValue = TRUE;
        } /* endif */
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}
