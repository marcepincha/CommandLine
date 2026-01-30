/**
 * \file command_user.c
 * \brief User-defined CLI command table and handlers.
 *
 * This file contains the command table definition and all user-specific
 * command handlers. Users should extend this file to add their own commands.
 *
 * \author Marcelo Haberman (marcelo.haberman@gmail.com)
 * \version 1.0
 */

#include "command_user.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * \brief Example AT command handler.
 *
 * This is a minimal example handler. Replace with your command logic.
 *
 * \param[in] arg Command argument string (may be NULL)
 */
void ATexec(const char* arg);

/**
 * \brief Command table definition.
 *
 * Each command defines its ID and optional handler pointers for:
 * - write: Called with "command=arg" format
 * - read:  Called with "command?" format
 * - excecute: Called with "command arg" or "command" format
 *
 * Set handler to NULL if operation is not supported.
 */
const cli_cmd_t cli_Table[] = {     
    {"AT", NULL, NULL, ATexec},
    /* Add more commands here following the same pattern:
     * {"COMMAND_NAME", write_handler, read_handler, execute_handler},
     */
};

/** \brief Size of the command table */
const uint16_t cli_TableSize = sizeof(cli_Table) / sizeof(cli_Table[0]);

/**
 * \brief Example AT command execution.
 *
 * Demonstrates a simple command handler.
 *
 * \param[in] arg Command argument (unused in this example)
 */
void ATexec(const char* arg){
	CMD_Print("OK\r\n");
}


