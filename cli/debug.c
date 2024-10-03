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
 * Module: debug.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: BOOLEAN PrintTokenType
 *
 *
 * Description:  This module implements the debug routines used by the LVM.EXE
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
#include <stdio.h>
#include "lvmcli.h"
#include "gbltypes.h"       /* BOOLEAN, CARDINAL, FALSE, TRUE */
#include "list.h"          /* LIST, NextToken, GetToken, GoToStartOfList */
#include "scanner.h"       /* Token, TokenTypes, EliminateTokenList */
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

/*--------------------------------------------------
 * Local Functions Available
 --------------------------------------------------*/

/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

void PrintTokenType(LIST Tokens)
{
  Token    CurrentToken;         /* Used to hold the token being examined. */
  CARDINAL Error = 0;            /* Used to hold the error return code from LIST operations. */
  BOOLEAN  ReturnValue = FALSE;  /* Used to hold the function return value. */
  unsigned int TokenPosition;

  GetToken(Tokens,sizeof(Token),&CurrentToken,&TokenPosition,&Error);

  switch (CurrentToken.TokenType) {
  case String:
    printf("Current Token Type is String '%s'\n",CurrentToken.pTokenText);
    break;
  case AcceptableCharsStr:
    printf("Current Token Type is AcceptableCharsStr '%s'\n",CurrentToken.pTokenText);
    break;
  case FileNameStr:
    printf("Current Token Type is FileNameStr '%s'\n",CurrentToken.pTokenText);
    break;
  case Separator  :
    printf("Current Token Type is Separator\n");
    break;
  case Number     :
    printf("Current Token Type is Number\n");
    break;
  case Space      :
    printf("Current Token Type is Space\n");
    break;
  case MultiSpace :
    printf("Current Token Type is MultiSpace\n");
    break;
  case Tab        :
    printf("Current Token Type is Tab\n");
    break;
  case MultiTab   :
    printf("Current Token Type is MultiTab\n");
    break;
  case Colon      :
    printf("Current Token Type is Colon\n");
    break;
  case SemiColon  :
    printf("Current Token Type is SemiColon\n");
    break;
  case Comma      :
    printf("Current Token Type is Comma\n");
    break;
  case Eof        :
    printf("Current Token Type is Eof\n");
    break;
  case Install    :
    printf("Current Token Type is Install\n");
    break;
  case All_CLI    :
    printf("Current Token Type is All\n");
    break;
  case BestFit    :
    printf("Current Token Type is BestFit\n");
    break;
  case BootDOS    :
    printf("Current Token Type is BootDos\n");
    break;
  case BootOS2    :
    printf("Current Token Type is BootOS2\n");
    break;
  case Bootable   :
    printf("Current Token Type is Bootable\n");
    break;
  case Bootmgr    :
    printf("Current Token Type is Bootmgr\n");
    break;
  case CR_CLI     :
    printf("Current Token Type is CR\n");
    break;
  case CRI        :
    printf("Current Token Type is CRI\n");
    break;
  case Compatibility:
    printf("Current Token Type is Compatibility\n");
    break;
  case Create     :
    printf("Current Token Type is Create\n");
    break;
  case Delete     :
    printf("Current Token Type is Delete\n");
    break;
  case Drive      :
    printf("Current Token Type is Drive\n");
    break;
  case Existing   :
    printf("Current Token Type is Existing\n");
    break;
  case Expand     :
    printf("Current Token Type is Expand\n");
    break;
  case FS         :
    printf("Current Token Type is FS\n");
    break;
  case File       :
    printf("Current Token Type is File\n");
    break;
  case FirstFit   :
    printf("Current Token Type is FirstFit\n");
    break;
  case Freespace  :
    printf("Current Token Type is Freespace\n");
    break;
  case FromLargest:
    printf("Current Token Type is FromLargest\n");
    break;
  case FromSmallest :
    printf("Current Token Type is FromSmallest\n");
    break;
  case Hide       :
    printf("Current Token Type is Hide\n");
    break;
  case LVM        :
    printf("Current Token Type is LVM\n");
    break;
  case LastFit    :
    printf("Current Token Type is LastFit\n");
    break;
  case Logical    :
    printf("Current Token Type is Logical\n");
    break;
  case New        :
    printf("Current Token Type is New\n");
    break;
  case NewMBR     :
    printf("Current Token Type is NewMBR\n");
    break;
  case NoBoot     :
    printf("Current Token Type is NoBoot\n");
    break;
  case Partition  :
    printf("Current Token Type is Partition\n");
    break;
  case Primary    :
    printf("Current Token Type is Primary\n");
    break;
  case Query      :
    printf("Current Token Type is Query\n");
    break;
  case RB         :
    printf("Current Token Type is RB\n");
    break;
  case SI         :
    printf("Current Token Type is SI\n");
    break;
  case SetName    :
    printf("Current Token Type is SetName\n");
    break;
  case SetStartable:
    printf("Current Token Type is SetStartable:\n");
    break;
  case Size       :
    printf("Current Token Type is Size\n");
    break;
  case SlashSize  :
    printf("Current Token Type is SlashSize\n");
    break;
  case Unusable   :
    printf("Current Token Type is Unusable\n");
    break;
  case Unused   :
    printf("Current Token Type is Unused\n");
    break;
  case Volume     :
    printf("Current Token Type is Volume\n");
    break;
  case Volumes    :
    printf("Current Token Type is Volumes\n");
    break;
  default:
    printf("Current Token Type is NOT RECOGNIZED\n");
    break;
  } /* endswitch */
}

