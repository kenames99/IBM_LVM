/******************************************************************************/
/*   SCCSID = src/lvm/cli/commexec.cpp, lvm.cli, c.fs32, 990312.1 99/02/23                                                         */
/*                                                                            */
/*   IBM Confidential                                                         */
/*                                                                            */
/*   OCO Source Materials                                                     */
/*                                                                            */
/*   Program number (  )                                                      */
/*                                                                            */
/*   (c) Copyright IBM Corp. 1998, 1998                                       */
/*                                                                            */
/*   The source code for this program is not published or otherwise divested  */
/*   of its tradesecrets, irrespective of what has been deposited with the    */
/*   U. S. Copyright Office.                                                  */
/*                                                                            */
/*   US Government Users Restricted Rights -- Use, duplication or disclosure  */
/*   restricted by GSA ADP Schedule Contract with IBM Corp.                   */
/*                                                                            */
/*                                                                            */
/* Notes:                                                                     */
/*                                                                            */
/*                              Modifications                                 */
/* Date      Name          Description                                        */
/* ---------------------------------------------------------------------------*/
/* 12/3/98  M. McBride     Added StartLog command (209170)                    */
/* 12/17/98 John Stiles    Add DriveLetter command.                   @214194 */
/* 01/21/99 Vince Rapp     Added /rediscoverPRM command                       */
/* 02/02/99 Vince Rapp     Added additional /startlog logic           @216357 */
/* 02/23/99 Ben Rafanello  Updated the PRM Rediscover code to use the         */
/*                         new calling convention.                    @218946 */
/******************************************************************************/
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
 * Module: commexec.cpp
 */ 

/*
 * Change History:
 * 
 */


/*
 *
 * Description:  Command execution for LVM's command line interpretter.
 *
 * Parses and executes commands requested via LVM's command line interface.
 *
 * Algorithm:
 *  - Open the LVM engine.
 *  - Loop through the list of parsed commands, executing each in order of
 *    occurance in list.
 *  - If any of the commands result in changes in the volume then commit
 *    the LVM changes.
 *  - If any of the commands result in any error then roll back (don't commit)
 *    the LVM changes and return TRUE.
 *  - Close the LVM engine.
 *
 * Return:
 *  - FALSE if all commands executed successfully
 *  - TRUE if one or more commands failed.
 *
 */

#define PCR1442 1

#define INCL_32
#define INCL_NOPMAPI
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#include <os2.h>
#include <bsedev.h>
#include <miscx.h>
#include <strat2.h>
#include <reqpktx.h>
#include <dskioctl.h>

#include <stdlib.h>
#include <stdio.h>
#ifdef DEBUG
#include <assert.h>
#endif

#include "commVDP.hpp"
#include "commMRI.hpp"


//
// Algorithm:
// - create a set of volumes by querying the LVM engine
// - test each volume in the set to determine if it is in the scope
//   of this query.
// - if the volume is within this query's scope, print the volume's
//   information
// - repeat the above steps for the set of drives on the system
//
// Return:
// - FALSE, always, as the query operations make no changes to the current
//   LVM configuration
//
BOOLEAN doQueryCmd( QueryStruct* pQueryData, CARDINAL32* LVMError )
{
   VDPQualifier   VDPQual;

   // Fill in the qualification record
   VDPQual.queryType = pQueryData->OptionType;

   if( (pQueryData->NameType == DriveLetterType) ||
       (pQueryData->NameType == DriveNameType)   ||
       (pQueryData->NameType == DriveNumberType) )
   {

      VDPQual.driveNameType   = pQueryData->NameType;
      VDPQual.pDriveName      = pQueryData->pName;
   }
   else
   {
      VDPQual.driveNameType   = AllNameType;
      VDPQual.pDriveName      = 0;
   }
   VDPQual.pVolumeName     = (pQueryData->NameType == VolumeNameType) ? pQueryData->pName : 0;
   VDPQual.pPartitionName  = (pQueryData->NameType == PartitionNameType) ? pQueryData->pName : 0;
   VDPQual.pFileSystemName = pQueryData->pFileSystemType ? pQueryData->pFileSystemType : 0;

   // If the query can be interpreted as pertaining to volumes then enumerate the
   // volumes and process the volume list.
   if( (VDPQual.queryType == Query_Volumes)       ||
       (VDPQual.queryType == Query_Compatibility) ||
       (VDPQual.queryType == Query_LVM)           ||
       (VDPQual.queryType == Query_Freespace)     ||
       (VDPQual.queryType == Query_Unusable)      ||
       (VDPQual.queryType == Query_Bootable)      ||   // @202719
       (VDPQual.queryType == Query_All) )
   {
      // Enumerate the system's set of volumes.
      CLIVolumeSet  volumeSet;

      if( !volumeSet.getStatus() && volumeSet._memberCount )
      {
         volumeSet.printMembers( VDPQual );
      }
   }

   // If the query can be interpreted as also pertaining to drives then enumerate the
   // drives and process the drive list.
   if( (VDPQual.queryType == Query_Freespace)     ||
       (VDPQual.queryType == Query_Logical)       ||
       (VDPQual.queryType == Query_Primary)       ||
       (VDPQual.queryType == Query_Unusable)      ||
//     (VDPQual.queryType == Query_Bootable)      || remove until Bootable works @202719
       (VDPQual.queryType == Query_All)           ||
       (VDPQual.queryType == Query_None) )         // so that /query works kind-of like legacy fdisk /query
   {
      CLIDriveSet   driveSet;

      if( !driveSet.getStatus() && driveSet._memberCount )
      {
         driveSet.printMembers( VDPQual );
      }
   }

   *LVMError = LVM_ENGINE_NO_ERROR;
   return FALSE;      // nothing to commit
}


