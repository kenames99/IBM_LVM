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
 * Module: volume.c
 */

/*
 * Change History:
 *
 */


/*
 * Functions: BOOLEAN Volume_Parameters
 *                    Partition_List_Parameters
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
extern Feature_Information_Array pAvailableFeatureArray;

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

/* The following functions do not report parsing errors to the user.
   They do report errors accessing the Tokens LIST, though.           */


/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Partition_List_Parameters                       */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Partition_List_Parameters' found in the    */
/*                      grammar.                                     */
/*                      It parses the token list using the production*/
/*                      rules for the 'Partition_List_Parameters'   */
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
BOOLEAN Feature_Name(LIST Tokens, CARDINAL32 *FeatureID, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition, rc;
  LVM_Feature_Specification_Record FeatureSpecRecord;
  int  ErrMsg;

  rc = 0;
  /* Get the current token so that we can examine it. */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error )
  {

    /* Make sure this is the right type of token */
    if ( CurrentToken.TokenType != AcceptableCharsStr )
    {
      ErrMsg = Expected_Feature_Name;
      rc = 1;
    }
    else
    {
      BOOLEAN done;
      int FeatureInformationIndex;

      /* search through the available features for matching feature name */
      FeatureInformationIndex = 0;
      done = FALSE;
      while ( !done )
      {
	pSTRING SrcString, TrgString;
	
	/* have we search through all the features already ? */
	if ( FeatureInformationIndex >= pAvailableFeatureArray.Count ) 
	{ /* yes
	   * set error msg pointer, exit loop, set return code type
	   */
	  ErrMsg = Expected_Feature_Name;
	  done = TRUE;
	  rc = 2;
	}
	else
	{  /* no */
	  int CmpRC;

	  /* compare specified feature name to this available features record */
	  SrcString = CurrentToken.pTokenText;
	  TrgString = pAvailableFeatureArray.Feature_Data[ FeatureInformationIndex ].Short_Name;
/*	  TrgString = "Test"; */
	  CmpRC = strnicmp( SrcString, TrgString, strlen( TrgString ) );
	  /* do they match? */
	  if ( CmpRC == 0 )
	  { /* yes
	     * return the feature id, exit loop, set return value
	     */
	    *FeatureID = pAvailableFeatureArray.Feature_Data[ FeatureInformationIndex ].ID;
/*  	    *FeatureID = 100; */
	    done = TRUE;
	    ReturnValue = TRUE;
	  }
	  FeatureInformationIndex++;
	}
      }
    }
  }

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
  {
    ReportError(Internal_Error);
  }

  return ReturnValue;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Partition_List_Parameters                       */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Partition_List_Parameters' found in the    */
