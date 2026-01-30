# Command Line Interface (CLI) Library

**Author:** Marcelo Haberman  
**Email:** marcelo.haberman@gmail.com  
**Version:** 1.1  
**Date:** January 30, 2026  

---

A lightweight, modular command-line interface library for embedded systems and applications. This library provides a flexible framework for parsing and dispatching user commands with support for read, write, and execute operations.

## Overview

The CLI library implements a command parser that supports:

- **Multiple Operation Types**: Execute, Read (?), and Write (=) operations
- **Configurable Case Sensitivity**: Commands can be case-sensitive or case-insensitive
- **Built-in Help Command**: Automatic HELP command that lists all available commands
- **Extensible Command Table**: Easy to add custom commands
- **Modular Design**: Separate configuration, core, and user implementation files
- **Embedded-Friendly**: Minimal dependencies, efficient buffer management

## Architecture

### Core Components

- **command.h / command.c**: Core CLI implementation
  - Command parsing logic
  - Command dispatching mechanism
  - Built-in help system

- **command_cfg.h**: Configuration header
  - Buffer size limits
  - Platform-specific macros (output function)
  - Command prompt and keywords

- **command_user.h / command_user.c**: User implementation
  - Application-specific command definitions
  - Command handlers
  - Command table

### File Structure

```
CommandLine/
├── command.h              # Core CLI definitions and declarations
├── command.c              # Core CLI implementation
├── command_cfg.h          # Configuration parameters
├── command_user.h         # User command declarations template
├── command_user.c         # User command implementations
├── test/
│   └── test.c            # Example test/demonstration
└── README.md             # This file
```

## Configuration

Configuration is managed in `command_cfg.h`:

```c
#define CLI_CMD_MAX_LEN 25        /* Maximum command name length */
#define CLI_ARG_MAX_LEN 40        /* Maximum argument length */
#define CLI_BUFFER_LEN 66         /* Total buffer size */
#define CMD_PROMPT "CMD> "        /* Command prompt string */
#define CMD_HELP_COMMAND "HELP"   /* Help command keyword */
#define CMD_Print(s) puts(s)      /* Output macro (platform-specific) */
```

### Customization

To modify output behavior for your platform (e.g., UART, custom logging):

```c
/* In command_cfg.h */
#define CMD_Print(s) my_uart_write(s)  /* Replace with your platform function */
```

## Usage

### Basic Initialization

```c
#include "command_user.h"

int main(void) {
    cli_instance_t cli_instance;
    
    /* Initialize CLI with user command table */
    cli_init(&cli_instance, cli_Table, cli_TableSize, 
             1,  /* Enable HELP command */
             0); /* Case-insensitive commands */
    
    /* Your command processing loop */
    while(1) {
        char buffer[CLI_BUFFER_LEN];
        
        /* Get user input (implementation-specific) */
        fgets(buffer, sizeof(buffer), stdin);
        
        /* Parse and execute command */
        cli_req_t request = cli_parse_command(buffer, &cli_instance);
        cli_dispatch_command(&request);
    }
    
    return 0;
}
```

### Supported Command Formats

The CLI parser supports three operation types:

#### 1. Execute Command

```
COMMAND [argument]
AT
AT some_argument
```

Parsed as: execute handler with optional argument

#### 2. Write/Set Operation

```
COMMAND=argument
TIMEOUT=5000
```

Parsed as: write handler with argument

#### 3. Read/Get Operation

```
COMMAND?
TIMEOUT?
```

Parsed as: read handler (argument portion is everything after '?')

### Defining Custom Commands

1. **Declare handlers** in `command_user.h`:

```c
/**
 * \brief Set device timeout.
 * \param[in] arg Timeout value as string
 */
void timeout_write(const char *arg);

/**
 * \brief Get current timeout.
 * \param[in] arg Unused
 */
void timeout_read(const char *arg);

/**
 * \brief Execute timeout functionality.
 * \param[in] arg Optional argument
 */
void timeout_exec(const char *arg);
```

2. **Implement handlers** in `command_user.c`:

