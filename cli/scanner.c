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
 * Module: scanner.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: LIST *  ScanCommandLine
 *            BOOLEAN MakeToken
 *
 * Description: Defines the interface to the scanner.  The scanner takes
 *              the command line passed to it and breaks it into tokens.
 *              These tokens are then appended to a linked list of tokens
 *              created by the scanner.  When the entire command line has
 *              been processed, the linked list is returned to the caller.
 *
 * Notes: The basic Token structure used by the scanner, screener, and parser
 *        is defined in SCANNER.H, as well as the MaxIdentifierLength, which
 *        limits the size of identifiers.
 *
 */

/* This module implements the scanner, which is the front end of the syntactic
   analyzer used by the LVM command.  The scanner is an FSA whose states and
   actions are as indicated in the table below:
                                                                Token Type
   State   Characters that trigger a           State to         Appended to
   Name           Transition                   Transition to    TokenList
   ----------------------------------------------------------------------------


   ----------------------------------------------------------------------------

   The scanner maintains a buffer.  Each time a character is used in a
   transition, it is placed into the buffer.  The buffer is cleared each
   time a transition to the Start state is made.  When the scanner reaches
   a state where it outputs a value (as indicated in the table), the output
   consists of two parts: the contents of the buffer, and a characterization
   of the contents of the buffer.  In the table above, only the characterization
   is shown in the output column.  In those cases where output occurs on a
   transition to the start state, the output takes place before the transition
   to the start state.  Each of the items "output" by the scanner is appended
   to a linked list, which is returned to the caller when scanning has been
   completed.  Thus, the scanner returns a linked list of tokens.                 */


/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/
#include <ctype.h>    /* toupper */
#include <stdlib.h>   /* malloc */
#include <stdio.h>
#include <string.h>
#include "gbltypes.h"  /* CARDINAL */
#include "scanner.h"  /* TokenTypes, Token, MaxIdentifierLength,
                         ScanCommandLine, DeleteTokenList */
#include "list.h"     /* LIST, AppendToken, GoToStartOfTokenList, InitializeTokenList,
                         GetToken, DeleteToken, TokenListEmpty, EliminateTokenList      */
#include "error.h"    /* ReportError */
#include "lvmcli.h"
#include "debug.h"
#include "utility.h"

/*--------------------------------------------------
 * There are no private constants
 --------------------------------------------------*/



/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/

/* The following enumeration has one entry for each state in the
   state table.  A variable of this type will be used to keep
   track of which state the FSA is in at any given time.          */
typedef enum
{
  Start,
  IsString,
  IsNumber,
  SingleSpace,
  IsMultiSpace,
  SingleTab,
  IsMultiTab,
  AcceptSingleSpace,
  AcceptMultiSpace,
  AcceptSingleTab,
  AcceptMultiTab,
  AcceptSeparator,
  AcceptString,
  AcceptNumber,
  EndState,
  ErrorState
} State;


/*--------------------------------------------------
 There are no private global variables.
--------------------------------------------------*/



/*--------------------------------------------------
 Local Function Prototypes
--------------------------------------------------*/

/* The following function creates a token using the the characters in the
   Buffer and appends the token to TokenList.                             */
static BOOLEAN MakeToken(pSTRING Buffer, CARDINAL Count, TokenTypes Characterization, unsigned int StartingTokenPosition, LIST TokenList);


