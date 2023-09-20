@echo off

go version
if %errorlevel% neq 0 (
    echo Go is not installed. Please install Go from the official website: https://golang.org/dl/
    exit /b 1
)

go mod init main.go
go mod tidy
go build -o bin main.exe

(
echo @echo off
echo if not exist .\bin\main.exe (
echo    echo Error: .\bin\main.exe does not exist. Please run the setup script first.
echo    exit /b 1
echo )
echo .\bin\main.exe
) > run_server.bat
