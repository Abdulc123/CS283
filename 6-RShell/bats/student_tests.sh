#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

# Helper function to clean up stripped output
strip_output() {
    echo "$output" | tr -d '[:space:]'
}

# Local Mode Tests
@test "Local Mode: Simple command" {
    run "./dsh" <<EOF
ls
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "dsh4>" ]]
}

@test "Local Mode: Pipe command" {
    run "./dsh" <<EOF
ls | grep .c
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ ".c" ]]
}

@test "Local Mode: cd command" {
    run "./dsh" <<EOF
cd ..
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ ".." ]]
}

@test "Local Mode: Multiple pipes" {
    run "./dsh" <<EOF
ls | grep .c | wc -l
exit
EOF
    [ "$status" -eq 0 ]
}

# Server Mode Tests
@test "Server Mode: Start server" {
    # Start server in background
    ./dsh -s -p 5555 &
    server_pid=$!
    sleep 1
    
    # Check if server process exists
    ps -p $server_pid > /dev/null
    [ "$?" -eq 0 ]
    
    # Cleanup
    kill $server_pid
}

@test "Server Mode: Client connection" {
    # Start server in background
    ./dsh -s -p 5556 &
    sleep 1
    
    # Run client
    run timeout 2 ./dsh -c -p 5556 <<EOF
exit
EOF
    
    # Cleanup
    pkill -f "./dsh -s"
    
    [ "$status" -eq 0 ] || [ "$status" -eq 124 ]
}

@test "Server Mode: Remote command execution" {
    # Start server in background
    ./dsh -s -p 5557 &
    sleep 1
    
    # Run client with commands
    run timeout 2 ./dsh -c -p 5557 <<EOF
ls
exit
EOF
    
    # Cleanup
    pkill -f "./dsh -s"
    
    [ "$status" -eq 0 ] || [ "$status" -eq 124 ]
    [[ "$output" =~ "dsh4>" ]]
}

@test "Server Mode: Remote pipe execution" {
    # Start server in background
    ./dsh -s -p 5558 &
    sleep 1
    
    # Run client with piped commands
    run timeout 2 ./dsh -c -p 5558 <<EOF
ls | grep .c
exit
EOF
    
    # Cleanup
    pkill -f "./dsh -s"
    
    [ "$status" -eq 0 ] || [ "$status" -eq 124 ]
    [[ "$output" =~ ".c" ]]
}

@test "Server Mode: Remote cd command" {
    # Start server in background
    ./dsh -s -p 5559 &
    server_pid=$!
    sleep 1
    
    # Create a temporary directory for testing
    mkdir -p test_dir
    
    # Run client with cd command
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
    
    # Check if both directory changes worked
    [ "$status" -eq 0 ]
    [[ "$output" =~ "test_dir" ]] || [[ "$output" =~ $(pwd) ]]
}

@test "Server Mode: Stop server command" {
    # Start server in background
    ./dsh -s -p 5560 &
    sleep 1
    
    # Run client with stop-server command
    run timeout 2 ./dsh -c -p 5560 <<EOF
stop-server
EOF
    
    # Check if server process exists
    sleep 1
    run pgrep -f "./dsh -s -p 5560"
    [ "$status" -eq 1 ]  # Process should not exist
}