/*--------------------------------------------------
 There are no public global variables.
--------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: ScanCommandLine                                  */
/*                                                                   */
/*   Descriptive Name: Generates a list of tokens derived from the   */
/*                     string passed to it.                          */
/*                                                                   */
/*   Input: pSTRING CommandLine - The string to be scanned and        */
/*                               tokenized.                          */
/*                                                                   */
/*   Output: If Success : The function return value will be a        */
/*                        pointer to a LIST containing the tokens    */
/*                        generated.                                 */
/*                                                                   */
/*           If Failure : The function return value will be NULL     */
/*                        and an error message will be displayed to  */
/*                        the user using the ReportError function    */
/*                        described in ERROR.H.                      */
/*                                                                   */
/*   Error Handling: If an error occurs, all memory allocated by this*/
/*                   function is freed, an error message is displayed*/
/*                   using ReportError, and the function return value*/
/*                   will be NULL.                                   */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
LIST ScanCommandLine(pSTRING CommandLine)
{
  char        Buffer[MaxIdentifierLength];  /* Used to hold the text associated with the token being built.       */
  CARDINAL    PositionInBuffer = 0;         /* The position in the buffer at which to place the next character.   */
  CARDINAL    PositionInCommandLine = 0;    /* The position in the command line of the next character to be read. */
  State       CurrentState = Start;         /* The current state within the FSA.                                  */
  char        CharacterToExamine = ' ';     /* The current character to examine.  Examination of a character will
                                               result in the FSA transitioning to a new state in accordance with
                                               the state table at the beginning of this file.                     */
  LIST        TokenList = (LIST) NULL;      /* All tokens are appended to this list and this list is returned to
                                               the caller when scanning is complete. */
  CARDINAL    Error = 0;                    /* This is used to hold the error code returned by list operations
                                               in accordance with LIST.H.                                         */
  unsigned int StartingTokenPosition;
  pSTRING  pUppercaseToken;       /* Pointer to string in which to stored uppercase*/
                                  /* copy of the buffer. */
  unsigned int    index=0;
  BOOLEAN     FirstStringChar = FALSE;
  BOOLEAN     QuoteInProgress = FALSE;


  /* Initialize variables */

  InitializeTokenList(&TokenList, &Error);
  if (Error )
  {
    return ( (LIST) NULL);
  }


  /* The FSA depicted in the state table at the beginning of this file is
     simulated using a switch statement inside of a loop.  Each case in the
     switch statement corresponds to a state in the state table.  The
     CurrentState variable is used to indicate which state the FSA is in.     */

  for (;;)
  {
    switch (CurrentState)
    {
      case Start :          /* The START state */

        if ( QuoteInProgress == TRUE )
        {
          CurrentState = IsString;
          break;
        }

        /* Initialize PositionInBuffer to 0 since we are beginning a new token. */
        PositionInBuffer = 0;
        StartingTokenPosition = PositionInCommandLine;

        /* Get a character to examine and place it in the buffer. */
        CharacterToExamine = (char)(CommandLine[PositionInCommandLine]);
        if ( CharacterToExamine == QuoteChar )
        {
          FirstStringChar = TRUE;
          CurrentState = IsString;
          break;
        }
        Buffer[PositionInBuffer] = CharacterToExamine;

        /* Increment our pointers.*/
        PositionInBuffer++;
        PositionInCommandLine++;

        /* Process the current character according to the state table. */

        /* Check for a space. */
        if (CharacterToExamine == SpaceChar)
        {
          CurrentState = SingleSpace; /* Transition to the SingleSpace state. */
          break;
        }

        /* Check for a tab. */
        if (CharacterToExamine == TabChar)
        {
          CurrentState = SingleTab; /* Transition to the SingleTab state. */
          break;
        }

        /* Check for a separator. */
        if ( (CharacterToExamine == CommaChar)
             ||
             (CharacterToExamine == ColonChar)
             ||
             (CharacterToExamine == SemiColonChar)
             ||
             (CharacterToExamine == OpenParenChar)
             ||
             (CharacterToExamine == CloseParenChar)
             ||
             (CharacterToExamine == OpenBracketChar)
             ||
             (CharacterToExamine == CloseBracketChar)
             ||
             (CharacterToExamine == OpenBraceChar)
             ||
             (CharacterToExamine == CloseBraceChar)
             ||
             (CharacterToExamine == EqualSignChar)
           )
        {
          CurrentState = AcceptSeparator;  /* Transition to the AcceptSeparator state. */
          break;
        }

        /* Check for the start of a number. */
        if ( isdigit(CharacterToExamine) )
        {
          CurrentState = IsNumber;   /* Transition to the IsNumber state. */
          break;
        }

        /* Check for end of input. */
        if (CharacterToExamine == '\0')
        {
          CurrentState = EndState;   /* Transition to the EndState. */
          break;
        }

        /* Since the input was not any of the defined terminals */
        /* nor a digit, accept it as a character that can be    */
        /* part of a generic string.                            */
        /* The determination of whether this string consists */
        /* of only Acceptable_Characters will be made by the */
        /* screener.                                         */
        {
          FirstStringChar = TRUE;
          CurrentState = IsString;  /* Transition to the IsString state. */
          break;
        }

        /****************************************************
        THE FOLLOWING CODE IS DEAD BECAUSE ALL ASCII CHARACTERS
        ARE CURRENTLY ACCEPTED AS MEMBERS OF STRINGS.
        ****************************************************/
        /* We have an error.  Since the error state
           assumes that the character causing the error
           is not yet in the buffer, we will adjust
           our pointers to give that illusion.          */
        PositionInBuffer = 0;
        PositionInCommandLine--;

        CurrentState = ErrorState;   /* We did not recognize the character so
                                        we will transition to the ErrorState. */
        break;

      case EndState :       /* The "END" state. */

        /* Make an EOF token. */
        MakeToken(Buffer,0,Eof,StartingTokenPosition,TokenList);

        /* Return the completed TokenList */
        return TokenList;

      case ErrorState :     /* The "ERROR" state. */

        /* An error has occured.  We must clean up and
           then report the error. */
        DeleteTokenList(&TokenList);  /* Free the TokenList. */

        /* We must place the offending character into the
           buffer.                                         */
        if ( PositionInBuffer < MaxIdentifierLength )    /* Don't overflow the buffer! */
        {
          /* Place the character being examined into the buffer. */
          Buffer[PositionInBuffer] = CharacterToExamine;
          PositionInBuffer++;
        }

        /* We need to NULL terminate the contents of the buffer. */
        if ( PositionInBuffer < MaxIdentifierLength )    /* Don't overflow the buffer! */
        {
          /* Place the NULL character into the buffer. */
          Buffer[PositionInBuffer] = '\0';
        }
        else
          Buffer[MaxIdentifierLength - 1] = '\0';

        /* Report the error. */
        /* ReportError(1, Buffer, NULL, NULL); */  /* Bad LVM parameter. */

        /* Return NULL so that the caller knows we have failed! */
        return ( (LIST) NULL);

      case IsString :       /* The IsString state. */

        /* Get the next character to examine. */
        CharacterToExamine = (char)(CommandLine[PositionInCommandLine]);

        switch ( CharacterToExamine )
        {
          case QuoteChar:
            {
              if ( QuoteInProgress == FALSE )
              {
                QuoteInProgress = TRUE;
                if ( FirstStringChar == FALSE )
                {
                  CurrentState = AcceptString;
                  PositionInCommandLine++;
                }
              }
              else
              {
                QuoteInProgress = FALSE;
                CurrentState = AcceptString;
                PositionInCommandLine++;
              }
            }
            break;
            /* Is this character is NOT any of the terminals, */
            /* accept it as part of the string.               */
          case CommaChar:
          case ColonChar:
          case SemiColonChar:
          case SpaceChar:
          case TabChar:
          case '\0':
          case OpenParenChar:
          case CloseParenChar:
          case OpenBracketChar:
          case CloseBracketChar:
          case OpenBraceChar:
          case CloseBraceChar:
          case EqualSignChar:
            {
              /* The character being examined was not part of a string.  Therefore,
                 the current string has ended.  We will change state, but not advance
                 the PositionInCommandLine pointer.  This way, the character being
                 examined will be examined again by the next state(s).                 */
              if ( QuoteInProgress == FALSE )
                CurrentState = AcceptString;  /* Change state. */
            }
            break;
        } /* end-switch */
        if ( CurrentState != AcceptString )
        {
          if ( PositionInBuffer < MaxIdentifierLength )    /* Don't overflow the buffer! */
          {
            if ( CharacterToExamine != QuoteChar )
            {
              /* Place the character being examined into the buffer. */
              Buffer[PositionInBuffer] = CharacterToExamine;
              PositionInBuffer++;
            }
          }

          PositionInCommandLine++;  /* Setup to read the next character from the command line. */
          FirstStringChar = FALSE;
          break;
        }
        break;

      case IsNumber:        /* The IsNumber state. */

        /* Get the next character to examine. */
        CharacterToExamine = (char)(CommandLine[PositionInCommandLine]);


        /* Is the character to examine part of a number? */
        if ( isdigit(CharacterToExamine) )
        {

          if ( PositionInBuffer < MaxIdentifierLength ) /* Don't overflow the buffer! */
          {
            /* Place the character being examined into the buffer. */
            Buffer[PositionInBuffer] = CharacterToExamine;
            PositionInBuffer++;
          }

          PositionInCommandLine++;  /* Setup to read the next character from the command line. */

          break;
        }
        else
        {
          /* Is this character is any of the terminals, */
          /* the current number has ended so accept it. */
          if ( (CharacterToExamine == CommaChar)
               ||
               (CharacterToExamine == ColonChar)
               ||
               (CharacterToExamine == SemiColonChar)
               ||
               (CharacterToExamine == SpaceChar)
               ||
               (CharacterToExamine == TabChar)
               ||
               (CharacterToExamine == '\0')
               ||
               (CharacterToExamine == OpenParenChar)
               ||
               (CharacterToExamine == CloseParenChar)
               ||
               (CharacterToExamine == OpenBracketChar)
               ||
               (CharacterToExamine == CloseBracketChar)
               ||
               (CharacterToExamine == OpenBraceChar)
               ||
               (CharacterToExamine == CloseBraceChar)
               ||
               (CharacterToExamine == EqualSignChar)
             )
          {
            /* The character being examined was not part of a number and it
               is not an acceptable character. Therefore, the current number
               has ended.  We will change state, but not advance
               the PositionInCommandLine pointer.  This way, the character
               being examined will be examined again by the next state(s). */
            CurrentState = AcceptNumber;
            break;
          }
          else
          {
            /* The character being examined was not part of a number but it
               is an acceptable character. Therefore, the current number
               is not a number afterall, it is a string that started with
               a number(s).  Switch to the IsString state and continue to
               accummulate characters as part of a generic string. */
            CurrentState = IsString;

            if ( PositionInBuffer < MaxIdentifierLength ) /* Don't overflow the buffer! */
            {
              /* Place the character being examined into the buffer. */
              Buffer[PositionInBuffer] = CharacterToExamine;
              PositionInBuffer++;
            }

            PositionInCommandLine++;  /* Setup to read the next character from the command line. */

            break;
          }  /* endif */
        } /* endif */


      case SingleSpace :    /* The SingleSpace state. */

        /* Get the next character to examine. */
        CharacterToExamine = (char)(CommandLine[PositionInCommandLine]);

        /* Is the character to examine a space? */
        if ( CharacterToExamine == SpaceChar)
        {

          if ( PositionInBuffer < MaxIdentifierLength )    /* Don't overflow the buffer! */
          {
            /* Place the character being examined into the buffer. */
            Buffer[PositionInBuffer] = CharacterToExamine;
            PositionInBuffer++;
          }

          PositionInCommandLine++;  /* Setup to read the next character from the command line. */

          CurrentState = IsMultiSpace; /* Change state */

          break;
        }

        /* The character being examined was not a space.  Therefore, the current
           token is a single space.  We will change state, but not advance
           the PositionInCommandLine pointer.  This way, the character being
           examined will be examined again by the next state(s).                 */
        CurrentState = AcceptSingleSpace;  /* Change state. */
        break;

      case IsMultiSpace :    /* The IsMultiSpace state. */

        /* Get the next character to examine. */
        CharacterToExamine = (char)(CommandLine[PositionInCommandLine]);

        /* Is the character to examine a space? */
        if ( CharacterToExamine == SpaceChar)
        {

          if ( PositionInBuffer < MaxIdentifierLength )    /* Don't overflow the buffer! */
          {
            /* Place the character being examined into the buffer. */
            Buffer[PositionInBuffer] = CharacterToExamine;
            PositionInBuffer++;
          }

          PositionInCommandLine++;  /* Setup to read the next character from the command line. */
          break;
        }

        /* The character being examined was not a space.  This ends the run of spaces
           forming the special MultiSpace token.  We will change state, but not
           advance the PositionInCommandLine pointer.  This way, the character being
           examined will be examined again by the next state(s).                      */
        CurrentState = AcceptMultiSpace;  /* Change state. */
        break;


      case AcceptSingleSpace: /* The AcceptSingleSpace State. */

        /* Make a token characterized as a Space token. */
        MakeToken(Buffer,PositionInBuffer,Space,StartingTokenPosition,TokenList);

        /* Transition back to the start state. */
        CurrentState = Start;
        break;

      case AcceptMultiSpace: /* The AcceptMultiSpace State. */

        /* Make a token characterized as a MultiSpace token. */
        MakeToken(Buffer,PositionInBuffer,MultiSpace,StartingTokenPosition,TokenList);

        /* Transition back to the start state. */
        CurrentState = Start;
        break;

      case SingleTab :    /* The SingleTab state. */

        /* Get the next character to examine. */
        CharacterToExamine = (char)(CommandLine[PositionInCommandLine]);

        /* Is the character to examine a Tab? */
        if ( CharacterToExamine == TabChar)
        {

          if ( PositionInBuffer < MaxIdentifierLength )    /* Don't overflow the buffer! */
          {
            /* Place the character being examined into the buffer. */
            Buffer[PositionInBuffer] = CharacterToExamine;
            PositionInBuffer++;
          }

          PositionInCommandLine++;  /* Setup to read the next character from the command line. */

          CurrentState = IsMultiTab; /* Change state */

          break;
        }

        /* The character being examined was not a Tab.  Therefore, the current
           token is a single Tab.  We will change state, but not advance
           the PositionInCommandLine pointer.  This way, the character being
           examined will be examined again by the next state(s).                 */
        CurrentState = AcceptSingleTab;  /* Change state. */
        break;

      case IsMultiTab :    /* The IsMultiTab state. */

        /* Get the next character to examine. */
        CharacterToExamine = (char)(CommandLine[PositionInCommandLine]);

        /* Is the character to examine a Tab? */
        if ( CharacterToExamine == TabChar)
        {

          if ( PositionInBuffer < MaxIdentifierLength )    /* Don't overflow the buffer! */
          {
            /* Place the character being examined into the buffer. */
            Buffer[PositionInBuffer] = CharacterToExamine;
            PositionInBuffer++;
          }

          PositionInCommandLine++;  /* Setup to read the next character from the command line. */
          break;
        }

        /* The character being examined was not a Tab.  This ends the run of Tabs
           forming the special MultiTab token.  We will change state, but not
           advance the PositionInCommandLine pointer.  This way, the character being
           examined will be examined again by the next state(s).                      */
        CurrentState = AcceptMultiTab;  /* Change state. */
        break;


      case AcceptSingleTab: /* The AcceptSingleTab State. */

        /* Make a token characterized as a Tab token. */
        MakeToken(Buffer,PositionInBuffer,Tab,StartingTokenPosition,TokenList);

        /* Transition back to the start state. */
        CurrentState = Start;
        break;

      case AcceptMultiTab: /* The AcceptMultiTab State. */

        /* Make a token characterized as a MultiTab token. */
        MakeToken(Buffer,PositionInBuffer,MultiTab,StartingTokenPosition,TokenList);

        /* Transition back to the start state. */
        CurrentState = Start;
        break;

      case AcceptSeparator : /* The AcceptSeparator State */

        /* Make a token characterized as a Separator token. */
        MakeToken(Buffer,PositionInBuffer,Separator,StartingTokenPosition,TokenList);

        /* Transition back to the start state. */
        CurrentState = Start;
        break;

      case AcceptNumber :    /* The AcceptNumber State. */

        /* Make a token characterized as a Number token. */
        MakeToken(Buffer,PositionInBuffer,Number,StartingTokenPosition,TokenList);

        /* Transition back to the start state. */
        CurrentState = Start;
        break;

      case AcceptString :    /* The AcceptString State. */
        /* The grammar for the special install command (i.e. /SI) */
        /* calls for the keyword /SIZE to be used to specify a    */
        /* size.  If the user does not include at least one space */
        /* between the SI option (i.e. FS, SIZE, or CRI) and the  */
        /* the /SIZE keyword, this will have been parsed as a     */
        /* single string (e.g. fs/size).  We must check for such  */
        /* a sequence before creating the token.  If one of the   */
        /* three following sequences exists, we need to break the */
        /* the string into two tokens: fs/size, size/size, or     */
        /* cri/size. */

        /* Convert the string to uppercase before comparing it with the    */
        /* tables elements because the elements are uppercase.             */
        pUppercaseToken = (pSTRING)malloc(PositionInBuffer);
        if (pUppercaseToken != NULL)
        {

          for (index=0; index < PositionInBuffer; index++)
          {
            pUppercaseToken[index] = toupper(Buffer[index]);
          } /* endfor */
          /* Append NULL character to uppercase version of string */
          pUppercaseToken[index] = '\0';

          if (strcmp(pUppercaseToken,"FS/SIZE") == 0)
          {
            MakeToken(pUppercaseToken,2,String,StartingTokenPosition,TokenList);
            strcpy(Buffer, "/SIZE");
            PositionInBuffer = PositionInBuffer - 2;
            StartingTokenPosition = StartingTokenPosition + 2;
            free(pUppercaseToken);
          }
          else
          {
            if (strcmp(pUppercaseToken,"SIZE/SIZE") == 0)
            {
              MakeToken(pUppercaseToken,4,String,StartingTokenPosition,TokenList);
              strcpy(Buffer, "/SIZE");
              PositionInBuffer = PositionInBuffer - 4;
              StartingTokenPosition = StartingTokenPosition + 4;
              free(pUppercaseToken);
            }
            else
            {
              if (strcmp(pUppercaseToken,"CRI/SIZE") == 0)
              {
                MakeToken(pUppercaseToken,3,String,StartingTokenPosition,TokenList);
                strcpy(Buffer, "/SIZE");
                PositionInBuffer = PositionInBuffer - 3;
                StartingTokenPosition = StartingTokenPosition + 3;
                free(pUppercaseToken);
              } /* endif */
            } /* endif */
          } /* endif */

          /* Make a token characterized as a String token. */
          MakeToken(Buffer,PositionInBuffer,String,StartingTokenPosition,TokenList);

          /* Transition back to the start state. */
          CurrentState = Start;
        }
        else
        {
          ReportError(Not_Enough_Memory);
          CurrentState = ErrorState;
        } /* endif */
        break;
    }
  }
}


