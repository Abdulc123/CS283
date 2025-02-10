1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**: fgets is a good choice since it reads inputs line by line, and stops once it reads a new line. It stops once a new line is read or buffer is filled, it will end the new line to the end and adds the null terminator as well.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  Using malloc to allocate memory for cmd_buff for being more flexible and dynamic. It also preents lots of stack memory usage if we were to hard declare it as a local variable. Since we use malloc, we can reuse it for multiple commands and free it at the end without wasting memory. 


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  Its necessary to ensure correct command parsing and execution in the shell. If we had an execution command " cd ", if we didn't trim the white spaces it would be contained within the execution command. Or if you are checking if the command inputted, "exit " == "exit" this will always bef false due to the spaces. It would also impact argument handling if it ended with spaces. Basically leading and trailing spaces would create false positives, where our shell would try and run commands that are not correct.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  Three exmaples of redirection we can implement for our shell is,  1. ls > file.txt : This will redirect the output from ls to file.txt. This can either create a new file or overwrite a previously created one. 2. wc -l < file.txt, this directs the contents from file.txt to wc -l and will output the amount of lines file.txt.  3. ls -l | wc -l, this will output the files line by line, and output that to wc -l, which will count the number of files.
    
   

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**: The difference between redirection and piping. Redirection is able to redirect input/output to or from a file, passed the output of one command to another from files. Piping is used to send output from one command  directly into another without the need of a file. Pipes use memory, while redirection uses disk operations. 

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**: Its important to keep these seperate in the shell so errors don't get mixed with other useful outputs that aren't errors. Keeping them seperate allows you to normal output in one file and errors in another file. Also it helps keeps automations reliable, so errors don't mess up their results. 

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**: The errors should always be displayed seperateley from the the STDOUT. Errors should be able to be redirected. Also allow for errors and stdout to be directed to the same output.txt file. Pipes should only be able to pass STDOUT