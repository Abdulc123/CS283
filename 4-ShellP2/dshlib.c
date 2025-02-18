#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h> // Used for fork()
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */


 int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff == NULL) {
        return ERR_MEMORY;
    }

    // Allocate the memory for the internal command input buffer
    cmd_buff->_cmd_buffer = (char *)malloc(SH_CMD_MAX); // Allocates SH_CMD_MAX amount of memory but malloc (normally returns a void *)
    // since _cmd_buffer is a char *, we want malloc to return a char * pointer instead
    if (cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }

    // Initializing the arguments array entries to a null pointer
    // memset(ptr, value, size), 
    // argv is an array of char pointers *, In C a Null pointer is represented as 0,
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv)); // Initializes each pointer in array argv to 0 ~ Null
    // Set argument count to = 0
    cmd_buff->argc = 0;

    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff == NULL) {
        return OK; // Nothing to free
    }

    // If internal buffer was allocated, free it
    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL; // prevents dangling pointer
    }

    // Reset the argv array to null pointers ~ 0
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    cmd_buff->argc = 0; // reset counter to 0

    return OK;
}



int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    // Command input parsing function 

    // reset the argument count and declare pointer to read the buff *str_p
    cmd_buff->argc = 0;
    char *str_p = cmd_line; // pointer starts at cmd_line start

    while (*str_p != '\0') {
        // Skip leading spaces
        while (*str_p != '\0' && isspace((unsigned char) *str_p)) { // isspace expects an int argument, so we change *str_p to be a valid argument type unsigned char
            str_p++; // continue to next character
            
        }

        if (*str_p == '\0') break;  // if we reach the end of the cmd_line, break the loop

        char *token_start;
        int token_len = 0;
        char *token;

        // Handling " double quote strings     ", once seen, the contents inside should not be altered until ending " 
        if (*str_p == '"') {
            str_p++; // skip opening quote
            token_start = str_p; // String will start after open quote

            while (*str_p != '\0' && *str_p != '"') { // until it doesn't equal last double quote
                str_p++; // stops once we are on the last double quote
                token_len++; // increment the length
            }

            // str_p is currently at the closing double quote
            // Allocating memory for the token, based on token_len, including the null terminator '\0'
            token = (char *)malloc(token_len + 1); // malloc will return char * after allocating memory for token (instead of regualr void *)
            if (token == NULL) return ERR_MEMORY;
            // Copy from the cmd_line token_start -> token_len
            strncpy(token, token_start, token_len); 
            token[token_len] = '\0';

            // Skip the closing quote
            if (*str_p == '"') {
                str_p++;
            }
            
        } else { // Handle unquoted cmd_line parsing, count the characters until you hit a space
            token_start = str_p;
            // Skip chars when they are in valid range, and they are not a space
            while (*str_p != '\0' && !isspace((unsigned char) *str_p)) {
                str_p++;
                token_len++;
            }

            // Allocate space for the token, and null terminator
            token = (char *)malloc(token_len + 1);
            if (token == NULL) return ERR_MEMORY;
            // Copy over to token pointer, from token_start based on token_len
            strncpy(token, token_start, token_len);
            token[token_len] = '\0'; // Null terminate
        }

        // Save the token into argv
        if (cmd_buff->argc < CMD_ARGV_MAX) { // Makes sure fixed size array capacity is not exceeded by count < max
            cmd_buff->argv[cmd_buff->argc] = token; 
            cmd_buff->argc++; // increment count
        } else { // Too many arguments in argv already
            free(token); 
            return ERR_TOO_MANY_COMMANDS;
        }      
    }

    // Null terminating the argv array expect for the execvp() usage
    if (cmd_buff->argc < CMD_ARGV_MAX) {
        cmd_buff->argv[cmd_buff->argc] = NULL;
    }

    return OK;
}



int exec_local_cmd_loop()
{
    char *cmd_buff;
    int rc = 0;
    cmd_buff_t cmd;

    // Allocate memory for the command buffer.
    if (alloc_cmd_buff(&cmd) != OK) {  // Make sure we pass a pointer to cmd, not a char **
        fprintf(stderr, "Error: Memory Allocation Failed for command buffer\n");
        return ERR_MEMORY;
    }

    // TODO IMPLEMENT MAIN LOOP
    while(1){
        printf("%s", SH_PROMPT);
        // char *fgets(char *str, int size, FILE *stream);
        // cmd_buff = pointer to buff where input is stored
        // max number of characters to read (max input size limit)
        // input source is standard input from the user
        // The command is stored in cmd._cmd_buffer (which was allocated by alloc_cmd_buff).
        if (fgets(cmd._cmd_buffer, ARG_MAX, stdin) == NULL){ // readings input from stdin until \n is seen or max size is reached
            printf("\n");
            break;
        }
        
        //remove the trailing \n from cmd_buff
        cmd._cmd_buffer[strcspn(cmd._cmd_buffer,"\n")] = '\0';

        // Check if the command is empty
        if (strlen(cmd._cmd_buffer) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue; // skip to next input prompt request;
        }

    
        // Make sure command parsing error is captured
        int build_result = build_cmd_buff(cmd._cmd_buffer, &cmd);
        if (build_result != OK) {
            if (build_result == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            } else {
                fprintf(stderr, "Error parsing command.\n");
            }
            continue;
        }
        
        // // Temp Debug print: iterate over the parsed tokens:
        // printf("Parsed tokens (argc = %d):\n", cmd.argc);
        // for (int i = 0; i < cmd.argc; i++) {
        //     printf("  argv[%d] = \"%s\"\n", i, cmd.argv[i]);
        // }

        // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
        // the cd command should chdir to the provided directory; if no directory is provided, do nothing
        // Check if command is exit command
        if (strcmp(cmd._cmd_buffer, EXIT_CMD) == 0) {
            break; // exits the loop when exit is seen
        }

        // Check if command is dragon 
        if (strcmp(cmd._cmd_buffer, "dragon") == 0) {
            print_dragon();
            continue;
        }

        // Include something for cd as well...
        // Check if the command is a built-in command
        if (strcmp(cmd.argv[0], "cd") == 0) {
            // If no directory is provided, do nothing
            if (cmd.argc < 2) {
                // Optionally print a message indicating no directory was provided
                continue;
            }
            // Change directory to the provided argument
            if (chdir(cmd.argv[1]) != 0) {
                perror("cd failed");
            }
            // Skip further processing for this command
            continue;
        }

        // TODO IMPLEMENT if not built-in command, fork/exec as an external command
        // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"

        // We know the command is not built in since, the above if else blocks were not triggered.
        pid_t pid = fork(); // The result from the fork statement

        if (pid < 0) { // If we get an error (ex -1)
            perror("fork");
            return ERR_EXEC_CMD;
            continue; // Prompts the user again if fork fails
        } else if (pid == 0) { // We are currently in the child fork process
            if (execvp(cmd.argv[0], cmd.argv) == -1) { // while trying to execute the command with arguments an error occurs, it returns -1
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else { // Currently in the parent/main process
            int status;
            waitpid(pid, &status, 0); // waits for the child to finish 

        }
    

    }

    // Create cmd struct
    free_cmd_buff(&cmd);

    return OK;
}

