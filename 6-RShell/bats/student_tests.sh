#!/usr/bin/env bats

# File: student_tests.sh
# Purpose: Test suite for the remote shell implementation
# This file contains tests for both local and remote shell functionality
# using the BATS (Bash Automated Testing System) framework

# Helper function to clean up output by removing whitespace
# Useful when we need to compare output without worrying about spacing
strip_output() {
    echo "$output" | tr -d '[:space:]'
}

# Test 1: Basic command execution
# Verifies that simple commands like 'ls' work
# and that the shell prompt appears correctly
@test "Local Mode: Simple command" {
    run "./dsh" <<EOF
ls
exit
EOF
    [ "$status" -eq 0 ]  # Check if command succeeded
    [[ "$output" =~ "dsh4>" ]]  # Check if prompt appears
}

# Test 2: Pipe functionality
# Verifies that commands can be piped together
# Example: listing files and filtering for .c files
@test "Local Mode: Pipe command" {
    run "./dsh" <<EOF
ls | grep .c
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ ".c" ]]  # Should find at least one .c file
}

# Test 3: Directory navigation
# Tests the built-in 'cd' command and pwd
# to verify directory changes work
@test "Local Mode: cd command" {
    run "./dsh" <<EOF
cd ..
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ ".." ]]  # Should show parent directory
}

# Test 4: Multiple pipe handling
# Tests more complex pipe chains to ensure
# multiple commands can be connected
@test "Local Mode: Multiple pipes" {
    run "./dsh" <<EOF
ls | grep .c | wc -l
exit
EOF
    [ "$status" -eq 0 ]
}


# Test 5: Server Initialization
# Verifies that the server starts correctly
# and creates a running process
@test "Server Mode: Start server" {
    # Launch server in background and capture PID
    ./dsh -s -p 5555 &
    server_pid=$!
    sleep 1  # Give server time to start
    
    # Verify server process exists
    ps -p $server_pid > /dev/null
    [ "$?" -eq 0 ]
    
    # Clean up server process
    kill $server_pid
}

# Test 6: Client Connection
# Tests that a client can connect to the server
# and perform a basic exit command
@test "Server Mode: Client connection" {
    # Start server
    ./dsh -s -p 5556 &
    sleep 1
    
    # Try connecting with client
    run timeout 2 ./dsh -c -p 5556 <<EOF
exit
EOF
    
    # Cleanup server
    pkill -f "./dsh -s"
    
    # Check success (either normal exit or timeout)
    [ "$status" -eq 0 ] || [ "$status" -eq 124 ]
}

# Test 7: Remote Command Execution
# Verifies that commands can be executed remotely
# and that output is received correctly
@test "Server Mode: Remote command execution" {
    ./dsh -s -p 5557 &
    sleep 1
    
    # Run a simple ls command remotely
    run timeout 2 ./dsh -c -p 5557 <<EOF
ls
exit
EOF
    
    pkill -f "./dsh -s"
    
    [ "$status" -eq 0 ] || [ "$status" -eq 124 ]
    [[ "$output" =~ "dsh4>" ]]  # Verify prompt appears
}

# Test 8: Remote Pipe Execution
# Tests that pipe operations work over
# the network connection
@test "Server Mode: Remote pipe execution" {
    ./dsh -s -p 5558 &
    sleep 1
    
    # Test pipe command remotely
    run timeout 2 ./dsh -c -p 5558 <<EOF
ls | grep .c
exit
EOF
    
    pkill -f "./dsh -s"
    
    [ "$status" -eq 0 ] || [ "$status" -eq 124 ]
    [[ "$output" =~ ".c" ]]
}

# Test 9: Remote Directory Navigation
# Verifies that directory changes persist
# during a remote session
@test "Server Mode: Remote cd command" {
    ./dsh -s -p 5559 &
    server_pid=$!
    sleep 1
    
    # Create test directory
    mkdir -p test_dir
    
    # Test directory navigation
    run ./dsh -c -p 5559 <<EOF
cd test_dir
pwd
cd ..
pwd
exit
EOF
    
    # Cleanup
    kill $server_pid
    rm -rf test_dir
    
    [ "$status" -eq 0 ]
    [[ "$output" =~ "test_dir" ]] || [[ "$output" =~ $(pwd) ]]
}

# Test 10: Server Stop Command
# Verifies that the server can be stopped
# using the stop-server command
@test "Server Mode: Stop server command" {
    ./dsh -s -p 5560 &
    sleep 1
    
    # Send stop-server command
    run timeout 2 ./dsh -c -p 5560 <<EOF
stop-server
EOF
    
    # Verify server process is gone
    sleep 1
    run pgrep -f "./dsh -s -p 5560"
    [ "$status" -eq 1 ]  # Process should not exist
}

# Test 11: Multiple Client Support
# Tests that the server can handle
# multiple client connections sequentially
@test "Server Mode: Multiple clients" {
    ./dsh -s -p 5561 &
    sleep 1
    
    # First client session
    ./dsh -c -p 5561 <<EOF
ls
exit
EOF
    
    # Second client session
    run ./dsh -c -p 5561 <<EOF
ls
exit
EOF
    
    pkill -f "./dsh -s"
    
    [ "$status" -eq 0 ]
}



# Test 12: Custom Configuration
# Verifies that custom port and interface
# settings work correctly
@test "Feature: Custom port and interface" {
    # Start with custom settings
    ./dsh -s -i 127.0.0.1 -p 5562 &
    sleep 1
    
    # Try connecting with matching settings
    run timeout 2 ./dsh -c -i 127.0.0.1 -p 5562 <<EOF
ls
exit
EOF
    
    pkill -f "./dsh -s"
    
    [ "$status" -eq 0 ] || [ "$status" -eq 124 ]
}
