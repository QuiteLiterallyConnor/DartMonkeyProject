import pygame
import sys
import time
import serial

# Initialize pygame and the joystick module
pygame.init()
pygame.joystick.init()

# Ensure a joystick is connected
if pygame.joystick.get_count() == 0:
    print("No joystick detected. Please connect an Xbox controller.")
    sys.exit()

# Initialize the first joystick
joystick = pygame.joystick.Joystick(0)
joystick.init()

# Set a threshold to avoid noise from minor movements
threshold = 0.5

# Variables to store previous commands (to prevent repeated printing)
prev_x_command = ""
prev_y_command = ""
x_button_prev_state = False

# Initialize serial communication
ser = serial.Serial('COM13', 115200, timeout=1)
time.sleep(2)  # Wait for the serial connection to initialize

print("Monitoring controller input. Movement on any joystick or the D-pad produces consistent commands.")

try:
    while True:
        pygame.event.pump()
        
        # Read left analog stick (axes 0 and 1)
        left_x = joystick.get_axis(0)
        left_y = joystick.get_axis(1)
        
        # Read right analog stick (commonly axes 3 and 4)
        right_x = joystick.get_axis(3)
        right_y = joystick.get_axis(4)
        
        # Read D-pad (hat 0)
        if joystick.get_numhats() > 0:
            dpad_x, dpad_y = joystick.get_hat(0)
        else:
            dpad_x, dpad_y = (0, 0)
        
        # Read buttons
        buttons = joystick.get_numbuttons()
        x_button_pressed = joystick.get_button(2)  # X button is usually button 2 on Xbox controllers
        
        # Determine combined X-axis command:
        # Any input indicating right yields XO2; any input indicating left yields XO-2.
        x_command = ""
        if (left_x > threshold) or (right_x > threshold) or (dpad_x == 1):
            x_command = "XO2"
        elif (left_x < -threshold) or (right_x < -threshold) or (dpad_x == -1):
            x_command = "XO-2"
        
        # Determine combined Y-axis command:
        # Analog sticks typically use negative for up, positive for down.
        # D-pad up is dpad_y == 1, down is dpad_y == -1.
        y_command = ""
        if (left_y < -threshold) or (right_y < -threshold) or (dpad_y == 1):
            y_command = "YO2"
        elif (left_y > threshold) or (right_y > threshold) or (dpad_y == -1):
            y_command = "YO-2"
        
        # Print and send commands even if they are the same as the previous state
        if x_command or y_command:
            combined_command = f"{x_command}; {y_command}".strip("; ")
            ser.write((combined_command + '\n').encode())
            print(combined_command)
            prev_x_command = x_command
            prev_y_command = y_command
        
        # Send specific commands if X button is pressed and not held
        if x_button_pressed and not x_button_prev_state:
            print("Debug: X button pressed")
            ser.write("CS50; DS50\n".encode())
            time.sleep(1)
            ser.write("AO15; BO-15\n".encode())
            time.sleep(1)
            ser.write("CS0; DS0\n".encode())
            ser.write("AO-15; BO15\n".encode())
        x_button_prev_state = x_button_pressed
        
        # Read and print messages received over serial
        if ser.in_waiting > 0:
            try:
                received_message = ser.readline().decode('utf-8').strip()
                print("Received:", received_message)
            except UnicodeDecodeError:
                print("Received a message that could not be decoded.")
        
        time.sleep(0.1)
except KeyboardInterrupt:
    print("\nExiting monitoring.")
finally:
    ser.close()
    pygame.quit()
