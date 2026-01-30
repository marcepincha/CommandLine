#include "../command_user.h"
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
