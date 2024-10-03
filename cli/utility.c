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
 * Module: deletion.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: void NumberToString
 *
 * Description:  This module holds utility functions used by multiple
 *               modules in LVM.EXE.
 *
 */

#include <string.h>        /* strcmp, strncmp, strlen */
#include <stdio.h>
#include "gbltypes.h"       /* BOOLEAN, CARDINAL, FALSE, TRUE */
#include "list.h"          /* LIST, NextToken, GetToken, GoToStartOfList */
#include "lvmcli.h"
#include "error.h"         /* ReportError */
#include "scanner.h"       /* Token, TokenTypes, EliminateTokenList */
#include "debug.h"
#include "utility.h"

/*--------------------------------------------------
 * There are No External Global Variables
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
 Local Functions
--------------------------------------------------*/

/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: SkipOptionalWhitespace                           */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'OptionalWhiteSpace' found in the grammar.   */
/*                      It parses the token list using the production*/
/*                      rules for the 'OptionalWhiteSpace'           */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
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
BOOLEAN SkipOptionalWhitespace(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  /* Get the current token so that we can examine it. */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error )
  {
    ReturnValue = TRUE;
    /* If the current token is any type of space or tab */
    if ( (CurrentToken.TokenType == Space )
         ||
         (CurrentToken.TokenType == MultiSpace)
         ||
         (CurrentToken.TokenType == Tab)
         ||
         (CurrentToken.TokenType == MultiTab)
       )
    {
      /* At least one space/tab sequence has been found, check for more */
      GetNextToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
      if ( !Error )
      {
         /* Check the next token until something other than a space or tab sequence is found */
         while ( (CurrentToken.TokenType == Space )
                 ||
                 (CurrentToken.TokenType == MultiSpace)
                 ||
                 (CurrentToken.TokenType == Tab)
                 ||
                 (CurrentToken.TokenType == MultiTab)
               )
         {
            GetNextToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
            if ( Error )
            {
               ReturnValue = FALSE;
               break;
            } /* endif */
         } /* endwhile */
      } else {
         ReturnValue = FALSE;
      } /* endif */
    } /* endif */
  } else {
     ReturnValue = FALSE;
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
   ReportError(Internal_Error);

  return ReturnValue;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: IsEof                                          */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Eof' found in the grammar.                */
/*                      It parses the token list using the production*/
/*                      rules for the 'Eof'                        */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
/*                                                                   */
/*   Output: If Success : The function return value will be TRUE.    */
/*                        The current item in the Tokens LIST will   */
/*                        still be the Eof.                          */
/*                                                                   */
/*           If Failure : The function return value will be FALSE.   */
/*                        The current item in the Tokens LIST will   */
/*                        be the token which was found not to be an  */
/*                        Eof.                                       */
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
/*          reason.                                                  */
/*                                                                   */
/*********************************************************************/
BOOLEAN IsEof(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     /* If the next non-whitespace token is a Eof */
     if (CurrentToken.TokenType == Eof) {
        ReturnValue = TRUE;
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: IsComma                                          */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Comma' found in the grammar.                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
/*                                                                   */
/*   Output: If Success : The function return value will be TRUE.    */
/*                        The current item in the Tokens LIST will   */
/*                        be the next token after the comma.         */
/*                                                                   */
/*           If Failure : The function return value will be FALSE.   */
/*                        The current item in the Tokens LIST will   */
/*                        the original token that was found not to   */
/*                        a comma.                                   */
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
BOOLEAN IsComma(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     if (CurrentToken.TokenType == Comma) {
        ReturnValue = TRUE;
        /* Skip over the comma */
        NextToken(Tokens, &Error);
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: IsColon                                          */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Colon' found in the grammar.                */
/*                      It parses the token list using the production*/
/*                      rules for the 'Colon'                        */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
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
BOOLEAN IsColon(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     /* The next non-whitespace token must be a colon */
     if (CurrentToken.TokenType == Colon) {
        ReturnValue = TRUE;
        /* Skip over the colon */
        NextToken(Tokens, &Error);
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: IsOpenParen                                      */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Open_Paren' found in the grammar.           */
/*                      It parses the token list using the production*/
/*                      rules for the 'Open_Paren'                   */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
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
BOOLEAN IsOpenParen(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     /* The next non-whitespace token must be a colon */
     if (CurrentToken.TokenType == Open_Paren) {
        ReturnValue = TRUE;
        /* Skip over the colon */
        NextToken(Tokens, &Error);
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: IsCloseParen                                     */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Close_Paren' found in the grammar.          */
/*                      It parses the token list using the production*/
/*                      rules for the 'Close_Paren'                  */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
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
BOOLEAN IsCloseParen(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     /* The next non-whitespace token must be a colon */
     if (CurrentToken.TokenType == Close_Paren) {
        ReturnValue = TRUE;
        /* Skip over the colon */
        NextToken(Tokens, &Error);
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: IsOpenBracket                                    */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Open_Bracket' found in the grammar.         */
/*                      It parses the token list using the production*/
/*                      rules for the 'Open_Bracket'                 */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
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
BOOLEAN IsOpenBracket(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     /* The next non-whitespace token must be a colon */
     if (CurrentToken.TokenType == Open_Bracket) {
        ReturnValue = TRUE;
        /* Skip over the colon */
        NextToken(Tokens, &Error);
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: IsCloseBracket                                   */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Close_Bracket' found in the grammar.        */
/*                      It parses the token list using the production*/
/*                      rules for the 'Close_Bracket'                */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
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
BOOLEAN IsCloseBracket(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     /* The next non-whitespace token must be a colon */
     if (CurrentToken.TokenType == Close_Bracket) {
        ReturnValue = TRUE;
        /* Skip over the colon */
        NextToken(Tokens, &Error);
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: IsOpenBrace                                      */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Open_Brace' found in the grammar.           */
/*                      It parses the token list using the production*/
/*                      rules for the 'Open_Brace'                   */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
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
BOOLEAN IsOpenBrace(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     /* The next non-whitespace token must be a colon */
     if (CurrentToken.TokenType == Open_Brace) {
        ReturnValue = TRUE;
        /* Skip over the colon */
        NextToken(Tokens, &Error);
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: IsCloseBrace                                     */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Close_Brace' found in the grammar.          */
/*                      It parses the token list using the production*/
/*                      rules for the 'Close_Brace'                  */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line.                   */
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
BOOLEAN IsCloseBrace(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     /* The next non-whitespace token must be a colon */
     if (CurrentToken.TokenType == Close_Brace) {
        ReturnValue = TRUE;
        /* Skip over the colon */
        NextToken(Tokens, &Error);
     } /* endif */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

