/**
 * \file EXAMPLES.md
 * \brief Advanced usage examples for the CLI library.
 *
 * This document provides comprehensive examples showing how to use the CLI
 * library in various scenarios.
 *
 * \author Marcelo Haberman (marcelo.haberman@gmail.com)
 * \version 1.1
 */

# CLI Library - Advanced Examples

## Example 1: Simple Device Control

This example shows a basic device control system with power and status commands.

### Implementation

**device_commands.h:**
```c
#ifndef DEVICE_COMMANDS_H_
#define DEVICE_COMMANDS_H_

#include "command_user.h"

/* Device state */
typedef struct {
    uint8_t power;
    uint16_t temperature;
    uint8_t status;
} device_state_t;

extern device_state_t device;

/* Command handlers */
void power_write(const char *arg);
void power_read(const char *arg);
void temp_read(const char *arg);
void status_exec(const char *arg);

#endif
```

**device_commands.c:**
```c
#include "device_commands.h"
#include <stdio.h>
#include <string.h>

device_state_t device = {
    .power = 0,
    .temperature = 25,
    .status = 0x00
};

void power_write(const char *arg) {
    if (arg == NULL) {
        CMD_Print("Usage: POWER=ON|OFF\r\n");
        return;
    }
    
    if (strcmp(arg, "ON") == 0) {
        device.power = 1;
        CMD_Print("Device powered ON\r\n");
    } else if (strcmp(arg, "OFF") == 0) {
        device.power = 0;
        CMD_Print("Device powered OFF\r\n");
    } else {
        CMD_Print("ERROR: Invalid value. Use ON or OFF\r\n");
    }
}

void power_read(const char *arg) {
    printf("Power: %s\r\n", device.power ? "ON" : "OFF");
}

void temp_read(const char *arg) {
    printf("Temperature: %d C\r\n", device.temperature);
}

void status_exec(const char *arg) {
    printf("=== Device Status ===\r\n");
    printf("Power:       %s\r\n", device.power ? "ON" : "OFF");
    printf("Temperature: %d C\r\n", device.temperature);
    printf("Status code: 0x%02X\r\n", device.status);
}
```

**command_user.c:**
```c
#include "command_user.h"
#include "device_commands.h"

const cli_cmd_t cli_Table[] = {
    {"AT",        NULL,           NULL,          ATexec},
    {"POWER",     power_write,    power_read,    NULL},
    {"TEMP",      NULL,           temp_read,     NULL},
    {"STATUS",    NULL,           NULL,          status_exec},
};

const uint16_t cli_TableSize = sizeof(cli_Table) / sizeof(cli_Table[0]);

void ATexec(const char* arg) {
    CMD_Print("OK\r\n");
}
```

### Usage

```
CMD> AT
OK

CMD> POWER=ON
Device powered ON

CMD> POWER?
Power: ON

CMD> TEMP?
Temperature: 25 C

CMD> STATUS
=== Device Status ===
Power:       ON
Temperature: 25 C
Status code: 0x00

CMD> HELP
Available Commands:
                           AT	[-R-]
                        POWER	[WR-]
                         TEMP	[-R-]
                       STATUS	[--X]
```

## Example 2: Configuration Commands

This example demonstrates how to implement configuration save/load functionality.

```c
#include "command_user.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint16_t timeout;
    uint8_t debug_mode;
    uint16_t retry_count;
} system_config_t;

static system_config_t config = {
    .timeout = 1000,
    .debug_mode = 0,
    .retry_count = 3
};

/* Configuration handlers */
void config_timeout_write(const char *arg) {
    if (arg == NULL) {
        CMD_Print("Usage: TIMEOUT=<milliseconds>\r\n");
        return;
    }
    
    int timeout = atoi(arg);
    if (timeout <= 0) {
        CMD_Print("ERROR: Timeout must be positive\r\n");
        return;
    }
    
    config.timeout = (uint16_t)timeout;
    printf("Timeout set to %d ms\r\n", config.timeout);
}

void config_timeout_read(const char *arg) {
    printf("Timeout: %d ms\r\n", config.timeout);
}

void config_debug_write(const char *arg) {
    if (arg == NULL) {
        CMD_Print("Usage: DEBUG=ON|OFF\r\n");
        return;
    }
    
    if (strcmp(arg, "ON") == 0) {
        config.debug_mode = 1;
        CMD_Print("Debug mode ON\r\n");
    } else if (strcmp(arg, "OFF") == 0) {
        config.debug_mode = 0;
        CMD_Print("Debug mode OFF\r\n");
    } else {
        CMD_Print("ERROR: Use ON or OFF\r\n");
    }
}

void config_debug_read(const char *arg) {
    printf("Debug mode: %s\r\n", config.debug_mode ? "ON" : "OFF");
}

void config_show_exec(const char *arg) {
    printf("=== System Configuration ===\r\n");
    printf("Timeout:     %d ms\r\n", config.timeout);
    printf("Debug mode:  %s\r\n", config.debug_mode ? "ON" : "OFF");
    printf("Retry count: %d\r\n", config.retry_count);
}

const cli_cmd_t cli_Table[] = {
    {"AT",       NULL,                    NULL,                  ATexec},
    {"TIMEOUT",  config_timeout_write,   config_timeout_read,   NULL},
    {"DEBUG",    config_debug_write,     config_debug_read,     NULL},
    {"CONFIG",   NULL,                    NULL,                  config_show_exec},
};

const uint16_t cli_TableSize = sizeof(cli_Table) / sizeof(cli_Table[0]);

void ATexec(const char* arg) {
    CMD_Print("OK\r\n");
}
```

