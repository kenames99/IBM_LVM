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
 * Module: creation.c
 */

/*
 * Change History:
 *
 */


/*
 * Functions: BOOLEAN Creation_Parameters
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
#include <stdio.h>
#include <stdlib.h>
#include "gbltypes.h"       /* BOOLEAN, CARDINAL, FALSE, TRUE */
#include "list.h"          /* LIST, NextToken, GetToken, GoToStartOfList */
#include "lvmcli.h"
#include "error.h"      /* ReportError */
#include "scanner.h"       /* Token, TokenTypes, EliminateTokenList */
#include "parser.h"
#include "names.h"
#include "query.h"
#include "install.h"
#include "creation.h"
#include "deletion.h"
#include "utility.h"
#include "volume.h"
#include "debug.h"

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
static BOOLEAN Partition_Parameters(LIST Tokens, pPartitionStruct pParameters, pSTRING pCommandLine);
static BOOLEAN Optional_Partition_Parameters(LIST Tokens, pPartitionStruct pParameters, pSTRING pCommandLine);
static BOOLEAN CLI_Allocation_Algorithm(LIST Tokens, pPartitionStruct pParameters, pSTRING pCommandLine);

/* Each of the following functions correspond to a non-terminal in the
   grammar.  Each function is responsible for parsing that part of
   the grammar which corresponds to the non-terminal the function
   represents.  Each function expects the current item in Tokens to
   be the first token that it should parse.  If it accepts the token,
   the function makes the next item in Tokens the current item.  Each
   function signals success by returning TRUE.                          */

/* The following functions also report parsing errors to the user.      */

/* The following functions do not report parsing errors to the user.
   They do report errors accessing the Tokens LIST, though.           */


/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Creation_Parameters                              */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Creation_Parameters' found in the grammar.  */
/*                      It parses the token list using the production*/
/*                      rules for the 'Creation_Parameters'          */
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
/*                    ReportError function from error.h.  The current*/
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
/*          via the ReportError function in error.h.                 */
/*                                                                   */
/*********************************************************************/
BOOLEAN Creation_Parameters(LIST Tokens, pCreationStruct pCreationParameters, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;
  pPartitionStruct pPartitionParameters = NULL;
  pVolumeStruct pVolumeParameters = NULL;

  /* Get the current token so that we can examine it. */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error )
  {
     switch (CurrentToken.TokenType) {
     case Partition:
        pCreationParameters->OptionType = Create_Partition_Option;
        /* Skip the partition reserved word */
        NextToken(Tokens,&Error);
        SkipOptionalWhitespace(Tokens);
        GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
        if ( !Error) {
           /* The first non-whitespace token following the partition */
           /* reserved word must be a colon. */
           if ( IsComma(Tokens) ) {
              /* The token sequence following the comma must satisfy */
              /* the Creation Parameters rule. */
              SkipOptionalWhitespace(Tokens);
              pPartitionParameters = (pPartitionStruct) malloc(sizeof(PartitionStruct));
              if (pPartitionParameters != NULL) {
                 pPartitionParameters->OptionType = Partition_Option_Error;
                 pPartitionParameters->NameType = NameErrorType;
                 pPartitionParameters->pPartitionName = NULL;
                 pPartitionParameters->SizeType = SizeNotSpecified;
                 pPartitionParameters->pSize = NULL;
                 pPartitionParameters->AllocationOptionType = Allocation_Error;
                 pPartitionParameters->AllocationSubOptionType = Allocation_FromError;
                 pPartitionParameters->pFreespaceID = NULL;   /* @206528 */

                 pCreationParameters->pParameters = pPartitionParameters;
                 if ( Partition_Parameters(Tokens, pPartitionParameters, pCommandLine) ) {
                    ReturnValue = TRUE;
                 } /* endif */
              } else {
                 ReportError(Not_Enough_Memory);
                 return ReturnValue;
              } /* endif */
           } else {
              ReportParseError(Expected_Comma, Tokens, pCommandLine);
           } /* endif */
        } /* endif */
        break;
     case Volume:
        pCreationParameters->OptionType = Create_Volume_Option;
        /* Skip the volume reserved word */
        NextToken(Tokens,&Error);
        SkipOptionalWhitespace(Tokens);
        GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
        if ( !Error) {
           /* The first non-whitespace token following the volume */
           /* reserved word must be a colon. */
           if ( IsComma(Tokens) ) {
              /* The token sequence following the comma must satisfy */
              /* the Volume Parameters rule. */
              SkipOptionalWhitespace(Tokens);
              pVolumeParameters = (pVolumeStruct) malloc(sizeof(VolumeStruct));
              if (pVolumeParameters != NULL) {
                 pVolumeParameters->OptionType = Volume_OptionError;
                 pVolumeParameters->SubOptionType = Volume_SubOptionError;
                 pVolumeParameters->pVolumeName = NULL;
                 pVolumeParameters->pDriveLetter = NULL;
                 pVolumeParameters->pFeatureListParameters = NULL;
                 pVolumeParameters->pPartitionListParameters = NULL;

                 pCreationParameters->pParameters = pVolumeParameters;
                 if ( Volume_Parameters(Tokens, pVolumeParameters, pCommandLine) ) {
                    ReturnValue = TRUE;
                 } /* endif */
              } else {
                 ReportError(Not_Enough_Memory);
              } /* endif */
           } else {
              ReportParseError(Expected_Comma, Tokens, pCommandLine);
           } /* endif */
        } /* endif */
        break;
     default:
        pCreationParameters->OptionType = Create_Error_Option;
        ReportParseError(Expected_Creation_Option, Tokens, pCommandLine);
     } /* endswitch */
  }

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

