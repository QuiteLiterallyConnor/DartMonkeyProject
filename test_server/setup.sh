#!/bin/bash

# Check for Go version
go version

# Check the exit status of the 'go version' command
if [ $? != 0 ]; then
    echo "Go is not installed. Please install with 'apt install golang'."
    exit 1
fi

# Initialize the Go module and tidy it up
go mod init main.go
go mod tidy

# Build the Go binary
go build -o bin main.go

echo '#!/bin/bash' > run_server.sh
echo 'if [ ! -f ./bin/main ]; then' >> run_server.sh
echo '    echo "Error: ./bin/main does not exist. Please run the setup script first."' >> run_server.sh
echo '    exit 1' >> run_server.sh
echo 'fi' >> run_server.sh
echo './bin/main' >> run_server.sh

# Provide execute permissions
chmod +x run_server.sh
