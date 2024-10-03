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
 * Module: parser.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: CARDINAL32 Parse_String
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
#include "lvm_cli.h"
#include "list.h"          /* LIST, NextToken, GetToken, GoToStartOfTokenList */
#include "lvmcli.h"
#include "error.h"         /* ReportError */
#include "scanner.h"       /* Token, TokenTypes, EliminateTokenList */
#include "screener.h"
#include "command.h"
#include "utility.h"
#include "debug.h"
#include "names.h"
#include "cmdfile.h"
#include "install.h"

Feature_Information_Array pAvailableFeatureArray;

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

static CARDINAL32 Analyze_Tokens(LIST Tokens, pCommandStruct * p_pFirstCommand, pSTRING pCommandLine,
                                 LVMCLI_BackEndToVIO* pVIO_Request );
static CARDINAL32 Command_Line_Parameters(LIST Tokens, pCommandStruct * p_pFirstCommand, pSTRING pCommandLine,
                                 LVMCLI_BackEndToVIO* pVIO_Request );
static void FreeCommandMemory(pCommandStruct pFirstCommand);
static void FreePartitionListMemory(pPartitionListStruct  pFirstPartitionList);

/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/


/* Execute the parsed command line using the LVM engine. */
extern CARDINAL32 ExecuteCommands( pCommandStruct pFirstCommand, LVMCLI_BackEndToVIO* pVIO_Request );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Parse_String                                     */
/*                                                                   */
/*   Descriptive Name: Parses a single string containing one or more */
/*                     commands.                                     */
/*                                                                   */
/*   Input: LIST Tokens - The list of tokens generated by the        */
/*                           scanner.                                */
/*                                                                   */
/*   Output: CARDINAL  Return Code.                                  */
/*                                                                   */
/*   Error Handling: If an error occurs, all memory allocated by this*/
/*                   function is freed, the token list is freed, an  */
/*                   error message is output to the user using the   */
/*                   ReportError function in ERROR.H, and the        */
/*                   function return value will be FALSE.            */
/*                                                                   */
/*   Notes:  This function is the entry point to a standard recursive*/
/*           descent parser.                                         */
/*                                                                   */
/*********************************************************************/
CARDINAL32 Parse_String(pSTRING pCommandLine, LVMCLI_BackEndToVIO* pVIO_Request )
{
   LIST             TokenList; /* The LIST of tokens generated by the scanner */
   CARDINAL32       ExitValue = LVM_Error;
   pCommandStruct   pFirstCommand = NULL; /* Pointer to first command */
   CARDINAL32       LVMError;


  Open_LVM_Engine2 ( FALSE, VIO_Interface, &LVMError);
  if ( LVMError )
  {
    ReportError2( MRIEngine_OpenFail, LVMError );
  }
  else
  {

     pAvailableFeatureArray = Get_Available_Features( &LVMError );
     if ( LVMError )
     {
  ReportError2( MRIEngine_OpenFail, LVMError );
     }
     else
     {
  /* Now we must tokenize the reconstructed command line. */
  TokenList = ScanCommandLine(pCommandLine);
  if ( TokenList )
  {
     /* Since we successfully tokenized the command line, now we can recharacterize the tokens. */
     if ( ScreenTokenList(TokenList) == TRUE ) {
        /* Since we successfully recharacterized the tokens, now we can parse them. */
        if ( (ExitValue = Analyze_Tokens(TokenList, &pFirstCommand, pCommandLine, pVIO_Request )) == LVM_Successful ) {
     ExitValue = ExecuteCommands( pFirstCommand, pVIO_Request );
        } /* endif */
        /* Free up any memory allocated to store the commands */
        FreeCommandMemory(pFirstCommand);
     } /* endif */
  } /* endif */
     }
     Close_LVM_Engine();
  }
  return ExitValue;
}

/*--------------------------------------------------
 * Local Functions Available
 --------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Analyze_Tokens                                   */
