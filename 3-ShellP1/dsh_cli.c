#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dshlib.h"


/*
 * Implement your main function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.  Since we want fgets to also handle
 * end of file so we can run this headless for testing we need to check
 * the return code of fgets.  I have provided an example below of how
 * to do this assuming you are storing user input inside of the cmd_buff
 * variable.
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
 *
 *   Expected output:
 *
 *      CMD_OK_HEADER      if the command parses properly. You will
 *                         follow this by the command details
 *
 *      CMD_WARN_NO_CMD    if the user entered a blank command
 *      CMD_ERR_PIPE_LIMIT if the user entered too many commands using
 *                         the pipe feature, e.g., cmd1 | cmd2 | ... |
 *
 *  See the provided test cases for output expectations.
 */

void print_dragon() {
    for (size_t i = 0; i < DRAGON_ROWS; i++) {
        for (size_t j = 0; j < DRAGON_COLS; j++) {
            DragonCompressed pair = dragon_data[i][j];
            if (pair.count == 0) break; // Stop early if padding (0,0) is found
            for (int k = 0; k < pair.count; k++) {
                putchar(pair.value);
            }
        }
        putchar('\n');
    }
}


int main()
{
    char *cmd_buff;
    int rc = 0;
    command_list_t clist;

    // Allocate memory for cmd_buff and make sure it went well
    cmd_buff = malloc(ARG_MAX);
    if (cmd_buff == NULL) {
        perror("Error: Unable to allocate memory for cmd_buff: ");
        return 1;
    }

    

    while(1){
         printf("%s", SH_PROMPT);
         // fgets stops reading when a \0 or newline character is seen, and appends the \0 at the end
         // basically a line by lien processor
         if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
            printf("\n");
            break;
         }
         //remove the trailing \n from cmd_buff
         cmd_buff[strcspn(cmd_buff,"\n")] = '\0'; // returns length of string until the reject character (\0) is seen
 
         //IMPLEMENT THE REST OF THE REQUIREMENTS

         // Exit when seen
         if (strcmp(cmd_buff, EXIT_CMD) == 0) { // 0 means they are equal
            exit(OK);
         }

         // Special Easter Egg for "dragon"
        if (strcmp(cmd_buff, "dragon") == 0) {
            // printf("[DRAGON for extra credit would print here]\n");
            print_dragon();
            continue;
        }

        // return code
        rc = build_cmd_list(cmd_buff, &clist); 

        if (rc == OK) {
            printf(CMD_OK_HEADER, clist.num);
            for (int i = 0; i < clist.num; i++) {
                if (strlen(clist.commands[i].args) > 0) {
                    printf("<%d> %s [%s]\n", i + 1, clist.commands[i].exe, clist.commands[i].args); // multiple arguments
                } else {
                    printf("<%d> %s\n", i + 1, clist.commands[i].exe); // no arguments
                }
            }
        } else if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        } else {
            printf("Unknown error: %d\n", rc);       
        }
    }

    free(cmd_buff);  
    return OK;

}