/*                      grammar.                                     */
/*                      It parses the token list using the production*/
/*                      rules for the 'Partition_List_Parameters'   */
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
BOOLEAN Feature_Specification_Parameters(LIST Tokens, LVM_Feature_Specification_Record *pFeatureSpecRecord, pSTRING pCommandLine)
{
  BOOLEAN  rc = FALSE;  /* Used to hold the function return value. */
  char *pMsg, pMsgBuf[100];
  CARDINAL32 LVMError;

  Parse_Feature_Parameters(Tokens, pFeatureSpecRecord, &pMsg, &LVMError);
  switch (LVMError)
  {
     case 0:
       rc = TRUE;
     break;
     case LVM_ENGINE_PARSING_ERROR:
       ReportFeatureParseError( pMsg, Tokens, pCommandLine );
     break;
     default:
       sprintf( pMsgBuf, "Internal feature plug-in error (RC=%d)", LVMError);
       ReportFeatureParseError( pMsgBuf, Tokens, pCommandLine );
     break;
  }
/*  ReturnValue = LVM_Parse_Feature_Parameters(Tokens, pFeatureSpecRecord, pCommandLine); */

  return( rc );
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Partition_List_Parameters                       */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Partition_List_Parameters' found in the    */
/*                      grammar.                                     */
/*                      It parses the token list using the production*/
/*                      rules for the 'Partition_List_Parameters'   */
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
BOOLEAN Feature_Specification(LIST Tokens, pFeatureListStruct pFeatureListParameters, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  pSTRING  pDriveName = NULL;
  pSTRING  pPartitionName = NULL;
  pSTRING  pDriveNumber = NULL;
  pSTRING  pSize = NULL;
  pSTRING  pFeatureName = NULL;
  CARDINAL32 Feature_ID;
  unsigned int TokenPosition, i;
  BOOLEAN  rc;

  rc = 0;
  for ( i = 0; ; i++ )
  {
    int ErrMsg;

    /* Get the current token so that we can examine it. */
    SkipOptionalWhitespace(Tokens);
    GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
    if ( !Error )
    {
      switch( i )
      {
	case 0:
	{
	  /* The next token must be a feature plug in name */
	  rc =  Feature_Name(Tokens, &Feature_ID, pCommandLine);
	  if ( rc == TRUE ) 
	  {
	    pFeatureListParameters->Feature_ID = Feature_ID;
	    NextToken(Tokens, &Error);
	  } 
	  else 
	  {
	    ErrMsg = Expected_Feature_Name;
	  } /* endif */
	}
	break;
        case 1:
        {
	  /* The token following the feature name must be a colon */
	  rc = IsColon(Tokens); 
	  if ( rc != TRUE ) 
	  {
	    ErrMsg = Expected_Colon;
	  } /* endif */
	}
	break;
        case 2:
        {
	  /* The token following the colon must be a open parenthesis */
	  rc = IsOpenParen(Tokens);
	  if ( rc  != TRUE ) 
	  {
            ErrMsg = Expected_Open_Paren;
	  } /* endif */
	}
	break;
        case 4:
	{
	  LVM_Feature_Specification_Record FeatureSpecRecord;

	  /* Pass the token list to the feature plug in so that it may continue parsing */
	  FeatureSpecRecord.Feature_ID = pFeatureListParameters->Feature_ID;
	  rc = Feature_Specification_Parameters(Tokens, &FeatureSpecRecord, pCommandLine );
	  if ( rc == TRUE )
	  {
	    pFeatureListParameters->Actual_Class = FeatureSpecRecord.Actual_Class;
	    pFeatureListParameters->Init_Data = FeatureSpecRecord.Init_Data;
	  }
	  else 
	  {
/*            ErrMsg = Expected_Feature_Parameters; */
            ErrMsg = 0;
	  } /* endif */
	}
	break;
	case 5:
	{
	  /* The token following the colon must be a open parenthesis */
	  rc = IsCloseParen(Tokens);
	  if ( rc != TRUE ) 
	  {
            ErrMsg = Expected_Close_Paren;
	  } /* endif */
	}
	break;
	case 6:
          ReturnValue = TRUE;
	break;
      }
    }

    if ( i == 6 )
    {
      break;
    }

    if ( rc != TRUE )
    {
      if ( ErrMsg )
      {
        ReportParseError( ErrMsg, Tokens, pCommandLine);
      }
      break;
    }

    /* Report any errors accessing the token list as an internal error! */
    if ( Error )
    {
      ReportError(Internal_Error);
      break;
    }
  } /* end-for */

  return ReturnValue;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Volume_Parameters                                */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Volume_Parameters' found in the grammar.    */
/*                      It parses the token list using the production*/
/*                      rules for the 'Volume_Parameters'            */
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
BOOLEAN Volume_Parameters(LIST Tokens, pVolumeStruct pVolumeParameters, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  pSTRING  pVolumeName = NULL;
  pSTRING  pDriveLetter = NULL;
  unsigned int TokenPosition;
  pPartitionListStruct  pPartitionListParameters = NULL;
  pFeatureListStruct pFeatureListParameters = NULL;


  /* Get the current token so that we can examine it. */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( Error )
  {
     /* Report any errors accessing the token list as an internal error! */
     ReportError(Internal_Error);
     return ReturnValue;
  } /* endif */

  switch (CurrentToken.TokenType) {
  case Compatibility:
     pVolumeParameters->OptionType = Volume_Compatibility;
     NextToken(Tokens, &Error);
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

        /* The next token must BootDOS, BootOS2, or NoBoot */
        if ( (CurrentToken.TokenType == BootDOS)
             ||
             (CurrentToken.TokenType == BootOS2)
             ||
             (CurrentToken.TokenType == NoBoot)
           ) {
           switch (CurrentToken.TokenType) {
           case BootDOS:
              pVolumeParameters->SubOptionType = Volume_BootDOS;
              break;
           case BootOS2:
              pVolumeParameters->SubOptionType = Volume_BootOS2;
              break;
           case NoBoot:
              pVolumeParameters->SubOptionType = Volume_NoBoot;
              break;
           } /* endswitch */

           /* Boot type found, advance to next token */
           NextToken(Tokens,&Error);
           SkipOptionalWhitespace(Tokens);
           GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
           if ( Error ) {
              /* Report any errors accessing the token list as an internal error! */
              ReportError(Internal_Error);
              return ReturnValue;
           } /* endif */

           /* The token following the boot type must be a comma */
           if ( IsComma(Tokens) ) {
              SkipOptionalWhitespace(Tokens);
              GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
              if ( Error) {
                 /* Report any errors accessing the token list as an internal error! */
                 ReportError(Internal_Error);
                 return ReturnValue;
              } /* endif */

              /* The tokens following the comma constitute a drive letter sequence */
              if ( Drive_Letter(Tokens, &pDriveLetter) ) {
                  pVolumeParameters->pDriveLetter = pDriveLetter;
                 /* The Drive_Letter routine has already advanced beyond the drive letter sequence */
                 SkipOptionalWhitespace(Tokens);
                 GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
                 if ( Error ) {
                    /* Report any errors accessing the token list as an internal error! */
                    ReportError(Internal_Error);
                    return ReturnValue;
                 } /* endif */
                 /* The next token past drive letter sequence must be a comma */
                 if ( IsComma(Tokens) ) {
                    SkipOptionalWhitespace(Tokens);
                    GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);

                    /* The next token must be a volume name */
                    if ( Volume_Name(Tokens, Comma, &pVolumeName) ) {
                       pVolumeParameters->pVolumeName = pVolumeName;
                       SkipOptionalWhitespace(Tokens);
                       GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
                       if ( Error ) {
                          /* Report any errors accessing the token list as an internal error! */
                          ReportError(Internal_Error);
                          return ReturnValue;
                       } /* endif */
                       /* The next token must be another comma */
                       if ( IsComma(Tokens) ) {
                          SkipOptionalWhitespace(Tokens);
                          /* The tokens following the comma must satisfy */
                          /* the Partition List Parameters rule.              */
                          pPartitionListParameters = (pPartitionListStruct) malloc(sizeof(PartitionListStruct));
                          if (pPartitionListParameters != NULL) {
                             pPartitionListParameters->NameType = NameErrorType;
                             pPartitionListParameters->pDrive = NULL;
                             pPartitionListParameters->pPartitionName = NULL;
                             pPartitionListParameters->pNext = NULL;
                             pVolumeParameters->pPartitionListParameters = pPartitionListParameters;
                             if ( Partition_List_Parameters(Tokens, pPartitionListParameters, pCommandLine) ) {
                                /* We have successfully accumulated a valid set of volume parameters */
                                ReturnValue = TRUE;
                             } /* endif */
                          } else {
                             ReportError(Not_Enough_Memory);
                          } /* endif */
                       } else {
                          ReportParseError(Expected_Comma, Tokens, pCommandLine);
                       } /* endif */
                    } else {
                       ReportParseError(Expected_Volume_Name, Tokens, pCommandLine);
                    } /* endif */
                 } else {
                    ReportParseError(Expected_Comma, Tokens, pCommandLine);
                 } /* endif */
              } else {
                 ReportParseError(Expected_Drive_Letter, Tokens, pCommandLine);
              } /* endif */
           } else {
              ReportParseError(Expected_Comma, Tokens, pCommandLine);
           } /* endif */
        } else {
           ReportParseError(Expected_Compatibility_Option, Tokens, pCommandLine);
        } /* endif */
     } else {
        ReportParseError(Expected_Comma, Tokens, pCommandLine);
     } /* endif */
     break;
  case LVM:
  {
     int i;
     BOOLEAN rc, LastCheck;
     pVolumeParameters->OptionType = Volume_LVM;
     NextToken(Tokens, &Error);
     SkipOptionalWhitespace(Tokens);

     LastCheck = FALSE;
     rc = TRUE;
     for (i = 0; ; i++ )
     {
	int ErrMsg;

	SkipOptionalWhitespace(Tokens);
	GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
	if ( Error ) 
	{
	   ErrMsg = Internal_Error;
	}
	else
	{
	   switch (i)
	   {
	      case 0:
	      case 2:
	      case 4:
	      {
		 /* The next token must be a comma */
		 rc = IsComma(Tokens); 
		 if ( rc != TRUE ) 
		 {
		    ErrMsg = Expected_Comma;
		 }
	      }
	      break;
	      case 1:
	      {
		 /* The next token must be a drive letter sequence */
		 rc = Drive_Letter(Tokens, &pDriveLetter);
		 if ( rc == TRUE ) 
		 {
		    pVolumeParameters->pDriveLetter = pDriveLetter;
		 }
		 else
		 {
		    ErrMsg = Expected_Drive_Letter;
		 }
	      }
	      break;
	      case 3:
	      {
		 /* The next token must be a volume name */
		 rc = Volume_Name(Tokens, Comma, &pVolumeName);
		 if ( rc == TRUE ) 
		 {
		    pVolumeParameters->pVolumeName = pVolumeName;
		 }
		 else
		 {
		    ErrMsg = Expected_Volume_Name;
		 }
	      }
	      break;
	      case 5:
	      {
		 /* The next token must be a open bracket */
		 rc = IsOpenBracket(Tokens);
		 if ( rc != TRUE ) 
		 {
		    ErrMsg = Expected_Open_Bracket;
		 }
	      }
	      break;
	      case 6:
	      {
		 rc = IsCloseBracket(Tokens);
		 if ( rc == TRUE )
		 {
		    /* The last call to Feature Specification was successful */
		    /* and the next token is an CloseBracket, we are finished looking */
		    /* for feature specifications. */
		    i = i + 1;
		 }
		 else
		 {
		    pFeatureListStruct pFeatureListParms;

		    /* The next token must be a the beginning of a feature specification */
		    pFeatureListParms = (pFeatureListStruct) malloc(sizeof(FeatureListStruct));
		    if (pFeatureListParms == NULL)
		    {
		       Error = 8;
		       ErrMsg = Not_Enough_Memory;
		    }
		    else
		    {
		       if ( pFeatureListParameters == NULL )
		       {
			  pFeatureListParameters = pFeatureListParms;
			  pVolumeParameters->pFeatureListParameters = pFeatureListParameters;
		       }
		       else
		       {
			  pFeatureListParameters->pNext = pFeatureListParms;
			  pFeatureListParameters = pFeatureListParms;
		       }
		       
		       pFeatureListParameters->Feature_ID = 0;
		       pFeatureListParameters->Actual_Class = 0;
		       pFeatureListParameters->Init_Data = NULL;
		       pFeatureListParameters->pPartitionListParameters = NULL;
		       pFeatureListParameters->pNext = NULL;
		       
		       rc = Feature_Specification(Tokens, pFeatureListParameters, pCommandLine);
		       if ( rc != TRUE )
		       {
			  ErrMsg = 0;
		       }
		    }
		 }
	      }
	      break;
	      case 7:
	      {			     
		 /* If the next token is another comma, then continue looking */
		 /* for another feature specification */
		 rc = IsComma(Tokens);
		 if ( rc == TRUE ) 
		 {
		    i = i-2;
		 }
		 else
		 {
		    rc = IsCloseBracket(Tokens);
		    if ( rc == TRUE )
		    {
		       /* The last call to Feature Specification was successful */
		       /* and the next token is an OpenBracket, we are finished looking */
		       /* for feature specifications. */
		    }
		    else
		    {
		       ErrMsg = Expected_Close_Bracket;
		    }
		 } /* endif */
	      }
	      break;
	      case 8:
	      {
		 /* The next token must be a comma */
		 rc = IsComma(Tokens); 
		 if ( rc != TRUE ) 
		 {
		    ErrMsg = Expected_Partition_List_Parms;
		 }
	      }
	      break;
	      case 9:
	      {
		 pPartitionListStruct pPartitionListParms;

		 /* Following the volume name, there must be at least one set */
		 /* of partition list parameters, preceded by a comma.  In other words */
		 /* there must be a comma following the drive letter sequence and then  */
		 /* series of tokens that satisfy the partition list parameters rule.  */
		 /* Once one series of partition list parameters have been found, if   */
		 /* these are followed by another comma, there must be another set of   */
		 /* partition list parameters. */
		 pPartitionListParms = (pPartitionListStruct) malloc(sizeof(PartitionListStruct));
		 if (pPartitionListParms == NULL) 
		 {
		    Error = 8;
		    ErrMsg = Not_Enough_Memory;
		 }
		 else
		 {
		    if ( pPartitionListParameters == NULL )
		    {
		       pPartitionListParameters = pPartitionListParms;
		       pVolumeParameters->pPartitionListParameters = pPartitionListParameters;
		    }
		    else
		    {
		       pPartitionListParameters->pNext = pPartitionListParms;
		       pPartitionListParameters = pPartitionListParms;
		    }

		    pPartitionListParameters->NameType = NameErrorType;
		    pPartitionListParameters->pDrive = NULL;
		    pPartitionListParameters->pPartitionName = NULL;
		    pPartitionListParameters->pNext = NULL;
				
		    rc = Partition_List_Parameters(Tokens, pPartitionListParameters, pCommandLine);
		 }
	      }
	      break;
	      case 10:
	      {	   
		 /* If the next token is another comma, then continue looking */
		 /* for another set of partition list parameters */
		 rc = IsComma(Tokens);
		 if ( rc == TRUE ) 
		 {
		    i = i - 2;
		 }
		 else
		 {
		    LastCheck = TRUE;
		    rc = TRUE;
		 }
	      }
	      break;
	   } /* end-switch */
	} /* end-else */

	/* Report any parsing related errors encountered at this level */
	if ( rc != TRUE )
	{
	   if ( ErrMsg )
	   {
	      ReportParseError( ErrMsg, Tokens, pCommandLine);
	   }
	   break;
	}

	/* Report any errors accessing the token list as an internal error! */
	if ( Error )
	{
	   ReportError( ErrMsg );
	   break;
	}

	if ( LastCheck )
	{
	   ReturnValue = TRUE;
	   break;
	}

     } /* end-for */
  }
  break;
  default:
     ReportParseError(Expected_Volume_Option, Tokens, pCommandLine);
  } /* endswitch */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Partition_List_Parameters                       */
/*                                                                   */
/*   Descriptive Name:  This function corresponds to the non-terminal*/
/*                      'Partition_List_Parameters' found in the    */
/*                      grammar.                                     */
/*                      It parses the token list using the production*/
/*                      rules for the 'Partition_List_Parameters'   */
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
BOOLEAN Partition_List_Parameters(LIST Tokens, pPartitionListStruct pPartitionListParameters, pSTRING pCommandLine)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  pSTRING  pDriveName = NULL;
  pSTRING  pPartitionName = NULL;
  pSTRING  pDriveNumber = NULL;
  pSTRING  pSize = NULL;
  unsigned int TokenPosition;


  /* Get the current token so that we can examine it. */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( Error )
  {
     /* Report any errors accessing the token list as an internal error! */
     ReportError(Internal_Error);
     return ReturnValue;
  } /* endif */

  /* The next token may be a drive number or drive name */
  /* If the current token is a drive number */
  if ( Drive_Number(Tokens, &pDriveNumber) ) {
     pPartitionListParameters->NameType = DriveNumberType;
     pPartitionListParameters->pDrive = pDriveNumber;
  } else {
     /* It wasn't a drive number and it is a drive name */
     if ( Drive_Name(Tokens, Comma, &pDriveName) ) {
        pPartitionListParameters->NameType = DriveNameType;
        pPartitionListParameters->pDrive = pDriveName;
     } else {
        ReportParseError(Expected_Drive_Number_or_Name, Tokens, pCommandLine);
        return ReturnValue;
     } /* endif */
  } /* endif */

  SkipOptionalWhitespace(Tokens);
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( Error ) {
     /* Report any errors accessing the token list as an internal error! */
     ReportError(Internal_Error);
     return ReturnValue;
  } /* endif */

  /* The token following the drive number or drive name must be a comma */
  if (  IsComma(Tokens) ) {
     SkipOptionalWhitespace(Tokens);
     GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
     if ( Error) {
        /* Report any errors accessing the token list as an internal error! */
        ReportError(Internal_Error);
        return ReturnValue;
     } /* endif */


     /* The next token(s) must be a partition name */
     if ( Partition_Name(Tokens, Comma, &pPartitionName) ) {
        /* A valid set of partition list parameters has been found */
        pPartitionListParameters->pPartitionName = pPartitionName;
        ReturnValue = TRUE;
     } else {
        ReportParseError(Expected_Partition_Name, Tokens, pCommandLine);
     } /* endif */
  } else {
     ReportParseError(Expected_Comma, Tokens, pCommandLine);
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ReturnValue;

}