/*********************************************************************/
/*                                                                   */
/*   Function Name: DeleteTokenList                                  */
/*                                                                   */
/*   Descriptive Name: Deletes the list of tokens created by the     */
/*                     ScanCommandLine function.  This function must */
/*                     be used to eliminate the list or memory leaks */
/*                     will result.                                  */
/*                                                                   */
/*   Input: LIST * pTokenList - The list of tokens to eliminate.     */
/*                                                                   */
/*   Output: If Success : The function return value will be TRUE.    */
/*                        *pTokenList will be NULL.                  */
/*                                                                   */
/*           If Failure : The function return value will be FALSE.   */
/*                        *pTokenList will be unchanged.             */
/*                                                                   */
/*   Error Handling: If an error occurs and is detected, processing  */
/*                   will continue as far as possible, i.e. as much  */
/*                   of the list will be freed as is possible.       */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function is intended to be called as part of any    */
/*          exit paths in the program.                               */
/*                                                                   */
/*********************************************************************/
BOOLEAN DeleteTokenList( LIST * pTokenList)
{
  BOOLEAN  ReturnValue = FALSE;   /* Used to hold the function return value. */
  Token    TokenToDelete;         /* The current token being processed.            */
  CARDINAL Error;                 /* Used to hold the error code returned by the
                                     list functions in LIST.H.                     */
  unsigned int TokenPosition;


  /* Since the list handler does not know the internal structure of the
     items in a list, it can not know about the pTokenText pointer in
     the tokens contained in pTokenList.  Therefore, it can not free the
     memory associated with the pTokenText pointer in each token.  To fix
     this, we will do a GetToken on each token.  This gives us a copy of
     the Token structure.  Since the list handler does not know the
     internal structure of the items in the list, both the copy of the
     token and the token in the list will have the same value for the
     pTokenText field.  This allows us to free the memory associated with
     the pTokenText field of the token.  Once this is done, DeleteToken can
     be used to remove the token from the list and free its remaining
     memory.                                                              */


  /* Start with the first token in the list. */
  GoToStartOfTokenList(*pTokenList,&Error);

  /* Loop through the tokens in the list. */
  while ( (Error == 0) && (! TokenListEmpty(*pTokenList,&Error )  ) )
  {
    /* Get the current token. */
    GetToken(*pTokenList,sizeof(Token),&TokenToDelete,&TokenPosition,&Error);

    if ( ( Error == 0 ) &&
         ( TokenToDelete.pTokenText != NULL ) )
      free(TokenToDelete.pTokenText);   /* Free the memory allocated for TokenText. */

    /* Remove the current token from the list.*/
    DeleteToken(*pTokenList,&Error);
  }

  /* If we successfully deleted all of the nodes, then Error will be 0.
     Since the call to DeleteTokenList will destroy this value, lets use it
     to set ReturnValue now.                                               */
  if ( !Error )
    ReturnValue = TRUE;

  /* We are through with the pTokenList, so lets get rid of it. */
  EliminateTokenList(pTokenList,&Error);
  if ( Error )
    ReturnValue = FALSE;   /* Error during DeleteTokenList. */

  return ReturnValue;
}



