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
 * Module: lvmcli.h
 */

/*
 * Change History:
 *
 */


/*
 * Description: Defines types and macros needed by the main program, the
 *              parser, and the subsystems for each device that MODE.EXE
 *              manipulates.
 *
 */

#ifndef LVM_CLI_LVMCLI_H

#define LVM_CLI_LVMCLI_H

#include "gbltypes.h"
#include <lvm_intr.h>

/* This type enumerates the type of names that can be specified for all rules */
typedef enum {
               AllNameType = 1,
               DriveLetterType,
               DriveNameType,
               DriveNumberType,
               FileNameType,
               FileSystemType,
               NoNameType,
               PartitionNameType,
               VolumeNameType,
               UnknownNameType,
               NameErrorType
             } NameTypes;

/****************************************************************************
****************************************************************************/

/****************************************************************************

<Deletion Parameters> ::=
     All , (Volumes|Unused|Compatibility|LVM|Primary|Logical)
   | Partition , (<Drive Number>|<Drive Name>) , <Partition Name>
   | Volume , <Volume Name>

****************************************************************************/

/* This type enumerates the options for the Deletion Parameters rule */
typedef enum {
               Deletion_None = 1,
               Deletion_All_Volumes,
               Deletion_All_Unused,
               Deletion_All_Compatibility,
               Deletion_All_LVM,
               Deletion_All_Primary,
               Deletion_All_Logical,
               Deletion_Partition,
               Deletion_Partitions,
               Deletion_Volume,
               Deletion_Error
             } DeletionOptionTypes;

/* This structure represents the potential data for the Deletion Parameters rule */
typedef struct {
               NameTypes            NameType;
               pSTRING              pDrive;
               pSTRING              pPartitionName;
               pSTRING              pVolumeName;
               DeletionOptionTypes  DeletionOption;
             } DeletionStruct, * pDeletionStruct;

/****************************************************************************/
/****************************************************************************/

/****************************************************************************

<Allocation Algorithm> ::=   BestFit [, (FromStart|FromEnd)]
                           | FirstFit [, (FromStart|FromEnd)]
                           | LastFit [, (FromStart|FromEnd)]
                           | FromLargest [, (FromStart|FromEnd)]
                           | FromSmallest [, (FromStart|FromEnd)]
                           | <Free Space ID> [, (FromStart|FromEnd)]

****************************************************************************/

/* This type enumerates the options for the Allocation Algorithm rule */
typedef enum {
               Allocation_None = 1,
               Allocation_BestFit,
               Allocation_FirstFit,
               Allocation_LastFit,
               Allocation_FromLargest,
               Allocation_FromSmallest,
               Allocation_FreespaceID,
               Allocation_Error
             } AllocationOptionTypes;

/* This type enumerates the suboptions for the Allocation Algorithm rule */
typedef enum {
               Allocation_FromNone = 1,
               Allocation_FromEnd,
               Allocation_FromStart,
               Allocation_FromError
             } AllocationSubOptionTypes;

/****************************************************************************
****************************************************************************/

/****************************************************************************

<Partition Parameters> ::=
     <Partition Name> , (<Drive Number>|<Drive Name>) [, <Size>]
        , (Logical|Primary) , (Bootable|NonBootable) [, <Allocation Algorithm>]

****************************************************************************/

/* This type enumerates the options for the Partition List Parameters rule */
typedef enum {
               Partition_Logical = 1,
               Partition_Primary,
               Partition_Option_Error
             } PartitionOptionTypes;

/* This type enumerates the suboptions for the Partition List Parameters rule */
typedef enum {
               Partition_Bootable =1,
               Partition_NonBootable,
               Partition_SubOption_Error
             } PartitionSubOptionTypes;

/* This type enumerates the options for the Size parameter which is now */
/* optional.  If it is specified, the SizeType field will be set to     */
/* SizeSpecified.  If it is not specified, the SizeType field will be   */
/* set to SizeNotSpecified. */
typedef enum {
               SizeSpecified = 1,
               SizeNotSpecified
             } SizeOptionTypes;

/* This structure represents the potential data for the Partition Parameters rule */
typedef struct {
                  pSTRING                          pPartitionName;
                  NameTypes                        NameType;
                  pSTRING                          pDrive;
                  SizeOptionTypes                  SizeType;
                  pSTRING                          pSize;
                  PartitionOptionTypes             OptionType;
                  PartitionSubOptionTypes          SubOptionType;
                  AllocationOptionTypes            AllocationOptionType;
                  pSTRING                          pFreespaceID;
                  AllocationSubOptionTypes         AllocationSubOptionType;
               } PartitionStruct, * pPartitionStruct;