/*                                                                   */
/*   Descriptive Name: Analyzes a list of tokens generated by the    */
/*                     scanner/screener and reduces the detected     */
/*                     commands into a linked list of structures     */
/*                     that will be used to execute them.            */
/*                                                                   */
/*   Input: LIST Tokens - The list of tokens generated by the        */
/*                           scanner.                                */
/*                                                                   */
/*   Output: If Success : The function return value will be TRUE and */
/*                        the global data structure ModeData will be */
/*                        filled in.  Also, Tokens will be freed.    */
/*                                                                   */
/*           If Failure : The function return value will be FALSE    */
/*                        and an error message will be displayed to  */
/*                        the user using the ReportError function    */
/*                        described in error.h.                      */
/*                                                                   */
/*   Error Handling: If an error occurs, all memory allocated by this*/
/*                   function is freed, the token list is freed, an  */
/*                   error message is output to the user using the   */
/*                   ReportError function in error.h, and the        */
/*                   function return value will be FALSE.            */
/*                                                                   */
/*   Side Effects: The global data structure ModeData is filled in   */
/*                 according to the contents of the command line.    */
/*                 If the function does not complete successfully,   */
/*                 then ModeData will not have all fields filled.    */
/*                 The token list is always freed by this function.  */
/*                                                                   */
/*   Notes:  This function is the entry point to a standard recursive*/
/*           descent parser.                                         */
/*                                                                   */
/*********************************************************************/
CARDINAL32 Analyze_Tokens(LIST Tokens, pCommandStruct * p_pFirstCommand, pSTRING pCommandLine,
                          LVMCLI_BackEndToVIO* pVIORequest )
{
  Token     CurrentToken;
  CARDINAL  Error = 0;
  CARDINAL32   ExitValue = LVM_Error;
  unsigned int TokenPosition;


  /* We must start at the beginning of the token list, and we will NOT assume
     that the current item in the list is the first item in the list.         */
  GoToStartOfTokenList(Tokens,&Error);
  if ( !Error )
  {
    /* Now that the current item in the list is the first item in the list,
       lets get the first token so that we can examine it.                    */
    GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
    if ( !Error )
    {
      /* The SkipOptionalWhitespace routine advances past all the tokens */
      /* that make up any contiguos whitespace at the beginning of the   */
      /* command line. */
      SkipOptionalWhitespace(Tokens);

      /* if there was something other than whitespace on the command line */
      if ( !IsEof(Tokens) ) {
         /* Check for the keyword LVM just in case it was prepended to a line */
         /* that was read in from a file.  If it is found, skip it and any    */
         /* whitespace following it. */
         if (CurrentToken.TokenType == LVM) {
            /* Skip over the LVM keyword */
            NextToken(Tokens, &Error);
            SkipOptionalWhitespace(Tokens);
         } /* endif */

         /* Call the routine that parses Command Line Parameters */
         if ( (ExitValue = Command_Line_Parameters(Tokens, p_pFirstCommand, pCommandLine, pVIORequest )) ==
              LVM_Successful ) {
            SkipOptionalWhitespace(Tokens);
            GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
            if ( Error) {
               /* Report any errors accessing the token list as an internal error! */
               ReportError(Internal_Error);
               return LVM_Error;
            } /* endif */

            if (CurrentToken.TokenType == Eof) {
               ExitValue = LVM_Successful;
            } else {
               if (CurrentToken.TokenType == SemiColon) {
                  ExitValue = LVM_Successful;
                  /* Skip over the semicolon */
                  NextToken(Tokens, &Error);
                  /* Now check for any additional non-whitespace tokens. */
                  /* No additional commands are allowed following the    */
                  /* Install or File commands. */
                  SkipOptionalWhitespace(Tokens);
                  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
                  if ( Error ) {
                     /* Report any errors accessing the token list as an internal error! */
                     ReportError(Internal_Error);
                     return LVM_Error;
                  } /* endif */

                  if (CurrentToken.TokenType == Eof) {
                     ExitValue = LVM_Successful;
                  } else {
                     ReportParseError(Expected_EOL, Tokens, pCommandLine);
                  } /* endif */
               } else {
                  /* The first token following the last successful command */
                  /* was not a semicolon or an Eof */
                  ReportParseError(Expected_Semicolon, Tokens, pCommandLine);
               } /* endif */
            } /* endif */
         } /* endif */
      } else {
         ReportParseError(Expected_LVM_Command, Tokens, pCommandLine);
      } /* endif */
    } /* endif */
  }  /* endif */

  /* Check for errors during LIST operations. */
  if ( Error )
    ReportError(Internal_Error);  /* Report the error. */

  /* Eliminate the token list regardless of whether or not there were errors. */
  DeleteTokenList(&Tokens);

  return ExitValue;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Command_Line_Parameters                          */
/*                                                                   */
/*   Descriptive Name: This function corresponds to the non-terminal */
/*                     'Command_Line_Parameters'found in the grammar.*/
/*                                                                   */
/*   Input: LIST * Tokens - The list of tokens created by the scanner*/
/*                          from the command line line.  Any leading */
/*                          whitespace must have already been        */
/*                          skipped over such that the first token   */
/*                          this routine encounters will not be      */
/*                          a token that is any type of whitespace.  */
/*                                                                   */
/*   Output: If Success : The function return value will be Success. */
/*                        The current item in the Tokens LIST will   */
/*                        be the next item to parse.                 */
/*                                                                   */
/*           If Failure : The function return value will be Error.   */
/*                        The current item in the Tokens LIST will   */
/*                        be the unrecognized token. The ReportError */
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
static CARDINAL32 Command_Line_Parameters(LIST Tokens, pCommandStruct * p_pFirstCmd, pSTRING pCommandLine,
                                          LVMCLI_BackEndToVIO* pVIO_Request )
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  CARDINAL32  ExitValue = LVM_Error;  /* Used to hold the exit value. */

  pCommandStruct    pFirstCmd = NULL;     /* Pointer to first command */
  pCommandStruct    pNextCmd = NULL;      /* Pointer to Next command */
  pCommandStruct    pPreviousCmd = NULL;  /* Pointer to Previous command */
  unsigned int      TokenPosition;
  pSTRING           pFileName = NULL;     /* Pointer to buffer allocated by File_Name */
  pInstallStruct    pInstallParameters = NULL;


  /* Allocate a Command structure to hold the first command */
  pFirstCmd = (pCommandStruct) malloc(sizeof(CommandStruct));
  if (pFirstCmd != NULL) {
     /* Initialize the CommandType field to indicate no command has been found yet */
     pFirstCmd->CommandType = NoCmd;

     /* There are no subsequent commands yet */
     pFirstCmd->pCommandData = NULL;
     pFirstCmd->pNextCommand = NULL;

    /* Pass back the address of the first structure allocated to store a command */
    *p_pFirstCmd = pFirstCmd;
  } else {
     /* Out of memory */
     ReportError(Not_Enough_Memory);
     return ExitValue;
  } /* endif */

  /* The assumption is that all contigous spaces and/or tabs  has been skipped */
  /* and the current token is first non-whitespace on the command line         */
  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
  if ( !Error ) {
     switch (CurrentToken.TokenType) {
     case File:
        /* Mark this command structure as the File command */
        pFirstCmd->CommandType = FileCmd;

        /* Skip over the keyword */
        NextToken(Tokens, &Error);
        if ( !Error ) {
           SkipOptionalWhitespace(Tokens);
           /* If the first non-whitespace token following the /File command is a colon */
           if ( IsColon(Tokens) ) {
              SkipOptionalWhitespace(Tokens);
              /* If the first non-whitespace token following the colon is a valid file name */
              if ( File_Name(Tokens, SemiColon, &pFileName) ) {
                 /* The data at pFileName was dynamically allocated by File_Name */
                 pFirstCmd->pCommandData = (ADDRESS)pFileName;

                 /* If the token following the /File keyword is a valid */
                 /* OS/2 filename, then open it, read the lines one at  */
                 /* a time, and recursively call this program's parser  */
                 /* entry point (i.e. Parse_String) with the line as    */
                 /* the command line argument. */
                 ExitValue = Process_Command_File(pFirstCmd, pVIO_Request);
              } else {
                 ReportParseError(Expected_File_Name, Tokens, pCommandLine);
              } /* endif */
           } else {
              ReportParseError(Expected_Colon, Tokens, pCommandLine);
           } /* endif */
        } /* endif */
        break;

     case SI:
        pFirstCmd->CommandType = SICmd;
        NextToken(Tokens,&Error);
        if ( !Error ) {
           SkipOptionalWhitespace(Tokens);
           /* If the first non-whitespace token following the /SI command is a colon */
           if ( IsColon(Tokens) ) {
              SkipOptionalWhitespace(Tokens);
              pInstallParameters = (pInstallStruct) malloc(sizeof(InstallStruct));
              if (pInstallParameters != NULL) {
                 pFirstCmd->pCommandData = (ADDRESS)pInstallParameters;
                 /* If the first non-whitespace tokens following the colon are valid install parameters */
                 if ( Install_Parameters(Tokens, pInstallParameters, pCommandLine) ) {
                    ExitValue = LVM_Successful;
                 } /* endif */
              } else {
                 ReportError(Not_Enough_Memory);
              } /* endif */
           } else {
              ReportParseError(Expected_Colon, Tokens, pCommandLine);
           } /* endif */
        } /* endif */
        break;

     default:
        /* Since the first token was not the /SIl keyword nor the */
        /* the /File keyword, check for any other valid command. */

        /* Pass the address of the first command for the first call to <Command> */
        pNextCmd = pFirstCmd;

        /* Check for at least one LVM command sequence */
        while ( Command(Tokens, pNextCmd, pCommandLine) ) {
           /* Save the address of next comand as the previous command */
           /* in preparation for allocating a new next command. */
           pPreviousCmd = pNextCmd;

           SkipOptionalWhitespace(Tokens);
           GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);
           if ( !Error ) {
              /* If the next token is a semicolon, then continue looking */
              /* for another command */
              if (CurrentToken.TokenType == SemiColon) {
                 NextToken(Tokens,&Error);
                 SkipOptionalWhitespace(Tokens);

                 /* Allocate the structure into which the a new next command is to be stored */
                 /* Set the pNextCmd value to NULL so we can detect any allocation error */
                 pNextCmd = NULL;
                 pNextCmd = (pCommandStruct) malloc(sizeof(CommandStruct));
                 if (pNextCmd != NULL) {
                    /* Store the new address into the pNextCmd field of */
                    /* the previous command structure. */
                    pPreviousCmd->pNextCommand = pNextCmd;

                    /* Initialize the CommandType field to indicate no command has been found yet */
                    pNextCmd->CommandType = NoCmd;

                    /* There are no subsequent commands yet */
                    pNextCmd->pCommandData = NULL;
                    pNextCmd->pNextCommand = NULL;
                 } else {
                    ReportError(Not_Enough_Memory);
                    break;
                 } /* endif */
                 /* Continue the search for more valid commands */
                 continue;
              } else {
                 /* The last call to Command was successful and the token following */
                 /* the last successful command is NOT a semicolon.  Therefore, we  */
                 /* finished looking for commands because we have either reached    */
                 /* the end of the command line or we have an error. */

                 /* If the next token is Eof, the command line ended properly */
                 if ( IsEof(Tokens) ) {
                    ExitValue = LVM_Successful;
                 } else {
                    ReportParseError(Expected_Semicolon_or_EOL, Tokens, pCommandLine);
                 } /* endif */
                 break;
              } /* endif */
           } /* endif */
        } /* endwhile */
     } /* endswitch */
  } /* endif */

  /* Report any errors accessing the token list as an internal error! */
  if ( Error )
    ReportError(Internal_Error);

  return ExitValue;

}