/*--------------------------------------------------
 * Local Functions Available
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: MakeToken                                        */
/*                                                                   */
/*   Descriptive Name: Creates a token from the contents of the scan */
/*                     buffer and appends the token to the token list*/
/*                                                                   */
/*   Input: pSTRING Buffer - The scan buffer to use when creating the */
/*                               token.                              */
/*          CARDINAL Count - The number of characters in the scan    */
/*                           buffer.                                 */
/*          TokenTypes Characterization - The characterization to    */
/*                                        assign to the token being  */
/*                                        made.                      */
/*          LIST TokenList - The list of tokens to add the newly     */
/*                           created token to.                       */
/*                                                                   */
/*   Output: If Success : The function return value will be TRUE,    */
/*                        and TokenList will have had a new token    */
/*                        appended to it.                            */
/*                                                                   */
/*           If Failure : The function return value will be FALSE,   */
/*                        TokenList will have been freed by a call   */
/*                        to DeleteTokenList, and an error message*/
/*                        will be displayed to the user via the      */
/*                        ReportError function defined in ERROR.H.   */
/*                                                                   */
/*   Error Handling: If an error occurs, all memory allocated by this*/
/*                   function is freed, an error message is displayed*/
/*                   using ReportError, and the function return value*/
/*                   will be FALSE.  Furthermore, the TokenList will */
/*                   be eliminated by a call to DeleteTokenList.  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  This function assumes that TokenList has already been   */
/*           initialized.                                            */
/*                                                                   */
/*********************************************************************/
static BOOLEAN MakeToken(pSTRING    Buffer,
                         CARDINAL   Count,
                         TokenTypes Characterization,
                         unsigned int StartingTokenPosition,
                         LIST       TokenList)
{
  Token    NewToken;      /* The token being built. */
  CARDINAL Index;         /* Used as an index when stepping through the Buffer. */
  CARDINAL Error = 0;     /* Used for error reporting */


  /* To make a token, we must allocate memory for the contents of
     Buffer and then copy the contents of Buffer.  Once this has been
     done, all we need to do is set the Characterization field of the
     token structure.                                                 */

  if (Count > 0)
  {
    /* Allocate memory. */
    NewToken.pTokenText = (pSTRING) malloc(Count + 1);
    if (NewToken.pTokenText == NULL)
    {
      /* Malloc failed!  We must be out of memory.  Report the error. */
      ReportError(Not_Enough_Memory);
      /* Release the memory consumed by the TokenList. */
      DeleteTokenList(&TokenList);
      /* Indicate failure to the caller. */
      return FALSE;
    }

    /* Copy the contents of Buffer. */
    for (Index = 0; Index < Count; Index++)
      NewToken.pTokenText[Index] = Buffer[Index];

    /* Make sure that the string we copied from Buffer is NULL terminated. */
    NewToken.pTokenText[Count] = 0;
  }
  else /* Buffer is empty */
    NewToken.pTokenText = NULL;

  /* Characterize the token. */
  NewToken.TokenType = Characterization;

  /* Add the new token to the TokenList. */
  AppendToken(TokenList,sizeof(NewToken),&NewToken,StartingTokenPosition,&Error);

  if ( Error )
  {
    DeleteTokenList(&TokenList);  /* Release the memory used by TokenList. */
    return FALSE;                    /* Tell the caller that we failed. */
  }

  /* Indicate success! */
  return TRUE;
}