/****************************************************************************
****************************************************************************/


/****************************************************************************

<Partition List Parameters> ::=
     (<Drive Number>|<Drive Name>) , <Partition Name>

****************************************************************************/

/* This structure represents the potential data for the Partition List Parameters rule */
typedef struct PartListStruct {
                  NameTypes                                 NameType;
                  pSTRING                                   pDrive;
                  pSTRING                                   pPartitionName;
                  struct PartListStruct *                   pNext;
               } PartitionListStruct, * pPartitionListStruct;

/****************************************************************************

<Feature Specification> ::=
     <Feature Name>: '('(<Feature Specific Commands>)+')'

****************************************************************************/

/* This structure represents the potential data for the Feature Specification rule */
typedef struct FeatListStruct {
                  CARDINAL32                                Feature_ID;
                  LVM_Classes                               Actual_Class;
                  ADDRESS                                   Init_Data;
                  PartitionListStruct *                     pPartitionListParameters;
                  struct FeatListStruct *                   pNext;
               } FeatureListStruct, * pFeatureListStruct;
/****************************************************************************

<Volume Parameters> ::=
     Compatibility , (BootDOS|BootOS2|NoBoot) , <Drive Letter>
                              , <Volume Name> , <Partition List Parameters>

    | LVM , <Volume Name> , <Drive Letter> [(, <Partition List Parameters>)+]

****************************************************************************/

/* This type enumerates the suboptions for the Volume Parameters rule. */
typedef enum {
               Volume_BootDOS = 1,
               Volume_BootOS2,
               Volume_NoBoot,
               Volume_SubOptionError
             } VolumeSubOptionTypes;

/* This type enumerates the options for the Volume Parameters rule. */
typedef enum {
               Volume_Compatibility = 1,
               Volume_LVM,
               Volume_OptionError
             } VolumeOptionTypes;

/* This structure represents the potential data for the Volume Parameters rule */
typedef struct {
                  VolumeOptionTypes                OptionType;
                  VolumeSubOptionTypes             SubOptionType;
                  pSTRING                          pDriveLetter;
                  pSTRING                          pVolumeName;
                  FeatureListStruct *              pFeatureListParameters;
                  PartitionListStruct *            pPartitionListParameters;
               } VolumeStruct, * pVolumeStruct;

/****************************************************************************
****************************************************************************/


/****************************************************************************

<Creation Parameters> ::=   Partition , <Partition Parameters>
                          | Volume , <Volume Parameters>

****************************************************************************/

/* This type enumerates the options for the Creation Parameters rule */
typedef enum {
               Create_Partition_Option = 1,
               Create_Volume_Option,
               Create_Error_Option
             } CreationOptionTypes;

/* This structure represents the potential data for the Creation Parameters rule */
typedef struct {
                  CreationOptionTypes           OptionType;
                  ADDRESS                       pParameters;
               } CreationStruct, * pCreationStruct;

/****************************************************************************
****************************************************************************/

/****************************************************************************

<Install Parameters> ::=   FS /SIZE:<Number>

****************************************************************************/
/* All the other /SI options have been eliminated from the grammar */
/* under defect 202485. */
/* This type enumerates the options for the Install Parameters rule */
typedef enum {
               Install_FS = 1,
               Install_Error
             } InstallOptionTypes;

/* This structure represents the potential data for an SI command */
typedef struct {
                  InstallOptionTypes   OptionType;
                  pSTRING              pSize;
               } InstallStruct, * pInstallStruct;

/****************************************************************************
****************************************************************************/

/****************************************************************************

                 /NewMBR : <Drive Number>

****************************************************************************/

/* This structure represents the potential data for an NewMBR command */
typedef struct {
                  NameTypes                     NameType;
                  pSTRING                       pName;
               } NewMBRStruct, * pNewMBRStruct;

/****************************************************************************
****************************************************************************/

/****************************************************************************

<Name Change Parameters> ::=
     Volume , <Volume Name> , <New Volume Name>
   | Partition , (Drive Name>|<Drive Number>) , <Partition Name> , <New Partition Name>
   | Drive , <Drive Name> , <New Drive Name>

****************************************************************************/

/* This type enumerates the options for the Name Change Parameters rule */
typedef enum {
               NameChange_Drive = 1,
               NameChange_Partition,
               NameChange_Volume,
               NameChange_Error
             } NameChangeOptionTypes;


