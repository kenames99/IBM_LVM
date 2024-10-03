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
 * Module: screener.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: BOOLEAN ScreenTokenList
 *
 * Descriptive Name: Scans the entire list of tokens created by
 *                   the ScanCommandLine function and further
 *                   characterizes them.
 *
 *                   Each token that is currently characterized as
 *                   a separator will have its TokenType field
 *                   set to the the enumerated value that
 *                   specifically represents that separator.  Its
 *                   pTokenText field will be set to NULL and the
 *                   memory freed.
 *
 *                   Each token that is currently characterized as
 *                   a single space will have its TokenType field
 *                   set to the the enumerated value that
 *                   specifically represents a space.  Its
 *                   pTokenText field will be set to NULL and the
 *                   memory freed.  MultiSpace tokens are not
 *                   changed.  They may be embedded within a
 *                   name that will be reconstituted by the parser.
 *
 *                   Each token that is currently characterized as
 *                   a single tab will have its TokenType field
 *                   set to the the enumerated value that
 *                   specifically represents a tab.  Its
 *                   pTokenText field will be set to NULL and the
 *                   memory freed.  MultiTab tokens are not
 *                   changed.  They may be embedded within a
 *                   name that will be reconstituted by the parser.
 *
 *                   Each token that is currently characterized as
 *                   a string will be compared to the set of
 *                   reserved words.  If it matches one of these
 *                   strings, its TokenType field will be set to
 *                   the enumerated value that specifically
 *                   represents that reserved word and its
 *                   pTokenText field will be set to NULL and the
 *                   memory freed.
 *
 *                   After recharacterizing any strings that are
 *                   reserved words, any remaining string will be
 *                   checked to determine if they consist of only
 *                   ASCII 7 characters whose ordinal value is
 *                   above 32 (i.e.space).  If set of characters
 *                   in the string meets this test, then the
 *                   TokenType field will be set to the value
 *                   AcceptableCharacters.   The characterization
 *                   of all strings that do not meet this test,
 *                   will be set to the enumerated value
 *                   FileNameCharacters, indicating that they are
 *                   only valid when used as OS/2 filenames.
 *
 */

#include <ctype.h>    /* toupper */
#include <stdlib.h>   /* malloc */
#include <string.h>   /* malloc */
#include <stdio.h>
#include "gbltypes.h"  /* CARDINAL */
#include "scanner.h"  /* TokenTypes, Token, MaxIdentifierLength,
                         ScanCommandLine, EliminateTokenList */
#include "list.h"     /* LIST, AppendToken, GoToStartOfTokenList, InitializeTokenList,
                         GetToken, DeleteToken, TokenListEmpty, EliminateTokenList      */
#include "error.h" /* ReportError */
#include "lvmcli.h"
#include "debug.h"
#include "utility.h"