## Example 3: Error Handling and Validation

Proper error handling in command handlers:

```c
#include "command_user.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Validate that string is a number */
static int is_numeric(const char *str) {
    if (str == NULL || *str == '\0') return 0;
    
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

/* Validate that string is hex */
static int is_hex(const char *str) {
    if (str == NULL || *str == '\0') return 0;
    
    while (*str) {
        if (!isxdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

void register_device(const char *arg) {
    if (arg == NULL || strlen(arg) == 0) {
        CMD_Print("ERROR: Device ID required\r\n");
        return;
    }
    
    /* Validate ID is 8 hex digits */
    if (strlen(arg) != 8 || !is_hex(arg)) {
        CMD_Print("ERROR: Device ID must be 8 hex digits (e.g., 12AB34CD)\r\n");
        return;
    }
    
    uint32_t device_id = (uint32_t)strtol(arg, NULL, 16);
    printf("Device registered: 0x%08X\r\n", device_id);
}

void set_parameter(const char *arg) {
    if (arg == NULL) {
        CMD_Print("Usage: PARAM=<value>\r\n");
        CMD_Print("Value must be numeric between 0-65535\r\n");
        return;
    }
    
    if (!is_numeric(arg)) {
        CMD_Print("ERROR: Invalid value. Must be numeric\r\n");
        return;
    }
    
    int value = atoi(arg);
    if (value < 0 || value > 65535) {
        CMD_Print("ERROR: Value out of range (0-65535)\r\n");
        return;
    }
    
    printf("Parameter set to %d\r\n", value);
}

const cli_cmd_t cli_Table[] = {
    {"AT",       NULL,                NULL,              ATexec},
    {"REGISTER", NULL,                NULL,              register_device},
    {"PARAM",    set_parameter,       NULL,              NULL},
};

const uint16_t cli_TableSize = sizeof(cli_Table) / sizeof(cli_Table[0]);

void ATexec(const char* arg) {
    CMD_Print("OK\r\n");
}
```

## Example 4: State Machine Commands

Using CLI to control a state machine:

```c
#include "command_user.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    STATE_IDLE,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_ERROR
} connection_state_t;

static connection_state_t state = STATE_IDLE;

void connect_exec(const char *arg) {
    if (state != STATE_IDLE) {
        printf("ERROR: Cannot connect from state %d\r\n", state);
        return;
    }
    
    state = STATE_CONNECTING;
    CMD_Print("Connecting...\r\n");
    
    /* Simulate connection */
    state = STATE_CONNECTED;
    CMD_Print("Connected.\r\n");
}

void disconnect_exec(const char *arg) {
    if (state != STATE_CONNECTED) {
        CMD_Print("Not connected\r\n");
        return;
    }
    
    state = STATE_IDLE;
    CMD_Print("Disconnected.\r\n");
}

void state_read(const char *arg) {
    const char *state_names[] = {"IDLE", "CONNECTING", "CONNECTED", "ERROR"};
    printf("Connection state: %s\r\n", state_names[state]);
}

const cli_cmd_t cli_Table[] = {
    {"CONNECT",    NULL,              NULL,              connect_exec},
    {"DISCONNECT", NULL,              NULL,              disconnect_exec},
    {"STATE",      NULL,              state_read,        NULL},
};

const uint16_t cli_TableSize = sizeof(cli_Table) / sizeof(cli_Table[0]);
```

## Example 5: Interactive Command Loop

A complete main function showing how to integrate CLI:

