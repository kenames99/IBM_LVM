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
 * Module: error.c
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
 * Notes: This module uses the OS/2 Message Management API.
 *
 */

#define INCL_DOSMISC
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "constant.h"
#include "gbltypes.h"
#include "debug.h"
#include "utility.h"
#include "error.h"


/*--------------------------------------------------
 * Global Variables
 --------------------------------------------------*/
pSTRING*    ppLVMEngineErrorText;
pSTRING*    ppPartitionTypeText;
pSTRING*    ppPartitionStatusText;
pSTRING*    ppVolumeTypeText;
pSTRING*    ppBootStatusText;
pSTRING*    ppFSNameText;

/* Number of messages in each array. */
ULONG       nLVMEngineError;
ULONG       nPartitionType;
ULONG       nPartitionStatus;
ULONG       nVolumeType;
ULONG       nBootStatus;
ULONG       nFSName;



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
 There are no Local Functions
--------------------------------------------------*/




/*--------------------------------------------------
 Implementation of Public Functions
--------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: ReportError                                      */
/*                                                                   */
/*   Descriptive Name: Builds an error message and displays it to the*/
/*                     user.                                         */
/*                                                                   */
/*   Input: CARDINAL ErrorNumber : One of the error numbers defined  */
/*                                 below under Notes.                */
/*                                                                   */
/*   Output: If Success : The requested error message is displayed   */
/*                        to the user.                               */
/*                                                                   */
/*           If Failure : A series of beeps will be generated using  */
/*                        the computer's speaker.                    */
/*                                                                   */
/*   Error Handling: If an error occurs, then a series of beeps is   */
/*                   generated using the system speaker.             */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*********************************************************************/
void ReportError(unsigned int ErrorNumber)
{

   if (ErrorNumber >= Exceeded_Max_Cmdline) {
     APIRET rc=NO_ERROR;
     ULONG returned_message_length=0;
     PCHAR  pMessageBuf;


     pMessageBuf = (PCHAR) malloc(MAX_MESSAGE_SIZE);
     if (pMessageBuf != NULL) {
        rc = DosGetMessage ( NULL, 0, pMessageBuf, MAX_MESSAGE_SIZE - 1,
                             (ULONG)ErrorNumber, MESSAGE_FILE, &returned_message_length );
        if (rc == NO_ERROR) {
           rc = DosPutMessage(HF_STDERR, returned_message_length, pMessageBuf);
           if (rc != NO_ERROR) {
              printf("DosPutMessage error: return code = %ld", rc);
           } /* endif */
        } else {
           printf("DosGetMessage error: return code = %ld", rc);
        } /* endif */

        free( pMessageBuf );
     } else {
        printf("Could not allocate buffer into which to store the message text.\n");
     } /* endif */
   } else {
      printf("Unknown Error\n");
   } /* endif */
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: ReportError2                                     */
/*                                                                   */
/*   Descriptive Name: Builds an error message and displays it to the*/
/*                     user.                                         */
/*                                                                   */
/*   Input: CARDINAL ErrorNumber : One of the error numbers defined  */
/*                                 below under Notes.                */
/*                                                                   */
/*   Output: If Success : The requested error message is displayed   */
/*                        to the user.                               */
/*                                                                   */
/*           If Failure : A series of beeps will be generated using  */
/*                        the computer's speaker.                    */
/*                                                                   */
/*   Error Handling: If an error occurs, then a series of beeps is   */
/*                   generated using the system speaker.             */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*********************************************************************/
void ReportError2(unsigned int ErrorNumber, CARDINAL32 Error )
{

   if (ErrorNumber >= Exceeded_Max_Cmdline) {
     APIRET rc=NO_ERROR;
     ULONG returned_message_length=0;
     PCHAR  pMessageBuf;


     pMessageBuf = (PCHAR) malloc(MAX_MESSAGE_SIZE);
     if (pMessageBuf != NULL) {
        rc = DosGetMessage ( NULL, 0, pMessageBuf, MAX_MESSAGE_SIZE - 1,
                             (ULONG)ErrorNumber, MESSAGE_FILE, &returned_message_length );
        if (rc == NO_ERROR) {
	   char StrBuf[10];
	   
	   sprintf( StrBuf, "RC = %d", Error );
	   strcat( pMessageBuf, StrBuf );
           rc = DosPutMessage(HF_STDERR, returned_message_length, pMessageBuf);
           if (rc != NO_ERROR) {
              printf("DosPutMessage error: return code = %ld", rc);
           } /* endif */
        } else {
           printf("DosGetMessage error: return code = %ld", rc);
        } /* endif */

        free( pMessageBuf );
     } else {
        printf("Could not allocate buffer into which to store the message text.\n");
     } /* endif */
   } else {
      printf("Unknown Error\n");
   } /* endif */
}


/*********************************************************************/
/*                                                                   */
/*   Function Name: ReportParseError                                 */
/*                                                                   */
/*   Descriptive Name:  Displays a message that indicates the parse  */
/*                      error that was encountered and where on the  */
/*                      command line the problem occurred.        .  */
/*                                                                   */
/*   Input: CARDINAL ErrorNumber - The number of the error message to*/
/*                        be displayed by the ReportError function   */
/*                        found in ERROR.H.                          */
/*          LIST Tokens - Token list from which to obtain starting   */
/*                        token position.                            */
/*          pSTRING Text - Pointer to the command line string.        */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
/* void ReportParseError(pSTRING pErrorText, LIST Tokens, pSTRING pCommandLine) */
void ReportParseError(ParseErrorType ParseErrorNumber, LIST Tokens, pSTRING pCommandLine)
{
   unsigned int   TokenStartingPosition;
   CARDINAL       Error;
   pSTRING        pArrowStr=NULL;
   int            i;
   char           LeaderChar[]=".";
   char           PointerChar[]="^";
   PCHAR          pLeaderBuf=NULL;
   PCHAR          pPointerBuf=NULL;
   char *         pPointerStr=NULL;

   TokenStartingPosition = GetTokenPosition(Tokens, &Error);
   if ( !Error ) {
      APIRET rc=NO_ERROR;
      ULONG returned_message_length=0;

      /* Retrieve the dot leader character that is to be used to create the line */
      /* that points to the starting column of the token in error. */
      pLeaderBuf = (PCHAR) malloc(MAX_MESSAGE_SIZE);
      if (pLeaderBuf != NULL) {
         rc = DosGetMessage ( NULL, 0, pLeaderBuf, MAX_MESSAGE_SIZE - 1,
                              (ULONG)Leader_Character, MESSAGE_FILE, &returned_message_length );
         if (rc == NO_ERROR) {
            LeaderChar[0] = pLeaderBuf[9];
            /* Retrieve the pointer character also */
            pPointerBuf = (PCHAR) malloc(MAX_MESSAGE_SIZE);
            if (pPointerBuf != NULL) {
               rc = DosGetMessage ( NULL, 0, pPointerBuf, MAX_MESSAGE_SIZE - 1,
                                 (ULONG)Pointer_Character, MESSAGE_FILE, &returned_message_length );
               if (rc == NO_ERROR) {
                  PointerChar[0] = pPointerBuf[9];
               } else {
                  printf("DosGetMessage error: return code = %ld", rc);
               } /* endif */
            } else {
               ReportError(Not_Enough_Memory);
            } /* endif */
         } else {
            printf("DosGetMessage error: return code = %ld", rc);
         } /* endif */
      } else {
         ReportError(Not_Enough_Memory);
      } /* endif */

      strcat(pCommandLine, "\r\n");
      rc = DosPutMessage(HF_STDERR, strlen(pCommandLine), pCommandLine);
      if (rc != NO_ERROR) {
         printf("DosPutMessage error: return code = %ld", rc);
         return;
      } /* endif */

      pPointerStr=(pSTRING) malloc(261);
      if (pPointerStr != NULL) {
         strcpy(pPointerStr, "");
         for (i=0;i<TokenStartingPosition;i++ ) {
            strcat(pPointerStr, LeaderChar);
         } /* endfor */

         if (TokenStartingPosition <= 260) {
            pPointerStr[TokenStartingPosition] = PointerChar[0];
            pPointerStr[TokenStartingPosition+1] = '\0';

            strcat(pPointerStr, "\r\n");
            rc = DosPutMessage(HF_STDERR, strlen(pPointerStr), pPointerStr);
            if (rc != NO_ERROR) {
               printf("DosPutMessage error: return code = %ld", rc);
               return;
            } /* endif */
         } else {
            printf("Error starting at column %d\n",TokenStartingPosition);
         } /* endif */
         ReportError(ParseErrorNumber);
      } else {
         ReportError(Not_Enough_Memory);
      } /* endif */
   } else {
      ReportError(Internal_Error);
   } /* endif */

   return;
}

/*********************************************************************/
/*                                                                   */
/*   Function Name: ReportParseError                                 */
/*                                                                   */
/*   Descriptive Name:  Displays a message that indicates the parse  */
/*                      error that was encountered and where on the  */
/*                      command line the problem occurred.        .  */
/*                                                                   */
/*   Input: CARDINAL ErrorNumber - The number of the error message to*/
/*                        be displayed by the ReportError function   */
/*                        found in ERROR.H.                          */
/*          LIST Tokens - Token list from which to obtain starting   */
/*                        token position.                            */
/*          pSTRING Text - Pointer to the command line string.        */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
/* void ReportParseError(pSTRING pErrorText, LIST Tokens, pSTRING pCommandLine) */
void ReportFeatureParseError( char *pMsg, LIST Tokens, pSTRING pCommandLine)
{
   unsigned int   TokenStartingPosition;
   CARDINAL       Error;
   pSTRING        pArrowStr=NULL;
   int            i;
   char           LeaderChar[]=".";
   char           PointerChar[]="^";
   PCHAR          pLeaderBuf=NULL;
   PCHAR          pPointerBuf=NULL;
   char *         pPointerStr=NULL;

   TokenStartingPosition = GetTokenPosition(Tokens, &Error);
   if ( !Error ) {
      APIRET rc=NO_ERROR;
      ULONG returned_message_length=0;

      /* Retrieve the dot leader character that is to be used to create the line */
      /* that points to the starting column of the token in error. */
      pLeaderBuf = (PCHAR) malloc(MAX_MESSAGE_SIZE);
      if (pLeaderBuf != NULL) {
         rc = DosGetMessage ( NULL, 0, pLeaderBuf, MAX_MESSAGE_SIZE - 1,
                              (ULONG)Leader_Character, MESSAGE_FILE, &returned_message_length );
         if (rc == NO_ERROR) {
            LeaderChar[0] = pLeaderBuf[9];
            /* Retrieve the pointer character also */
            pPointerBuf = (PCHAR) malloc(MAX_MESSAGE_SIZE);
            if (pPointerBuf != NULL) {
               rc = DosGetMessage ( NULL, 0, pPointerBuf, MAX_MESSAGE_SIZE - 1,
                                 (ULONG)Pointer_Character, MESSAGE_FILE, &returned_message_length );
               if (rc == NO_ERROR) {
                  PointerChar[0] = pPointerBuf[9];
               } else {
                  printf("DosGetMessage error: return code = %ld", rc);
               } /* endif */
            } else {
               ReportError(Not_Enough_Memory);
            } /* endif */
         } else {
            printf("DosGetMessage error: return code = %ld", rc);
         } /* endif */
      } else {
         ReportError(Not_Enough_Memory);
      } /* endif */

      strcat(pCommandLine, "\r\n");
      rc = DosPutMessage(HF_STDERR, strlen(pCommandLine), pCommandLine);
      if (rc != NO_ERROR) {
         printf("DosPutMessage error: return code = %ld", rc);
         return;
      } /* endif */

      pPointerStr=(pSTRING) malloc(261);
      if (pPointerStr != NULL) {
         strcpy(pPointerStr, "");
         for (i=0;i<TokenStartingPosition;i++ ) {
            strcat(pPointerStr, LeaderChar);
         } /* endfor */

         if (TokenStartingPosition <= 260) {
            pPointerStr[TokenStartingPosition] = PointerChar[0];
            pPointerStr[TokenStartingPosition+1] = '\0';

            strcat(pPointerStr, "\r\n");
            rc = DosPutMessage(HF_STDERR, strlen(pPointerStr), pPointerStr);
            if (rc != NO_ERROR) {
               printf("DosPutMessage error: return code = %ld", rc);
               return;
            } /* endif */
         } else {
            printf("Error starting at column %d\n",TokenStartingPosition);
         } /* endif */
         printf("%s\r\n", pMsg);
      } else {
         ReportError(Not_Enough_Memory);
      } /* endif */
   } else {
      ReportError(Internal_Error);
   } /* endif */

   return;
}


/*
 * Initialize global translated message text.  The messages initialized here
 * are shared between the LVM VIO and LVM CLI backend.  Only the MRI message
 * text in lvm.txt is shared, the memory structures that contain the translated
 * messages are seperate and LVM VIO has its own initialization for these
 * messages.
 */
ULONG GlobalMRI_Initialize()
{
   ULONG   rc = 0;

   /* Allocate and initialize LVM Engine Error strings. */
   rc = MRI_Initialize( &ppLVMEngineErrorText, &nLVMEngineError,
                        LVM_ENGINE_ERROR_MSGLEN, LVM_ENGINE_ERRORS );
   if( rc ) return rc;

   /* Allocate and initialize Partition Type strings. */
   rc = MRI_Initialize( &ppPartitionTypeText, &nPartitionType,
                        PARTITION_TYPE_STRING_MSGLEN, PARTITION_TYPE_STRINGS );
   if( rc ) return rc;

   /* Allocate and initialize Partition Status strings. */
   rc = MRI_Initialize( &ppPartitionStatusText, &nPartitionStatus,
                        PARTITION_STATUS_STRING_MSGLEN, PARTITION_STATUS_STRINGS );
   if( rc ) return rc;

   /* Allocate and initialize Volume Type strings. */
   rc = MRI_Initialize( &ppVolumeTypeText, &nVolumeType,
                        VOLUME_TYPE_STRING_MSGLEN, VOLUME_TYPE_STRINGS );
   if( rc ) return rc;

   /* Allocate and initialize Bootable Status strings. */
   rc = MRI_Initialize( &ppBootStatusText, &nBootStatus,
                        BOOT_STATUS_STRING_MSGLEN, BOOT_STATUS_STRINGS );

   /* Allocate and initialize Filesystem Name strings. */
   rc = MRI_Initialize( &ppFSNameText, &nFSName,
                        FS_NAME_STRING_MSGLEN, FS_NAME_STRINGS );

   return rc;
}


/*
 * Get the translated messaage text and load it into fixed length records so they
 * can be displayed in tabular formats.
 */
CARDINAL32 MRI_Initialize( pSTRING** ppMessageArray, CARDINAL32* nMessages, CARDINAL32 messageLength, CARDINAL32 messageNum )
{
   PCHAR      pRawMessageBuf;
   ULONG      returnedMessageLength;
   ULONG      rc = -1;
   PCHAR      pLineStart, pBuf;
   ULONG      messageIndex;

   /* Malloc a temporary buffer to hold the raw message text. */
   pRawMessageBuf = (PCHAR)malloc(MAX_MESSAGE_SIZE);
   if( !pRawMessageBuf ) goto ErrorExit;

   /* Get the raw message text. */
   rc = DosGetMessage( NULL, 0, pRawMessageBuf, MAX_MESSAGE_SIZE - 1,
                       messageNum, MESSAGE_FILE, &returnedMessageLength );
   if( rc ) goto ErrorExit;

   /* Count the number of messages in the file.  One message per line. */
   *nMessages = 0;
   pRawMessageBuf[returnedMessageLength] = '\0';
   for( pBuf = pRawMessageBuf; *pBuf; pBuf++ )
   {
      if( *pBuf == '\n' ) (*nMessages)++;
   }

   /* Allocate an array of pointers for these messages. */
   *ppMessageArray = (PCHAR*)malloc( *nMessages * sizeof(PCHAR) );
   if( !*ppMessageArray ) goto ErrorExit;

   /* Copy each message to its own buffer. */
   messageIndex = 0;
   pLineStart = pRawMessageBuf;
   if( pRawMessageBuf[0] == '\r' && pRawMessageBuf[1] == '\n' )
   {
      /* Skip first \r\n */
      pLineStart += 2;
   }

   /* Isolate the individual messages and copy them out to new buffers */
   for( pBuf = pLineStart; *pBuf; pBuf++ )
   {
      if( *pBuf == '\r' )
      {
         /* Ignore carriage returns. */
         *pBuf = ' ';
      }
      else if( *pBuf == '\n' )
      {
         /* New line means end of message, so copy it out, unless its a comment. */
         *pBuf = '\0';
         if( messageIndex < *nMessages )
         {
            (*ppMessageArray)[messageIndex] = malloc( messageLength + 1 );
            if( !(*ppMessageArray)[messageIndex] ) goto ErrorExit;

            /* Blank fill the message so all messages are same length for display purposes. */
            memset( (*ppMessageArray)[messageIndex], ' ', messageLength );

            /* Copy message except for null terminator. */
            strncpy( (*ppMessageArray)[messageIndex], pLineStart, (pBuf - pLineStart) );

            /* Null terminate the new message. */
            ((*ppMessageArray)[messageIndex])[messageLength] = '\0';

            messageIndex++;
            pLineStart = pBuf + 1;                   /* skip over \0 */
         }
      }
   }

   /* Free the buffer used to hold the raw message text. */
   free( pRawMessageBuf );
   return 0;

ErrorExit:
   if( pRawMessageBuf ) free( pRawMessageBuf );

   if( *ppMessageArray )
   {
      PCHAR*   ppBuf;

      for( ppBuf = *ppMessageArray, messageIndex = 0;
           messageIndex < *nMessages; ppBuf++, messageIndex++ )
      {
         if( *ppBuf ) free( *ppBuf );
      }
      free( *ppMessageArray );
   }

   return rc;
}
