/**
 * \file command_cfg.h
 * \brief CLI Configuration and platform-specific definitions.
 *
 * Contains configuration parameters for buffer sizes, command prompt,
 * and platform-specific output macro.
 *
 * \author Marcelo Haberman (marcelo.haberman@gmail.com)
 * \version 1.1
 * \date 2026-01-30
 */

#ifndef COMMAND_CFG_H
#define COMMAND_CFG_H

/** \defgroup Configuration CLI Configuration Parameters
 *  Compile-time configuration for buffer sizes and behavior.
 *  @{
 */

/** \brief Maximum length of command identifier string */
#define CLI_CMD_MAX_LEN 25

/** \brief Maximum length of command argument string */
#define CLI_ARG_MAX_LEN 40

/** \brief Maximum length of complete command buffer (command + argument + separator) */
#define CLI_BUFFER_LEN (CLI_CMD_MAX_LEN + CLI_ARG_MAX_LEN + 1)

/** @} */

/** \defgroup Interface CLI User Interface Configuration
 *  @{
 */

/** \brief Command prompt string displayed to user */
#define CMD_PROMPT         "CMD> "

/** \brief Help command keyword */
#define CMD_HELP_COMMAND   "HELP"

/** @} */

/** \defgroup PlatformSpecific Platform-Specific Definitions
 *  @{
 */

#include <stdio.h>

extern void CDC_PrintStr(const char* s);
#define CMD_Print(s)       CDC_PrintStr(s)

/** @} */

#endif /* COMMAND_CFG_H */