/* Reserved Word Table */
/* This table must always have Eof as its last element */
Token ReservedWords[] = {
  InstallStr        ,  Install       ,
  AllStr            ,  All_CLI       ,
  BestFitStr        ,  BestFit       ,
  BootDOSStr        ,  BootDOS       ,
  BootOS2Str        ,  BootOS2       ,
  BootableStr       ,  Bootable      ,
  BootmgrStr        ,  Bootmgr       ,
  CRStr             ,  CR_CLI        ,
  CRIStr            ,  CRI           ,
  CompatibilityStr  ,  Compatibility ,
  CreateStr         ,  Create        ,
  DeleteStr         ,  Delete        ,
  DriveStr          ,  Drive         ,
  DriveLetterStr    ,  DriveLetter   ,
  ExistingStr       ,  Existing      ,
  ExpandStr         ,  Expand        ,
  FSStr             ,  FS            ,
  FileStr           ,  File          ,
  FirstFitStr       ,  FirstFit      ,
  FreespaceStr      ,  Freespace     ,
  FromEndStr        ,  FromEnd       ,
  FromLargestStr    ,  FromLargest   ,
  FromSmallestStr   ,  FromSmallest  ,
  FromStartStr      ,  FromStart     ,
  HideStr           ,  Hide          ,
  LVMStr            ,  LVM           ,
  LastFitStr        ,  LastFit       ,
  LogicalStr        ,  Logical       ,
  NewMBRStr         ,  NewMBR        ,
  NewStr            ,  New           ,
  NoBootStr         ,  NoBoot        ,
  NonBootableStr    ,  NonBootable   ,
  NotBootableStr    ,  NotBootable   ,
  PartitionStr      ,  Partition     ,
  PrimaryStr        ,  Primary       ,
  QueryStr          ,  Query         ,
  RBStr             ,  RB            ,
  RediscoverPRMStr  ,  RediscoverPRM ,
  SIStr             ,  SI            ,
  SetNameStr        ,  SetName       ,
  SetStartableStr   ,  SetStartable  ,
  SizeStr           ,  Size          ,
  SlashSizeStr      ,  SlashSize     ,
  StartLogStr       ,  StartLog      ,
  UnusableStr       ,  Unusable      ,
  UnusedStr         ,  Unused        ,
  VolumeStr         ,  Volume        ,
  VolumesStr        ,  Volumes       ,
  ""                ,  Eof
};


