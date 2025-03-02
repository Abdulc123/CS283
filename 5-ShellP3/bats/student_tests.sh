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

# Test: Ensure `pwd` outputs the correct directory
@test "Check pwd command" {
    run ./dsh <<EOF
pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"/"* ]]  # Expect output to be a valid directory path
}

# Test: Ensure `cd` changes directory
@test "Check cd command" {
    run ./dsh <<EOF
cd /
pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"/"* ]]
}

# Test : Ensure `exit` command works
@test "Exit command should terminate the shell" {
    run ./dsh <<EOF
exit
EOF
    [[ "$output" == *"exiting..."* ]]
}

# Test: Check command execution with arguments
@test "Check echo with arguments" {
    run ./dsh <<EOF
echo Hello, world!
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello, world!"* ]]
}

# Test: Ensure quoted arguments are treated as one
@test "Check echo handles quoted arguments" {
    run ./dsh <<EOF
echo "Hello world"
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello world"* ]]
}

# Test: Ensure piping works (`ls | grep`)
@test "Check piping with ls and grep" {
    run ./dsh <<EOF
ls | grep .c
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *".c"* ]]  # Expect at least one .c file in the output
}

# Test: Ensure multiple pipes work
@test "Check multiple pipes" {
    run ./dsh <<EOF
echo "HELLO WORLD" | tr 'A-Z' 'a-z' | tr 'o' '0'
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"hell0 w0rld"* ]]
}


# Test: Ensure empty input does not crash shell
@test "Check empty input is handled correctly" {
    run ./dsh <<EOF

EOF
    [ "$status" -eq 0 ]
}

# Test: Ensure excessive spaces are handled
@test "Check handling of excessive spaces" {
    run ./dsh <<EOF
ls       -l
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"total"* ]]  # ls -l should contain "total" in output
}