```c
void timeout_write(const char *arg) {
    if (arg != NULL) {
        uint32_t timeout = atoi(arg);
        set_device_timeout(timeout);
        CMD_Print("Timeout set.\r\n");
    } else {
        CMD_Print("ERROR: Timeout value required\r\n");
    }
}

void timeout_read(const char *arg) {
    uint32_t current_timeout = get_device_timeout();
    printf("Current timeout: %lu ms\r\n", current_timeout);
}

void timeout_exec(const char *arg) {
    CMD_Print("TIMEOUT command executed.\r\n");
}
```

3. **Add command to table** in `command_user.c`:

```c
const cli_cmd_t cli_Table[] = {     
    {"AT",      NULL,            NULL,            ATexec},
    {"TIMEOUT", timeout_write,   timeout_read,    timeout_exec},
    /* Additional commands... */
};

const uint16_t cli_TableSize = sizeof(cli_Table) / sizeof(cli_Table[0]);
```

### Example: Extended Command Handler

```c
#include "command_user.h"
#include <stdio.h>
#include <string.h>

/* Status structure */
typedef struct {
    uint32_t device_id;
    uint16_t version;
    uint8_t  enabled;
} device_status_t;

static device_status_t status = {
    .device_id = 0x1234,
    .version = 0x0100,
    .enabled = 1
};

void status_read(const char *arg) {
    printf("Device ID:  0x%04X\r\n", status.device_id);
    printf("Version:    0x%04X\r\n", status.version);
    printf("Enabled:    %s\r\n", status.enabled ? "YES" : "NO");
}

void status_write(const char *arg) {
    if (strcmp(arg, "ON") == 0) {
        status.enabled = 1;
        CMD_Print("Device enabled.\r\n");
    } else if (strcmp(arg, "OFF") == 0) {
        status.enabled = 0;
        CMD_Print("Device disabled.\r\n");
    } else {
        CMD_Print("ERROR: Expected ON or OFF\r\n");
    }
}

void status_exec(const char *arg) {
    status_read(NULL);
}

const cli_cmd_t cli_Table[] = {
    {"AT",     NULL,          NULL,         ATexec},
    {"STATUS", status_write,  status_read,  status_exec},
};

const uint16_t cli_TableSize = sizeof(cli_Table) / sizeof(cli_Table[0]);
```

## Command Parsing Details

### Parser Behavior

- **Buffer length validation**: Commands must be non-empty and fit within `CLI_BUFFER_LEN`
- **Command truncation**: Command names are truncated to `CLI_CMD_MAX_LEN`
- **Argument truncation**: Arguments are truncated to `CLI_ARG_MAX_LEN`
- **Case sensitivity**: Controlled by `cli_init()` parameter
- **Delimiter detection**: Searches for space (execute), '=' (write), or '?' (read)

### Invalid Command Handling

When `cli_parse_command()` returns a request with `handler == NULL`:

```c
cli_req_t request = cli_parse_command(buffer, &cli_instance);

if (request.handler == NULL) {
    /* Command not found or parsing failed */
    CMD_Print("ERROR: Unknown command\r\n");
} else {
    cli_dispatch_command(&request);
}
```

### Help Command Output

When help is enabled (`has_help_command = 1`):

```
CMD> HELP
Available Commands:
                           AT	[-R-]
                       TIMEOUT	[WRX]
                       STATUS	[WRX]
```

Legend:
- `W` = Write operation supported
- `R` = Read operation supported
- `X` = Execute operation supported
- `-` = Operation not supported

## API Reference

### Core Functions

#### `cli_init()`

```c
void cli_init(cli_instance_t *instance,
              const cli_cmd_t *table,
              uint16_t table_size,
              uint8_t has_help_command,
              uint8_t case_sensitive);
```

Initialize a CLI instance with command table and configuration.

**Parameters:**
- `instance`: CLI instance to initialize
- `table`: Pointer to command table array
- `table_size`: Number of commands in table
- `has_help_command`: 1 to enable HELP command, 0 to disable
- `case_sensitive`: 1 for case-sensitive commands, 0 for case-insensitive

#### `cli_parse_command()`

```c
cli_req_t cli_parse_command(const char *buffer,
                            const cli_instance_t *instance);
```

