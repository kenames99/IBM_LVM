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
 * Module: lvm_hand.h
 */ 

/*
 * Change History:
 * 
 */

/*
 * Functions: ADDRESS Create_Handle
 *            void    Destroy_Handle
 *            void    Destroy_All_Handles
 *            void    Translate_Handle
 *            BOOLEAN Initialize_Handle_Manager
 *
 * Description: This module provides a uniform way for creating a handle
 *              and associating it with something.
 *
 * Notes: This module makes use of DLIST.
 *
 */

#ifndef LVM_MANAGE_HANDLES

#define LVM_MANAGE_HANDLES

#include "LVM_GBLS.h"  /* ADDRESS, BOOLEAN, CARDINAL32 */
#include "LVM_LIST.h"     /* TAG */


/* The following defines represent the error codes that can be returned by this module. */
#define HANDLE_MANAGER_NO_ERROR          0
#define HANDLE_MANAGER_NOT_INITIALIZED   1
#define HANDLE_MANAGER_BAD_HANDLE        2
#define HANDLE_MANAGER_INTERNAL_ERROR    3
#define HANDLE_MANAGER_OUT_OF_MEMORY     4


/*********************************************************************/
/*                                                                   */
/*   Function Name: Initialize_Handle_Manager                        */
/*                                                                   */
/*   Descriptive Name: Initializes the Handle Manager for use.       */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: If successful, then the function return value will be   */
/*              TRUE.                                                */
/*           If there is a failure, the function return value will   */
/*              be FALSE.                                            */
/*                                                                   */
/*   Error Handling: The function return value will be FALSE if an   */
/*                   error occurs.                                   */
/*                                                                   */
/*   Side Effects: Memory will be allocated for internal structures. */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System Initialize_Handle_Manager(void);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Create_Handle                                    */
/*                                                                   */
/*   Descriptive Name: Takes an object, its TAG, and associates a    */
/*                     Handle with them.                             */
/*                                                                   */
/*   Input: ADDRESS Object : The address of the object to be         */
/*                           associated with a handle.               */
/*          TAG ObjectTag : The TAG value of Object.  TAG values are */
/*                          explained in the DLIST module.           */
/*          CARDINAL32 ObjectSize : The size of the item whose       */
/*                                  address is given in Object.      */
/*          CARDINAL32 * Error_Code : This is the address of a       */
/*                                    variable which is to hold any  */
/*                                    error codes generated by this  */
/*                                    function.                      */
/*                                                                   */
/*   Output: If successful, then the function return value will be   */
/*              a non-zero Handle and *Error_Code will be            */
/*              HANDLE_MANAGER_NO_ERROR.                                 */
/*           If there is a failure, *Error will contain a non-zero   */
/*              error code and the function return value will be 0.  */
/*                                                                   */
/*   Error Handling: *Error_Code will be non-zero if an error occurs.*/
/*                                                                   */
/*   Side Effects: A new handle may be created.                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
ADDRESS _System Create_Handle( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Destroy_Handle                                   */
/*                                                                   */
/*   Descriptive Name: Removes the association between an object and */
/*                     a Handle.  The Handle is thereby eliminated.  */
/*                                                                   */
/*   Input: ADDRESS Handle : The handle to be eliminated.            */
/*          CARDINAL32 * Error_Code : This is the address of a       */
/*                                    variable which is to hold any  */
/*                                    error codes generated by this  */
/*                                    function.                      */
/*                                                                   */
/*   Output: If successful, then *Error_Code will be                 */
/*              HANDLE_MANAGER_NO_ERROR.                             */
/*           If there is a failure, *Error will contain a non-zero   */
/*              error code.                                          */
/*                                                                   */
/*   Error Handling: *Error_Code will be non-zero if an error occurs.*/
/*                                                                   */
/*   Side Effects: A handle may be eliminated.                       */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Destroy_Handle( ADDRESS Handle, CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Destroy_All_Handles                              */
/*                                                                   */
/*   Descriptive Name: Eliminates all existing handles being tracked */
/*                     by the Handle Manager.                        */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code : This is the address of a       */
/*                                    variable which is to hold any  */
/*                                    error codes generated by this  */
/*                                    function.                      */
/*                                                                   */
/*   Output: If successful, then *Error_Code will be                 */
/*              HANDLE_MANAGER_NO_ERROR.                             */
/*           If there is a failure, *Error will contain a non-zero   */
/*              error code.                                          */
/*                                                                   */
/*   Error Handling: *Error_Code will be non-zero if an error occurs.*/
/*                                                                   */
/*   Side Effects: All handles may be eliminated.                    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Destroy_All_Handles( CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Translate_Handle                                 */
/*                                                                   */
/*   Descriptive Name: Returns a pointer to, and the TAG value of,   */
/*                     the object associated with a handle.          */
/*                                                                   */
/*   Input: ADDRESS Handle : The handle to translate.                */
/*          ADDRESS * Object : The location of a pointer into which  */
/*                             the address of the object associated  */
/*                             with Handle is to be placed.          */
/*          TAG * ObjectTag : The address of a variable to hold the  */
/*                            TAG value of the object associated     */
/*                            with Handle.  TAG values are explained */
/*                            in the DLIST module.                   */
/*          CARDINAL32 * Error_Code : This is the address of a       */
/*                                    variable which is to hold any  */
/*                                    error codes generated by this  */
/*                                    function.                      */
/*                                                                   */
/*   Output: If successful, then *Object will be set to the address  */
/*              of the object associated with Handle, *ObjectTag will*/
/*              contain the TAG of the object associated with Handle,*/
/*              and *Error_Code will be HANDLE_MANAGER_NO_ERROR.     */
/*           If there is a failure, *Error will contain a non-zero   */
/*              error code.                                          */
/*                                                                   */
/*   Error Handling: *Error_Code will be non-zero if a recoverable   */
/*                   error occurs.  If Handle is invalid, then a     */
/*                   trap or exception may occur.                    */
/*                                                                   */
/*   Side Effects: If Handle is invalid, a trap or exception may     */
/*                 occur.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Translate_Handle( ADDRESS Handle, ADDRESS * Object, TAG * ObjectTag, CARDINAL32 * Error_Code );

#endif
