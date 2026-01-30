/**
 * \file command.c
 * \brief Command Line Interface (CLI) implementation.
 *
 * Core implementation of the CLI parser and dispatcher. Provides command parsing
 * with support for multiple operation types (read, write, execute) and configurable
 * case sensitivity.
 *
 * \author Marcelo Haberman (marcelo.haberman@gmail.com)
 * \version 1.1
 * \date 2026-01-30
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "command.h"

/**
 * \brief Display help information listing all available commands.
 *
 * Shows all commands with their supported operations (Write, Read, Execute).
 *
 * \param[in] arg Unused argument (required by handler signature)
 */
static void _helpExec(const char *arg);

/**
 * \brief Convert a string to uppercase.
 *
 * Safely converts a source string to uppercase and stores the result in destination buffer.
 * Ensures null termination and respects destination buffer size limit.
 *
 * \param[in] src       Source string to convert
 * \param[out] dest     Destination buffer for uppercase string
 * \param[in] dest_size Size of destination buffer
 */
static void to_upper(const char *src, char *dest, size_t dest_size);

void cli_init(cli_instance_t *instance, const cli_cmd_t *table, uint16_t table_size, uint8_t has_help_command, uint8_t case_sensitive)
{
	instance->Table = table;
	instance->TableSize = table_size;
	instance->HasHelpCommand = has_help_command;
	instance->CaseSensitive = case_sensitive;
}

void cli_dispatch_command(cli_req_t *cmd_req)
{
	if (NULL != cmd_req->handler)
		cmd_req->handler((const char *)cmd_req->arg);
}

cli_req_t cli_parse_command(const char *buffer, const cli_instance_t *instance)
{
	int8_t length;

	char *pmark;
	char mark = 0;

	int8_t i;
	int8_t elegido;

	char cmd_input[CLI_CMD_MAX_LEN];

	cli_req_t out = {.handler = NULL, .arg = NULL};

	/* Get command length - return empty if length is 0 or exceeds limits */
	length = strnlen(buffer, CLI_BUFFER_LEN);
	if (0 == length || length >= CLI_BUFFER_LEN)
		return out;

	/* Copy buffer to cmd_input for manipulation and convert to uppercase if needed */
	if(instance->CaseSensitive == 0){
		to_upper(buffer, cmd_input, CLI_CMD_MAX_LEN);
	}else{
		strncpy(cmd_input, buffer, CLI_CMD_MAX_LEN);
		cmd_input[CLI_CMD_MAX_LEN - 1] = '\0';
	}

	/* Check if it's a help command */
	if (instance->HasHelpCommand)
	{
		char cmd_help[CLI_CMD_MAX_LEN];

		if(instance->CaseSensitive == 0){
			to_upper(CMD_HELP_COMMAND, cmd_help, CLI_CMD_MAX_LEN);
		}else{
			strncpy(cmd_help, CMD_HELP_COMMAND, CLI_CMD_MAX_LEN);
			cmd_help[CLI_CMD_MAX_LEN - 1] = '\0';
		}

		if (0 == strcmp(cmd_input, cmd_help))
		{
			out.handler = _helpExec;
			out.arg = NULL;
			return out;
		}
	}

	/* Find delimiter character (space, equals, or question mark) */
	pmark = strpbrk(buffer, " =?");
	/* If no delimiter character found */
	if (NULL == pmark)
	{
		if (length >= CLI_CMD_MAX_LEN) /* Return if command length exceeds limit */
			return out;
	}
	else /* Delimiter found */
	{
		uint8_t command_length = pmark - buffer;
		uint8_t arg_length = strlen(pmark + 1);

		if (command_length >= CLI_CMD_MAX_LEN) /* Return if command length exceeds limit */
			return out;
		if (arg_length >= CLI_ARG_MAX_LEN) /* Return if argument length exceeds limit */
			return out;

		mark = *pmark;

		cmd_input[command_length] = '\0';
	}

	elegido = -1;
	for (i = 0; (i < instance->TableSize) && (elegido < 0); i++)
	{		
		char cmd_tbl[CLI_CMD_MAX_LEN];

		if(instance->CaseSensitive == 0){
			to_upper(instance->Table[i].commandID, cmd_tbl, CLI_CMD_MAX_LEN);
		}else{
			strncpy(cmd_tbl, instance->Table[i].commandID, CLI_CMD_MAX_LEN);
			cmd_tbl[CLI_CMD_MAX_LEN - 1] = '\0';
		}

		if (0 == strcmp(cmd_input, cmd_tbl))
			elegido = i;
	}

	if (elegido < 0)
		return out;

	if (NULL == pmark)
	{
		out.handler = instance->Table[elegido].excecute;
		out.arg = NULL;
	}
	else
	{
		out.arg = pmark + 1;
		switch (mark)
		{
		case ' ':
			out.handler = instance->Table[elegido].excecute;
			break;
		case '=':
			out.handler = instance->Table[elegido].write;
			break;
		case '?':
			out.handler = instance->Table[elegido].read;
			break;
		}
	}

	return out;
}

void _helpExec(const char *arg)
{
	CMD_Print("\r\nAvailable Commands:\r\n");

	int i;
	char temp[36];
	for (i = 0; i < cli_TableSize; i++)
	{
		snprintf(temp, 36, "\t%25s\t[%c%c%c]\r\n", cli_Table[i].commandID,
				 cli_Table[i].write == NULL ? '-' : 'W',
				 cli_Table[i].read == NULL ? '-' : 'R',
				 cli_Table[i].excecute == NULL ? '-' : 'X');
		CMD_Print(temp);
	}
}

/**
 * \brief Convert a string to uppercase.
 *
 * Helper function that safely converts a source string to uppercase and stores
 * the result in a destination buffer. Ensures proper null termination and respects
 * the destination buffer size limit.
 *
 * \param[in] src       Source string to convert (null-terminated)
 * \param[out] dest     Destination buffer for uppercase string
 * \param[in] dest_size Size of destination buffer (including null terminator)
 */
static void to_upper(const char *src, char *dest, size_t dest_size) {
    while (*src && dest_size > 1) {
        *dest = (char)toupper((unsigned char)*src);
        src++;
        dest++;
        dest_size--;
    }
    *dest = '\0'; /* Null terminator */
}
