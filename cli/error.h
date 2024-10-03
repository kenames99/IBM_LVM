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
 * Module: error.h
 */

/*
 * Change History:
 *
 */

/*
 * Functions: void ReportError
 *
 * Description:  This module implements a simple error reporting function.
 *
 */


#ifndef LVM_CLI_ERROR_H

#define LVM_CLI_ERROR_H 1


#define HF_STDOUT  1
#define HF_STDERR  2

#include "lvm_cli.h"
#include "list.h"

#define LVM_Successful  0
#define LVM_Error       1

#define MESSAGE_FILE             "LVM.MSG"
#define MAX_MESSAGE_SIZE         (1024 * 12)
#define MAX_LINE_SIZE            (80 * 3)

/* This type enumerates the numbers of the messages displayed by this subsystem */
/* The origin of the first entry in this enum list must match the message number */
/* of the first entry in the message text file (i.e. lvm.txt).  The order of the */
/* remaining entries much be the same as their order in the message text file.   */
typedef enum {
     Leader_Character = 400,
     Pointer_Character,
     Exceeded_Max_Cmdline,
     Expected_Spaces_or_Tabs,
     Expected_Colon,
     Expected_Comma,
     Expected_Semicolon,
     Expected_Number,
     Expected_Semicolon_or_EOL,
     Expected_Volume_Name,
     Expected_Partition_Name,
     Expected_Drive_Letter,
     Expected_Drive_Number,
     Expected_Drive_Name,
     Expected_File_Name,
     Expected_File_System_Type,
     Expected_Free_Space_ID,
     Expected_1_or_2,
     Expected_Drive_Number_or_Name,
     Expected_Partition_or_Volume_Name,
     Expected_ALL_or_Partition_Name,
     Expected_ALL_or_Drive_Number_or_Name,
     Expected_LVM_Command,
     Expected_Creation_Option,
     Expected_Partition_Option,
     Expected_Allocation_SubOption,
     Expected_Query_Option,
     Expected_Name_Change_Option,
     Expected_Compatibility_Option,
     Expected_Volume_Option,
     Expected_Partition_SubOption,
     Expected_SIZE,
     Expected_SIZE_or_EOL,
     Expected_SI_Option,
     Expected_Partition_List_Parms,
     Expected_Allocation_Option,
     Expected_Deletion_Option,
     Not_Enough_Memory,
     Internal_Error,
     Expected_EOL,
     Recursive_File_Cmd,
     File_Not_Found,
     File_Read_Error,
     File_Close_Error,
     Expected_Deletion_Suboption,
     Command_Failed,
     Expected_SetStartable_Option,
     Expected_Open_Paren,
     Expected_Close_Paren,
     Expected_Open_Bracket,
     Expected_Close_Bracket,
     Expected_Open_Brace,
     Expected_Close_Brace,
     Expected_Feature_Name,
     Expected_Feature_Parameters
} ParseErrorType;

/*********************************************************************/
/*                                                                   */
/*   Function Name: ReportError                                      */
/*                                                                   */
/*   Descriptive Name: Builds an error message and displays it to the*/
/*                     user.                                         */
/*                                                                   */
/*   Input: CARDINAL ErrorNumber : One of the error numbers defined  */
/*                                 below under Notes.                */
/*          pSTRING Parameter1 : The text to replace the %1 in the    */
/*                              error message with.                  */
/*          pSTRING Parameter2 : The text to replace the %2 in the    */
/*                              error message with.                  */
/*          pSTRING Parameter3 : The text to replace the %3 in the    */
/*                              error message with.                  */
/*                                                                   */
/*   Output: If Success : The requested error message is displayed   */
/*                        to the user.                               */
/*                                                                   */
/*           If Failure : The generic error message is displayed to  */
/*                        the user.                                  */
/*                                                                   */
/*   Error Handling: If an error occurs, then the generic error      */
/*                   message is displayed.  The generic error message*/
/*                   is: "An internal processing error has occured!" */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function uses the OS/2 message api.                 */
/*                                                                   */
/*          The following values for ErrorNumber are defined:        */
/*                                                                   */
/*            ( %1, %2, and %3 represent replaceable parameters. )   */
/*                                                                   */
/*            2 : SYS0008: There is not enough storage available to  */
/*                          process this command. All available      */
/*                          storage is in use.                       */
/*            3 : SYS1459: An internal error occurred.               */
/*           31 : SYS0005: Access is denied.                         */
/*                                                                   */
/*********************************************************************/
extern void ReportError(unsigned int ErrorNumber);
extern void ReportError2(unsigned int ErrorNumber, CARDINAL32 Error);
extern void ReportParseError(ParseErrorType ErrorNumber, LIST Tokens, pSTRING pCommandLine);
extern void ReportFeatureParseError( char *pMsg, LIST Tokens, pSTRING pCommandLine);


/*
 * LVM CLI NLV message numbers.  (Correspond to messages in lvm.txt.)
 * This type enumerates the numbers of the messages displayed by the CLI's backend.
 * The CLI backend performes the requested LVM engine operations after the CLI's
 * frontend parses the user's command line.
 * The origin of the first entry in this enum list must match the message number
 * of the first entry in the message text file (i.e. lvm.txt).  The order of the
 * remaining entries much be the same as their order in the message text file.
 *
 */
