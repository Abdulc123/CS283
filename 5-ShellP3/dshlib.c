#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
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

char *trim(char *str) {
    while (isspace((unsigned char)*str)) str++;  // Trim leading spaces
    if (*str == '\0') return str;  // Return immediately if empty

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) *end-- = '\0';  // Trim trailing spaces

    return str;
}

 int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (!cmd_line || !clist) {
        return WARN_NO_CMDS;
    }

    memset(clist, 0, sizeof(command_list_t));

    char *token = strtok(cmd_line, PIPE_STRING);
    int cmd_count = 0;

    while (token != NULL) {
        if (cmd_count >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }

        char *command = trim(token);
        if (strlen(command) == 0) {
            token = strtok(NULL, PIPE_STRING);
            continue;
        }

        command_t *cmd = &clist->commands[cmd_count];

        // Copy command into buffer for safe tokenization
        char token_copy[SH_CMD_MAX];
        strncpy(token_copy, command, SH_CMD_MAX - 1);
        token_copy[SH_CMD_MAX - 1] = '\0';

        char *saveptr;
        char *arg_token = strtok_r(token_copy, " ", &saveptr);
        if (!arg_token) {
            token = strtok(NULL, PIPE_STRING);
            continue;
        }

        // Set executable name
        if (strlen(arg_token) >= EXE_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        strcpy(cmd->exe, arg_token);

        // Parse arguments into argv array
        cmd->argc = 0;
        while ((arg_token = strtok_r(NULL, " ", &saveptr)) != NULL) {
            if (cmd->argc >= N_ARG_MAX) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            cmd->argv[cmd->argc] = strdup(arg_token);  // Allocate memory and copy argument
            cmd->argc++;
        }

        cmd->argv[cmd->argc] = NULL;  // Required by execvp()
        cmd_count++;
        token = strtok(NULL, PIPE_STRING);
    }

    clist->num = cmd_count;
    return (cmd_count == 0) ? WARN_NO_CMDS : OK;
}





int exec_local_cmd_loop() {
    char cmd_buffer[SH_CMD_MAX];
    command_list_t clist;
    
    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buffer, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0'; // Remove newline
        
        if (strlen(cmd_buffer) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        int result = build_cmd_list(cmd_buffer, &clist);
        if (result != OK) {
            if (result == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            } else {
                fprintf(stderr, "Error parsing command.\n");
            }
            continue;
        }
        
        int num_cmds = clist.num;
        int pipes[num_cmds - 1][2];
        pid_t pids[num_cmds];

        // Create pipes
        for (int i = 0; i < num_cmds - 1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                return ERR_EXEC_CMD;
            }
        }

        // Fork and execute each command
        for (int i = 0; i < num_cmds; i++) {
            pids[i] = fork();
            if (pids[i] < 0) {
                perror("fork");
                return ERR_EXEC_CMD;
            }
            
            if (pids[i] == 0) { // Child process
                if (i > 0) { // If not first command, read from previous pipe
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }
                if (i < num_cmds - 1) { // If not last command, write to next pipe
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
                
                // Close all pipes in child process
                for (int j = 0; j < num_cmds - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                
                // Execute command
                command_t *cmd = &clist.commands[i];
                execlp(cmd->exe, cmd->exe, cmd->args, NULL);
                perror("exec");
                exit(EXIT_FAILURE);
            }
        }
        
        // Close pipes in parent process
        for (int i = 0; i < num_cmds - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        
        // Wait for all child processes
        for (int i = 0; i < num_cmds; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }
    return OK;
}
