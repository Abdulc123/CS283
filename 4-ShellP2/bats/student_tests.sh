#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 1: Empty Input
@test "Empty input returns warning" {
  run ./dsh <<EOF

exit
EOF
    # We expect the shell to print the warning message for empty input.
    echo "Output: $output"
    [[ "$output" =~ "warning: no commands provided" ]]
    [ "$status" -eq 0 ]
}

# Test 2: Multiple spaces between tokens
@test "Command with multiple spaces between tokens is parsed correctly" {
  run ./dsh <<EOF
ls  -l 
exit
EOF
    # "ls", "-l"
    echo "Output: $output"
    echo "Exit Status: $status"

    [[ "$output" =~ "total" ]] # ls -l starts with "total X"
    [[ "$output" =~ "dsh" ]]   # Expect a file named "dsh" to be listed
    [[ "$output" =~ "dshlib.h" ]] # Expect another file present in the directory
    [ "$status" -eq 0 ]
}


# Test 3: Built-in cd with an invalid directory should print an error
@test "Invalid cd command prints error" {
  run ./dsh <<EOF
cd /nonexistent_directory
pwd
exit
EOF
  # Expect the error message from cd (for example, "cd failed" or similar message using perror)
  [[ "$output" =~ "cd failed" ]]
  [ "$status" -eq 0 ]
}

# Test 4: Too many commands given for cd
@test "Too many commands given for cd" {
  run ./dsh <<EOF
cd bats test
pwd
exit
EOF
  # Expect the error message from cd (for example, "cd failed" or similar message using perror)
  [[ "$output" =~ "Error: cd, too many arguments provided" ]]
  [ "$status" -eq 0 ]
}



