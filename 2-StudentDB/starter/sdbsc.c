#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      //c library for system call file routines
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

//database include files
#include "db.h"
#include "sdbsc.h"

/*
 *  open_db
 *      dbFile:  name of the database file
 *      should_truncate:  indicates if opening the file also empties it
 * 
 *  returns:  File descriptor on success, or ERR_DB_FILE on failure
 * 
 *  console:  Does not produce any console I/O on success
 *            M_ERR_DB_OPEN on error
 *             
 */
int open_db(char *dbFile, bool should_truncate){
    // Set permissions: rw-rw----
    // see sys/stat.h for constants
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

    //open the file if it exists for Read and Write, 
    //create it if it does not exist
    int    flags = O_RDWR | O_CREAT;

    if (should_truncate) 
        flags += O_TRUNC;

    // Now open file
    int fd = open(dbFile, flags, mode);

    if (fd == -1) {
        // Handle the error
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }

    return fd;
}

/*
 *  get_student
 *      fd:  linux file descriptor
 *      id:  the student id we are looking forname of the
 *      *s:  a pointer where the located (if found) student data will be
 *           copied
 * 
 *  returns:  NO_ERROR       student located and copied into *s
 *            ERR_DB_FILE    database file I/O issue
 *            SRCH_NOT_FOUND student was not located in the database
 * 
 *  console:  Does not produce any console I/O used by other functions
 */
int get_student(int fd, int id, student_t *s){
    
    // Calculate the bys offset in the file
    // Mulitiply students ID with STUDENT_RECORD_SIZE(64), provides the byte position where students record should start in the file
    // Stored in off_t which can store 64 bit integers, while int can do 32 bit
    off_t offset =(off_t)id * STUDENT_RECORD_SIZE;

    //lseek() : Seeks to a position in a file. The next read() or write() will happen based on 
    // where you instructed lseek() to go within the file.
    if (lseek(fd, offset, SEEK_SET) < 0) {
        return ERR_DB_FILE;
    }

    // Read one student sized chunk to see if it is empty or contains a student
    student_t buffer;
    ssize_t bytes_read = read(fd, &buffer, STUDENT_RECORD_SIZE); // transfers 64 bytes from file into memory address of buffer

    if (bytes_read < 0) {
        // read error since bytes_read should normally not be negative
        return ERR_DB_FILE;
    } else if (bytes_read == 0) {
        // record does not exists, meaning not found since its empty at that  location
        return SRCH_NOT_FOUND;
    } else if (bytes_read < STUDENT_RECORD_SIZE) {
        // record is partially read, is also an error (could mean it was not added correctly)
        return ERR_DB_FILE;
    }

    // Compare read record to an EMPTY_STUDENT_RECORD (which is all 0's)
    // memcmp (*string1, *string2, number of bytes to compare)
    // If it returns 0, it means all the memory blocks are equal
    if (memcmp (&buffer, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) == 0) {
        // all bytes are == 0, meaning no student is stored in that memory location
        return SRCH_NOT_FOUND;
    }

    // Copy into the storage for the student if its found, the pointer *s
    memcpy(s, &buffer, STUDENT_RECORD_SIZE);
    return NO_ERROR;



}

/*
 *  add_student
 *      fd:     linux file descriptor
 *      id:     student id (range is defined in db.h )
 *      fname:  student first name
 *      lname:  student last name
 *      gpa:    GPA as an integer (range defined in db.h)
 * 
 *  Adds a new student to the database.  After calculating the index for the
 *  student, check if there is another student already at that location.  A good
 *  way is to use something like memcmp() to ensure that the location for this
 *  student contains all zero byes indicating the space is empty. 
 * 
 *  returns:  NO_ERROR       student added to database
 *            ERR_DB_FILE    database file I/O issue
 *            ERR_DB_OP      database operation logically failed (aka student
 *                           already exists)  
 * 
 * 
 *  console:  M_STD_ADDED       on success
 *            M_ERR_DB_ADD_DUP  student already exists
 *            M_ERR_DB_READ     error reading or seeking the database file
 *            M_ERR_DB_WRITE    error writing to db file (adding student)
 *            
 */