/*--------------------------------------------------
 * Local Functions Available
 --------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Partition_Parameters                             */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Partition_Parameters' found in the grammar. */
/*                      It parses the token list using the production*/
/*                      rules for the 'Partition_Parameters'         */
/*                      non-terminal.                                */
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the Partition_Parameters line.      */
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
/*                    ReportError function from error.h.  The current*/
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
/*          via the ReportError function in error.h.                 */
/*                                                                   */
/*********************************************************************/
static BOOLEAN Partition_Parameters(LIST Tokens, pPartitionStruct pPartitionParameters, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  pSTRING   pAcceptableName = NULL;
  pSTRING   pPartitionName = NULL;
  pSTRING   pDriveName = NULL;
  pSTRING   pDriveNumber = NULL;
  pSTRING   pSize = NULL;
  unsigned int TokenPosition;


  /* Get the current token so that we can examine it. */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( Error )
  {
     /* Report any errors accessing the token list as an internal error! */
     ReportError(Internal_Error);
     return ReturnValue;
  } /* endif */

  SkipOptionalWhitespace(Tokens);
  /* The first token must be a valid partition name */
  if ( Partition_Name(Tokens, Comma, &pPartitionName) ) {
     /* Partition name found */
     pPartitionParameters->pPartitionName = pPartitionName;
     SkipOptionalWhitespace(Tokens);
     GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
     if ( Error ) {
        /* Report any errors accessing the token list as an internal error! */
        ReportError(Internal_Error);
        return ReturnValue;
     } /* endif */

     /* The next token must be a comma */
     if ( IsComma(Tokens) ) {
        SkipOptionalWhitespace(Tokens);
        GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
        if ( Error ) {
           /* Report any errors accessing the token list as an internal error! */
           ReportError(Internal_Error);
           return ReturnValue;
        } /* endif */

        /* The next token may be a drive number or drive name */
        /* If the current token is not a drive number */
        if ( Drive_Number(Tokens, &pDriveNumber) ) {
           pPartitionParameters->NameType = DriveNumberType;
           pPartitionParameters->pDrive = pDriveNumber;
        } else {
           /* It wasn't a drive number, check for drive name */
           if ( Drive_Name(Tokens, Comma, &pDriveName) ) {
              pPartitionParameters->NameType = DriveNameType;
              pPartitionParameters->pDrive = pDriveName;
           } else {
              ReportParseError(Expected_Drive_Number_or_Name, Tokens, pCommandLine);
              return ReturnValue;
           } /* endif */
        } /* endif */

        /* Drive number or name found */
        SkipOptionalWhitespace(Tokens);
        GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
        if ( Error ) {
           /* Report any errors accessing the token list as an internal error! */
           ReportError(Internal_Error);
           return ReturnValue;
        } /* endif */

        /* The next token must be a comma */
        if ( IsComma(Tokens) ) {
           SkipOptionalWhitespace(Tokens);
           GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
           if ( Error ) {
              /* Report any errors accessing the token list as an internal error! */
              ReportError(Internal_Error);
              return ReturnValue;
           } /* endif */

           /* The next token may be a size or the Logical/Primary parameter */
           if (CurrentToken.TokenType == Number) {
              pSize = (pSTRING) malloc(strlen(CurrentToken.pTokenText)+1);
              if (pSize != NULL) {
                strcpy(pSize, CurrentToken.pTokenText);
                pPartitionParameters->SizeType = SizeSpecified;
                pPartitionParameters->pSize = pSize;
              } else {
                 ReportError(Not_Enough_Memory);
                 return ReturnValue;
              } /* endif */

              /* Number found, advance to next token */
              NextToken(Tokens,&Error);
              SkipOptionalWhitespace(Tokens);
              GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
              if ( Error ) {
                 /* Report any errors accessing the token list as an internal error! */
                 ReportError(Internal_Error);
                 return ReturnValue;
              } /* endif */

              /* The next token must be a comma */
              if ( IsComma(Tokens) ) {
                 SkipOptionalWhitespace(Tokens);
                 GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
                 if ( Error ) {
                    /* Report any errors accessing the token list as an internal error! */
                    ReportError(Internal_Error);
                    return ReturnValue;
                 } /* endif */
              } else {
                 ReportParseError(Expected_Comma, Tokens, pCommandLine);
                 return ReturnValue;
              } /* endif */
           } else {
              /* Indicate the Size parameter was not specified */
              pPartitionParameters->SizeType = SizeNotSpecified;
              /* If the token is not Logical or Primary either, then we have an error */
              if ( (CurrentToken.TokenType != Logical) && (CurrentToken.TokenType != Primary) ) {
                 /***********************************************************************/
                 /* The following message should be changed to be a new one that says   */
                 /* a number or logical or primary was expected.  For now, I will leave */
                 /* the original message that just says a number was expected.  The     */
                 /* message text file is currently locked and getting the new function  */
                 /* operational is more important. I will add the new message later.    */
                 /***********************************************************************/
                 ReportParseError(Expected_Number, Tokens, pCommandLine);
                 return ReturnValue;
              } /* endif */
              /* Since the token must be Logical or Primary, then the missing Size */
              /* parameter is OK.  Since the SizeType field of the structure has   */
              /* been marked are not specified, we can proceed with the processing */
              /* of the remainder of the command as if Size had been specified. */
           } /* endif */

           /* The next token must be either Logical or Primary */
           if (CurrentToken.TokenType == Logical) {
              pPartitionParameters->OptionType = Partition_Logical;
           } else {
              if (CurrentToken.TokenType == Primary) {
                 pPartitionParameters->OptionType = Partition_Primary;
              } else {
                 ReportParseError(Expected_Partition_Option, Tokens, pCommandLine);
                 return ReturnValue;
              } /* endif */
           } /* endif */

           /* Logical or Primary found, advance to next token */
           NextToken(Tokens,&Error);
           SkipOptionalWhitespace(Tokens);
           GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
           if ( Error ) {
              /* Report any errors accessing the token list as an internal error! */
              ReportError(Internal_Error);
              return ReturnValue;
           } /* endif */

           /* If the next token is a comma, then the one following it */
           /* must be either Bootable or NonBootable/NotBootable. */
           if ( IsComma(Tokens) ) {
              SkipOptionalWhitespace(Tokens);
              GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
              if ( Error ) {
                 /* Report any errors accessing the token list as an internal error! */
                 ReportError(Internal_Error);
                 return ReturnValue;
              } /* endif */

              /* The next token must be either Bootable or NonBootable */
              if (CurrentToken.TokenType == Bootable) {
                 pPartitionParameters->SubOptionType = Partition_Bootable;
              } else {
                 if ( (CurrentToken.TokenType == NonBootable) || (CurrentToken.TokenType == NotBootable) ) {
                    pPartitionParameters->SubOptionType = Partition_NonBootable;
                 } else {
                    ReportParseError(Expected_Partition_SubOption, Tokens, pCommandLine);
                    return ReturnValue;
                 } /* endif */
              } /* endif */

              /* Bootable or NonBootable found, advance to next token */
              NextToken(Tokens,&Error);
              SkipOptionalWhitespace(Tokens);
              GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
              if ( Error ) {
                 /* Report any errors accessing the token list as an internal error! */
                 ReportError(Internal_Error);
                 return ReturnValue;
              } /* endif */
           } else {
              ReportParseError(Expected_Comma, Tokens, pCommandLine);
              return ReturnValue;
           } /* endif */

           /* If the next token is a comma, then the ones following it */
           /* must satisfy the Optional_Partition_Parameters rule. */
           if ( IsComma(Tokens) ) {
              if ( Optional_Partition_Parameters(Tokens, pPartitionParameters, pCommandLine) ) {
                 ReturnValue = TRUE;
              } /* endif */
           } else {
              /* Any further partition parameters are optional */
              pPartitionParameters->AllocationOptionType = Allocation_None;
              pPartitionParameters->AllocationSubOptionType = Allocation_FromNone;
              ReturnValue = TRUE;
           } /* endif */
        } else {
           ReportParseError(Expected_Comma, Tokens, pCommandLine);
        } /* endif */
     } else {
        ReportParseError(Expected_Comma, Tokens, pCommandLine);
     } /* endif */
  } else {
     ReportParseError(Expected_Partition_Name, Tokens, pCommandLine);
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Optional_Partition_Parameters                    */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Optional_Partition_Parameters'.             */
/*                      It parses the token list using the production*/
/*                      rules for the 'Optional_Partition_Parameters'*/
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
/*                    ReportError function from error.h.  The current*/
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
/*          via the ReportError function in error.h.                 */
/*                                                                   */
/*********************************************************************/
static BOOLEAN Optional_Partition_Parameters(LIST Tokens, pPartitionStruct pPartitionParameters, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;


  /* The next token must satisfy the Allocation_Algorithm rule */
  if ( CLI_Allocation_Algorithm(Tokens, pPartitionParameters, pCommandLine) ) {
     SkipOptionalWhitespace(Tokens);
     GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
     if ( Error ) {
        /* Report any errors accessing the token list as an internal error! */
        ReportError(Internal_Error);
        return ReturnValue;
     } /* endif */

     /* If the next token is a comma, then the following token */
     /* must be either the FromStart or FromEnd keyword.       */
     if ( IsComma(Tokens) ) {
        SkipOptionalWhitespace(Tokens);
        GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
        if ( Error ) {
           /* Report any errors accessing the token list as an internal error! */
           ReportError(Internal_Error);
           return ReturnValue;
        } /* endif */

        /* The token following the comma must be FromStart or FromEnd */
        if ( (CurrentToken.TokenType == FromStart) || (CurrentToken.TokenType == FromEnd) ) {
           /* We have a complete /create command */
           ReturnValue = TRUE;
           if (CurrentToken.TokenType == FromStart) {
              pPartitionParameters->AllocationSubOptionType = Allocation_FromStart;
           } else {
              pPartitionParameters->AllocationSubOptionType = Allocation_FromEnd;
           } /* endif */
           NextToken(Tokens,&Error);
        } else {
           ReportParseError(Expected_Allocation_SubOption, Tokens, pCommandLine);
        } /* endif */
     } else {
        /* The next token was NOT a comma but we have a complete command anyway */
        ReturnValue = TRUE;
     } /* endif */
  } else {
     ReportParseError(Expected_Allocation_Option, Tokens, pCommandLine);
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error ) {
     ReportError(Internal_Error);
  }

  return ReturnValue;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: CLI_Allocation_Algorithm                         */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Allocation_Algorithm' found in the grammar. */
/*                      It parses the token list using the production*/
/*                      rules for the 'Allocation_Algorithm'         */
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
/*                    ReportError function from error.h.  The current*/
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
/*          via the ReportError function in error.h.                 */
/*                                                                   */
/*********************************************************************/
static BOOLEAN CLI_Allocation_Algorithm(LIST Tokens, pPartitionStruct pPartitionParameters, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  pSTRING  pFreespaceID = NULL;
  unsigned int TokenPosition;


  /* Get the current token so that we can examine it. */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error )
  {
    switch (CurrentToken.TokenType) {
    case BestFit:
       pPartitionParameters->AllocationOptionType = Allocation_BestFit;
       ReturnValue = TRUE;
       NextToken(Tokens,&Error);
       break;
    case FirstFit:
       pPartitionParameters->AllocationOptionType = Allocation_FirstFit;
       ReturnValue = TRUE;
       NextToken(Tokens,&Error);
       break;
    case LastFit:
       pPartitionParameters->AllocationOptionType = Allocation_LastFit;
       ReturnValue = TRUE;
       NextToken(Tokens,&Error);
       break;
    case FromLargest:
       pPartitionParameters->AllocationOptionType = Allocation_FromLargest;
       ReturnValue = TRUE;
       NextToken(Tokens,&Error);
       break;
    case FromSmallest:
       pPartitionParameters->AllocationOptionType = Allocation_FromSmallest;
       ReturnValue = TRUE;
       NextToken(Tokens,&Error);
       break;
    default:
       /* must satisfy free space id rule */
       if ( Acceptable_Name(Tokens, Comma, &pFreespaceID) ) {
          /* Freespace ID found */
          pPartitionParameters->AllocationOptionType = Allocation_FreespaceID;
          pPartitionParameters->pFreespaceID = pFreespaceID;
          ReturnValue = TRUE;
       } /* endif */
    } /* endswitch */
  }

  /* Report any errors accessing the token list as an internal error! */
  if ( Error ) {
     ReportError(Internal_Error);
  }

  return ReturnValue;

}
