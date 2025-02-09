#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
char *trim(char *str);  // Function prototype

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (!cmd_line || !clist) {
        return WARN_NO_CMDS;
    }

    memset(clist, 0, sizeof(command_list_t));

    //printf("DEBUG: cmd_line = \"%s\"\n", cmd_line);

    // Outer tokenization: split on pipe characters
    char *token = strtok(cmd_line, PIPE_STRING);
    int cmd_count = 0;

    while (token != NULL) {
        if (cmd_count >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }
        //printf("DEBUG: Found token: \"%s\"\n", token);

        // Trim spaces from the token
        char *command = trim(token);
        if (strlen(command) == 0) {
            token = strtok(NULL, PIPE_STRING);
            continue;
        }

        command_t *cmd = &clist->commands[cmd_count];

        // Make a copy of the command token for inner tokenization
        char token_copy[SH_CMD_MAX];
        strncpy(token_copy, command, SH_CMD_MAX - 1);
        token_copy[SH_CMD_MAX - 1] = '\0';

        // Inner tokenization: use strtok_r to split on spaces
        char *saveptr;
        char *command_parts = strtok_r(token_copy, " ", &saveptr);
        if (!command_parts) {
            token = strtok(NULL, PIPE_STRING);
            continue;
        }

        if (strlen(command_parts) >= EXE_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        strcpy(cmd->exe, command_parts);

        char args_buffer[ARG_MAX] = "";
        while ((command_parts = strtok_r(NULL, " ", &saveptr)) != NULL) {
            if (strlen(args_buffer) + strlen(command_parts) + 2 >= ARG_MAX) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strcat(args_buffer, command_parts);
            strcat(args_buffer, " ");
        }

        if (strlen(args_buffer) > 0) {
            args_buffer[strlen(args_buffer) - 1] = '\0'; // Remove trailing space
            strncpy(cmd->args, args_buffer, ARG_MAX - 1);
            cmd->args[ARG_MAX - 1] = '\0';
        }

        cmd_count++;
        token = strtok(NULL, PIPE_STRING);
    }

    clist->num = cmd_count;
    if (cmd_count == 0) { // if no commands were found
        return WARN_NO_CMDS;
    }

    return OK;
}

// Helper trim function
char *trim(char *str) {
     while (isspace((unsigned char)*str)) str++;  // Trim leading spaces
    if (*str == '\0') return str;  // Return immediately if empty

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) *end-- = '\0';  // Trim trailing spaces


    return str;

}