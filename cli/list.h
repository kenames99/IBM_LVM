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
 * Module: list.h
 */

/*
 * Change History:
 *
 */

/*
 * Functions: void        InitializeTokenList
 *            void        AppendToken
 *            void        GetToken
 *            void        ReplaceToken
 *            void        DeleteToken
 *            unsigned int GetTokenPosition
 *            ADDRESS     GetTokenHandle
 *            CARDINAL32  GetTokenListSize
 *            BOOLEAN     TokenListEmpty
 *            void        EliminateTokenList
 *            void        NextToken
 *            void        GoToStartOfTokenList
 *            void        GoToSpecifiedToken
 *            void        GetNextToken
 *
 * Description:  This module implements a simple, generic, singly linked list.
 *               Data objects of any type can be placed into a linked list
 *               created by this module.  Futhermore, data objects of different
 *               types may be placed into the same linked list.
 *
 * Notes:  This linked list implementation makes use of the concept of the
 *         current item.  In any non-empty list, one item in the list will
 *         be designated as the current item.  When any of the following
 *         functions are called, they will operate upon the current item
 *         only: GetToken, ReplaceToken, DeleteToken, GetTokenPosition, NextToken.
 *         The user of this module may set the current item through the use
 *         of the GoToStartOfTokenList and NextToken functions.  GoToStartOfTokenList
 *         makes the first item in the list the current item.  NextToken
 *         takes the current item in the list, and makes the item which
 *         follows it the current item.  By using these two functions
 *         the user may make any item in the list the current item.
 *
 *         This module is single threaded.  If used in a multi-threaded
 *         environment, the user must implement appropriate access controls.
 *
 *         When an item is appended to a list, this module allocates memory
 *         on the heap to hold the item and then copies the item to the memory
 *         that it allocated.  This allows local variables to be safely
 *         appended to a list.  However, it should be noted that under
 *         certain circumstances a copy of the entire data item will NOT
 *         be made.  Specifically, if the data item is a structure or array
 *         containing pointers, then the data pointed to by the pointers will
 *         NOT be copied even though the structure or array is!  This results
 *         from the fact that, when an item is being appended to a list, the
 *         user provides just an address and size.  This module assumes that
 *         the item to append lies in a contiguous block of memory at the
 *         address provided by the user.  This module has no way of knowing
 *         the structure of the data at the specified address, and therefore
 *         can not know about any embedded pointers which may lie within
 *         that block of memory.
 *
 * 6/12/95 This module now employs the concept of a handle.  A handle is a
 *         reference to a specific item in a list which allows that item to
 *         be made the current item in the list quickly.  Example:  If you
 *         use the GetTokenHandle function to get a handle for the current item
 *         (lets call the item B1), then, regardless of where you are in the
 *         list (or any reodering of the items in the list), you can make item
 *         B1 the current item by passing its handle to the GoToSpecifiedItem
 *         function.
 *
 */

#ifndef LVM_CLI_LIST_H

#define LVM_CLI_LIST_H  1

/*--------------------------------------------------
 * Type definitions
 --------------------------------------------------*/

#include "gbltypes.h"
#include "lvm_list.h"

typedef DLIST LIST;


/************************************************
 *           Types Available                    *
 ************************************************/

/* When the scanner creates a token, it characterizes the token.  The various
   characterizations available are defined in the state table above.  Each
   characterization in TokenTypes corresponds to the characterization listed
   in the state table.                                                        */
typedef enum {
                 AcceptableCharsStr,
                 All_CLI,
                 BestFit,
                 BootDOS,
                 BootOS2,
                 Bootable,
                 CR_CLI,
                 CRI,
                 Compatibility,
                 Drive,
                 Existing,
                 Expand,
                 FS,
                 FirstFit,
                 Freespace,
                 FromEnd,
                 FromLargest,
                 FromSmallest,
                 FromStart,
                 LVM,
                 LastFit,
                 Logical,
                 New,
                 NoBoot,
                 NonBootable,
                 NotBootable,
                 Partition,
                 Primary,
                 RB,
                 Size,
                 Unusable,
                 Unused,
                 Volume,
                 Volumes,
                 Comma,
                 Number,
                 Colon,
                 Space,
                 Tab,
                 MultiSpace,
                 MultiTab,
                 String,
                 FileNameStr,
                 SemiColon,
                 Eof,
                 Separator,
                 Open_Paren,                    /* ( */
                 Close_Paren,                   /* ) */
                 Open_Bracket,                  /* [ */
                 Close_Bracket,                 /* ] */
                 Open_Brace,                    /* { */
                 Close_Brace,                   /* } */
                 EQ_Sign,                       /* = */
                 Bootmgr,
                 Create,
                 Delete,
                 DriveLetter,
                 File,
                 Hide,
                 Install,
                 NewMBR,
                 Query,
                 RediscoverPRM,
                 SetName,
                 SetStartable,
                 SI,
                 SlashSize,
                 StartLog
             } TokenTypes;

