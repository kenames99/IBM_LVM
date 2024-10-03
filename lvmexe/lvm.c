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
 * Module: lvm.c
 */

/*
 * Change History:
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "constant.h"
#include "display.h"
#include "getkey.h"
#include "panels.h"
#include "user.h"
#include "strings.h"
#include "LVM_Cli.h"


#define DEFAULT_ATTRIBUTE       PANEL_ATTRIBUTE
#define EXIT_ATTRIBUTE          BLACK_BG | WHITE_FG  /* screen on exit */


PUBLIC
bool    Install_time = FALSE;                              /* installing or not */


/*
 * signal_handler ignores the break signal
 *
 */

PRIVATE
int   last_signal_value;

PRIVATE
void
signal_handler ( int   signal_value )
{
    last_signal_value = signal_value;

    signal ( SIGINT, signal_handler );
}


/*
 * main
 * Returns the last error code that was received from the engine on
 * Commit_Changes. So the return value is one of the LVM engine codes,
 * with 0 being no error.
 *
 */


PUBLIC
uint
main ( int  argc,
       char *argv[] )
{
    uint                    min_install_size = 0,
                            error;
    LVMCLI_BackEndToVIO     *cli;

    if ( argc > 1 ) {
        cli = lvmcli ( argc, argv );
        if ( cli->operation & DisplayMinInstallSize ) {
            min_install_size = cli->minPartitonInstallSize;
            Install_time = TRUE;
        }
    }

    #ifndef DEBUG
        signal ( SIGINT, signal_handler );
    #endif

    InitializeUserInterface ( DEFAULT_ATTRIBUTE, EXIT_ATTRIBUTE );
    error = RunUserInterface ( min_install_size );

    ReInitializePanels ( EXIT_ATTRIBUTE, EXIT_ATTRIBUTE );

    return  error;
}