void FreeCommandMemory(pCommandStruct pCurrentCommand)
{
   pPartitionStruct  pPartitionParameters;
   pVolumeStruct     pVolumeParameters;
   pExpandStruct     pExpandParameters;
   pDeletionStruct   pDeletionParameters;
   pCreationStruct   pCreationParameters;
   pQueryStruct      pQueryParameters;
   pNameChangeStruct pNameChangeParameters;
   pSetStartableStruct pSetStartableParameters;
   pInstallStruct    pInstallParameters;
   pDriveLetterStruct pDriveLetterParameters;

   if (pCurrentCommand != NULL) {
      if (pCurrentCommand->pNextCommand != NULL) {
         /* This is not the last command, we must free the next one first */

         /* We must actually free the command structures in reverse order. */
         /* i.e. The last one first, then the previous one, etc. until the */
         /* we have worked our way back to the first one. */

         /* Even though the command structures only contains forward references, */
         /* calling this routine recursively allows us work our way down to the  */
         /* last one before actually starting the deallocation of list members.  */
         FreeCommandMemory(pCurrentCommand->pNextCommand);

         /* Now that the next member of the list and all its decendants have */
         /* been freed, we can free the current one. */
      } /* endif */

      /* Free the allocated memory for this command, we have already */
      /* deallocated all its decendants. */

      /* If the pCommandData field is not NULL, then there may */
      /* be a substructure to deallocate. */
      if (pCurrentCommand->pCommandData != NULL) {
         /* Depending of the command represented by this entry, there may be dynamically */
         /* allocated substructures when optional data is present. */
         switch (pCurrentCommand->CommandType) {
         case NoCmd:
            break;

         case BootmgrCmd:
            /* For a Bootmgr command, the pCommandData field points to a number */
            /* which was dynamically allocated by the Command routine. */
            if (pCurrentCommand->pCommandData != NULL) {
               free(pCurrentCommand->pCommandData);
            } /* endif */
            break;

         case CreateCmd:
            if ( pCurrentCommand->pCommandData != NULL) {
               pCreationParameters = (pCreationStruct)pCurrentCommand->pCommandData;

               switch ( pCreationParameters->OptionType ) {
               case Create_Partition_Option:
                  pPartitionParameters = (pPartitionStruct)pCreationParameters->pParameters;
                  if (pPartitionParameters != NULL) {
                     if ( pPartitionParameters->pPartitionName != NULL) {
                        free( pPartitionParameters->pPartitionName );
                     } /* endif */

                     if ( pPartitionParameters->pDrive != NULL) {
                        free( pPartitionParameters->pDrive );
                     } /* endif */

                     if ( pPartitionParameters->pSize != NULL) {
                        free( pPartitionParameters->pSize );
                     } /* endif */

                     if ( pPartitionParameters->pFreespaceID != NULL) {
                        free( pPartitionParameters->pFreespaceID );
                     } /* endif */
                  } /* endif */
                  break;
               case Create_Volume_Option:
                  pVolumeParameters = (pVolumeStruct)pCreationParameters->pParameters;
                  if (pVolumeParameters != NULL) {
                     if ( pVolumeParameters->pDriveLetter != NULL) {
                        free( pVolumeParameters->pDriveLetter );
                     } /* endif */

                     if ( pVolumeParameters->pVolumeName != NULL) {
                        free( pVolumeParameters->pVolumeName );
                     } /* endif */

                     FreePartitionListMemory( pVolumeParameters->pPartitionListParameters );
                  } /* endif */
                  break;
               } /* endswitch */

               free( pCreationParameters );
            } /* endif */
            break;

         case DeleteCmd:
            if ( pCurrentCommand->pCommandData != NULL) {
               pDeletionParameters = (pDeletionStruct)pCurrentCommand->pCommandData;

               /* Free the fields that point to allocated memory */
               if ( pDeletionParameters->pDrive != NULL) {
                  free( pDeletionParameters->pDrive);
               } /* endif */

               if ( pDeletionParameters->pPartitionName != NULL) {
                  free( pDeletionParameters->pPartitionName);
               } /* endif */

               if ( pDeletionParameters->pVolumeName != NULL) {
                  free( pDeletionParameters->pVolumeName);
               } /* endif */

               free( pDeletionParameters );
            } /* endif */
            break;

         case ExpandCmd:
            if ( pCurrentCommand->pCommandData != NULL) {
               pExpandParameters = (pExpandStruct)pCurrentCommand->pCommandData;

               if ( pExpandParameters->pVolumeName != NULL) {
                  free( pExpandParameters->pVolumeName );
               } /* endif */

               FreePartitionListMemory( pExpandParameters->pPartitionListParameters );

               free ( pExpandParameters );
            } /* endif */
            break;

         case FileCmd:
            /* For a File command, the pCommandData field points to a file name */
            /* which was dynamically allocated by the File_Name routine. */
            if (pCurrentCommand->pCommandData != NULL) {
               free(pCurrentCommand->pCommandData);
            } /* endif */
            break;

         case HideCmd:
            /* For a Hide command, the pCommandData field points to a volume name */
            /* which was dynamically allocated by the Volume_Name routine. */
            if (pCurrentCommand->pCommandData != NULL) {
               free(pCurrentCommand->pCommandData);
            } /* endif */
            break;

         case NewMBR:
            /* For a NewMBR command, the pCommandData field points to a drive    */
            /* number that was dynamically allocated by the  Drive_Number routine. */
            if (pCurrentCommand->pCommandData != NULL) {
               free(pCurrentCommand->pCommandData);
            } /* endif */
            break;

         case QueryCmd:
            if ( pCurrentCommand->pCommandData != NULL) {
               pQueryParameters = (pQueryStruct)pCurrentCommand->pCommandData;

               if ( pQueryParameters->pName != NULL) {
                  free( pQueryParameters->pName );
               } /* endif */

               if ( pQueryParameters->pFileSystemType != NULL) {
                  free( pQueryParameters->pFileSystemType );
               } /* endif */

               free( pQueryParameters );
            } /* endif */
            break;

         case SetNameCmd:
            if ( pCurrentCommand->pCommandData != NULL) {
               pNameChangeParameters = (pNameChangeStruct)pCurrentCommand->pCommandData;

               if ( pNameChangeParameters->pDrive != NULL ) {
                  free( pNameChangeParameters->pDrive );
               } /* endif */

               if ( pNameChangeParameters->pOldName != NULL ) {
                  free( pNameChangeParameters->pOldName );
               } /* endif */

               if ( pNameChangeParameters->pNewName != NULL ) {
                  free( pNameChangeParameters->pNewName );
               } /* endif */

               free( pNameChangeParameters );
            } /* endif */
            break;

         case SetStartableCmd:
            /* For a SetStartable command, the pCommandData field points to a volume*/
            /* name which was dynamically allocated by the Volume_Name routine, */
            /* a partition name that was dynamically allocated by the Partition_Name */
            /* routine, and a drive name/number that was dynmically allocated by */
            /* the Drive_Number or Drive_Name routine, respectively. */
            if ( pCurrentCommand->pCommandData != NULL) {
               pSetStartableParameters = pCurrentCommand->pCommandData;

               if ( pSetStartableParameters->pDrive != NULL ) {
                  free( pSetStartableParameters->pDrive );
               } /* endif */

               if ( pSetStartableParameters->pPartitionName != NULL ) {
                  free( pSetStartableParameters->pPartitionName );
               } /* endif */

               if ( pSetStartableParameters->pVolumeName != NULL ) {
                  free( pSetStartableParameters->pVolumeName );
               } /* endif */

               free( pSetStartableParameters );
            } /* endif */
            break;

         case SICmd:
            if ( pCurrentCommand->pCommandData != NULL) {
               pInstallParameters = pCurrentCommand->pCommandData;

               if ( pInstallParameters->pSize != NULL) {
                  free( pInstallParameters->pSize );
               } /* endif */

               free ( pInstallParameters );
            } /* endif */
            break;
         case StartLogCmd:
            /* For a StartLog command, the pCommandData field points to a file name */
            /* which was dynamically allocated by the File_Name routine. */
            if (pCurrentCommand->pCommandData != NULL) {
               free(pCurrentCommand->pCommandData);
            } /* endif */
            break;
         case DriveLetterCmd:
            /* For a DriveLetter command, the pCommandData field points to a */
            /* structure of type DriveLetterStruct that has been dynamically */
            /* allocated. */
            if ( pCurrentCommand->pCommandData != NULL ) {
               pDriveLetterParameters = pCurrentCommand->pCommandData;

               if ( pDriveLetterParameters->pVolumeName != NULL ) {
                  free( pDriveLetterParameters->pVolumeName );
               } /* endif */

               if ( pDriveLetterParameters->pDriveLetter != NULL ) {
                  free( pDriveLetterParameters->pDriveLetter );
               } /* endif */

               free ( pDriveLetterParameters );
            } /* endif */
            break;
         case RediscoverPRMCmd:
            /* For a RediscoverPRM command, there is no command data */
            if (pCurrentCommand->pCommandData != NULL) {
               free(pCurrentCommand->pCommandData);
            } /* endif */
            break;

         default:
            break;
         } /* endswitch */
      } /* endif */

      /* Now that any dynamically allocated substructures have been freed, */
      /* free the main command structure itself. */
      free(pCurrentCommand);
   } /* endif */
}