/* The following structure represents a Token.  The scanner breaks the
   command line into a series of tokens, which are then placed into
   a list.  This list is returned to the caller, who passes the list
   to the parser for parsing.                                           */
typedef struct {
                 pSTRING          pTokenText;   /* Actual text from the command line */
                 TokenTypes       TokenType;    /* The characterization of the token by the scanner. */
               } Token;

/* The LIST module requires that all items placed into the list have a
   TAG associated with them.  The TAG used for the tokens will be
   TokenTag.                                                            */
#define TokenTag     10



/************************************************
 *           Functions Available                *
 ************************************************/



/*
 * The parameter *Error is set by every function in this module.  It
 * will be set to 0 to indicate success, and will be > 0 if an
 * error occurs.  The following table lists the possible error codes:
 *     0 : No error.
 *     1 : Out of memory
 *     2 : Memory has been corrupted!
 *     3 : Bad List Record!
 *     4 : List Record not initialized yet!
 *     5 : List Record already initialized!
 *     6 : List is empty!
 *     7 : Item size mismatch!
 *     8 : Bad item pointer!
 *     9 : Item has zero size!
 *    11 : Already at end of list!
 *    12 : Already at start of list!
 *    13 : Bad Handle!
 */


/*********************************************************************/
/*                                                                   */
/*   Function Name:  InitializeTokenList                             */
/*                                                                   */
/*   Descriptive Name: This function allocates and initializes the   */
/*                     data structures associated with a list and    */
/*                     then returns a pointer to these structures.   */
/*                                                                   */
/*   Input: LIST * NewList : The address of a variable of type LIST  */
/*                           which needs to be initialized to point  */
/*                           to a new list.                          */
/*          CARDINAL * Error : The address of a variable into which  */
/*                             any error codes are to be placed.     */
/*                                                                   */
/*   Output: If Success :                                            */
/*                        *NewList is set to point to the            */
/*                                 ControlNode of the new list.      */
/*                        *Error is set to 0.                        */
/*                                                                   */
/*           If Failure : *NewList is set to NULL unless it already  */
/*                                 points to a valid list.           */
/*                        *Error is set to an error code.  The error */
/*                                 codes are specified in list.h     */
/*                                                                   */
/*   Error Handling:  The function will only fail if it can not      */
/*                    allocate enough memory to create the new list  */
/*                    or if *NewList already points to a valid list. */
/*                    In the first case (memory allocation failure)  */
/*                    this function frees any memory that it had     */
/*                    allocated prior to the failure and then returns*/
/*                    an error code.  In the case where *NewList     */
/*                    already points to a valid list, *NewList is not*/
/*                    altered and *Error is set to an error code.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  If *NewList is not NULL, then this module will          */
/*           dereference it assuming that it points to a list and    */
/*           check for the VerifyValue in the Verify field.  This    */
/*           could result in a protection violation under a          */
/*           protected mode operating system.  However, under a real */
/*           mode operating system such as DOS, this is not a problem*/
/*           and the check performed is often useful for debugging   */
/*           purposes.                                               */
/*                                                                   */
/*           It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void InitializeTokenList( LIST *  NewList, CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name: AppendToken                                      */
/*                                                                   */
/*   Descriptive Name:  This function appends an item to the LIST.   */
/*                                                                   */
/*   Input:  LIST          ListToAppendTo : The list to which the    */
/*                                          data item is to be       */
/*                                          appended                 */
/*           CARDINAL      ItemSize : The size of the data item, in  */
/*                                    bytes.                         */
/*           Token *       MyToken : The address of the data         */
/*                                   to append to the list           */
/*           unsigned int  Position: The starting position of the    */
/*                                   item being appended to the list */
/*           CARDINAL *    Error : The address of a variable to hold */
/*                                 the error return code.            */
/*                                                                   */
/*   Output:  If the operation is successful, then *Error will be    */
/*            set to 0.  If the operation fails, then *Error will    */
/*            contain an error code.                                 */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                       ListToAppendTo does not point to a valid    */
/*                           list                                    */
/*                       ItemSize is 0                               */
/*                       ItemLocation is NULL                        */
/*                       The memory required can not be allocated.   */
/*                   If this routine fails, an error code is returned*/
/*                   and any memory allocated by this function is    */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  The item to append is copied to the heap to             */
/*           avoid possible conflicts with the usage of              */
/*           local variables in functions which process              */
/*           LISTs.  However, a pointer to a local variable          */
/*           should not be appended to the LIST.                     */
/*                                                                   */
/*           It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
void AppendToken( LIST          ListToAppendTo,
                  CARDINAL      ItemSize,
                  Token *       MyToken,
                  unsigned int  ItemPosition,
                  CARDINAL *    Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  DeleteToken                                     */