```c
#include "command_user.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    cli_instance_t cli;
    char buffer[CLI_BUFFER_LEN];
    
    /* Initialize CLI */
    cli_init(&cli, cli_Table, cli_TableSize, 
             1,  /* Enable HELP */
             0); /* Case-insensitive */
    
    printf("CLI Library Example\r\n");
    printf("Type 'HELP' for available commands\r\n\r\n");
    
    /* Command loop */
    while (1) {
        printf(CMD_PROMPT);
        
        /* Get user input */
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }
        
        /* Remove newline */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        /* Skip empty lines */
        if (strlen(buffer) == 0) {
            continue;
        }
        
        /* Parse and execute command */
        cli_req_t request = cli_parse_command(buffer, &cli);
        
        if (request.handler == NULL) {
            CMD_Print("ERROR: Unknown command. Type HELP for assistance.\r\n");
        } else {
            cli_dispatch_command(&request);
        }
    }
    
    printf("Goodbye!\r\n");
    return 0;
}
```

## Example 6: Logging and Debugging

Integration with logging for debugging:

```c
#include "command_user.h"
#include <stdio.h>
#include <time.h>

/* Simple logging */
#define LOG_BUFFER_SIZE 256

static char log_buffer[LOG_BUFFER_SIZE];
static uint16_t log_level = 1; /* 0=off, 1=error, 2=warning, 3=info */

void log_message(uint8_t level, const char *format, ...) {
    if (level > log_level) return;
    
    va_list args;
    va_start(args, format);
    
    const char *level_str[] = {"", "ERROR", "WARN", "INFO"};
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    
    printf("[%02d:%02d:%02d] %s: ",
           timeinfo->tm_hour,
           timeinfo->tm_min,
           timeinfo->tm_sec,
           level_str[level]);
    
    vprintf(format, args);
    printf("\r\n");
    va_end(args);
}

void loglevel_write(const char *arg) {
    if (arg == NULL) {
        CMD_Print("Usage: LOGLEVEL=0|1|2|3\r\n");
        CMD_Print("  0=off, 1=error, 2=warning, 3=info\r\n");
        return;
    }
    
    int level = atoi(arg);
    if (level < 0 || level > 3) {
        CMD_Print("ERROR: Log level must be 0-3\r\n");
        return;
    }
    
    log_level = (uint8_t)level;
    printf("Log level set to %d\r\n", log_level);
    log_message(3, "Logging enabled at level %d", level);
}

void loglevel_read(const char *arg) {
    const char *names[] = {"OFF", "ERROR", "WARNING", "INFO"};
    printf("Log level: %s (%d)\r\n", names[log_level], log_level);
}

const cli_cmd_t cli_Table[] = {
    {"AT",       NULL,                NULL,             ATexec},
    {"LOGLEVEL", loglevel_write,     loglevel_read,    NULL},
};

const uint16_t cli_TableSize = sizeof(cli_Table) / sizeof(cli_Table[0]);

void ATexec(const char* arg) {
    log_message(3, "AT command received");
    CMD_Print("OK\r\n");
}
```

## Best Practices

### 1. Always Validate Arguments

```c
void my_handler(const char *arg) {
    /* Check for NULL argument if required */
    if (arg == NULL && operation_requires_arg) {
        CMD_Print("ERROR: Argument required\r\n");
        return;
    }
    
    /* Validate argument format and range */
    if (!is_valid_format(arg)) {
        CMD_Print("ERROR: Invalid argument format\r\n");
        return;
    }
}
```

### 2. Provide Clear Error Messages

```c
CMD_Print("ERROR: Invalid command\r\n");
CMD_Print("Usage: COMMAND=<value>\r\n");
CMD_Print("Example: COMMAND=12345\r\n");
```

### 3. Document Command Behavior

```c
/**
 * \brief Write device mode.
 * 
 * Sets the device operating mode. Supported modes:
 * - "NORMAL": Standard operation mode
 * - "DEBUG": Enhanced debug output
 * - "STANDBY": Low-power mode
 *
 * \param[in] arg Mode string (NORMAL|DEBUG|STANDBY)
 */
void mode_write(const char *arg);
```

### 4. Handle Edge Cases

```c
void buffer_clear_exec(const char *arg) {
    /* Even if arg is provided, ignore it if not needed */
    memset(buffer, 0, sizeof(buffer));
    printf("Buffer cleared (%zu bytes)\r\n", sizeof(buffer));
}
```

### 5. Test All Command Formats

```c
/*
 * Test these formats for each command:
 * - COMMAND              (execute)
 * - COMMAND arg          (execute with arg)
 * - COMMAND=value        (write)
 * - COMMAND?             (read)
 * - command              (case sensitivity)
 */
```

---

For more information, see the main README.md and API documentation.