void FreePartitionListMemory(pPartitionListStruct  pFirstPartitionList)
{
   pPartitionListStruct pPrevPartitionList;
   pPartitionListStruct pNextPartitionList;


   /* Make sure was haven't been passed an empty list */
   if (pFirstPartitionList != NULL) {
      /* If the Drive Name/Number field points to some dynamically */
      /* allocated memory, free it.
      if ( pFirstPartitionList->pDrive != NULL ) {
         free( pFirstPartitionList->pDrive );
      } /* endif */

      /* If the PartitionName field points to some dynamically */
      /* allocated memory, free it.
      if ( pFirstPartitionList->pPartitionName != NULL ) {
         free( pFirstPartitionList->pPartitionName );
      } /* endif */

      /* If there is more than one pair of Partition List Parameters */
      if ( pFirstPartitionList->pNext != NULL) {
         /* Save the address of the first partition list structure */
         /* in the pNextPartionList variable to set up the while   */
         /* loop which will process the remaining elements in the  */
         /* linked list. */
         pNextPartitionList = pFirstPartitionList;

         /* Free all the subsequent pairs of partition list parameters */
         while ( pNextPartitionList->pNext != NULL ) {
            /* Save the address of the current partition list structure */
            /* so it can be freed after we have retrieved the address */
            /* of the next structure in the linked list. */
            pPrevPartitionList = pNextPartitionList;

            /* Get the address of the next partition list structure in the */
            /* listed list from the pNext field. */
            pNextPartitionList = pNextPartitionList->pNext;

            /* Now that we have advanced to the next structure in the */
            /* linked list, it is OK to free the previous structure. */
            free( pPrevPartitionList );

            /* If the Drive Name/Number field points to some dynamically */
            /* allocated memory, free it.
            if ( pNextPartitionList->pDrive != NULL ) {
               free( pNextPartitionList->pDrive );
            } /* endif */

            /* If the PartitionName field points to some dynamically */
            /* allocated memory, free it.
            if ( pNextPartitionList->pPartitionName != NULL ) {
               free( pNextPartitionList->pPartitionName );
            } /* endif */
         } /* endwhile */
      } else {
         /* There was only one pair of partition list parameters, */
         /* go ahead and free the dynamically allocated partition */
         /* list structure. */
         free ( pFirstPartitionList );
      } /* endif */
   } /* endif */
}