Parse input buffer into a command request.

**Parameters:**
- `buffer`: Input command string
- `instance`: Initialized CLI instance

**Returns:** `cli_req_t` structure with handler and argument (handler is NULL if command not found)

#### `cli_dispatch_command()`

```c
void cli_dispatch_command(cli_req_t *cmd_req);
```

Execute a parsed command request.

**Parameters:**
- `cmd_req`: Parsed command request structure

**Behavior:** Calls the handler with the argument if handler is not NULL

### Data Types

#### `cli_instance_t`

```c
typedef struct {
    const cli_cmd_t *Table;      /* Command table pointer */
    uint16_t TableSize;          /* Number of commands */
    uint8_t HasHelpCommand;      /* Help command enabled */
    uint8_t CaseSensitive;       /* Case sensitivity flag */
} cli_instance_t;
```

#### `cli_cmd_t`

```c
typedef struct {
    const char commandID[CLI_CMD_MAX_LEN];  /* Command name */
    cli_cmdHandler_t write;                 /* Write handler (set to NULL if unused) */
    cli_cmdHandler_t read;                  /* Read handler (set to NULL if unused) */
    cli_cmdHandler_t excecute;              /* Execute handler (set to NULL if unused) */
} cli_cmd_t;
```

#### `cli_req_t`

```c
typedef struct {
    volatile cli_cmdHandler_t handler;  /* Handler function or NULL */
    volatile const char *arg;           /* Argument pointer or NULL */
} cli_req_t;
```

#### `cli_cmdHandler_t`

```c
typedef void (*cli_cmdHandler_t)(const char *arg);
```

Function pointer type for command handlers.

## Building

### Compilation

Basic compilation with GCC:

```bash
gcc -c command.c -o command.o
gcc -c command_user.c -o command_user.o
gcc command.o command_user.o main.c -o cli_app
```

### With Test

```bash
cd test
gcc test.c ../*.c -o test.exe
./test.exe
```

## Documentation

### Doxygen

This library includes complete Doxygen documentation. Generate documentation with:

```bash
doxygen Doxyfile
```

Documentation will be generated in `doc/html/index.html`

## Improvements and Enhancements

### Version 1.1 Improvements

- ✅ Complete Doxygen documentation added
- ✅ All comments translated to English
- ✅ Improved code structure and formatting
- ✅ Enhanced error handling
- ✅ Better buffer management validation
- ✅ Clear API documentation
- ✅ Comprehensive README with examples

### Potential Future Enhancements

- Command history/line editing support
- Argument parsing utilities
- Command aliasing
- Parameter validation framework
- Sub-command support
- Command macros/scripting
- Asynchronous command execution
- Command timeout handling

## Error Handling

### Parser Errors

The parser silently returns a NULL handler for invalid commands. It's recommended to check:

```c
cli_req_t req = cli_parse_command(input, &cli);

if (req.handler == NULL) {
    /* Handle invalid command */
    CMD_Print("Unknown command. Type HELP for assistance.\r\n");
} else {
    cli_dispatch_command(&req);
}
```

### Handler Errors

Handlers are responsible for validating arguments:

```c
void my_handler(const char *arg) {
    if (arg == NULL) {
        CMD_Print("ERROR: Argument required\r\n");
        return;
    }
    
    /* Process argument safely */
}
```

## Platform Porting

To port to a different platform:

1. **Modify `CMD_Print` macro** in `command_cfg.h`:
   ```c
   #define CMD_Print(s) uart_write_string(s)  /* UART */
   #define CMD_Print(s) log_message(s)        /* Logging system */
   ```

2. **Update buffer sizes** if needed for your platform

3. **Implement input reading** in your main loop (not part of library)

## License

This library is provided as-is for educational and commercial use.

## Contributing

Suggestions and improvements are welcome. Consider:
- Additional documentation
- Platform-specific examples
- Performance optimizations
- New feature implementations

## Contact

For questions or issues, please refer to the code documentation or examine the example implementations in `command_user.c` and `test/test.c`.

---

**Version:** 1.1  
**Last Updated:** January 30, 2026  
**Author:** Marcelo