/*********************************************************************/
/*                                                                   */
/*   Function Name: ScreenTokenList                                  */
/*                                                                   */
/*   Descriptive Name: Scans the entire list of tokens created by    */
/*                     the ScanCommandLine function and further      */
/*                     characterizes them.                           */
/*                                                                   */
/*                     Each token that is currently characterized as */
/*                     a separator will have its TokenType field     */
/*                     set to the the enumerated value that          */
/*                     specifically represents that separator.  Its  */
/*                     pTokenText field will be set to NULL and the  */
/*                     memory freed.                                 */
/*                                                                   */
/*                     Each token that is currently characterized as */
/*                     a single space will have its TokenType field  */
/*                     set to the the enumerated value that          */
/*                     specifically represents a space.  Its         */
/*                     pTokenText field will be set to NULL and the  */
/*                     memory freed.  MultiSpace tokens are not      */
/*                     changed.  They may be embedded within a       */
/*                     name that will be reconstituted by the parser.*/
/*                                                                   */
/*                     Each token that is currently characterized as */
/*                     a single tab will have its TokenType field    */
/*                     set to the the enumerated value that          */
/*                     specifically represents a tab.  Its           */
/*                     pTokenText field will be set to NULL and the  */
/*                     memory freed.  MultiTab tokens are not        */
/*                     changed.  They may be embedded within a       */
/*                     name that will be reconstituted by the parser.*/
/*                                                                   */
/*                     Each token that is currently characterized as */
/*                     a string will be compared to the set of       */
/*                     reserved words.  If it matches one of these   */
/*                     strings, its TokenType field will be set to   */
/*                     the enumerated value that specifically        */
/*                     represents that reserved word and its         */
/*                     pTokenText field will be set to NULL and the  */
/*                     memory freed.                                 */
/*                                                                   */
/*                     After recharacterizing any strings that are   */
/*                     reserved words, any remaining string will be  */
/*                     checked to determine if they consist of only  */
/*                     ASCII 7 characters whose ordinal value is     */
/*                     above 32 (i.e.space).  If set of characters   */
/*                     in the string meets this test, then the       */
/*                     TokenType field will be set to the value      */
/*                     AcceptableCharacters.   The characterization  */
/*                     of all strings that do not meet this test,    */
/*                     will be set to the enumerated value           */
/*                     FileNameCharacters, indicating that they are  */
/*                     only valid when used as OS/2 filenames.       */
/*                                                                   */
/*   Input: LIST TokenList - The list of tokens to characterize.     */
/*                                                                   */
/*   Output: If Success : The function return value will be TRUE.    */
/*                        *TokenList will be NULL.                   */
/*                                                                   */
/*           If Failure : The function return value will be FALSE.   */
/*                        *TokenList will be unchanged.              */
/*                                                                   */
/*   Error Handling: If an error occurs and is detected, processing  */
/*                   will continue as far as possible, i.e. as much  */
/*                   of the list will be freed as is possible.       */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN ScreenTokenList( LIST TokenList )
{
  BOOLEAN  ReturnValue = FALSE;   /* Used to hold the function return value. */
  Token    TokenToCharacterize;   /* The current token being processed.            */
  CARDINAL Error;                 /* Used to hold the error code returned by the
                                     list functions in LIST.H.                     */
  pSTRING  pUppercaseToken;       /* Pointer to string in which to stored uppercase*/
                                  /* copy of the TokenToCharacterize               */
  unsigned int   TokenPosition;


  /* Start with the first token in the list. */
  GoToStartOfTokenList(TokenList,&Error);

  if (Error)
  {
    ReportError(Internal_Error);  /* Report the error. */
    return ReturnValue;
  } /* endif */

  /* Loop through the tokens in the list. */
  while ( (Error == 0) && (! TokenListEmpty(TokenList,&Error)  ) )
  {
    /* Get the current token. */
    GetToken(TokenList,sizeof(Token),&TokenToCharacterize,&TokenPosition,&Error);

    if (Error == 0)
    {
      /* Select the action to take based on the current value of the TokenType field. */
      switch (TokenToCharacterize.TokenType)
      {
  case Space:
  case MultiSpace:
  case Tab:
  case MultiTab:
    DeleteToken(TokenList,&Error);
    break;
        case Separator:
          switch (TokenToCharacterize.pTokenText[0])
          {
            case ':':
              TokenToCharacterize.TokenType = Colon;
              break;
            case ';':
              TokenToCharacterize.TokenType = SemiColon;
              break;
            case ',':
              TokenToCharacterize.TokenType = Comma;
              break;
            case '(' :
              TokenToCharacterize.TokenType = Open_Paren;
              break;
            case ')' :
              TokenToCharacterize.TokenType = Close_Paren;
              break;
            case '[' :
              TokenToCharacterize.TokenType = Open_Bracket;
              break;
            case ']' :
              TokenToCharacterize.TokenType = Close_Bracket;
              break;
            case '{' :
              TokenToCharacterize.TokenType = Open_Brace;
              break;
            case '}' :
              TokenToCharacterize.TokenType = Close_Brace;
              break;
            case '=' :
              TokenToCharacterize.TokenType = LVM_EQ_Sign;
              break;
          } /* endswitch */
          /* Replace the current token in the list. */
          ReplaceToken(TokenList,sizeof(Token),&TokenToCharacterize,TokenPosition,&Error);
          break;
        case String:
          {
            unsigned int    index1=0;

            /* Compare the identifier's value against the value element of the */
            /* table that is marked as a reserved word until a match is found  */
            /* or the end of the table is reached.   (The last element in the  */
            /* table is marked as type "None" such that when this element is   */
            /* reached, the while loop will terminate.                         */
            /* Convert the string to uppercase before comparing it with the    */
            /* tables elements because the elements are uppercase.             */
            pUppercaseToken = (pSTRING)malloc(strlen(TokenToCharacterize.pTokenText)+1);
            if (pUppercaseToken != NULL)
            {

              for (index1=0; index1 < strlen(TokenToCharacterize.pTokenText); index1++)
              {
                pUppercaseToken[index1] = toupper(TokenToCharacterize.pTokenText[index1]);
              } /* endfor */
              /* Append NULL character to uppercase version of string */
              pUppercaseToken[index1] = '\0';

              index1 = 0;
              while (ReservedWords[index1].TokenType != Eof)
              {
                /* Compare the uppercase version of the token with the TokenText */
                /* field in the ReservedWords table. */
                if (strcmp(pUppercaseToken, ReservedWords[index1].pTokenText)==0)
                {
                  /* Since the uppercase version of the token matched an entry */
                  /* in the ReservedWords table, change the TokenType field    */
                  /* from String to the TokenType value for this table entry.  */
                  TokenToCharacterize.TokenType = ReservedWords[index1].TokenType;


                  /* If the token's ordinal value is greater than FileNameStr */
                  if (TokenToCharacterize.TokenType > FileNameStr)
                  {
                    /* Free the memory allocated for this token's string value, */
                    /* it can never be a valid acceptable name nor filename     */
                    /* because it does not begin with a letter. */

                    /* NOTE: Testing the range of ordinal values is made possible */
                    /* because the grouping of the entries in the enumerated type */
                    /* TokenTypes has been specifically done such that any value  */
                    /* above FileNameStr represents a token which can never be an */
                    /* "acceptable character" string or a filename. */
                    if (TokenToCharacterize.pTokenText != NULL)
                      free(TokenToCharacterize.pTokenText);

                    /* Make sure the pointer to this token's text field is NULL */
                    TokenToCharacterize.pTokenText = NULL;
                  } /* endif */

                  /* Replace the current token in the list. */
                  ReplaceToken(TokenList,sizeof(Token),&TokenToCharacterize,TokenPosition,&Error);

                  break;
                }
                else
                {
                  /* Increment the index1 variable to access the next element. */
                  index1++;
                } /* endif */
              } /* endwhile */
              /* Free the memory used to store the uppercase version of the token */
              free(pUppercaseToken);
            }
            else
            {
              ReportError(Internal_Error);  /* Report the error. */
              return ReturnValue;
            } /* endif */

            /* If the token type is still String (i.e. it was not a reserved word) */
            if (TokenToCharacterize.TokenType == String)
            {
              { /* start of block */
                unsigned int    index2=0;
                unsigned int    index3=0;

                /* Assume this string contains only "acceptable characters" as define */
                /* by the grammar.  Then check each character in the string to make   */
                /* sure this is truly the case.  If a character is found that is not  */
                /* "acceptable", change the TokenType to FileNameStr.                 */
                TokenToCharacterize.TokenType = AcceptableCharsStr;
                for (index2=0; index2 < strlen(TokenToCharacterize.pTokenText); index2++)
                {
                  if (!isgraph(TokenToCharacterize.pTokenText[index2]))
                  {
                    /* An OS/2 filename cannot contain a forward slash or */
                    /* double quote so check the remaining characters to  */
                    /* make sure neither of these two exist in the        */
                    /* remainder of the string. */
                    for (index3=index2;index3 < strlen(TokenToCharacterize.pTokenText); index3++)
                    {
                      if ((TokenToCharacterize.pTokenText[index2] == '/')
                          ||
                          (TokenToCharacterize.pTokenText[index2] == '"'))
                      {
                        break;
                      } /* endif */
                      TokenToCharacterize.TokenType = FileNameStr;
                    } /* endfor */
                    break;
                  } /* endif */
                } /* endfor */

                /* Replace the current token in the list. */
                ReplaceToken(TokenList,sizeof(Token),&TokenToCharacterize,TokenPosition,&Error);
              } /* end of block */
            } /* endif */
          }
          break;
      } /* endswitch */

      /* If the current token is NOT Eof, Space, MultiSpace, Tab, MultiTab,
   advance to the next item in the list. */
      switch(TokenToCharacterize.TokenType)
      {
   case Eof:
   case Space:
   case MultiSpace:
   case Tab:
   case MultiTab:
     break;
   default:
     NextToken(TokenList,&Error);
     break;
      }
      /* If Eof, then break out of loop */
      if ( TokenToCharacterize.TokenType == Eof )
   break;
    } /* endif */
  } /* end of while */

  /* If we successfully characterized all of the tokens */
  if ( !Error )
  {
    ReturnValue = TRUE;
  }
  else
  {
    ReturnValue = FALSE;
    ReportError(Internal_Error);  /* Report the error. */
  }

  return ReturnValue;
}
