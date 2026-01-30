/**
 * \file command.h
 * \brief Command Line Interface (CLI) core definitions and function prototypes.
 *
 * This header file defines the core structures and functions for a flexible,
 * modular command line interface system. It supports read, write, and execute
 * operations with configurable case sensitivity.
 *
 * \author Marcelo Haberman (marcelo.haberman@gmail.com)
 * \version 1.1
 * \date 2026-01-30
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include <stdint.h>
#include "command_cfg.h"

/** \brief Forward declaration of command structure type */
typedef struct _cli_str cli_cmd_t;
/** \brief Forward declaration of request structure type */
typedef struct _cli_req cli_req_t;

/**
 * \brief Function pointer type for command handlers.
 *
 * All command handlers (read, write, execute) use this signature.
 * The handler receives an optional argument string that can be NULL.
 *
 * \param[in] arg Pointer to command argument string (may be NULL)
 */
typedef void (*cli_cmdHandler_t) (const char*);


/**
 * \brief CLI instance configuration structure.
 *
 * Contains all configuration for a CLI instance including command table,
 * size, and behavior flags (help command enabled, case sensitivity).
 */
typedef struct {
	const cli_cmd_t *Table;         /**< Pointer to command table */
	uint16_t TableSize;             /**< Number of commands in table */
    uint8_t HasHelpCommand;        /**< Enable built-in HELP command (1=enabled, 0=disabled) */
    uint8_t CaseSensitive;         /**< Command case sensitivity (1=sensitive, 0=insensitive) */
}cli_instance_t;

/**
 * \brief Parsed command request structure.
 *
 * Result of parsing a command string. Contains the handler function to execute
 * and an optional pointer to the argument portion of the command.
 */
typedef struct _cli_req{
	volatile cli_cmdHandler_t handler;  /**< Handler function to execute (NULL if invalid command) */
	volatile const char *arg;           /**< Pointer to argument string (NULL if no argument) */
} cli_req_t;

/**
 * \brief Command definition structure.
 *
 * Defines a single command with its ID and three optional operation handlers:
 * - write: invoked with "command=arg"
 * - read: invoked with "command?arg" or "command?"
 * - execute: invoked with "command arg" or just "command"
 *
 * Set handler to NULL if operation is not supported.
 */
typedef struct _cli_str{
    const char commandID[CLI_CMD_MAX_LEN];  /**< Command identifier string */
    cli_cmdHandler_t write;                 /**< Write/set operation handler (NULL if not supported) */
    cli_cmdHandler_t read;                  /**< Read/get operation handler (NULL if not supported) */
    cli_cmdHandler_t excecute;              /**< Execute operation handler (NULL if not supported) */
}cli_cmd_t;

/***************** Implemented in command.c ********************/

/**
 * \brief Initialize a CLI instance.
 *
 * Configures a CLI instance with a command table and behavioral flags.
 *
 * \param[out] instance      Pointer to CLI instance to initialize
 * \param[in]  table         Pointer to command definition table
 * \param[in]  table_size    Number of commands in the table
 * \param[in]  has_help_command  Enable HELP command (1=yes, 0=no)
 * \param[in]  case_sensitive    Command case sensitivity (1=sensitive, 0=insensitive)
 */
void cli_init(  cli_instance_t* instance,
                const cli_cmd_t* table, 
                uint16_t table_size, 
                uint8_t has_help_command,
                uint8_t case_sensitive);

/**
 * \brief Execute a parsed command request.
 *
 * Calls the handler function with the provided argument if handler is not NULL.
 *
 * \param[in] cmd_req Pointer to command request structure with handler and argument
 */
void cli_dispatch_command(cli_req_t* cmd_req);

/**
 * \brief Parse a command string into a request structure.
 *
 * Parses the input buffer according to the command table and CLI configuration.
 * Returns a request structure with handler and argument (or NULL handler if invalid).
 *
 * Supported formats:
 * - "COMMAND" - Execute with no argument
 * - "COMMAND arg" - Execute with argument
 * - "COMMAND=arg" - Write operation with argument
 * - "COMMAND?" or "COMMAND?arg" - Read operation
 *
 * \param[in] buffer      Input command string to parse
 * \param[in] instance    Pointer to initialized CLI instance
 * \return    Parsed command request structure
 */
cli_req_t cli_parse_command(const char *buffer, const cli_instance_t *instance);


/***************** User implementation interface ********************/

/** \brief User-defined command table. Must be defined in command_user.c */
extern const cli_cmd_t cli_Table[];

/** \brief Size of user command table. Must be defined in command_user.c */
extern const uint16_t cli_TableSize;


#endif // COMMAND_H_