BOOLEAN PrintCommands(pCommandStruct pCurrentCommand)
{
   char  CommandStr[260];

   while (pCurrentCommand != NULL) {
      switch (pCurrentCommand->CommandType) {
      case NoCmd:
         /* printf("There is no command stored in this entry\n"); */
         break;
      case CreateCmd:
         strcpy(CommandStr,"Command=Create, Option=");
         if (pCurrentCommand->pCommandData != NULL) {
            switch ( ((pCreationStruct)pCurrentCommand->pCommandData)->OptionType ) {
            case Create_Partition_Option:
               strcat(CommandStr, "Partition");
               if ( ((pCreationStruct)pCurrentCommand->pCommandData)->pParameters != NULL ) {
                  strcat(CommandStr, ", Partition=");
                  if ( ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->pPartitionName != NULL ) {
                     strcat(CommandStr, ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->pPartitionName);
                  } else {
                     strcat(CommandStr, "Error");
                  } /* endif */

                  if ( ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->pDrive != NULL ) {
                     if ( ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->NameType == DriveNumberType ) {
                        strcat(CommandStr, ", Drive Number=");
                     } else {
                        if ( ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->NameType == DriveNameType ) {
                           strcat(CommandStr, ", Drive Name=");
                        } else {
                           strcat(CommandStr, ", Drive(Error)=");
                        } /* endif */
                     } /* endif */
                     strcat(CommandStr, ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->pDrive);
                  } else {
                     strcat(CommandStr, ", Drive=Error");
                  } /* endif */

                  strcat(CommandStr, ", Size=");
                  if ( ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->pSize != NULL ) {
                     strcat(CommandStr, ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->pSize);
                  } else {
                     strcat(CommandStr, "Error");
                  } /* endif */

                  strcat(CommandStr, ", Option=");
                  switch ( ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->OptionType ) {
                  case Partition_Logical:
                     strcat(CommandStr, "Logical");
                     break;
                  case Partition_Primary:
                     strcat(CommandStr, "Primary");
                     break;
                  default:
                     strcat(CommandStr, "Error");
                    break;
                  } /* endswitch */

                  strcat(CommandStr, ", AllocationOption=");
                  switch ( ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->AllocationOptionType ) {
                  case Allocation_BestFit:
                     strcat(CommandStr, "BestFit");
                     break;
                  case Allocation_FirstFit:
                     strcat(CommandStr, "FirstFit");
                     break;
                  case Allocation_LastFit:
                     strcat(CommandStr, "LastFit");
                     break;
                  case Allocation_FromLargest:
                     strcat(CommandStr, "FromLargest");
                     break;
                  case Allocation_FromSmallest:
                     strcat(CommandStr, "FromSmallest");
                     break;
                  case Allocation_None:
                     strcat(CommandStr, "none specified");
                     break;
                  case Allocation_FreespaceID:
                     strcat(CommandStr, "Freespace ID(");
                     if ( ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->pFreespaceID != NULL ) {
                        strcat(CommandStr, ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->pFreespaceID);
                        strcat(CommandStr, ")");
                     } else {
                        strcat(CommandStr, "Error)");
                     } /* endif */
                     break;
                  default:
                     strcat(CommandStr, "Error");
                    break;
                  } /* endswitch */
                  strcat(CommandStr,", Suboption=");
                  switch ( ((pPartitionStruct)((pCreationStruct)pCurrentCommand->pCommandData)->pParameters)->AllocationSubOptionType ) {
                  case Allocation_FromStart:
                     strcat(CommandStr, "FromStart");
                     break;
                  case Allocation_FromEnd:
                     strcat(CommandStr, "FromEnd");
                     break;
                  case Allocation_FromNone:
                     strcat(CommandStr, "none specified");
                     break;
                  default:
                     strcat(CommandStr, "Error");
                    break;
                  } /* endswitch */
               } else {
                  strcat(CommandStr, "Error");
               } /* endif */
               break;
            case Create_Volume_Option:
               strcat(CommandStr, "Volume");
               if ( ((pCreationStruct)pCurrentCommand->pCommandData)->pParameters != NULL ) {
                  ;
               } else {
                  strcat(CommandStr, "Error");
               } /* endif */
               break;
            case Create_Error_Option:
            default:
               strcat(CommandStr, "Error");
               break;
            } /* endswitch */
         } else {
            strcat(CommandStr, "Error");
         } /* endif */
         printf("%s\n",CommandStr);
         break;
      case BootmgrCmd:
         strcpy(CommandStr,"Command=Bootmgr, Drive=");
         if ( pCurrentCommand->pCommandData != NULL) {
            strcat(CommandStr, (char *)pCurrentCommand->pCommandData);
         } else {
            strcat(CommandStr, "none specified");
         } /* endif */
         printf("%s\n",CommandStr);
         break;
      case DeleteCmd:
         strcpy(CommandStr, "Command=Delete");
         strcat(CommandStr, ", Drive=");
         if ( pCurrentCommand->pCommandData != NULL) {
            switch (((pDeletionStruct)pCurrentCommand->pCommandData)->NameType) {
            case DriveNameType:
            case DriveNumberType:
               strcat(CommandStr, ((pDeletionStruct)pCurrentCommand->pCommandData)->pDrive);
               break;
            case NoNameType:
               strcat(CommandStr, "none specified");
               break;
            default:
               strcat(CommandStr, " ?");
              break;
            } /* endswitch */
         } else {
            strcat(CommandStr, " ?");
         } /* endif */

         strcat(CommandStr, ", Option=");
         switch (((pDeletionStruct)pCurrentCommand->pCommandData)->DeletionOption) {
         case Deletion_All_Compatibility:
            strcat(CommandStr, "Compatibility");
            break;
         case Deletion_All_Logical:
            strcat(CommandStr, "Logical");
            break;
         case Deletion_All_LVM:
            strcat(CommandStr, "LVM");
            break;
         case Deletion_All_Primary:
            strcat(CommandStr, "Primary");
            break;
         case Deletion_All_Unused:
            strcat(CommandStr, "Unused");
            break;
         case Deletion_All_Volumes:
            strcat(CommandStr, "Volumes");
            break;
         default:
            strcat(CommandStr, "?");
            break;
         } /* endswitch */
         printf("%s\n",CommandStr);
         break;
      case ExpandCmd:
         printf("Command=Expand\n");
         break;
      case FileCmd:
         strcpy(CommandStr,"Command=File, FileName=");
         if ( pCurrentCommand->pCommandData != NULL) {
            strcat(CommandStr, pCurrentCommand->pCommandData);
         } else {
            strcat(CommandStr, "none specified");
         } /* endif */
         printf("%s\n",CommandStr);
         break;
      case HideCmd:
         strcpy(CommandStr,"Command=Hide, VolumeName=");
         if ( pCurrentCommand->pCommandData != NULL) {
            strcat(CommandStr, pCurrentCommand->pCommandData);
         } else {
            strcat(CommandStr, "none specified");
         } /* endif */
         printf("%s\n",CommandStr);
         break;
      case NewMBRCmd:
         strcpy(CommandStr,"Command=NewMBR");
         if ( pCurrentCommand->pCommandData != NULL) {
            switch ( ((pNewMBRStruct)pCurrentCommand->pCommandData)->NameType ) {
            case DriveNameType:
               strcat(CommandStr, ", DriveName=");
               break;
            case DriveNumberType:
               strcat(CommandStr, ", DriveNumber=");
               break;
            default:
               strcat(CommandStr, ", Error=");
              break;
            } /* endswitch */
            if (((pNewMBRStruct)pCurrentCommand->pCommandData)->pName != NULL) {
               strcat(CommandStr, ((pNewMBRStruct)pCurrentCommand->pCommandData)->pName);
            } else {
               strcat(CommandStr, "Error");
            } /* endif */
         } else {
            strcat(CommandStr, "no target specified");
         } /* endif */
         printf("%s\n",CommandStr);
         break;
      case QueryCmd:
         strcpy(CommandStr,"Command=Query");
         if ( pCurrentCommand->pCommandData != NULL) {
            strcat(CommandStr, ", Option=");
            switch ( ((pQueryStruct)pCurrentCommand->pCommandData)->OptionType ) {
            case Query_All:
               strcat(CommandStr, "All");
               break;
            case Query_Bootable:
               strcat(CommandStr, "Bootable");
               break;
            case Query_Compatibility:
               strcat(CommandStr, "Compatibility");
               break;
            case Query_Freespace:
               strcat(CommandStr, "Freespace");
               break;
            case Query_Logical:
               strcat(CommandStr, "Logical");
               break;
            case Query_LVM:
               strcat(CommandStr, "LVM");
               break;
            case Query_Primary:
               strcat(CommandStr, "Primary");
               break;
            case Query_Unusable:
               strcat(CommandStr, "Unusable");
               break;
            case Query_Volumes:
               strcat(CommandStr, "Volumes");
               break;
            default:
               strcat(CommandStr, "None Specified");
               break;
            } /* endswitch */

            strcat(CommandStr, ", Drive=");
            switch ( ((pQueryStruct)pCurrentCommand->pCommandData)->NameType ) {
            case AllNameType:
               strcat(CommandStr, "All");
               break;
            case DriveNameType:
               if ( ((pQueryStruct)pCurrentCommand->pCommandData)->pName != NULL) {
                  strcat(CommandStr, ((pQueryStruct)pCurrentCommand->pCommandData)->pName);
                  strcat(CommandStr, "(Name)");
               } /* endif */
               break;
            case DriveNumberType:
               if ( ((pQueryStruct)pCurrentCommand->pCommandData)->pName != NULL) {
                  strcat(CommandStr, ((pQueryStruct)pCurrentCommand->pCommandData)->pName);
                  strcat(CommandStr, "(Number)");
               } /* endif */
               break;
            case NoNameType:
               strcat(CommandStr, "None Specified");
               break;
            default:
               strcat(CommandStr, " ?");
              break;
            } /* endswitch */

            strcat(CommandStr, ", File System Type=");
            if ( ((pQueryStruct)pCurrentCommand->pCommandData)->pFileSystemType != NULL) {
               strcat(CommandStr, ((pQueryStruct)pCurrentCommand->pCommandData)->pFileSystemType);
            } else {
               strcat(CommandStr,"None Specified");
            } /* endif */
         } else {
            strcat(CommandStr, ", Option=none specified");
         } /* endif */
         printf("%s\n",CommandStr);
         break;
      case SetNameCmd:
         strcpy(CommandStr,"Command=SetName");
         if ( pCurrentCommand->pCommandData != NULL) {
            strcat(CommandStr, ", Option=");
            switch ( ((pNameChangeStruct)pCurrentCommand->pCommandData)->OptionType ) {
            case NameChange_Drive:
               strcat(CommandStr, "Drive");
               break;
            case NameChange_Partition:
               strcat(CommandStr, "Partition");
               break;
            case NameChange_Volume:
               strcat(CommandStr, "Volume");
               break;
            default:
               strcat(CommandStr, "Error");
               break;
            } /* endswitch */

            strcat(CommandStr, ", CurrentName=");
            if ( ((pNameChangeStruct)pCurrentCommand->pCommandData)->pOldName != NULL) {
               strcat(CommandStr, ((pNameChangeStruct)pCurrentCommand->pCommandData)->pOldName);
            } else {
               strcat(CommandStr, ", Error");
            } /* endif */

            strcat(CommandStr, ", NewName=");
            if ( ((pNameChangeStruct)pCurrentCommand->pCommandData)->pNewName != NULL) {
               strcat(CommandStr, ((pNameChangeStruct)pCurrentCommand->pCommandData)->pNewName);
            } else {
               strcat(CommandStr, ", Error");
            } /* endif */
         } else {
            strcat(CommandStr, ", Error");
         } /* endif */
         printf("%s\n",CommandStr);
         break;
      case SetStartableCmd:
         strcpy(CommandStr,"Command=SetStartable");
         if ( pCurrentCommand->pCommandData != NULL) {
            switch ( ((pSetStartableStruct)pCurrentCommand->pCommandData)->NameType) {
            case PartitionNameType:
               strcat(CommandStr, ", PartitionName=");
               break;
            case VolumeNameType:
               strcat(CommandStr, ", VolumeName=");
               break;
            default:
               strcat(CommandStr, ", Unknown Name Type=");
              break;
            } /* endswitch */
            strcat(CommandStr, ((pSetStartableStruct)pCurrentCommand->pCommandData)->pDrive );
         } else {
            strcat(CommandStr, " no target specified");
         } /* endif */
         printf("%s\n",CommandStr);
         break;
      case SICmd:
         strcpy(CommandStr,"Command=SI, Option=");
         if ( pCurrentCommand->pCommandData != NULL) {
            /* All the other options have been eliminated from the grammar */
            /* under defect 202485.   I will leave the code structured as  */
            /* a switch statement just in case someone whats to add any of */
            /* the options back. */
            switch ( ((pInstallStruct)pCurrentCommand->pCommandData)->OptionType) {
            case Install_FS:
               strcat(CommandStr, "FS size=");
               if ( ((pInstallStruct)pCurrentCommand->pCommandData)->pSize != NULL) {
                  strcat(CommandStr, ((pInstallStruct)pCurrentCommand->pCommandData)->pSize);
               } else {
                  strcat(CommandStr, "Error");
               } /* endif */
               break;
            default:
               strcat(CommandStr, "Error");
              break;
            } /* endswitch */
         } else {
            strcat(CommandStr, "Error");
         } /* endif */
         printf("%s\n",CommandStr);
         break;
      default:
         printf("This entry contains a invalid command type\n");
         break;
      } /* endswitch */

      if (pCurrentCommand->pNextCommand != NULL) {
         pCurrentCommand = pCurrentCommand->pNextCommand;
      } else {
         return TRUE;
      } /* endif */
   } /* end of while */

   printf("There were no commands to execute\n");
   return TRUE;
}
