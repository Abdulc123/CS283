#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
int  setup_buff(char *, char *, int);
int find_word_start(char *buff, const char *old_word);



//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
void print_buff(char *buff, int len, char, int status);
int count_words(char *buff, int len, int str_len);
void reverse_string(char *buff, int len, int str_len);
void word_print(char *buff, int len, int str_len);
int replace_word(char *buff, int len, int str_len, char *old_word, char *new_word);


int setup_buff(char *buff, char *user_str, int buffer_size){
    //TODO: #4:  Implement the setup buff as per the directions
    // (Internal buffer buff, user supplied string, and size of buff)
    
    char *buff_ptr = buff;
    char *str_ptr = user_str; // Create pointer for user_str
    int len = 0;
    char *last_non_space_char = NULL;

    // Skip the leadings spaces
    while (*str_ptr == ' ' || *str_ptr == '\t') {
        str_ptr++;
    }

    // 1. Check if user string exceeds the buffer size and copy over characters to buffer

    while (*str_ptr != '\0') {
        // Skipping the white space characters
        if (*str_ptr == ' ' || *str_ptr == '\t') {
            // If consecutive white spaces == 0, add the space to buffer
            if (buff_ptr > buff && *(buff_ptr - 1) != ' ') {
                *buff_ptr++ = ' '; // Copy single space character over to buffer 
                len++;  
            } 

        } else { 
            // Copy over the non white space characters
            *buff_ptr = *str_ptr; // Copies over character from string to the buffer since
            buff_ptr++;
            len++;
            last_non_space_char = buff_ptr; // The last time a non space was seen was at buff_ptr
        }

        // If length is ever longer than the buffer size
        if (len > buffer_size) {
            return -1; // Provided User String is too long
        }

        str_ptr++; // Iterate to the next char in the user string
    }

    len = last_non_space_char - buff;

    // The string length, before the ... are added
    int str_len = len;
    //printf("Last non space character seen (%c) at index %d\n", *last_non_space_char, last_non_space_char - buff);
   
    // Fill in the remaining spots in the buffer with dots
    *last_non_space_char = '.';
    while (len < buffer_size) {
        *buff_ptr = '.';
        buff_ptr++;
        len++;
    }   
    // Null terminate the buffer
    *buff_ptr = '\0';

    return str_len; // Return length of the string (not including the extra dots)
    
}


void print_buff(char *buff, int len, char operation, int status){
    if (operation == 'x' && status == -1) {
        printf("Buffer:  [");
        for (int i=0; i<len; i++){
            putchar(*(buff+i));
        }
        printf("]");
        putchar('\n');
    } else if (operation == 'x') {
        printf("Not Implemented");
    } else {
        printf("Buffer:  [");
        for (int i=0; i<len; i++){
            putchar(*(buff+i));
        }
        printf("]");
        putchar('\n');
    }
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    //YOU MUST IMPLEMENT

    bool word_start = false;
    int word_count = 0;
    char *str_ptr = buff; // Pointer to traverse through buffer

    for (int i = 0; i < str_len; i++) {
        if (!word_start) {
            // If we were not on a word, and we dont see a white space, thats the start of a word
            if (*str_ptr != ' ') {
                word_count++;
                word_start = true;
            } 
        } else { // We were at a word, and encounter a space, we are no longer on a word
            if (*str_ptr == ' ') {
                word_start = false;
            }
        }
        str_ptr++;
    }

    return word_count;
}

void reverse_string(char *buff, int len, int str_len) {
    // Reverses the string in place and 
    int end_index = str_len - 1;
    int start_index = 0;
    char tmp_char;
    char *str_ptr = buff;

    while (start_index < end_index) {
        tmp_char = *(str_ptr + end_index);                  // Capture the end letter in temp
        *(str_ptr + end_index) = *(str_ptr + start_index);  // swap the end with the start
        *(str_ptr + start_index) = tmp_char;                // swap the start with the temp (which holds value of end)

        // Iterate the indexes for the loop
        end_index--;
        start_index++;
    }

}

void word_print(char *buff, int len, int str_len) {

    int last_char_index = str_len - 1;
    int word_count = 0;
    int word_length = 0;
    bool word_start = false;
    char *str_ptr = buff;

    for (int i = 0; i < str_len + 1; i++) { 
        
        char current_char = *(str_ptr + i);
        // We were not on a word
        if (!word_start) {

            // If we see a non white space character and we are not at a word
            if (current_char != ' ') {
                // Means start of a new word, so increment word_count and the words length by 1
                word_count++;
                word_length++;
                word_start = true;
                printf("%d. ", word_count); // Prints the word we are on .
                printf("%c", current_char); // Prints each letter of the word one by one
            }
        } else {
            // We were previosly in a word
            if (current_char == ' ' || current_char == '.') {
                // We have reached the end of the word, so print out its length, and change word_start
                printf("(%d)\n", word_length);
                word_start = false;
                word_length = 0;
            } else {
                // We were previously in a word, and continue finding another letter
                printf("%c", current_char);
                word_length++;
            }
        }
    }
    printf("\nNumber of words returned: %d\n", word_count);
}