//
// Create a partition or a volume.
//
// Simplification: can only create volumes from existing partition(s).
//
// All volumes must have at least 1 partition and all compatability
// volumes must have exactly 1 partition.
//
// Return TRUE for successful creation to commit all changes in the
// LVM engine.
BOOLEAN doCreateCmd( const CreationStruct* createData, CARDINAL32* LVMError )
{

   pPartitionStruct  pCreatePart = (pPartitionStruct)createData->pParameters;
   pVolumeStruct     pCreateVol  = (pVolumeStruct)createData->pParameters;

   // Enumerate system drives and ALL partitions.
   CLIDriveSet  driveSet;
   if( driveSet.getStatus() )
   {
      MRI message;
      message << MRICreate_Enumeration << driveSet.getStatus() << MRI_EOM;
//      printf( "Error: Create option failed system drive enumeration, LVM error = %d.\n",
//              driveSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   switch( createData->OptionType )
   {
   case Create_Partition_Option:     // Create a new partition
      ADDRESS           createOnHandle;   // handle to use to create new partition on
      VDPQualifier      VDPQual;
      CLIBase*          pDP;              // pointer to drive or a partition

      // The partition will be created on a drive or from an area of free space.  If
      // the partition is being created from free space then we need to get the handle
      // of the LVM partition identified by in command line.  If the partition is being
      // created from a drive then need the handle to the drive.
      if( pCreatePart->AllocationOptionType == Allocation_FreespaceID )
      {
         // Partition is being created from an LVM free space partition.  Get the partition
         // handle to that free space.
         // Find the existing partiton.  Fill in the qualification record
         VDPQual.queryType      = Query_All;
         VDPQual.driveNameType  = pCreatePart->NameType;
         VDPQual.pVolumeName    = 0;
         VDPQual.pDriveName     = pCreatePart->pDrive;
         VDPQual.pPartitionName = pCreatePart->pFreespaceID;

         pDP = driveSet.findPartition( VDPQual );
         if( !pDP )
         {
            MRI message;
            message << MRICreate_PartNoFreeSpace << VDPQual.pPartitionName << VDPQual.pDriveName << MRI_EOM;
//            printf( "Error: Partition creation failed to find existing free space partition: %s on drive: %s.\n",
//                    VDPQual.pPartitionName, VDPQual.pDriveName );
            return FALSE;
         }
      }
      else
      {
         CLIIndex  index;
         BOOLEAN   found;

         // Partition is being created from a drive.  Find the drive and then get the
         // LVM handle to the drive.
         VDPQual.queryType      = Query_All;
         VDPQual.driveNameType  = pCreatePart->NameType;
         VDPQual.pVolumeName    = 0;
         VDPQual.pDriveName     = pCreatePart->pDrive;
         VDPQual.pPartitionName = 0;

         for( pDP = driveSet.getNext( index ), found = FALSE;
              pDP  && !found;
              pDP = driveSet.getNext( index ) )
         {
            if( pDP->thisIsA( VDPQual ) )
            {
               found = TRUE;
               break;
            }
         }

         if( !found )
         {
            MRI message;
            message << MRICreate_PartNnoDrive << VDPQual.pDriveName << MRI_EOM;
//            printf( "Error: Partition creation failed to find existing drive: %s.\n",
//                    VDPQual.pDriveName );
            return FALSE;
         }
      }

      // Get the handle of the disk or partition to create the new partition on.
      createOnHandle = pDP->getLVMHandle();

      // Map the user's allocation alogrithm into LVM's allocation alogrithm.
      Allocation_Algorithm   allocationAlgorithm;

      switch( pCreatePart->AllocationOptionType )
      {
      case Allocation_BestFit:
         allocationAlgorithm = Best_Fit;
         break;

      case Allocation_FirstFit:
         allocationAlgorithm = First_Fit;
         break;

      case Allocation_LastFit:
         allocationAlgorithm = Last_Fit;
         break;

      case Allocation_FromLargest:
         allocationAlgorithm = From_Largest;
         break;

      case Allocation_FromSmallest:
         allocationAlgorithm = From_Smallest;
         break;

      case Allocation_None:
      case Allocation_FreespaceID:
      case Allocation_Error:
      default:
         allocationAlgorithm = First_Fit;  // change to minimize fragmentation @205849
         break;
      }

      // if we are creating a bootable partition, change to allocate from start of free space @205849
      if( pCreatePart->AllocationSubOptionType == Allocation_FromNone &&
      pCreatePart->SubOptionType == Partition_Bootable )
        pCreatePart->AllocationSubOptionType = Allocation_FromStart;

      // Use LVM engine to create the new partition.
      ADDRESS      partitionHandle;
      CARDINAL32   size;

      // Begin changes for @214194

      // Check if the partition size is specified or not...
      if( pCreatePart->SizeType == SizeNotSpecified )
      {
        allocationAlgorithm = All;  // use the whole free space or drive
        size = 0;  // the engine will set the proper size.
      }
      else
      {
        // Convert the speficifed size in MB (1MB = 1024 * 1024 bytes) to 512-byte sectors.
        size = strtol( pCreatePart->pSize, (char**)NULL, 0 ) * 1024 * 2;
      }

      // End of changes for @214194

      partitionHandle = Create_Partition( createOnHandle,
                                          size,
                                          pCreatePart->pPartitionName,
                                          allocationAlgorithm,
                                          pCreatePart->SubOptionType == Partition_Bootable ? TRUE : FALSE,
                                          pCreatePart->OptionType == Partition_Primary ? TRUE : FALSE,
                                          pCreatePart->AllocationSubOptionType == Allocation_FromStart ? TRUE : FALSE,
                                          LVMError );
      if( *LVMError || !partitionHandle )
      {
         MRI message;
         message << MRICreate_Part << pCreatePart->pPartitionName << *LVMError << MRI_EOM;
//         printf( "Error: failed to create partition: %s, LVM error = %d.\n",
//                 pCreatePart->pPartitionName, *LVMError );
         return FALSE;
      }

      break;

   case Create_Volume_Option:        // create a new volume

      pPartitionListStruct       pPartitionList;
      pFeatureListStruct         pFeatureList;
      CARDINAL32                 FeatureCount;
      CARDINAL32                 partitionCount;
      CLIBase*                   pPartition;     // existing partition
      LVM_Feature_Specification_Record* pFeatureSpecificationArray;

      // Count the number of partitions specified for this volume on the command line so
      // we know how much memory to allocate for the array of handles.
      for( pPartitionList = pCreateVol->pPartitionListParameters, partitionCount = 0;
           pPartitionList;
           pPartitionList = pPartitionList->pNext )
      {
         partitionCount++;
      }

      // Error test for no partitions.
      if( !partitionCount )
      {
         MRI message;
         message << MRICreate_VolNoPartList << pCreateVol->pVolumeName << MRI_EOM;
//         printf( "Error: volume creation requires partition list to create volume: %s.\n",
//                 pCreateVol->pVolumeName );
         return FALSE;
      }

      // Error test for a compatability volume requesting more than 1 partition.
      else if( (partitionCount > 1) && (pCreateVol->OptionType == Volume_Compatibility) )
      {
         MRI message;
         message << MRICreate_CompatVol1Part << pCreateVol->pVolumeName << MRI_EOM;
//         printf( "Error: creation of compatability volume: %s, requires specification of exactly one partition.\n",
//                 pCreateVol->pVolumeName );
         return FALSE;
      }

      // Create a list of partition handles from the partitionSet.
      ADDRESS* ppPartitionHandleArray = new ADDRESS[partitionCount];

      // Populate partitionSet with the new and existing partitions specified on the command line.
      for( pPartitionList = pCreateVol->pPartitionListParameters, partitionCount = 0;
           pPartitionList;
           pPartitionList = pPartitionList->pNext )
      {
         VDPQualifier   partQual;
         // Find each existing partition specified in the command line parameters.
         partQual.queryType      = Query_All;
         partQual.driveNameType  = pPartitionList->NameType;
         partQual.pVolumeName    = 0;
         partQual.pDriveName     = pPartitionList->pDrive;
         partQual.pPartitionName = pPartitionList->pPartitionName;

         pPartition = driveSet.findPartition( partQual );
         if( !pPartition )
         {
            MRI message;
            message << MRICreate_VolNoPart << partQual.pPartitionName << partQual.pDriveName << MRI_EOM;
//            printf( "Error: Volume creation failed to find existing partition: %s on drive: %s.\n",
//                    partQual.pPartitionName, partQual.pDriveName );
            return FALSE;
         }

         // Save the partition's handle.
         ppPartitionHandleArray[partitionCount] = pPartition->getLVMHandle();
         partitionCount++;
      }

      // Copy the Selected Features from the linked list into our
      // local array before calling Create_Volume

      pFeatureSpecificationArray = NULL;
      if (pCreateVol->OptionType == Volume_LVM)
      {
   // Count the number of features specified for this volume on the command line so
   // we know how much memory to allocate for the array of handles.
   for( pFeatureList = pCreateVol->pFeatureListParameters, FeatureCount = 0;
        pFeatureList;
        pFeatureList = pFeatureList->pNext )
   {
      FeatureCount++;
   }

   // Error test for no features.
   if( !FeatureCount )
   {
      MRI message;
      message << MRICreate_VolNoPartList << pCreateVol->pVolumeName << MRI_EOM;
//         printf( "Error: volume creation requires partition list to create volume: %s.\n",
//                 pCreateVol->pVolumeName );
      return FALSE;
   }

   // Create a list of partition handles from the partitionSet.
   pFeatureSpecificationArray = new LVM_Feature_Specification_Record[FeatureCount];

   // Populate partitionSet with the new and existing partitions specified on the command line.
   for( pFeatureList = pCreateVol->pFeatureListParameters, FeatureCount = 0;
        pFeatureList;
        pFeatureList = pFeatureList->pNext )
   {
      // Save the feature specification data.
      pFeatureSpecificationArray[ FeatureCount ].Feature_ID = pFeatureList->Feature_ID;
      pFeatureSpecificationArray[ FeatureCount ].Actual_Class = pFeatureList->Actual_Class;
      pFeatureSpecificationArray[ FeatureCount ].Init_Data = pFeatureList->Init_Data;
      FeatureCount++;
   }
      }

      // Use LVM engine to create the new volume.
      Create_Volume2( pCreateVol->pVolumeName,
                      pCreateVol->OptionType == Volume_LVM ? TRUE : FALSE,
                      pCreateVol->SubOptionType == Volume_BootOS2 ? TRUE : FALSE,
                      pCreateVol->pDriveLetter[0],
                      FeatureCount,
                      pFeatureSpecificationArray,
                      partitionCount,
                      ppPartitionHandleArray,
                      LVMError );

      delete [] pFeatureSpecificationArray;
      delete [] ppPartitionHandleArray;

      if( *LVMError )
      {
         MRI message;
         message << MRICreate_Volume << pCreateVol->pVolumeName << *LVMError << MRI_EOM;
//         printf( "Error: failed to create volume: %s, LVM error = %d.\n",
//                 pCreateVol->pVolumeName, *LVMError );
         return FALSE;
      }

      break;
   }

   // Volume or partition creation worked OK, return true to commit in LVM engine.
   return TRUE;
}


//
// Change name of a Volume, Drive, or Partition.
BOOLEAN doSetNameCmd( NameChangeStruct* pNameChangeData, CARDINAL32* LVMError )
{
   ADDRESS        handle = 0;
   VDPQualifier   VDPQual;
   CLIBase*       pVDP;


   // Enumerate systems volumes.  Need this set of volumes so we can find the old one.
   CLIVolumeSet  volumeSet;
   if( volumeSet.getStatus() )
   {
      MRI message;
      message << MRISetName_VolumeEnumeration << volumeSet.getStatus() << MRI_EOM;
//      printf( "Error: Setname failed system volume enumeration, LVM error = %d.\n",
//              volumeSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   // Enumerate system drives and ALL partitions.  Used to find an old drive or partition.
   CLIDriveSet  driveSet;
   if( driveSet.getStatus() )
   {
      MRI message;
      message << MRISetName_DriveEnumeration << driveSet.getStatus() << MRI_EOM;
//      printf( "Error: Setname failed system drive enumeration, LVM error = %d.\n",
//              driveSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   switch( pNameChangeData->OptionType )
   {
   case NameChange_Volume:

      // Find the existing volume.
      pVDP = volumeSet.find( pNameChangeData->pOldName );
      if( !pVDP )
      {
         MRI message;
         message << MRISetName_NoVolume << pNameChangeData->pOldName << MRI_EOM;
//         printf( "Error: Volume rename can't find existing volume: %s.\n",
//                 pNameChangeData->pOldName );
         return FALSE;
      }

      break;

   case NameChange_Drive:

      // Find the existing drive.
      pVDP = driveSet.find( pNameChangeData->pOldName );
      if( !pVDP )
      {
         MRI message;
         message << MRISetName_NoDrive << pNameChangeData->pOldName << MRI_EOM;
//         printf( "Error: Drive rename can't find existing drive: %s.\n",
//                 pNameChangeData->pOldName );
         return FALSE;
      }

      break;

   case NameChange_Partition:

      // Find the existing partiton.  Fill in the qualification record
      VDPQual.queryType      = Query_All;
      VDPQual.driveNameType  = pNameChangeData->NameType;
      VDPQual.pVolumeName    = 0;
      VDPQual.pDriveName     = pNameChangeData->pDrive;
      VDPQual.pPartitionName = pNameChangeData->pOldName;

      pVDP = driveSet.findPartition( VDPQual );
      if( !pVDP )
      {
         MRI message;
         message << MRISetName_NoPartition << VDPQual.pPartitionName << VDPQual.pDriveName << MRI_EOM;
//         printf( "Error: Partition rename can't find existing partition: %s on drive: %s.\n",
//                 VDPQual.pPartitionName, VDPQual.pDriveName );
         return FALSE;
      }

      break;
   }

   // Retrieve handle of the existing partition.
   handle = pVDP->getLVMHandle();

   // Change the name.
   Set_Name( handle,
             pNameChangeData->pNewName,
             LVMError );
   if( *LVMError )
   {
      MRI message;
      message << MRISetName_Failure << pNameChangeData->pOldName << pNameChangeData->pNewName << *LVMError << MRI_EOM;
//      printf( "Error: failed to rename: %s to %s, LVM error = %d.\n",
//               pNameChangeData->pOldName, pNameChangeData->pNewName, *LVMError );
      return FALSE;
   }

   // Rename operation worked, return TRUE to commit LVM changes.
   return TRUE;
}


//
// Delete the specified Volume(s) or Partition(s).
BOOLEAN doDeleteCmd( DeletionStruct* pDeletionData, CARDINAL32* LVMError )
{
   BOOLEAN        done, deleteIt;
   CLIBase*       pVDSet;                  // pointer to the set of volumes or drives
   VDPQualifier   VDPQual;
   CLIBase*       pVolOrPart = 0;
   BOOLEAN        noLVMDeletions = TRUE;   // assume nothing will be deleted


   // Fill in the qualification record.
   VDPQual.queryType       = Query_All;
   VDPQual.driveNameType   = pDeletionData->NameType;
   VDPQual.pDriveName      = pDeletionData->pDrive;
   VDPQual.pVolumeName     = pDeletionData->pVolumeName ? pDeletionData->pVolumeName : 0;
   VDPQual.pPartitionName  = pDeletionData->pPartitionName ? pDeletionData->pPartitionName : 0;
   VDPQual.pFileSystemName = 0;

   // Enumerate the system's current volumes or partitions depeding upon what has
   // been asked to be deleted.  Volume enumeration will not necessarially show all
   // the partitions in the system, so if deleting a partition then must enumerate
   // the disks to get a complete set of partitions.
   if( (pDeletionData->DeletionOption == Deletion_All_Volumes)       ||
       (pDeletionData->DeletionOption == Deletion_All_LVM)           ||
       (pDeletionData->DeletionOption == Deletion_All_Compatibility) ||
       (pDeletionData->DeletionOption == Deletion_Volume) )
   {
      // Enumerate the volumes if deleting one or more volumes.
      pVDSet = new CLIVolumeSet();
   }
   //
   else if( (pDeletionData->DeletionOption == Deletion_Partitions) ||
       (pDeletionData->DeletionOption == Deletion_All_Unused)      ||
       (pDeletionData->DeletionOption == Deletion_All_Primary)     ||
       (pDeletionData->DeletionOption == Deletion_All_Logical)     ||
       (pDeletionData->DeletionOption == Deletion_Partition) )
   {
      // Enumerate the drives if deleting one or more partitions.
      pVDSet = new CLIDriveSet();
   }

   // Check for an enumeration problem.
   if( pVDSet->getStatus() )
   {
      MRI message;
      message << MRIDelete_Enumeration << pVDSet->getStatus() << MRI_EOM;
//      printf( "Error: Deletion operation failed system enumeration, LVM error = %d.\n",
//              pVDSet->getStatus() );
      delete pVDSet;
      return FALSE;  // nothing to commit
   }

   CLIIndex       index;       // keep position info here

   // Iterate the volumes and partitions and determine which one(s) should be deleted.
   for( done = FALSE; !done; )
   {
      if( (pDeletionData->DeletionOption == Deletion_All_Volumes)       ||
          (pDeletionData->DeletionOption == Deletion_All_LVM)           ||
          (pDeletionData->DeletionOption == Deletion_All_Compatibility) ||
          (pDeletionData->DeletionOption == Deletion_Volume) )
      {
         // Look for next Volume to delete.
         pVolOrPart = pVDSet->getNext( index );
      }
      //
      else if( (pDeletionData->DeletionOption == Deletion_Partitions) ||
          (pDeletionData->DeletionOption == Deletion_All_Unused)      ||
          (pDeletionData->DeletionOption == Deletion_All_Primary)     ||
          (pDeletionData->DeletionOption == Deletion_All_Logical)     ||
          (pDeletionData->DeletionOption == Deletion_Partition) )
      {
         // Look for next Partition to delete.
         pVolOrPart = pVDSet->getNextPartition( index );
      }

      // Test for no more deletions.
      if( !pVolOrPart )
      {
         done = TRUE;
         break;
      }
      else
      {
         // Continue, look for a volume or partition to delete.  Assume
         // there are no more to delete.
         deleteIt = FALSE;
      }

      if( pVolOrPart->thisIsA( VDPQual ) )
      {
         // Determine if this volume or partition should be deleted.
         switch( pDeletionData->DeletionOption )
         {
         // Filter the volumes to determine which ones should be deleted.
         case Deletion_Volume:
         case Deletion_Partition:
            done = TRUE;         // only 1 volume or partition to delete
            deleteIt = TRUE;     // need to delete this volume or partition
            break;

         case Deletion_All_Volumes:
         case Deletion_Partitions:
            deleteIt = TRUE;
            break;

         case Deletion_All_LVM:
            if( pVolOrPart->isLVM() )
            {
               deleteIt = TRUE;
            }
            break;

         case Deletion_All_Compatibility:
            if( pVolOrPart->isCompatible() )
            {
               deleteIt = TRUE;
            }
            break;

         // Filter the partitions to determine which ones should be deleted.
         case Deletion_All_Unused:
            if( pVolOrPart->hasUnusableSpace() )
            {
               deleteIt = TRUE;
            }
            break;

         case Deletion_All_Primary:
            if( pVolOrPart->isPrimary() )
            {
               deleteIt = TRUE;
            }
            break;

         case Deletion_All_Logical:
            if( pVolOrPart->isLogical() )
            {
               deleteIt = TRUE;
            }
            break;
         }
      }

      // Call the LVM engine to do the deletions.  Don't forget: when the engine
      // deletes a volume, it also deletes all the partitions the volume contained.
      if( deleteIt )
      {
         // Delete this volume or partition.
         *LVMError = pVolOrPart->deleteLVM();
         if( *LVMError )
         {
            MRI message;
            char* pName = new char[max(VOLUME_NAME_SIZE, PARTITION_NAME_SIZE)];
            pVolOrPart->getLVMName( pName );
            message << MRIDelete_Volume << pName << pVolOrPart->getStatus() << MRI_EOM;
//            printf( "Error: failed to delete: %s, LVM error = %d.\n",
//                    pVolOrPart->getLVMName( pName ), pVolOrPart->getStatus() );
            delete [] pName;
            delete pVDSet;
            return FALSE;
         }

         noLVMDeletions = FALSE;   // latch state of something was deleted
      }  // if something to delete
   }  // while !done

   // Warn user if nothing was done.
   if( noLVMDeletions )
   {
      MRI message;
      message << MRIDelete_Failure << MRI_EOM;
//      printf( "Error: no volumes or partitions were found to delete.\n" );
      delete pVDSet;
      return FALSE;  // nothing to commit
   }

   delete pVDSet;
   return TRUE;  // commit LVM deletions
}


//
// Set the indicated compatibility volume or partition to active.  Set all other
// primary partitions to inactive.
//
// Algorithm:
//  - Enumerate all partitions in the system via the drive object.  This provides
//    a complete list of all partitions even if they are not in a volume.
//  - If a volume was requested, verify it is a compatability volume and that it
//    contains a primary partition.
//  - If a partition was reqested, verify the partition exists.
//  - Set all primary partitions in the system to inactive.
//  - Set the requested partition to active.
BOOLEAN doSetStartableCmd( SetStartableStruct* pSetStartableData, CARDINAL32* pLVMError )
{
   ADDRESS   partitionLVMHandle;
   CLIBase*  pPartition;        // pointer to new startable partition

   // Enumerate system drives.  This set includes all partitions in the system.
   CLIDriveSet  driveSet;
   if( driveSet.getStatus() )
   {
      MRI message;
      message << MRISetStart_Enumeration << driveSet.getStatus() << MRI_EOM;
//      printf( "Error: Set startable failed system enumeration, LVM error = %d.\n",
//              driveSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   // User specified a volume.
   if( pSetStartableData->pVolumeName )
   {
      // Enumerate the system's current volumes and all contained partitions.
      CLIVolumeSet   volumeSet;
      if( volumeSet.getStatus() )
      {
         MRI message;
         message << MRISetStart_EnumerateVolumes << volumeSet.getStatus() << MRI_EOM;
//         printf( "Error: Set startable failed to enumerate volumes, LVM error = %d.\n",
//                 volumeSet.getStatus() );
         return FALSE;  // nothing to commit
      }

      CLIBase*     pVolume;
      pVolume = volumeSet.find( pSetStartableData->pVolumeName );
      if( !pVolume )
      {
         MRI message;
         message << MRISetStart_FindVolume << pSetStartableData->pVolumeName << MRI_EOM;
//         printf( "Error: Set startable failed to find volume: %s.\n",
//                 pSetStartableData->pVolumeName );
         return FALSE;  // nothing to commit
      }

      // Verify this volume is a compatability volume.
      if( !pVolume->isCompatible() )
      {
         MRI message;
         message << MRISetStart_BootCompatible << pSetStartableData->pVolumeName << MRI_EOM;
//         printf( "Error: To be set bootable, a volume: %s, must be a compatability volume.\n",
//                 pSetStartableData->pVolumeName );
         return FALSE;  // nothing to commit
      }

      // Verify this volume's partition exists.
      CLIIndex index;
      pPartition = pVolume->getNextPartition( index );
      if( !pPartition )
      {
         MRI message;
         message << MRISetStart_NoPartOnVolume << pSetStartableData->pVolumeName << MRI_EOM;
//         printf( "Error: Set startable failed to find a partition in volume: %s.\n",
//                 pSetStartableData->pVolumeName );
         return FALSE;  // nothing to commit
      }

      // Verify this volume's partition is a primary partition.
      if( !pPartition->isPrimary() )
      {
         MRI message;
         char* pName = new char[PARTITION_NAME_SIZE];
         pPartition->getLVMName( pName );
         message << MRISetStart_FirstMustBePrimary << pSetStartableData->pVolumeName << pName << MRI_EOM;
//         printf( "Error: To be set startable, volume: %s's first partition: %s must be a primary partition.\n",
//                 pSetStartableData->pVolumeName, pPartition->getLVMName( pName ) );
         delete [] pName;
         return FALSE;  // nothing to commit
      }

      // Save the LVM handle to the partition that is to be set active.
      partitionLVMHandle = pPartition->getLVMHandle();
   }
   else if( pSetStartableData->pPartitionName )
   {
      VDPQualifier     VDPQual;

      VDPQual.queryType       = Query_All;
      VDPQual.driveNameType   = pSetStartableData->NameType;
      VDPQual.pDriveName      = pSetStartableData->pDrive;
      VDPQual.pVolumeName     = 0;
      VDPQual.pPartitionName  = pSetStartableData->pPartitionName;
      VDPQual.pFileSystemName = 0;

      pPartition = driveSet.findPartition( VDPQual );
      if( !pPartition )
      {
         MRI message;
         message << MRISetStart_NoPartOnDrive << VDPQual.pPartitionName << VDPQual.pDriveName << MRI_EOM;
//         printf( "Error: Set startable failed to find partition: %s, on drive: %s.\n",
//                 VDPQual.pPartitionName, VDPQual.pDriveName );
         return FALSE;  // nothing to commit
      }

      // Verify this partition is a primary partition.
      if( !pPartition->isPrimary() )
      {
         MRI message;
         message << MRISetStart_MustBePrimary << pSetStartableData->pPartitionName << MRI_EOM;
//         printf( "Error: To be set bootable, partition: %s, must be a primary partition.\n",
//                 pSetStartableData->pPartitionName );
         return FALSE;  // nothing to commit
      }

      // Save the LVM handle to the partition that is to be set active.
      partitionLVMHandle = pPartition->getLVMHandle();
   }
   else
   {
      MRI message;
      message << MRISetStart_NoName << MRI_EOM;
//      printf( "Error: Set startable must specify a partition or volume name.\n" );
      return FALSE;  // nothing to commit
   }

   // Now have a partition to set startable.  First set all other primary partitions
   // as inactive.
   CLIBase*         pPart;
   CLIIndex         index;

   for( pPart = driveSet.getNextPartition( index );
        pPart;
        pPart = driveSet.getNextPartition( index ) )
   {
      if( pPart->isPrimary() )
      {
         Set_Active_Flag( pPart->getLVMHandle(),
                          Partition_Inactive,
                          pLVMError );
         if( *pLVMError )
         {
#ifdef DEBUG
            assert( *pLVMError == LVM_ENGINE_NO_ERROR );
#endif
            MRI message;
            char* pName = new char[PARTITION_NAME_SIZE];
            pPart->getLVMName( pName );
            message << MRISetStart_PartStillActive << pName << *pLVMError << MRI_EOM;
//            printf( "Error: Set startable failed to set partition: %s, to inactive, LVM error = %d.\n",
//                    pPart->getLVMName( pName ), *pLVMError );
            delete [] pName;
            return FALSE;  // nothing to commit
         }
      }
   }

   // Set requested partition to active.
   Set_Active_Flag( partitionLVMHandle,
                    Partition_Active,
                    pLVMError );
   if( *pLVMError )
   {
#ifdef DEBUG
      assert( *pLVMError == LVM_ENGINE_NO_ERROR );
#endif
      MRI message;
      char* pName = new char[PARTITION_NAME_SIZE];
      pPartition->getLVMName( pName );
      message << MRISetStart_PartStillInactive << pName << *pLVMError << MRI_EOM;
//      printf( "Error: Set startable failed to set partition: %s, to active, LVM error = %d.\n",
//              pPartition->getLVMName( pName ), *pLVMError );
      delete [] pName;
      return FALSE;  // nothing to commit
   }

   // Commit all LVM changes.
   return TRUE;
}


//
// Call LVM engine to write a new Master Boot Record to the specified physical drive.
BOOLEAN doNewMBRCmd( NewMBRStruct* pNewMBRData, CARDINAL32* pLVMError )
{
   CARDINAL32   driveNumber;
   VDPQualifier VDPQual;

   // Initialize the search record.
   VDPQual.queryType       = Query_All;
   VDPQual.driveNameType   = DriveNumberType;
   VDPQual.pDriveName      = pNewMBRData->pName;
   VDPQual.pVolumeName     = 0;
   VDPQual.pPartitionName  = 0;
   VDPQual.pFileSystemName = 0;

   if( VDPQual.pDriveName )
   {
      driveNumber = strtol( VDPQual.pDriveName, (char**)NULL, 0 );
   }
   else
   {
      // Define a default drive number
      driveNumber = 1;
      VDPQual.pDriveName = "1";
   }

   // Validate drive number.
   if( driveNumber < 1 || driveNumber > 4 )
   {
      MRI message;
      message << MRINewMBR_InvalidPhysDrive << MRI_EOM;
//      printf( "Error: New MBR command must specify a physical drive in the range of 1 to 4.\n" );
      return FALSE;
   }

   CLIDriveSet  driveSet;   // enumerate the drives
   if( driveSet.getStatus() )
   {
      MRI message;
      message << MRINewMBR_Enumeration << driveSet.getStatus() << MRI_EOM;
//      printf( "Error: New MBR failed system enumeration, LVM error = %d.\n",
//              driveSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   // Locate the physical drive.
   CLIBase*     pDrive;
   CLIIndex     index;
   BOOLEAN      found = FALSE;
   for( pDrive = driveSet.getNext( index );
        pDrive;
        pDrive = driveSet.getNext( index ) )
   {
      if( pDrive->thisIsA( VDPQual ) )
      {
         found = TRUE;
         break;
      }
   }

   // Check that we found the drive.
   if( !found )
   {
      MRI message;
      message << MRINewMBR_NoPhysDrive << (const short)driveNumber << MRI_EOM;
//      printf( "Error: New MBR failed to find physical drive: %d.\n", driveNumber );
      return FALSE;  // nothing to commit
   }

   // We have a drive, so tell LVM engine to write the new MBR.
   New_MBR( pDrive->getLVMHandle(), pLVMError );
   if( *pLVMError )
   {
#ifdef DEBUG
      assert( *pLVMError == LVM_ENGINE_NO_ERROR );
#endif
      MRI message;
      message << MRINewMBR_WriteFail << (const short)driveNumber << *pLVMError << MRI_EOM;
//      printf( "Error: New MBR failed to write new MBR to drive: %d, LVM error = %d.\n",
//              driveNumber, *pLVMError );
      return FALSE;  // nothing to commit
   }

   return TRUE;   // it worked! commit LVM changes
}


//
// Install boot manager on the specified drive.  Note: used CommandStruct as
// the interface sturcture with the parser because this command is so simple.
BOOLEAN doBootmgrCmd( CommandStruct* pBootMgrData, CARDINAL32* pLVMError )
{
   CARDINAL32    driveNumber;
   VDPQualifier VDPQual;

   // Initialize the search record.
   VDPQual.queryType       = Query_All;
   VDPQual.driveNameType   = DriveNumberType;
   VDPQual.pDriveName      = (char *)pBootMgrData->pCommandData;
   VDPQual.pVolumeName     = 0;
   VDPQual.pPartitionName  = 0;
   VDPQual.pFileSystemName = 0;

   if( VDPQual.pDriveName )
   {
      driveNumber = strtol( (const char *)VDPQual.pDriveName, (char**)NULL, 0 );
   }
   else
   {
      MRI message;
      message << MRIBootMgr_NoPhysDrive << MRI_EOM;
//      printf( "Error: must specify the physical drive to install boot manager.\n" );
      return FALSE;  // nothing to commit
   }

   // Validate drive number.
   if( driveNumber < 1 || driveNumber > 2 )
   {
      MRI message;
      message << MRIBootMgr_BadPhysDrive << MRI_EOM;
//      printf( "Error: Install boot manager command must specify a physical drive in the range of 1 to 2.\n" );
      return FALSE;  // nothing to commit
   }

   CLIDriveSet  driveSet;   // enumerate the drives
   if( driveSet.getStatus() )
   {
      MRI message;
      message << MRIBootMgr_Enumeration << driveSet.getStatus() << MRI_EOM;
//      printf( "Error: Install boot manager failed system enumeration, LVM error = %d.\n",
//              driveSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   // Locate the physical drive.
   CLIBase*     pDrive;
   CLIIndex     index;
   BOOLEAN      found = FALSE;
   for( pDrive = driveSet.getNext( index );
        pDrive;
        pDrive = driveSet.getNext( index ) )
   {
      if( pDrive->thisIsA( VDPQual ) )
      {
         found = TRUE;
         break;
      }
   }

   // Check that we found the drive.
   if( !found )
   {
      MRI message;
      message << MRIBootMgr_FindPhysDrive << (const short)driveNumber << MRI_EOM;
//      printf( "Error: Install boot manager failed to find physical drive: %d.\n", driveNumber );
      return FALSE;  // nothing to commit
   }

   // We have a drive, so tell LVM engine to install boot manager on it.
   // Only drives 1 and 2 are acceptable.
   Install_Boot_Manager( driveNumber, pLVMError );
   if( *pLVMError )
   {
#ifdef DEBUG
      assert( *pLVMError == LVM_ENGINE_NO_ERROR );
#endif
      MRI message;
      message << MRIBootMgr_FindDrive << (const short)driveNumber << *pLVMError << MRI_EOM;
//      printf( "Error: Install boot manager failed on drive: %d, LVM error = %d.\n",
//              driveNumber, *pLVMError );
      return FALSE;  // nothing to commit
   }

   return TRUE;   // it worked! commit LVM changes
}


//
// Do special install (SI) commands.
BOOLEAN doSICmd( InstallStruct* pInstallData, LVMCLI_BackEndToVIO* pVIORequest, CARDINAL32* pLVMError )
{
   CARDINAL32  minInstallSize;

   switch( pInstallData->OptionType )
   {
   case Install_FS:
      // Compute minimum installation size in sectors.
      minInstallSize = strtol( pInstallData->pSize, (char**)NULL, 0 ) * 1024 * 2;

      // Set minimum installation size in LVM engine.
      Set_Min_Install_Size( minInstallSize );
      *pLVMError = LVM_ENGINE_NO_ERROR;   // assume this worked?

      // Tell the VIO to display the minimum installation partition size.
      pVIORequest->operation              = DisplayMinInstallSize;
      pVIORequest->minPartitonInstallSize = minInstallSize / (1024 * 2);
      pVIORequest->LVMError               = *pLVMError;
      break;

   default:
      MRI message;
      message << MRISICmd_BadOption << (const short)(pInstallData->OptionType) << MRI_EOM;
//      printf( "Error: special install failed to recognize option: %d.\n", pInstallData->OptionType );
      return FALSE;  // nothing to commit
   }

   return TRUE;   // it worked! commit LVM changes
}


//
// Perform a PRM rediscovery operation.  This is done without opening the LVM engine
// to avoid the overhead of accessing all the drives.  Only PRM devices that have new
// media be accessed.

#define REDISCOVER_DRIVE_IOCTL   0x6A    // Category 9, function 6AH, Rediscover Partitions

BOOLEAN doRediscoverPRMCmd( CARDINAL32* pLVMError )
{
   ULONG                   driveCount        = 0;
   HFILE                   driveHandle       = 0;
   CHAR                    driveName[]       = "1:";
   DDI_Rediscover_param    Rediscovery_Parameters;  /* Used to do a PRM Rediscover so that PRMs with new media will be recognized. */  /* BMR @218946 */
   DDI_Rediscover_data     Rediscovery_Data;        /* Used to do a PRM Rediscover so that PRMs with new media will be recognized. */  /* BMR @218946 */
   CARDINAL32              Parameter_Size;                                                                                             /* BMR @218946 */
   CARDINAL32              Data_Size;                                                                                                  /* BMR @218946 */

   // Get the number of partitionable disks in the system so we know how
   // large a DDI_Rediscover_param structure to allocate.
   *pLVMError = DosPhysicalDisk( INFO_COUNT_PARTITIONABLE_DISKS,
                                 &driveCount,
                                 2,                 // length of driveCount must be 2 bytes
                                 0,
                                 0 );
   if( *pLVMError != NO_ERROR )
   {
      *pLVMError = LVM_ENGINE_INTERNAL_ERROR;
      return FALSE;
   }

   // Continue only if system has 1 or more partitionable drives.
   if( !driveCount )
   {
      return TRUE;
   }

   // Open the first physical drive.  Any drive will do for this.
   *pLVMError = DosPhysicalDisk( INFO_GETIOCTLHANDLE,
                                 &driveHandle,
                                 2,                 // length of driveHandle must be 2 bytes
                                 driveName,
                                 strlen(driveName) + 1 );
   if( *pLVMError != NO_ERROR )
   {
      *pLVMError = LVM_ENGINE_INTERNAL_ERROR;
      return FALSE;
   }

/* BMR Start Fix for @218946 */

   /* Perform a PRM Rediscover operation so that any PRMs with new media will be recognized. */
   Rediscovery_Parameters.DDI_TotalDrives = 0;   /* 0 here indicates that a PRM only rediscover is to be performed instead of a full rediscover. */
   Rediscovery_Parameters.DDI_aDriveNums[0] = 1;
   Rediscovery_Data.DDI_TotalExtends = 0;
   Rediscovery_Data.NewIFSMUnits = 0;

   /* Initialize the IOCTL data. */
   Data_Size =  ( Rediscovery_Data.DDI_TotalExtends * sizeof(DDI_ExtendRecord) ) + sizeof(DDI_Rediscover_data) - sizeof(DDI_ExtendRecord);
   Rediscovery_Data.NewIFSMUnits = 0;

   /* Initialize the IOCTL Parameters. */
   Parameter_Size = ( Rediscovery_Parameters.DDI_TotalDrives * sizeof(UCHAR) ) + sizeof(DDI_Rediscover_param) - sizeof(BYTE);

   // Do the IOCTL call.
   *pLVMError = DosDevIOCtl( driveHandle,
                             IOCTL_PHYSICALDISK,
                             REDISCOVER_DRIVE_IOCTL,
                             &Rediscovery_Parameters,
                             Parameter_Size,
                             &Parameter_Size,
                             &Rediscovery_Data,
                             Data_Size,
                             &Data_Size );

/* BMR End Fix for @218946 */

   // Return TRUE if the IOCTL was successful, FALSE otherwise.
   if( *pLVMError == NO_ERROR )
   {
      *pLVMError = LVM_ENGINE_NO_ERROR;
      return TRUE;
   }
   else
   {
      *pLVMError = LVM_ENGINE_INTERNAL_ERROR;
      return FALSE;
   }

}


// Begin changes for @203285

//
// This command expands the Volume passed in.  Expand allows a Volume
//  to be enlarged by adding additional partitions to it.
//
BOOLEAN doExpandCmd( pExpandStruct pExpandData, CARDINAL32* pLVMError )
{
   ADDRESS    VolHandle = 0;
   CARDINAL32 NumPartitions;
   pPartitionListStruct       pPartitionList;
   CLIBase*   pVDP;

   // Enumerate system volumes so we can look for the one we are interested in.
   CLIVolumeSet  volumeSet;
   if( volumeSet.getStatus() )
   {
      MRI message;
      message << MRIExpand_Enumeration << volumeSet.getStatus() << MRI_EOM;
//      printf( "Error: Expand Volume failed system volume enumeration, LVM error = %d.\n",
//              volumeSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   // Enumerate system drives and ALL partitions.
   CLIDriveSet  driveSet;
   if( driveSet.getStatus() )
   {
      MRI message;
      message << MRIExpand_DriveEnumeration << driveSet.getStatus() << MRI_EOM;
//      printf( "Error: Expand Volume failed system drive enumeration, LVM error = %d.\n",
//              driveSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   // Find the volume of interest.
   pVDP = volumeSet.find( pExpandData->pVolumeName );

   if( !pVDP )
   {
      MRI message;
      message << MRIExpand_NoVolume << pExpandData->pVolumeName << MRI_EOM;
//      printf( "Error: Expand Volume can't find existing volume: %s.\n",
//              pExpandData->pVolumeName );
      return FALSE;
   }

   // Error test for a compatibility volume requesting Expand Volume.
   if( !(pVDP->isLVM() ) )
   {
      MRI message;
      message << MRIExpand_NotLVMVolume << pExpandData->pVolumeName << MRI_EOM;
//      printf( "Error: cannot Expand compatibility volume: %s, Expand requires an LVM volume.\n",
//               pExpandData->pVolumeName );
      return FALSE;
   }


   // Retrieve handle of the Volume of interest.
   VolHandle = pVDP->getLVMHandle();

   // Now convert the Partition List structures into Partition Handles...

   // Count the number of partitions specified for this volume on the command line so
   // we know how much memory to allocate for the array of handles.
   CLIBase*    pPartition;     // existing partition

   for( pPartitionList = pExpandData->pPartitionListParameters, NumPartitions = 0;
        pPartitionList;
        pPartitionList = pPartitionList->pNext )
   {
      NumPartitions++;
   }

   // Error test for no partitions.
   if( !NumPartitions )
   {
      MRI message;
      message << MRIExpand_NoPartList << pExpandData->pVolumeName << MRI_EOM;
//      printf( "Error: Expand Volume requires partition list to expand volume: %s.\n",
//               pExpandData->pVolumeName );
      return FALSE;
   }

   // Create a list of partition handles from the partitionSet.
   ADDRESS* ppPartitionHandleArray = new ADDRESS[NumPartitions];

   // Populate partitionSet with the new and existing partitions specified on the command line.
   for( pPartitionList = pExpandData->pPartitionListParameters, NumPartitions = 0;
        pPartitionList;
        pPartitionList = pPartitionList->pNext )
   {
      VDPQualifier   partQual;
      // Find each existing partition specified in the command line parameters.
      partQual.queryType      = Query_All;
      partQual.driveNameType  = pPartitionList->NameType;
      partQual.pVolumeName    = 0;
      partQual.pDriveName     = pPartitionList->pDrive;
      partQual.pPartitionName = pPartitionList->pPartitionName;

      pPartition = driveSet.findPartition( partQual );
      if( !pPartition )
      {
         MRI message;
         message << MRIExpand_NoPart << partQual.pPartitionName << partQual.pDriveName << MRI_EOM;
//         printf( "Error: Expand Volume failed to find existing partition: %s on drive: %s.\n",
//                 partQual.pPartitionName, partQual.pDriveName );
         delete [] ppPartitionHandleArray;
         return FALSE;
      }

      // Save the partition's handle.
      ppPartitionHandleArray[NumPartitions] = pPartition->getLVMHandle();
      NumPartitions++;
   }

   Expand_Volume( VolHandle, NumPartitions, ppPartitionHandleArray, pLVMError );
   if ( *pLVMError )
   {
      MRI message;
      message << MRIExpand_Volume << pExpandData->pVolumeName << *pLVMError << MRI_EOM;
   }
   delete [] ppPartitionHandleArray;
   if ( *pLVMError )
   {
     return FALSE;
   }
   else
   {
     return TRUE;
   }
}

//
// This command hides the Volume passed in.  A hide makes the Volume
//  inaccessible by the operating system, but does not destroy the data.
//
BOOLEAN doHideCmd( pSTRING pVolumetoHide, CARDINAL32* pLVMError )
{
   ADDRESS    VolHandle = 0;
   CLIBase*   pVDP;

   // Enumerate system volumes so we can look for the one we are interested in.
   CLIVolumeSet  volumeSet;
   if( volumeSet.getStatus() )
   {
      MRI message;
      message << MRIHide_Enumeration << volumeSet.getStatus() << MRI_EOM;
//      printf( "Error: Hide Volume failed system volume enumeration, LVM error = %d.\n",
//              volumeSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   // Find the volume of interest.
   pVDP = volumeSet.find( pVolumetoHide );
   if( !pVDP )
   {
      MRI message;
      message << MRIHide_NoVolume << pVolumetoHide << MRI_EOM;
//      printf( "Error: Hide Volume can't find existing volume: %s.\n",
//              pVolumetoHide );
      return FALSE;
   }

   // Retrieve handle of the Volume of interest.
   VolHandle = pVDP->getLVMHandle();

   Hide_Volume( VolHandle, pLVMError );
   if ( *pLVMError )
   {
      MRI message;
      message << MRIHide_Volume << pVolumetoHide << *pLVMError << MRI_EOM;
      return FALSE;
   }
   else
   {
      return TRUE;
   }
}

// End of changes for @203285

// Start of changes for 209170
// This command enables the logging of the LVM Engine activity to a
// specified file.
BOOLEAN doStartLogCmd( pSTRING pLogFile, CARDINAL32* pLVMError )
{
   // Close any current LVM log file before opening a new one.
   Stop_Logging( pLVMError );                                           // @216357

   Start_Logging( pLogFile, pLVMError );
   if( *pLVMError != LVM_ENGINE_NO_ERROR )
   {                                                                    //
      MRI message;
      message << MRIStart_Logfile << pLogFile << *pLVMError << MRI_EOM;
   }

   // Nothing ever to commit for starting an LVM log file.
   return FALSE;
}
// End of changes for 209170

// Begin changes for @214194
// This command assigns a Drive Letter to the volume of interest.
//  If the volume is hidden, it will become visible again.  If the volume
//  was already visible, it is unmounted and mounted as the new Drive Letter.
BOOLEAN doDriveLetterCmd( pDriveLetterStruct pDriveLetterData, CARDINAL32* pLVMError )
{
   ADDRESS    VolHandle = 0;
   CLIBase*   pVDP;
   char       newDriveLetter;

   // Enumerate system volumes so we can look for the one we are interested in.
   CLIVolumeSet  volumeSet;
   if( volumeSet.getStatus() )
   {
      MRI message;
      message << MRIHide_Enumeration << volumeSet.getStatus() << MRI_EOM;
//      printf( "Error: Assign Drive Letter failed system volume enumeration, LVM error = %d.\n",
//              volumeSet.getStatus() );
      return FALSE;  // nothing to commit
   }

   // Find the volume of interest.
   pVDP = volumeSet.find( pDriveLetterData->pVolumeName );
   if( !pVDP )
   {
      MRI message;
      message << MRIHide_NoVolume << pDriveLetterData->pVolumeName << MRI_EOM;
//      printf( "Error: Assign Drive Letter can't find existing volume: %s.\n",
//              pDriveLetterData->pVolumeName );
      return FALSE;
   }

   // Retrieve handle of the Volume of interest.
   VolHandle = pVDP->getLVMHandle();
   newDriveLetter = pDriveLetterData->pDriveLetter[0];

   Assign_Drive_Letter( VolHandle, newDriveLetter, pLVMError );

   if ( *pLVMError )
   {
     return FALSE;
   }
   else
   {
     return TRUE;
   }
}
// End of changes for @214194

extern "C"
CARDINAL32 ExecuteCommands( pCommandStruct pFirstCommand, LVMCLI_BackEndToVIO* pVIORequest )
{
   CARDINAL32        LVMError = LVM_ENGINE_NO_ERROR;
   pCommandStruct    pCommand;
   BOOLEAN           commitRequired = FALSE;

   // Load translated global text messages.  Includes: LVM errors and some
   // partition/drive status text.
   if( GlobalMRI_Initialize() )
   {
      printf( "LVMCLI, failed to initialize MRI messages." );
      goto exit;
   }

   // Start: 216357
   // Get pointer to first command and execute an initial start logging request.
   // This will cause it to start logging before the engine is opened if it is
   // the first LVM command.  Otherwise a start logging command will take effect
   // after the engine is opened.
   pCommand = pFirstCommand;
   if( pCommand->CommandType == StartLogCmd )
   {
      doStartLogCmd( (pSTRING)pCommand->pCommandData, &LVMError );
      if( LVMError != LVM_ENGINE_NO_ERROR ) goto exit;

      // Get pointer to next command.  Check for last command.
      pCommand = pCommand->pNextCommand;
      if( !pCommand ) goto exit;
   }
   // End: 216357

   if( pCommand->CommandType == SICmd )  // SI cannot open/close the engine
   {
     // Special Install requests can return requests for the VIO in VIORequest.
     doSICmd( (InstallStruct*)pCommand->pCommandData, pVIORequest, &LVMError );
   }
   else if( pCommand->CommandType == RediscoverPRMCmd )  // Rediscover PRM does not open the engine
   {
     // Rediscover Partitionable/Removable drives.
     doRediscoverPRMCmd( &LVMError );
   }
   else  // all other commands
   {
     while( pCommand && (LVMError == LVM_ENGINE_NO_ERROR)  )
     {
        switch( pCommand->CommandType )
        {
        case BootmgrCmd :
           commitRequired |= doBootmgrCmd( pCommand, &LVMError );
           break;

        case CreateCmd :
           commitRequired |= doCreateCmd( (CreationStruct*)pCommand->pCommandData, &LVMError );
           break;

        case DeleteCmd :
           commitRequired |= doDeleteCmd( (DeletionStruct*)pCommand->pCommandData, &LVMError );
           break;

  // Begin changes for @203285
        case ExpandCmd :
           commitRequired |= doExpandCmd( (pExpandStruct)pCommand->pCommandData, &LVMError );
           break;

        case HideCmd :
           commitRequired |= doHideCmd( (pSTRING)pCommand->pCommandData, &LVMError );
           break;
  // End of changes for @203285

  // Begin changes for @214194
        case DriveLetterCmd :
           commitRequired |= doDriveLetterCmd( (pDriveLetterStruct)pCommand->pCommandData, &LVMError );
           break;
  // End of changes for @214194

        case NewMBRCmd :
           commitRequired |= doNewMBRCmd( (NewMBRStruct*)pCommand->pCommandData, &LVMError );
           break;

        case NoCmd :
           LVMError = LVM_ENGINE_NO_ERROR;
           break;

        case QueryCmd :
           commitRequired |= doQueryCmd( (QueryStruct*)pCommand->pCommandData, &LVMError );
           break;

        case SetNameCmd :
           commitRequired |= doSetNameCmd( (NameChangeStruct*)pCommand->pCommandData, &LVMError );
           break;

        case SetStartableCmd :
           commitRequired |= doSetStartableCmd( (SetStartableStruct*)pCommand->pCommandData, &LVMError );
           break;

  // Begin changes for @209170
        case StartLogCmd :
           doStartLogCmd( (pSTRING)pCommand->pCommandData, &LVMError );
           break;
  // End of changes for @209170

        case SICmd :
        {
           // Special Install doesn't belong here anymore.
           MRI message;
           message << MRISIOption_OneAtATime << MRI_EOM;
//           printf( "Error: the SI option cannot operate with other LVM commands.\n" );
           LVMError = LVM_ENGINE_OPERATION_NOT_ALLOWED;
        }
           break;

        case ErrorCmd :
        default:
           // Bad command structure, normally ignore this.
#ifdef DEBUG
           assert( pCommand->CommandType == 0 );
#endif
           break;
        } // end-switch

        // Any error causes termination without commit.
        if( LVMError != LVM_ENGINE_NO_ERROR ) goto exit;

        // Get pointer to next command.
        pCommand = pCommand->pNextCommand;
     } // end-while

     if( commitRequired )
     {
        // Commit the LVM updates.
        Commit_Changes( &LVMError );

        // If physical disk partition changes were made such that a system reboot
        // is required, then tell the user.
        if( Reboot_Required() )
        {
           // If an error was generated above then keep it, otherwise se the error
           // return to the reboot required code.
           if( LVMError == LVM_ENGINE_NO_ERROR ) LVMError = LVM_ENGINE_REBOOT_REQUIRED;
           {
              MRI message;
              message << MRIReBoot_Required << MRI_EOM;
//              printf( "Warning: Reboot system before attempting more LVM commands.\n" );
           } // end-if
        } // end-if
     } // end-if
   } // end-else

exit:
   // Save the LVM engine error in the VIO request structure.
   pVIORequest->LVMError = LVMError;

   return LVMError;
}