typedef enum
{
   MRIFirst_BackEnd = 600,
   MRIEngine_OpenFail,
   MRICreate_Enumeration,
   MRICreate_PartNoFreeSpace,
   MRICreate_PartNnoDrive,
   MRICreate_Part,
   MRICreate_VolNoPartList,
   MRICreate_CompatVol1Part,
   MRICreate_VolNoPart,
   MRICreate_Volume,
   MRISetName_VolumeEnumeration,
   MRISetName_DriveEnumeration,
   MRISetName_NoVolume,
   MRISetName_NoDrive,
   MRISetName_NoPartition,
   MRISetName_Failure,
   MRIDelete_Enumeration,
   MRIDelete_Volume,
   MRIDelete_Failure,
   MRISetStart_Enumeration,
   MRISetStart_EnumerateVolumes,
   MRISetStart_FindVolume,
   MRISetStart_BootCompatible,
   MRISetStart_NoPartOnVolume,
   MRISetStart_FirstMustBePrimary,
   MRISetStart_NoPartOnDrive,
   MRISetStart_MustBePrimary,
   MRISetStart_NoName,
   MRISetStart_PartStillActive,
   MRISetStart_PartStillInactive,
   MRINewMBR_InvalidPhysDrive,
   MRINewMBR_Enumeration,
   MRINewMBR_NoPhysDrive,
   MRINewMBR_WriteFail,
   MRIBootMgr_NoPhysDrive,
   MRIBootMgr_BadPhysDrive,
   MRIBootMgr_Enumeration,
   MRIBootMgr_FindPhysDrive,
   MRIBootMgr_FindDrive,
   MRISICmd_BadOption,
   MRIExpand_Enumeration,
   MRIExpand_DriveEnumeration,
   MRIExpand_NoVolume,
   MRIExpand_NotLVMVolume,
   MRIExpand_NoPartList,
   MRIExpand_NoPart,
   MRIHide_Enumeration,
   MRIHide_NoVolume,
   MRIReBoot_Required,
   MRISIOption_OneAtATime,
   MRIVolumeQuery_Header,
   MRIDriveQuery_Header,
   MRIPartitionQuery_Header1,
   MRIPartitionQuery_Header2,
   MRILVMError,
   MRIExpand_Volume,
   MRIHide_Volume,
   MRIStart_Logfile,
   MRILastError
} LVMCLIMessage;


/* NLV LVM engine errors. */
#define LVM_ENGINE_ERRORS                56      /* lvm.txt message number */
#define LVM_ENGINE_ERROR_MSGLEN          80
extern pSTRING*    ppLVMEngineErrorText;
extern CARDINAL32  nLVMEngineError;
typedef CARDINAL32 LVMCLIEngineError;

/* NLV partition types. */
#define PARTITION_TYPE_STRINGS           49      /* lvm.txt message number */
#define PARTITION_TYPE_STRING_MSGLEN      9
extern pSTRING*    ppPartitionTypeText;
extern CARDINAL32  nPartitionType;
typedef enum
{
   MRIPartition_Logical = 0,
   MRIPartition_Primary,
} LVMCLIPartitionType;

/* NLV partition status. */
#define PARTITION_STATUS_STRINGS         50      /* lvm.txt message number */
#define PARTITION_STATUS_STRING_MSGLEN   12
extern pSTRING*    ppPartitionStatusText;
extern CARDINAL32  nPartitionStatus;
typedef enum
{
   MRIPartition_InUse = 1,
   MRIPartition_Available
} LVMCLIPartitionStatus;

/* NLV volume types. */
#define VOLUME_TYPE_STRINGS              28      /* lvm.txt message number */
#define VOLUME_TYPE_STRING_MSGLEN        15
extern pSTRING*    ppVolumeTypeText;
extern CARDINAL32  nVolumeType;
typedef enum
{
   MRIVolume_Compatibility = 0,
   MRIVolume_LVM
} LVMCLIVolumeType;

/* NLV partition boot status. */
#define BOOT_STATUS_STRINGS              78      /* lvm.txt message number */
#define BOOT_STATUS_STRING_MSGLEN        10
extern pSTRING*    ppBootStatusText;
extern CARDINAL32  nBootStatus;
typedef enum
{
   MRIBoot_Bootable = 1,
   MRIBoot_Startable,
   MRIBoot_Installable
} LVMCLIBootStatus;

/* NLV partition unformatted file system name. */
#define FS_NAME_STRINGS                  72      /* lvm.txt message number */
#define FS_NAME_STRING_MSGLEN            11
extern pSTRING*    ppFSNameText;
extern CARDINAL32  nFSName;
typedef enum
{
   MRIFSName_Unformatted = 0
} LVMCLIFSName;

/* End of Message signal for an MRI message. */
typedef enum
{
   MRI_EOM     = HF_STDERR,          /* send message to standard error out */
   MRI_EOM_OUT = HF_STDOUT           /* send message to standard out */
} LVMCLI_EOM;

CARDINAL32  GlobalMRI_Initialize();
CARDINAL32  MRI_Initialize( pSTRING** ppMessageArray, CARDINAL32* nMessages, CARDINAL32 messageLength, CARDINAL32 messageNum );

#endif