int add_student(int fd, int id, char *fname, char *lname, int gpa){
    
    //Check if the student record already exists
    student_t temp_student;
    int record = get_student(fd, id, &temp_student); //if it does temp_student will be filled with the students contents

    if (record == NO_ERROR) {
        // A record was succesfully found at the ID
        printf(M_ERR_DB_ADD_DUP, id); // "Cant add student with ID=%d, already exists in db.\n"
        return ERR_DB_OP;
        
    } else if (record == ERR_DB_FILE) {
        // get_student incountered a I/O error
        printf(M_ERR_DB_READ); // "Error reading DB file, exiting!\n"
        return ERR_DB_FILE;
    }

    // If no student records are found and no errors occur -> Continue with creating student and adding to DB
    student_t new_student;
    memset(&new_student, 0, STUDENT_RECORD_SIZE); // Zero initializes the new student 
    new_student.id = id; // Set the ID

    // Copy over the first and last names
    strncpy(new_student.fname, fname, sizeof(new_student.fname) - 1);
    strncpy(new_student.lname, lname, sizeof(new_student.lname) - 1);

    new_student.gpa = gpa; // Set the GPA

    // Calculate offset (position) to place id within the DB
    off_t offset = (off_t)id *STUDENT_RECORD_SIZE;

    // Seek to that position within the DB file
    if (lseek(fd, offset, SEEK_SET) < 0) { // lseek returns -1 if an error occurs
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    // Write the new student record at position found be lseek
    ssize_t bytes_written = write(fd, &new_student, STUDENT_RECORD_SIZE); // write 64 bits from new student entry pointer to file DB
    if (bytes_written < 0 || bytes_written < STUDENT_RECORD_SIZE) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    // Return success Message
    printf(M_STD_ADDED, id); // "Student %d added to database.\n"
    return NO_ERROR;
}

/*
 *  del_student
 *      fd:     linux file descriptor
 *      id:     student id to be deleted
 * 
 *  Removes a student to the database.  Use the get_student() function to
 *  locate the student to be deleted. If there is a student at that location
 *  write an empty student record - see EMPTY_STUDENT_RECORD from db.h at 
 *  that location. 
 * 
 *  returns:  NO_ERROR       student deleted from database
 *            ERR_DB_FILE    database file I/O issue
 *            ERR_DB_OP      database operation logically failed (aka student
 *                           not in database)  
 * 
 * 
 *  console:  M_STD_DEL_MSG      on success
 *            M_STD_NOT_FND_MSG  student not in database, cant be deleted
 *            M_ERR_DB_READ      error reading or seeking the database file
 *            M_ERR_DB_WRITE     error writing to db file (adding student)
 *            
 */
int del_student(int fd, int id){
    
    // Check if student exists using get_student()
    student_t temp_student;
    int record = get_student(fd, id, &temp_student); // if found student will be written at temp_student

    if (record == SRCH_NOT_FOUND) {
        // When student does not exists, (unable to delete it if not there)
        printf(M_STD_NOT_FND_MSG, id);
        return ERR_DB_OP;
    } else if (record == ERR_DB_FILE) {
        // When get student returns a I/O error
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    // If found write with EMPTY_STUDENT_RECORD from db.h (Means record == NO_ERROR)

    // Calculate the bys offset in the file
    // Mulitiply students ID with STUDENT_RECORD_SIZE(64), provides the byte position where students record should start in the file
    // Stored in off_t which can store 64 bit integers, while int can do 32 bit
    off_t offset = (off_t)id * STUDENT_RECORD_SIZE;

    //lseek() : Seeks to a position in a file. The next read() or write() will happen based on 
    // where you instructed lseek() to go within the file.
    // SEEK_SET seeks (offset bits) from the start of the file, if 100, it would be 100 bytes from start of the file
    // SEEK_CUR seekf from current file position
    if (lseek(fd, offset, SEEK_SET) < 0) { // if it reads 0 bytes that is an error
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    // Write zeros to the slot so it is an empty student record
    ssize_t bytes_written = write(fd, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE); // write 64 bits from EMPTY STUDENT RECORD pointer to file DB (will fill with all 0's)
    if (bytes_written < 0 || bytes_written < STUDENT_RECORD_SIZE) { // If nothing is written over || or not enough if its < STUDENT_RECORD_SIZE
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    // Everything went succesfully
    printf(M_STD_DEL_MSG, id);
    return NO_ERROR;
}

/*
 *  count_db_records
 *      fd:     linux file descriptor
 * 
 *  Counts the number of records in the database.  Start by reading the 
 *  database at the beginning, and continue reading individual records
 *  until you it EOF.  EOF is when the read() syscall returns 0. Check
 *  if a slot is empty or previously deleted by investigating if all of 
 *  the bytes in the record read are zeros - I would suggest using memory
 *  compare memcmp() for this. Create a counter variable and initialize it
 *  to zero, every time a non-zero record is read increment the counter.  
 * 
 *  returns:  <number>       returns the number of records in db on success
 *            ERR_DB_FILE    database file I/O issue
 *            ERR_DB_OP      database operation logically failed (aka student
 *                           not in database)  
 * 
 * 
 *  console:  M_DB_RECORD_CNT  on success, to report the number of students in db
 *            M_DB_EMPTY       on success if the record count in db is zero
 *            M_ERR_DB_READ    error reading or seeking the database file
 *            M_ERR_DB_WRITE   error writing to db file (adding student)
 *            
 */
int count_db_records(int fd){
    
    // Seek to the start of the file:
    // If lseek is not used it will read from where other functions left off. 
    // lseek returns -1 if there is an error
    if (lseek(fd, 0, SEEK_SET) < 0) {
        // When it fails, I/O error
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    // Create buffer to read each 64 byte chunk
    student_t buffer;
    int count = 0; // initialize record count to 0

    // Loop through database file until EOF occurs (when read() syscall returns 0)
    while (true) {
        // Data is read from fd into buffer, and the student record size is the maximum number of bytes being read into the buffer each loop iteration
        ssize_t bytes_read = read(fd, &buffer, STUDENT_RECORD_SIZE);

        if (bytes_read < 0) {
            // Something went wrong when reading, didn't read anything || 
            printf(M_ERR_DB_READ);
            return ERR_DB_FILE;
        } else if (bytes_read == 0) {// Encountered EOF return of 0 from read (End of DB is reached)
            break;
        } else if (bytes_read < STUDENT_RECORD_SIZE) {
            // A Partial Read error, should have read 64 bytes
            printf(M_ERR_DB_READ);
            return ERR_DB_FILE;
        }

        // Check if the record read is empty
        // memcmp returns 0 if both strings are the same
        // else they are different
        if (memcmp(&buffer, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0) {
            // non-zero record has been ready since they are not the same
            count++;
        }
    }

    // Output the results based on count
    if (count == 0) {
        printf(M_DB_EMPTY); //“Database contains no student records.”
    } else { // Succesfully found records
        printf(M_DB_RECORD_CNT, count); // “Database contains X student record(s).”
    }
    return count;
}

/*
 *  print_db
 *      fd:     linux file descriptor
 * 
 *  Prints all records in the database.  Start by reading the 
 *  database at the beginning, and continue reading individual records
 *  until you it EOF.  EOF is when the read() syscall returns 0. Check
 *  if a slot is empty or previously deleted by investigating if all of 
 *  the bytes in the record read are zeros - I would suggest using memory
 *  compare memcmp() for this. Be careful as the database might be empty.
 *  on the first real row encountered print the header for the required output:
 * 
 *     printf(STUDENT_PRINT_HDR_STRING, "ID", 
 *                  "FIRST NAME", "LAST_NAME", "GPA");
 * 
 *  then for each valid record encountered print the required output: 
 * 
 *     printf(STUDENT_PRINT_FMT_STRING, student.id, student.fname, 
 *                    student.lname, calculated_gpa_from_student);
 * 
 *  The code above assumes you are reading student records into a local
 *  variable named student that is of type student_t. Also dont forget that
 *  the GPA in the student structure is an int, to convert it into a real
 *  gpa divide by 100.0 and store in a float variable.
 * 
 *  returns:  NO_ERROR       on success
 *            ERR_DB_FILE    database file I/O issue
 * 
 * 
 *  console:  <see above>      on success, print table or database empty
 *            M_ERR_DB_READ    error reading or seeking the database file
 *            
 */
int print_db(int fd){
    
    // lseek to the start of the file (Without this you may start reading where other programs left off)
    // lseek returns -1 when there is an error
    if (lseek(fd, 0, SEEK_SET) < 0) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    // Create a temp_student of student_t type to store each 64 byte record read
    student_t temp_student;
    bool printed_header = false;
    int valid_records = 0;
    
    // Loop through DB file until EOF occurs -> read returns 0
    while (true) {
        ssize_t bytes_read = read(fd, &temp_student, STUDENT_RECORD_SIZE);

        if (bytes_read < 0) {
            // Something went wrong and nothing weas read
            printf(M_ERR_DB_READ);
            return ERR_DB_FILE;
        } else if (bytes_read == 0) {
            // EOF Eror occured so exit the loop
            break;
        } else if (bytes_read < STUDENT_RECORD_SIZE) {
            // Partial read error
            printf(M_ERR_DB_READ);
            return ERR_DB_FILE;
        }

        // If record read is non empty, 
        if (memcmp(&temp_student, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0) {
            // 0 means they match, != 0 means they are different, so not empty
            valid_records++;

            // If printed header is false -> !false = true
            if (!printed_header) {
                printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST NAME", "LAST_NAME", "GPA");
                printed_header = true;
            }

            // Format the new gpa to int format
            float formatted_gpa = temp_student.gpa / 100.0f;
            // Else print it out as a format string
            printf(STUDENT_PRINT_FMT_STRING, temp_student.id, temp_student.fname, temp_student.lname, formatted_gpa);

        }
    }

    // If no valid records, that means DB is empty
    if (valid_records == 0) {
        printf(M_DB_EMPTY);
    }

    return NO_ERROR;
}


/*
 *  print_student
 *      *s:   a pointer to a student_t structure that should
 *            contain a valid student to be printed
 * 
 *  Start by ensuring that provided student pointer is valid.  To do this
 *  make sure it is not NULL and that s->id is not zero.  After ensuring 
 *  that the student is valid, print it the exact way that is described
 *  in the print_db() function by first printing the header then the
 *  student data:
 * 
 *     printf(STUDENT_PRINT_HDR_STRING, "ID", 
 *                  "FIRST NAME", "LAST_NAME", "GPA");
 * 
 *     printf(STUDENT_PRINT_FMT_STRING, s->id, s->fname, 
 *                    student.lname, calculated_gpa_from_s);
 * 
 *  Dont forget that  the GPA in the student structure is an int, to convert 
 *  it into a real gpa divide by 100.0 and store in a float variable. 
 * 
 *  returns:  nothing, this is a void function
 * 
 * 
 *  console:  <see above>      on success, print table or database empty
 *            M_ERR_STD_PRINT  if the function argument s is NULL or if
 *                             s->id is zero
 *            
 */
void print_student(student_t *s){
    
    // Make sure student pointer is Valid
    // Check if not Null and id != 0
    if (s == NULL || s->id == 0) {
        printf(M_ERR_STD_PRINT);
    }

    // Once Valid, print out the header than the student data
    printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST NAME", "LAST_NAME", "GPA");

    // Format the GPA;
    float formattedGPA = s->gpa / 100.0f;

    // Print out the student record
    printf(STUDENT_PRINT_FMT_STRING, s->id, s->fname, s->lname, formattedGPA);
}

/*
 *  NOTE IMPLEMENTING THIS FUNCTION IS EXTRA CREDIT
 *
 *  compress_db
 *      fd:     linux file descriptor
 * 
 *  This assignment takes advantage of the way Linux handles sparse files
 *  on disk. Thus if there is a large hole between student records, Linux
 *  will not use any physical storage.  However, when a database record is
 *  deleted storage is used to write a blank - see EMPTY_STUDENT_RECORD from
 *  db.h - record.  
 * 
 *  Since Linux provides no way to delete data in the middle of a file, and
 *  deleted records take up physical storage, this function will compress the
 *  database by rewriting a new database file that only includes valid student
 *  records. There are a number of ways to do this, but since this is extra credit
 *  you need to figure this out on your own. 
 * 
 *  At a high level create a temporary database file then copy all valid students from
 *  the active database (passed in via fd) to the temporary file. When this is done
 *  rename the temporary database file to the name of the real database file. See
 *  the constants in db.h for required file names:
 *
 *         #define DB_FILE     "student.db"        //name of database file
 *         #define TMP_DB_FILE ".tmp_student.db"   //for extra credit 
 * 
 *  Note that you are passed in the fd of the database file to be compressed, 
 *  it is very likely you will need to close it to overwrite it with the
 *  compressed version of the file.  To ensure the caller can work with the
 *  compressed file after you create it, it is a good design to return the fd
 *  of the new compressed file from this function
 * 
 *  returns:  <number>       returns the fd of the compressed database file
 *            ERR_DB_FILE    database file I/O issue
 * 
 * 
 *  console:  M_DB_COMPRESSED_OK  on success, the db was successfully compressed.
 *            M_ERR_DB_OPEN    error when opening/creating temporary database file.
 *                             this error should also be returned after you
 *                             compressed the database file and if you are unable
 *                             to open it to pass the fd back to the caller
 *            M_ERR_DB_CREATE  error creating the db file. For instance the
 *                             inability to copy the temporary file back as
 *                             the primary database file. 
 *            M_ERR_DB_READ    error reading or seeking the the db or tempdb file
 *            M_ERR_DB_WRITE   error writing to db or tempdb file (adding student)
 *            
 */
int compress_db(int fd){
    printf(M_NOT_IMPL);
    return fd;
}


/*
 *  validate_range
 *      id:  proposed student id
 *      gpa: proposed gpa
 * 
 *  This function validates that the id and gpa are in the allowable ranges
 *  as per the specifications.  It checks if the values are within the
 *  inclusive range using constents in db.h
 * 
 *  returns:    NO_ERROR       on success, both ID and GPA are in range
 *              EXIT_FAIL_ARGS if either ID or GPA is out of range
 * 
 *  console:  This function does not produce any output
 *            
 */
int validate_range(int id, int gpa){

    if ((id < MIN_STD_ID) || (id > MAX_STD_ID))
        return EXIT_FAIL_ARGS;

    if ((gpa < MIN_STD_GPA) || (gpa > MAX_STD_GPA))
        return EXIT_FAIL_ARGS;

    return NO_ERROR;
}

/*
 *  usage
 *      exename:  the name of the executable from argv[0]
 * 
 *  Prints this programs expected usage
 * 
 *  returns:    nothing, this is a void function
 * 
 *  console:  This function prints the usage information
 *            
 */
void usage(char *exename){
    printf("usage: %s -[h|a|c|d|f|p|z] options.  Where:\n", exename);
    printf("\t-h:  prints help\n");
    printf("\t-a id first_name last_name gpa(as 3 digit int):  adds a student\n");
    printf("\t-c:  counts the records in the database\n");
    printf("\t-d id:  deletes a student\n");
    printf("\t-f id:  finds and prints a student in the database\n");
    printf("\t-p:  prints all records in the student database\n");
    printf("\t-x:  compress the database file [EXTRA CREDIT]\n");
    printf("\t-z:  zero db file (remove all records)\n");
}


//Welcome to main()
int main(int argc, char *argv[]){
    char opt;           //user selected option
    int fd;             //file descriptor of database files
    int rc;             //return code from various operations
    int exit_code;      //exit code to shell
    int id;             //userid from argv[2]
    int gpa;            //gpa from argv[5]

    //space for a student structure which we will get back from
    //some of the functions we will be writing such as get_student(),
    //and print_student(). 
    student_t student = {0};

    //This function must have at least one arg, and the arg must start
    //with a dash
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    //The option is the first character after the dash for example
    //-h -a -c -d -f -p -x -z
    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(EXIT_OK);
    }

    //now lets open the file and continue if there is no error
    //note we are not truncating the file using the second
    //parameter
    fd = open_db(DB_FILE, false);
    if (fd < 0){
        exit(EXIT_FAIL_DB);
    }

    //set rc to the return code of the operation to ensure the program
    //use that to determine the proper exit_code.  Look at the header
    //sdbsc.h for expected values. 

    exit_code = EXIT_OK;
    switch(opt){
        case 'a':
            //   arv[0] arv[1]  arv[2]      arv[3]    arv[4]  arv[5]         
            //prog_name     -a      id  first_name last_name     gpa
            //-------------------------------------------------------
            //example:  prog_name -a 1 John Doe 341
            if (argc != 6){
                usage(argv[0]);
                exit_code = EXIT_FAIL_ARGS;
                break;
            }

            //convert id and gpa to ints from argv.  For this assignment assume
            //they are valid numbers
            id = atoi(argv[2]);
            gpa = atoi(argv[5]);

            exit_code = validate_range(id,gpa);
            if (exit_code == EXIT_FAIL_ARGS){
                printf(M_ERR_STD_RNG);
                break;
            }

            rc = add_student(fd, id, argv[3], argv[4], gpa);
            if (rc < 0)
                exit_code = EXIT_FAIL_DB;

            break;

        case 'c':
            //    arv[0] arv[1]    
            //prog_name     -c 
            //-----------------
            //example:  prog_name -c  
            rc = count_db_records(fd);
            if (rc < 0)
                exit_code = EXIT_FAIL_DB;
            break;

        case 'd':
            //   arv[0]  arv[1]  arv[2]    
            //prog_name     -d      id 
            //-------------------------
            //example:  prog_name -d 100    
            if (argc != 3){
                usage(argv[0]);
                exit_code = EXIT_FAIL_ARGS;
                break;
            }
            id = atoi(argv[2]);
            rc = del_student(fd, id);
            if (rc < 0)
                exit_code = EXIT_FAIL_DB;

            break;

        case 'f':
            //    arv[0] arv[1]  arv[2]    
            //prog_name     -f      id
            //-------------------------
            //example:  prog_name -f 100       
            if (argc != 3){
                usage(argv[0]);
                exit_code = EXIT_FAIL_ARGS;
                break;
            }
            id = atoi(argv[2]);
            rc = get_student(fd, id, &student);

           
            switch (rc){
                case NO_ERROR:
                    print_student(&student);
                    break;
                case SRCH_NOT_FOUND:
                    printf(M_STD_NOT_FND_MSG, id);
                    exit_code = EXIT_FAIL_DB;
                    break;
                default:
                    printf(M_ERR_DB_READ);
                    exit_code = EXIT_FAIL_DB;
                    break;
            }
            break;

        case 'p':
            //    arv[0] arv[1]    
            //prog_name     -p 
            //-----------------
            //example:  prog_name -p  
            rc = print_db(fd);
            if (rc < 0)
                exit_code = EXIT_FAIL_DB;
            break;

        case 'x':
            //    arv[0] arv[1]    
            //prog_name     -x 
            //-----------------
            //example:  prog_name -x 

            //remember compress_db returns a fd of the compressed database.
            //we close it after this switch statement 
            fd = compress_db(fd);
            if (fd < 0)
                exit_code = EXIT_FAIL_DB;
            break;

        case 'z':
            //    arv[0] arv[1]    
            //prog_name     -x 
            //-----------------
            //example:  prog_name -x 
            //HINT:  close the db file, we already have fd 
            //       and reopen db indicating truncate=true
            close(fd);
            fd = open_db(DB_FILE, true);
            if (fd < 0){
                exit_code = EXIT_FAIL_DB;
                break;
            }
            printf(M_DB_ZERO_OK);
            exit_code = EXIT_OK;
            break;
        default:
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
    }

    //dont forget to close the file before exiting, and setting the 
    //proper exit code - see the header file for expected values
    close(fd);
    exit(exit_code);
}
