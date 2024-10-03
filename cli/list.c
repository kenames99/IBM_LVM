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
 * Module: list.c
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
 * Notes:  SEE THE INITIAL COMMENT IN LIST.H!
 *
 * 6/12/95 This module now employs the concept of a handle.  A handle is a
 *         reference to a specific item in a list which allows that item to
 *         be made the current item in the list quickly.  Example:  If you
 *         use the GetTokenHandle function to get a handle for the current item
 *         (lets call the item B1), then, regardless of where you are in the
 *         list (or any reodering of the items in the list), you can make item
 *         B1 the current item by passing its handle to the GoToSpecifiedToken
 *         function.
 *
 *         Handles are implemented in a rather simple manner.  A handle
 *         is merely the address of the LinkNode corresponding to the
 *         item that the handle is for.  When a handle is passed to the
 *         GoToSpecifiedToken function, the function checks the LinkNode
 *         to see if the LinkNode's ControlNodeLocation field points to the
 *         control node associated with the list being manipulated.  If so,
 *         then the LinkNode belongs to the list and we can safely make it
 *         the current item in the list.
 *
 */


#include <stddef.h>
#include "list.h"     /* Import list.h so that the compiler can check the
                         consistency of the declarations in list.h against
                         those in this module.                             */
#include "lvm_type.h" /* Import LVM types so that a list of tokens can be
                         passed to LVM plug-ins.                           */
/*--------------------------------------------------
 * There are no private constants
 --------------------------------------------------*/



/*--------------------------------------------------
 * There are no private type definitions
 --------------------------------------------------*/



/*--------------------------------------------------
 There are no private global variables.
--------------------------------------------------*/



/*--------------------------------------------------
 Private Functions.
--------------------------------------------------*/
CARDINAL Translate_Error_Code( CARDINAL Error_Code );