// Helper function for replace word
int find_word_start(char *buff, const char *old_word) {
    char *str_ptr = buff; // Pointer to iterate over the whole string buff
    char *word_ptr; // Pointer to iterate over the old_word
    int word_location = 0; 

    // Iterate through the string
    while (*str_ptr != '\0') {
        
        word_ptr = old_word; // Points to the start of the old word
        char *temp_ptr = str_ptr; // Temporary pointer to remember the start of the current section we are in based on buff

        // Compare the current section of buff with the old word (look if temp_ptr letter match the old_ptr letters)
        while (*word_ptr  != '\0' && *temp_ptr == *word_ptr) {
            word_ptr++;
            temp_ptr++;
        }

        // Check to see if the old_word ptr was able to reach the end, = '\0', this means the old word has been found
        if (*word_ptr == '\0') {
            //printf("Word Location is at Index: %d\n", word_location);
            return word_location; // Return str_ptr since that is where the section started for the word and where temp_ptr was able to find the word
        }

        // Iterate to the next character in buff
        str_ptr++;
        word_location++;
    }
    
    return -1; // When old word is not found
}

int replace_word(char *buff, int len, int str_len, char *old_word, char *new_word) {
    char *str_ptr = buff;
    int word_start = find_word_start(buff, old_word);
    char *temp_ptr;

    // If finding the word was not succesfull, print out message and return nothing
    if (word_start == -1) {
        return -1;
        exit(1);
    }

    // printf("Word is found at Index: %d [%c]\n", word_start, *(str_ptr + word_start));

    // Get the lengths of the old and new word (without using strlen)
    int old_word_len = 0, new_word_len = 0;

    char *old_ptr = old_word;
    char *new_ptr = new_word;
    
    while (*old_ptr != '\0') {
        // If the pointers are valid aka != '\0'

        if (*old_ptr != '\0') {
            old_word_len++;
            old_ptr++;
        }
    }

    while (*new_ptr != '\0') {
        // If the pointers are valid aka != '\0'
        if (*new_ptr != '\0') {
            new_word_len++;
            new_ptr++;
        }
    }

    // printf("Got the old and new word lengths, %d, %d\n", old_word_len, new_word_len);

    // Determine whether it is viable to swap them without creating an error, if size after replace is >= 50, raise an error
    if (str_len - old_word_len + new_word_len> len) {
        printf("Error: Buffer overflow would occur with replacement.\n");
        return;
    }

    char *read_new_ptr = new_word;
    int suffix_len = str_len - (word_start + old_word_len);       // 38 - 33 = 5
    //buff points to the beginning of the string.
    char *suffix_start = buff + word_start + old_word_len;

    // Create a suffix copy so it can be added to the end of the buffer once new word is added in
    char *suffix_copy = malloc(suffix_len + 5); //Allocates the memory

    if (suffix_copy == NULL) {
        perror("Failed to allocate memory");
        exit(1);
    }

    // Copy from buff right side of word(aka suffix) to suffix_copy;
    temp_ptr = suffix_start;
    char* temp_copy = suffix_copy; //temp_copy pointer is used to avoid using original suffix copy pointer
    while(*temp_ptr) { 
        *temp_copy++ = *temp_ptr++;
    }

    char *write_ptr = buff + word_start; // point to the start of the new word
    temp_ptr = new_word; // point to the new word start
    
    // Overwrite the old word with the new word in the buffer
    while (*temp_ptr) {
        // When you are writing to the buff using str_ptr, shift the elments to the right by 1
        *write_ptr++ = *temp_ptr++;
    }

    // Add the suffix back manually to the list;
    int j = 0;
    while(*(suffix_copy + j) != '\0') {
        *write_ptr++ = *(suffix_copy + j);
        j++;
    }

    *write_ptr = '\0';

    free(suffix_copy);

    // printf("Modified Buffer: %s\n", buff);
    return 1;
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    
    //      The following code is safe because first it checks if there are at least 3 arguments
    //      provided, the program name, the operation, and the string. If this is valid then it will 
    //      try to do the || or statement and dereference argv to access the operation in it 
    //      This ensures that the operation starts with '-', else it will be exited

    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag
    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING

    // This if statement ensures that the required number of arguments 
    // has been provided by the user. Specifically:
    //  If fewer than three arguments (including the program name) are supplied,
    //  it indicates improper usage of the program.
    //  The `usage()` function is called to display correct usage information
    //  to the user, and the program exits with an error code (`exit(1)`).

    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = (char *) malloc(BUFFER_SZ * sizeof(char));
    if (buff == NULL) {
        printf("Error: Memory allocation failed.\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, size = %d\n", user_str_len);
        exit(2);
    } else if (user_str_len == -1) {
        printf("This is a string that does not work as it is too long");
    }
    
    // printf("The size of the string is %d\n", user_str_len);
    int status = 0;
    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        
        case 'r':
            // Reverses the string
            reverse_string(buff, BUFFER_SZ, user_str_len);
            break;

        case 'w':
            // Prints out the word, and word length of each word
            /*
            1. C (1)
            2. programming (11)
            3. is (2)
            4. fun (3)
            */
            printf("Word Print\n----------\n");
            word_print(buff, BUFFER_SZ, user_str_len);
            break;
        
        case 'x':
            // stringfun -x "Replacing words in strings is not fun!" not super
            // Modified String: Replacing words in strings is super fun!
            if (argc < 5) {
                printf("Error: Missing arguments for replace function.\n");
                exit(1);
            }

            char *old_word = argv[3];
            char *new_word = argv[4];


            // Check if old_word and new_word are not null
            if (old_word == NULL || new_word == NULL) {
                printf("Error: Invalid input for words.\n");
                exit(1);
            }

            
            status = replace_word(buff, BUFFER_SZ, user_str_len, old_word, new_word);

            break;

        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ, opt, status);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE
/*
It's a good practice to pass both the pointer and the length of the buffer because it makes the function more flexible.
Even though we know the buffer is 50 bytes in main(), passing the length allows the function to work with buffers of any size. 
This also helps prevent buffer overflows by making sure the function doesn't write past the buffer's capacity. Overall, it ensures 
the function can handle different scenarios safely and clearly.
*/