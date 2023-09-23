package main

import (
	"bufio"
	"fmt"
	"os"
	"os/exec"
	"strings"
	"time"
)

var cmd *exec.Cmd

func startServer() error {
	if cmd != nil && cmd.Process != nil {
		return fmt.Errorf("server already running")
	}
	cmd = exec.Command("python", "camera_server.py", "6001")
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Start()
}

func stopServer() error {
	if cmd != nil && cmd.Process != nil {
		err := cmd.Process.Kill()
		if err != nil {
			return err
		}
		cmd.Wait()
		cmd = nil
		return nil
	}
	return fmt.Errorf("server not running")
}

func restartServer() error {
	err := stopServer()
	if err != nil {
		return fmt.Errorf("error stopping server: %v", err)
	}
	// Wait for a while to make sure everything is stopped before starting again
	time.Sleep(1 * time.Second)
	return startServer()
}

func printUsage() {
	fmt.Println("Enter one of the following commands: start, stop, restart, exit")
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	printUsage()
	for scanner.Scan() {
		input := scanner.Text()
		var err error
		switch strings.ToLower(input) {
		case "start":
			err = startServer()
		case "stop":
			err = stopServer()
		case "restart":
			err = restartServer()
		case "exit":
			if cmd != nil && cmd.Process != nil {
				stopServer() // Stop the server before exiting if it's running
			}
			os.Exit(0)
		default:
			printUsage()
			continue
		}

		if err != nil {
			fmt.Printf("Error executing %s: %v\n", input, err)
		} else {
			fmt.Printf("Successfully executed %s command\n", input)
		}
	}

	if err := scanner.Err(); err != nil {
		fmt.Println("Error reading from stdin:", err)
	}
}