/* This structure represents the potential data for the Name Change Parameters rule */
typedef struct {
                  NameChangeOptionTypes         OptionType;
                  NameTypes                     NameType;
                  pSTRING                       pDrive;
                  pSTRING                       pOldName;
                  pSTRING                       pNewName;
               } NameChangeStruct, * pNameChangeStruct;


/****************************************************************************
****************************************************************************/

/****************************************************************************
                 /SetStartable : (<Partition Name>|<Volume Name>)
****************************************************************************/

/* This structure represents the potential data for an SetStartable command */
typedef struct {
                  NameTypes                     NameType;
                  pSTRING                       pDrive;
                  pSTRING                       pPartitionName;
                  pSTRING                       pVolumeName;
               } SetStartableStruct, * pSetStartableStruct;


/****************************************************************************
****************************************************************************/


/***************************************************************************

<Query Parameters> ::=
     Primary [, <Optional Query Parameters> ]
   | Logical [,  <Optional Query Parameters> ]
   | Freespace [, (<Drive Number>|<Drive Name>|All ) ]
   | Unusable [, (<Drive Number>|<Drive Name>|All ) ]
   | Volumes [, <Optional Query Parameters> ]
   | Compatibility [, <Optional Query Parameters> ]
   | LVM [, <Optional Query Parameters> ]
   | Bootable [, (<Drive Number>|<Drive Name>|All ) ]
   | All [, <Optional Query Parameters> ]

****************************************************************************/

/* This type enumerates the options for the Query Parameters rule */
typedef enum {
               Query_All = 1,
               Query_Bootable,
               Query_Compatibility,
               Query_Freespace,
               Query_Logical,
               Query_LVM,
               Query_None,
               Query_Primary,
               Query_Volumes,
               Query_Unusable,
               Query_Error
             } QueryOptionTypes;

/* This structure represents the potential data for an Query Parameters rule */
typedef struct {
                  QueryOptionTypes              OptionType;
                  NameTypes                     NameType;
                  pSTRING                       pName;
                  pSTRING                       pFileSystemType;
               } QueryStruct, * pQueryStruct;

/****************************************************************************
****************************************************************************/

/****************************************************************************

<Expand Parameters> ::=  <Volume Name> (,<Partition List Parameters>)+

****************************************************************************/

/* This structure represents the potential data for an Expand command */
typedef struct {
                  pSTRING                 pVolumeName;
                  pPartitionListStruct    pPartitionListParameters;
               } ExpandStruct, * pExpandStruct;



/****************************************************************************
****************************************************************************/

/* This structure represents the data for a DriveLetter command */
typedef struct {
                  pSTRING                 pVolumeName;
                  pSTRING                 pDriveLetter;
               } DriveLetterStruct, * pDriveLetterStruct;

/****************************************************************************
****************************************************************************/

/***************************************************************************

<Command> ::=     /Query [: <Query Parameters> ]

                | /Create : <Creation Parameters>

                | /Delete : <Deletion Parameters>

                | /Hide : <Volume Name>

                | /Bootmgr [: (1|2) ]

                | /SetName : <Name Change Parameters>

                | /SetStartable : (<Partition Name>|<Volume Name>)

                | /NewMBR : (<Drive Number>)

                | /Expand : <Expand Parameters>

                | /StartLog : <File Name>

                | /DriveLetter : <Volume Name> , <Drive Letter>

                | /RediscoverPRM

****************************************************************************/

/* This type enumerates the set of options for the Command rule */
typedef enum {
               BootmgrCmd = 1,
               CreateCmd,
               DeleteCmd,
               ExpandCmd,
               FileCmd,
               HideCmd,
               NewMBRCmd,
               NoCmd,
               QueryCmd,
               SetNameCmd,
               SetStartableCmd,
               SICmd,
               FromFileCmd,
               ErrorCmd,
               StartLogCmd,
               DriveLetterCmd,
               RediscoverPRMCmd
             } CommandTypes;


/* This structure in the basic element of a dynamically allocated linked list */
/* into which each Command rule sequence found on the command line is stored. */
/* As each command is parsed, a structure of this type is allocated to        */
/* represent it and the allocated structure is linked onto the end of the     */
/* chain.  The data stored in this structure is generic, in other words it is */
/* not specific to the type of command found.  A second dynamically allocated */
/* structure that is specific the command type is hung off of this one,       */
/* if necessary. */

typedef struct CmdStruct {
                  CommandTypes                CommandType;
                  ADDRESS                     pCommandData;
                  struct CmdStruct *          pNextCommand;
               } CommandStruct, * pCommandStruct;


/****************************************************************************
****************************************************************************/

#endif