/*--------------------------------------------------
 There are no public global variables.
--------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name:  InitializeTokenList                             */
/*                                                                   */
/*   Descriptive Name: This function allocates and initializes the   */
/*                     data structures associated with a list and    */
/*                     then returns a pointer to these structures.   */
/*                                                                   */
/*   Input: LIST * pNewList : The address of a variable of type LIST */
/*                           which needs to be initialized to point  */
/*                           to a new list.                          */
/*          CARDINAL * Error : The address of a variable into which  */
/*                             any error codes are to be placed.     */
/*                                                                   */
/*   Output: If Success :                                            */
/*                        *pNewList is set to point to the           */
/*                                 ControlNode of the new list.      */
/*                        *Error is set to 0.                        */
/*                                                                   */
/*           If Failure : *pNewList is set to NULL unless it already */
/*                                 points to a valid list.           */
/*                        *Error is set to an error code.  The error */
/*                                 codes are specified in list.h     */
/*                                                                   */
/*   Error Handling:  The function will only fail if it can not      */
/*                    allocate enough memory to create the new list  */
/*                    or if *pNewList already points to a valid list.*/
/*                    In the first case (memory allocation failure)  */
/*                    this function frees any memory that it had     */
/*                    allocated prior to the failure and then returns*/
/*                    an error code.  In the case where *pNewList    */
/*                    already points to a valid list, *pNewList is   */
/*                    not altered and *Error is set to an error code.*/
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  If *pNewList is not NULL, then this module will         */
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
void InitializeTokenList( LIST *  pNewList, CARDINAL * Error)
{

  *pNewList = CreateList();
  if ( *pNewList == NULL )
    *Error = 1;                /* Out of memory. */
  else
    *Error = 0;                /* Success. */

  return;

}


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
/*           unsigned int  ItemPosition : The item starting position */
/*                            of the item being appended to the list */
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
void AppendToken( LIST         ListToAppendTo,
                 CARDINAL      ItemSize,
                 Token *       MyToken,
                 unsigned int  ItemPosition,
                 CARDINAL *    Error)
{

  LVM_Token   New_Token;

  if ( ItemSize != sizeof(Token) )
  {

    *Error = 7;  /* Item size mismatch. */
    return;

  }

  New_Token.TokenText = MyToken->pTokenText;
  New_Token.Position = ItemPosition;
  New_Token.TokenType = (Token_Characterizations) MyToken->TokenType;

  InsertItem(ListToAppendTo, sizeof(LVM_Token), &New_Token, LVM_TOKEN_TAG, NULL, AppendToList, FALSE, (CARDINAL32 *) Error);
  *Error = Translate_Error_Code(*Error);

  return;

}


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
void DeleteToken (LIST      ListToDeleteFrom,
                 CARDINAL * Error)
{

  DeleteItem(ListToDeleteFrom, TRUE, NULL, (CARDINAL32 *) Error);
  *Error = Translate_Error_Code(*Error);

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetToken                                        */
/*                                                                   */
/*   Descriptive Name:  This function copies the current item in the */
/*                      list to a buffer provided by the caller.     */
/*                                                                   */
/*   Input:  LIST   ListToGetTokenFrom : The list whose current item */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL ItemSize : What the caller thinks the size of  */
/*                               the current item is.                */
/*           Token *       MyToken : The address of a buffer to hold */
/*                                   the data being returned.        */
/*           unsigned int * ItemPosition : The starting position of  */
/*                             of the current item is.               */
/*           CARDINAL * Error : The address of a variable to hold    */
/*                              the error return code.               */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The buffer at ItemLocation will contain a copy of */
/*                    the current item from ListToGetTokenFrom.      */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                                                                   */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetTokenFrom is not a valid list    */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemLocation is NULL                      */
/*                         ItemPosition does not match the item tag  */
/*                             of the current item in the list       */
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
void GetToken( LIST          ListToGetTokenFrom,
              CARDINAL       ItemSize,
              Token *        MyToken,
              unsigned int * pItemPosition,
              CARDINAL *     Error)
{

  LVM_Token   New_Token;

  if ( ItemSize != sizeof(Token) )
  {

    *Error = 7;  /* Item size mismatch. */

    return;

  }

  GetItem(ListToGetTokenFrom, sizeof(LVM_Token), &New_Token, LVM_TOKEN_TAG, NULL, FALSE, (CARDINAL32 *) Error);
  if ( *Error != DLIST_SUCCESS )
  {

    *Error = Translate_Error_Code(*Error);

  }
  else
  {

    MyToken->pTokenText = New_Token.TokenText;
    MyToken->TokenType = (TokenTypes) New_Token.TokenType;
    *pItemPosition = New_Token.Position;
    *Error = 0;

  }

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ReplaceToken                                    */
/*                                                                   */
/*   Descriptive Name:  This function replaces the current item in   */
/*                      the list with the one provided as its        */
/*                      argument.                                    */
/*                                                                   */
/*   Input: LIST   ListToReplaceTokenIn : The list whose current item*/
/*                                       is to be replaced           */
/*          CARDINAL ItemSize : The size, in bytes, of the           */
/*                              replacement item                     */
/*           Token *       MyToken : The address of the replacement  */
/*                                   data.                           */
/*          unsigned int ItemPosition : The item tag that the user   */
/*                                      wishes to associate with the */
/*                                      replacement item             */
/*          CARDINAL * Error : The address of a variable to hold the */
/*                             error return code                     */
/*                                                                   */
/*   Output:  If Successful then *Error will be set to 0.            */
/*            If Unsuccessful, then *Error will be set to a non-zero */
/*              error code.                                          */
/*                                                                   */
/*   Error Handling:  This function will fail under the following    */
/*                    conditions:                                    */
/*                         ListToReplaceTokenIn is empty             */
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
void ReplaceToken( LIST        ListToReplaceTokenIn,
                  CARDINAL     ItemSize,
                  Token *      MyToken,
                  unsigned int ItemPosition,
                  CARDINAL *   Error)
{

  LVM_Token   New_Token;

  if ( ItemSize != sizeof(Token) )
  {

    *Error = 7;  /* Item size mismatch. */
    return;

  }

  New_Token.TokenText = MyToken->pTokenText;
  New_Token.Position = ItemPosition;
  New_Token.TokenType = (Token_Characterizations) MyToken->TokenType;

  ReplaceItem(ListToReplaceTokenIn, sizeof(LVM_Token), &New_Token, LVM_TOKEN_TAG, NULL, FALSE, (CARDINAL32 *) Error);

  *Error = Translate_Error_Code(*Error);

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetTokenPosition                                */
/*                                                                   */
/*   Descriptive Name:  This function returns the item tag associated*/
/*                      with the current item in the list.           */
/*                                                                   */
/*   Input:  LIST   ListToGetTokenPositionFrom : The list from which */
/*                              the item tag of the current item is  */
/*                              to be returned                       */
/*           CARDINAL * Error : The address of a variable to hold the*/
/*                              error return code                    */
/*                                                                   */
/*   Output:  If successful, the function returns the item tag       */
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
unsigned int GetTokenPosition( LIST  ListToGetTokenPositionFrom, CARDINAL * Error)
{

  LVM_Token    New_Token;
  unsigned int Position = 0;

  GetItem(ListToGetTokenPositionFrom, sizeof(LVM_Token), &New_Token, LVM_TOKEN_TAG, NULL, FALSE, (CARDINAL32 *) Error);
  if ( *Error != DLIST_SUCCESS )
  {

    *Error = Translate_Error_Code(*Error);

  }
  else
  {

    Position = New_Token.Position;
    *Error = 0;

  }


  return Position;

}


/*********************************************************************/
/*                                                                   */
/*   Added 6/12/95 by BMR                                            */
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
/*           The handle returned is a pointer to the LinkNode of the */
/*           current item in the list.  This allows the item to move */
/*           around in the list without losing its associated handle.*/
/*           However, if the item is deleted from the list, then the */
/*           handle is invalid and its use could result in a trap.   */
/*                                                                   */
/*********************************************************************/
ADDRESS GetTokenHandle ( LIST ListToGetTokenHandleFrom, CARDINAL * Error)
{

  ADDRESS   Handle;

  Handle = GetHandle( ListToGetTokenHandleFrom, (CARDINAL32 *) Error);

  *Error = Translate_Error_Code(*Error);

  return Handle;

}



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
CARDINAL32 GetTokenListSize( LIST ListToGetSizeOf, CARDINAL * Error)
{
  CARDINAL32  ListSize;

  ListSize = GetListSize( ListToGetSizeOf, (CARDINAL32 *) Error);

  *Error = Translate_Error_Code(*Error);

  return ListSize;

}


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
BOOLEAN TokenListEmpty( LIST ListToCheck, CARDINAL * Error)
{
  BOOLEAN  Empty;

  Empty = ListEmpty( ListToCheck, (CARDINAL32 *) Error);

  *Error = Translate_Error_Code(*Error);

  return Empty;

}


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
void EliminateTokenList( LIST *  pListToDestroy, CARDINAL * Error)
{

  DestroyList( pListToDestroy, TRUE, (CARDINAL32 *) Error);
  *Error = Translate_Error_Code(*Error);

  return;

}


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
void NextToken( LIST  ListToAdvance, CARDINAL * Error)
{

  NextItem( ListToAdvance, (CARDINAL32 *) Error);
  *Error = Translate_Error_Code(*Error);

  return;

}


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
/*   Error Handling: This function will fail if ListToAdvance is not */
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
void GoToStartOfTokenList( LIST ListToReset, CARDINAL * Error)
{

  GoToStartOfList( ListToReset, (CARDINAL32 *) Error);
  *Error = Translate_Error_Code(*Error);

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Added 6/12/95 by BMR                                            */
/*                                                                   */
/*   Function Name: GoToSpecifiedToken                               */
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
void GoToSpecifiedToken( LIST ListToReposition, ADDRESS Handle, CARDINAL * Error)
{

  GoToSpecifiedItem( ListToReposition, Handle, (CARDINAL32 *) Error);
  *Error = Translate_Error_Code(*Error);

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetNextToken                                    */
/*                                                                   */
/*   Descriptive Name:  This function copies the next item in the    */
/*                      list to a buffer provided by the caller.     */
/*                                                                   */
/*   Input:  LIST   ListToGetNextTokenFrom : The list whose next item*/
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL ItemSize : What the caller thinks the size of  */
/*                               the next item is.                   */
/*           Token *       MyToken : The address of a buffer to hold */
/*                                   the data being returned.        */
/*           unsigned int * ItemPosition : The starting position of  */
/*                             of the next item is.                  */
/*           CARDINAL * Error : The address of a variable to hold    */
/*                              the error return code.               */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The buffer at ItemLocation will contain a copy of */
/*                    the next item from ListToGetNextTokenFrom.     */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                                                                   */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetNextTokenFrom is not a valid list*/
/*                         ItemSize does not match the size of the   */
/*                             next item in the list                 */
/*                         ItemLocation is NULL                      */
/*                         ItemPosition does not match the item tag  */
/*                             of the next item in the list          */
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
void GetNextToken( LIST      ListToGetNextTokenFrom,
              CARDINAL       ItemSize,
              Token *        MyToken,
              unsigned int * pItemPosition,
              CARDINAL *     Error)
{

  LVM_Token   New_Token;

  if ( ItemSize != sizeof(Token) )
  {

    *Error = 7;  /* Item size mismatch. */

    return;

  }

  GetNextItem(ListToGetNextTokenFrom, sizeof(LVM_Token), &New_Token, LVM_TOKEN_TAG, (CARDINAL32 *) Error);
  if ( *Error != DLIST_SUCCESS )
  {

    *Error = Translate_Error_Code(*Error);

  }
  else
  {

    MyToken->pTokenText = New_Token.TokenText;
    MyToken->TokenType = (TokenTypes) New_Token.TokenType;
    *pItemPosition = New_Token.Position;
    *Error = 0;

  }


  *Error = Translate_Error_Code(*Error);

  return;


}


/*--------------------------------------------------
 Private Functions.
--------------------------------------------------*/

CARDINAL Translate_Error_Code( CARDINAL Error_Code )
{

  switch ( Error_Code )
  {
    case DLIST_SUCCESS :
    case DLIST_OUT_OF_MEMORY :
    case DLIST_CORRUPTED :
    case DLIST_BAD :
    case DLIST_NOT_INITIALIZED : /* No changes required. */
                                 break;              /* Keep the compiler happy. */
    case DLIST_EMPTY :
    case DLIST_ITEM_SIZE_WRONG :
    case DLIST_BAD_ITEM_POINTER :
    case DLIST_ITEM_SIZE_ZERO :
    case DLIST_ALREADY_AT_START :
    case DLIST_BAD_HANDLE :
    case DLIST_END_OF_LIST :
                             Error_Code += 1;
                             break;        /* Keep the compiler happy. */
    case DLIST_ITEM_TAG_WRONG :
                                Error_Code = 7;  /* Item size mismatch! */
                                break;     /* Keep the compiler happy. */
    default :
              Error_Code = 2;   /* Memory has been corrupted! */
              break;      /* Keep the compiler happy. */

  }

  return Error_Code;

}