/*                                                                   */
/*   Descriptive Name:  This function removes the curren item from   */
/*                      list and frees the memory associated with it.*/
/*                                                                   */
/*   Input:  LIST       ListToDeleteFrom : The list whose current    */
/*                                         item is to be deleted.    */
/*           CARDINAL * Error : The address of a variable to hold    */
/*                                 the error return code.            */
/*                                                                   */
/*   Output:  If the operation is successful, then *Error will be    */
/*            set to 0.  If the operation fails, then *Error will    */
/*            contain an error code.                                 */
/*                                                                   */
/*   Error Handling: This function will fail if ListToDeleteFrom is  */
/*                   not a valid list, or if ListToDeleteFrom is     */
/*                   empty.                                          */
/*                   If this routine fails, an error code is returned*/
/*                   in *Error.                                      */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void DeleteToken (LIST ListToDeleteFrom, CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetToken                                        */
/*                                                                   */
/*   Descriptive Name:  This function copies the current item in the */
/*                      list to a buffer provided by the caller.     */
/*                                                                   */
/*   Input:  LIST   ListToGetItemFrom : The list whose current item  */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL ItemSize : What the caller thinks the size of  */
/*                               the current item is.                */
/*           Token *       MyToken : The address of a buffer to hold */
/*                                   the data being returned.        */
/*           unsigned int * ItemPosition : Starting position of token*/
/*           CARDINAL * Error : The address of a variable to hold    */
/*                              the error return code.               */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The buffer at ItemLocation will contain a copy of */
/*                    the current item from ListToGetItemFrom.       */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                                                                   */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemLocation is NULL                      */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void GetToken( LIST           ListToGetItemFrom,
               CARDINAL       ItemSize,
               Token *        MyToken,
               unsigned int * ItemPosition,
               CARDINAL *     Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ReplaceToken                                    */
/*                                                                   */
/*   Descriptive Name:  This function replaces the current item in   */
/*                      the list with the one provided as its        */
/*                      argument.                                    */
/*                                                                   */
/*   Input: LIST   ListToReplaceItemIn : The list whose current item */
/*                                       is to be replaced           */
/*          CARDINAL ItemSize : The size, in bytes, of the           */
/*                              replacement item                     */
/*           Token *       MyToken : The address of the replacement  */
/*                                   data.                           */
/*          unsigned int  ItemPosition : Starting position of token  */
/*          CARDINAL * Error : The address of a variable to hold the */
/*                             error return code                     */
/*                                                                   */
/*   Output:  If Successful then *Error will be set to 0.            */
/*            If Unsuccessful, then *Error will be set to a non-zero */
/*              error code.                                          */
/*                                                                   */
/*   Error Handling:  This function will fail under the following    */
/*                    conditions:                                    */
/*                         ListToReplaceItemIn is empty              */
/*                         ItemSize is 0                             */
/*                         ItemLocation is NULL                      */
/*                         The memory required can not be allocated. */
/*                    If any of these conditions occurs, *Error      */
/*                    will contain a non-zero error code.            */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
void ReplaceToken( LIST         ListToReplaceItemIn,
                   CARDINAL     ItemSize,
                   Token *      MyToken,
                   unsigned int ItemPosition,
                   CARDINAL *   Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetTokenPosition                                */
/*                                                                   */
/*   Descriptive Name:  This function returns the starting position  */
/*                      of the current item in the list.             */
/*                                                                   */
/*   Input:  LIST   ListToGetTokenPositionFrom : The list from which */
/*                    the starting position of the current item is to*/
/*                                     be returned                   */
/*           CARDINAL * Error : The address of a variable to hold the*/
/*                              error return code                    */
/*                                                                   */
/*   Output:  If successful, the function returns the item position  */
/*               associated with the current item in                 */
/*               ListToGetTokenPositionFrom and *Error is set to 0.  */
/*            If unsuccessful, the function returns 0 and *Error is  */
/*               set to a non-zero error code.                       */
/*                                                                   */
/*   Error Handling: This function will fail if                      */
/*                   ListToGetTokenPositionFrom is not a valid list  */
/*                   or is an empty list.  In either of these cases, */
/*                   *Error is set to a non-zero error code.         */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
unsigned int GetTokenPosition( LIST       ListToGetTokenPositionFrom,
                               CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetTokenHandle                                  */
/*                                                                   */
/*   Descriptive Name:  This function returns a handle for the       */
/*                      current item in the list.  This handle is    */
/*                      then associated with that item regardless of */
/*                      its position in the list.  This handle can be*/
/*                      used to make its associated item the current */
/*                      item in the list.                            */
/*                                                                   */
/*   Input:  LIST   ListToGetTokenHandleFrom : The list from which a */
/*                                        handle is needed.          */
/*           CARDINAL * Error : The address of a variable to hold the*/
/*                              error return code                    */
/*                                                                   */
/*   Output:  If successful, the function returns a handle for the   */
/*               the current item in ListToGetTokenHandleFrom, and   */
/*               *Error is set to 0.                                 */
/*            If unsuccessful, the function returns 0 and *Error is  */
/*               set to a non-zero error code.                       */
/*                                                                   */
/*   Error Handling: This function will fail if                      */
/*                   ListToGetTokenHandleFrom is not a valid list or */
/*                   is an empty list.  In either of these cases,    */
/*                   *Error is set to a non-zero error code.         */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*           The handle returned is a pointer to an internal         */
/*           structure within the list.  If the item associated      */
/*           with this handle is removed from the list, the handle   */
/*           will be invalid and should not be used as the internal  */
/*           structure it points to will nolonger exist!             */
/*                                                                   */
/*********************************************************************/
ADDRESS GetTokenHandle ( LIST ListToGetTokenHandleFrom, CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetTokenListSize                                */
/*                                                                   */
/*   Descriptive Name:  This function returns the number of items in */
/*                      a list.                                      */
/*                                                                   */
/*   Input:  LIST   ListToGetSizeOf : The list whose size we wish to */
/*                                    know                           */
/*           CARDINAL * Error : The address of a variable to hold the*/
/*                              error return code                    */
/*                                                                   */
/*   Output:  If successful, the function returns the a count of the */
/*               number of items in the list, and *Error is set to 0.*/
/*            If unsuccessful, the function returns 0 and *Error is  */
/*               set to a non-zero error code.                       */
/*                                                                   */
/*   Error Handling: This function will fail if ListToGetSizeOf is   */
/*                   not a valid list.  If this happens, then *Error */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
CARDINAL32 GetTokenListSize( LIST ListToGetSizeOf, CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  TokenListEmpty                                  */
/*                                                                   */
/*   Descriptive Name:  This function returns TRUE if the            */
/*                      specified list is empty, otherwise it returns*/
/*                      FALSE.                                       */
/*                                                                   */
/*   Input:  LIST       ListToCheck : The list to check to see if it */
/*                                    is empty                       */
/*           CARDINAL * Error : The address of a variable to hold the*/
/*                              error return code                    */
/*                                                                   */
/*   Output:  If successful, the function returns TRUE if the        */
/*               number of items in the list is 0, otherwise it      */
/*               returns FALSE.  Also, *Error is set to 0.           */
/*            If unsuccessful, the function returns TRUE and         */
/*               *Error is set to a non-zero error code.             */
/*                                                                   */
/*   Error Handling: This function will fail if ListToCheck is not   */
/*                   a valid list.  If this happens, then *Error     */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
BOOLEAN TokenListEmpty( LIST ListToCheck, CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  EliminateTokenList                              */
/*                                                                   */
/*   Descriptive Name:  This function releases the memory associated */
/*                      with the internal data structures of a LIST. */
/*                      Once a LIST has been eliminated by this      */
/*                      function, it must be reinitialized before it */
/*                      can be used again.                           */
/*                                                                   */
/*   Input:  LIST       ListToDestroy : The list to be eliminated    */
/*                                      from memory.                 */
/*           CARDINAL * Error : The address of a variable to hold the*/
/*                              error return code                    */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail if ListToDestroy is not */
/*                   a valid list.  If this happens, then *Error     */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*           This function will delete any items which may be in the */
/*           list.  However, since this function has no way of       */
/*           knowing the internal structure of an item, items which  */
/*           contain embedded pointers will not be entirely freed.   */
/*           This can lead to memory leaks.  The programmer should   */
/*           ensure that any list passed to this function is empty   */
/*           or does not contain any items with embedded pointers.   */
/*                                                                   */
/*********************************************************************/
void EliminateTokenList( LIST *  ListToDestroy, CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  NextToken                                       */
/*                                                                   */
/*   Descriptive Name:  This function makes the next item in the list*/
/*                      the current item in the list (i.e. it        */
/*                      advances the current item pointer).          */
/*                                                                   */
/*   Input:  LIST       ListToAdvance : The list whose current item  */
/*                                      pointer is to be advanced    */
/*           CARDINAL * Error : The address of a variable to hold the*/
/*                              error return code                    */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                        ListToAdvance is not a valid list          */
/*                        ListToAdvance is empty                     */
/*                        The current item is the last item in the   */
/*                           list                                    */
/*                   If any of these conditions occurs, then *Error  */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void NextToken( LIST  ListToAdvance, CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name: GoToStartOfTokenList                             */
/*                                                                   */
/*   Descriptive Name:  This function makes the first item in the    */
/*                      list the current item in the list.           */
/*                                                                   */
/*   Input:  LIST       ListToReset : The list whose current item    */
/*                                    is to be set to the first item */
/*                                    in the list                    */
/*           CARDINAL * Error : The address of a variable to hold the*/
/*                              error return code                    */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail if ListToReset is not   */
/*                   a valid list.  If this occurs, then *Error      */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void GoToStartOfTokenList( LIST ListToReset, CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name: GoToSpecifiedToken                                */
/*                                                                   */
/*   Descriptive Name:  This function makes the item associated with */
/*                      Handle the current item in the list.         */
/*                                                                   */
/*   Input:  LIST  ListToReposition:  The list whose current item    */
/*                                    is to be set to the item       */
/*                                    associated with Handle.        */
/*           ADDRESS Handle : A handle obtained by using the         */
/*                            GetTokenHandle function.  This handle  */
/*                            identifies a unique item in the list.  */
/*           CARDINAL * Error : The address of a variable to hold the*/
/*                              error return code                    */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail if ListToReposition is  */
/*                   not a valid list.  If this occurs, then *Error  */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*                                                                   */
/*           It is assumed that Handle is a valid handle and that    */
/*           the item associated with Handle is still in the list.   */
/*           If these conditions are not met, an exception or trap   */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void GoToSpecifiedToken( LIST       ListToReposition,
                         ADDRESS    Handle,
                         CARDINAL * Error);


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetNextToken                                    */
/*                                                                   */
/*   Descriptive Name:  This function copies the next item in the    */
/*                      list to a buffer provided by the caller.     */
/*                                                                   */
/*   Input:  LIST   ListToGetNextItemFrom : The list whose next item */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL ItemSize : What the caller thinks the size of  */
/*                               the next item is.                   */
/*           Token *       MyToken : The address of a buffer to hold */
/*                                   the data being returned.        */
/*           unsigned int * ItemPosition : The starting position of  */
/*                                  next token                       */
/*           CARDINAL * Error : The address of a variable to hold    */
/*                              the error return code.               */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The buffer at ItemLocation will contain a copy of */
/*                    the next item from ListToGetNextItemFrom.      */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                                                                   */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetNextItemFrom is not a valid list */
/*                         ItemSize does not match the size of the   */
/*                             next item in the list                 */
/*                         ItemLocation is NULL                      */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void GetNextToken( LIST           ListToGetNextItemFrom,
                   CARDINAL       ItemSize,
                   Token *        MyToken,
                   unsigned int * ItemPosition,
                   CARDINAL *     Error);

#endif

