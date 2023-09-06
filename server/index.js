$(document).ready(function() {
    const ws = new WebSocket("ws://localhost:8080/ws");
    let powerLevel = 0; // To keep track of the power level for the SX command

    ws.onopen = function() {
        console.log('Connected to the WebSocket');
    };

    ws.onmessage = function(e) {
        $('#messages').append(`<p>${e.data}</p>`);
        $('#messages').scrollTop($('#messages')[0].scrollHeight); // Auto scroll to bottom

        // Check for DEVICE_STATUS delimiter
        if (e.data.includes('%%%_DEVICE_STATUS')) {
            const lines = e.data.split('\n');
            for (let line of lines) {
                if (line.startsWith('X_SERVO_POS:')) {
                    $('#xDisplay').text(line.split(':')[1].trim());
                } else if (line.startsWith('Y_SERVO_POS:')) {
                    $('#yDisplay').text(line.split(':')[1].trim());
                } else if (line.startsWith('MOTOR_A_SERVO_POS:')) {
                    $('#servoADisplay').text(line.split(':')[1].trim() + '°');
                } else if (line.startsWith('MOTOR_B_SERVO_POS:')) {
                    $('#servoBDisplay').text(line.split(':')[1].trim() + '°');
                } else if (line.startsWith('MOTOR_SPEED:')) {
                    $('#speedDisplay').text(line.split(':')[1].trim());
                }
            }
        }
    };

    $('#sendButton').click(function() {
        const command = $('#command').val().trim();
        if (command) {
            ws.send(command);
            $('#command').val('');
        }
    });

    $('#uploadButton').click(function() {
        const file = $('#gcodeFile')[0].files[0];
        if (file) {
            const reader = new FileReader();
            reader.onload = function(e) {
                const lines = e.target.result.split('\n');
                for (let line of lines) {
                    if (line.trim()) {
                        ws.send(line.trim());
                    }
                }
            };
            reader.readAsText(file);
        }
    });

    function updateInputValue() {
        const selectedStatus = $('#statusSelect').val();
        let valueToSet = '';

        switch (selectedStatus) {
            case 'servoA':
                valueToSet = $('#servoADisplay').text().replace('°', '');
                break;
            case 'servoB':
                valueToSet = $('#servoBDisplay').text().replace('°', '');
                break;
            case 'xAxis':
                valueToSet = $('#xDisplay').text();
                break;
            case 'yAxis':
                valueToSet = $('#yDisplay').text();
                break;
            case 'speed':
                valueToSet = $('#speedDisplay').text();
                break;
        }

        $('#angleInput').val(valueToSet);
    }

    $('#statusSelect').change(function() {
        console.log("Dropdown change detected."); // Debugging line

        const selectedStatus = $(this).val();
        let valueToSet = '';

        console.log("Selected status:", selectedStatus); // Debugging line

        switch (selectedStatus) {
            case 'servoA':
                valueToSet = $('#servoADisplay').text().split('°')[0]; // Split and take the numeric part
                console.log("Servo A value:", valueToSet); // Debugging line
                break;
            case 'servoB':
                valueToSet = $('#servoBDisplay').text().split('°')[0]; // Split and take the numeric part
                console.log("Servo B value:", valueToSet); // Debugging line
                break;
            case 'xAxis':
                valueToSet = $('#xDisplay').text();
                console.log("X Axis value:", valueToSet); // Debugging line
                break;
            case 'yAxis':
                valueToSet = $('#yDisplay').text();
                console.log("Y Axis value:", valueToSet); // Debugging line
                break;
            case 'speed':
                valueToSet = $('#speedDisplay').text();
                console.log("Speed value:", valueToSet); // Debugging line
                break;
        }

        $('#angleInput').val(valueToSet);
        console.log("Input field set to:", $('#angleInput').val()); // Debugging line
    });

    // Initially update the input value based on the default selected status
    updateInputValue();

    // Gamepad Controls
    $('#upButton').click(function() {
        ws.send('Y5');
        updateDisplay('yDisplay', 5);
    });

    $('#downButton').click(function() {
        ws.send('Y-5');
        updateDisplay('yDisplay', -5);
    });

    $('#leftButton').click(function() {
        ws.send('X-5');
        updateDisplay('xDisplay', -5);
    });

    $('#rightButton').click(function() {
        ws.send('X5');
        updateDisplay('xDisplay', 5);
    });

    $('#powerUp').click(function() {
        powerLevel += 5;
        ws.send('S' + powerLevel);
        $('#speedDisplay').text(powerLevel);
    });

    $('#powerDown').click(function() {
        powerLevel -= 5;
        ws.send('S' + powerLevel);
        $('#speedDisplay').text(powerLevel);
    });

    $('#servoSlider').on('input', function() {
        const value = $(this).val();
        ws.send('A' + value);
        ws.send('B' + value);
        $('#servoADisplay').text(value + '°');
        $('#servoBDisplay').text(value + '°');
    });

    $('#setAngleButton').click(function() {
        const angle = $('#angleInput').val();
        if (angle) {
            ws.send('A' + angle);
            ws.send('B' + angle);
            $('#servoADisplay').text(angle + '°');
            $('#servoBDisplay').text(angle + '°');
        }
    });

    $('#statusSelect').change(function() {
        const selectedStatus = $(this).val();
        switch (selectedStatus) {
            case 'servoA':
                $('#angleInput').val($('#servoADisplay').text().replace('°', ''));
                break;
            case 'servoB':
                $('#angleInput').val($('#servoBDisplay').text().replace('°', ''));
                break;
            case 'xAxis':
                $('#angleInput').val($('#xDisplay').text());
                break;
            case 'yAxis':
                $('#angleInput').val($('#yDisplay').text());
                break;
            case 'speed':
                $('#angleInput').val($('#speedDisplay').text());
                break;
        }
    });

    function updateDisplay(elementId, value) {
        const currentVal = parseInt($('#' + elementId).text());
        $('#' + elementId).text(currentVal + value);
    }

    ws.onmessage = function(e) {
        $('#messages').append(`<p>${e.data}</p>`);
        $('#messages').scrollTop($('#messages')[0].scrollHeight); // Auto scroll to bottom

        // Check for DEVICE_STATUS delimiter
        if (e.data.includes('%%%_DEVICE_STATUS')) {
            const lines = e.data.split('\n');
            for (let line of lines) {
                if (line.startsWith('X_SERVO_POS:')) {
                    $('#xDisplay').text(line.split(':')[1].trim());
                } else if (line.startsWith('Y_SERVO_POS:')) {
                    $('#yDisplay').text(line.split(':')[1].trim());
                } else if (line.startsWith('MOTOR_A_SERVO_POS:')) {
                    $('#servoADisplay').text(line.split(':')[1].trim() + '°');
                } else if (line.startsWith('MOTOR_B_SERVO_POS:')) {
                    $('#servoBDisplay').text(line.split(':')[1].trim() + '°');
                } else if (line.startsWith('MOTOR_SPEED:')) {
                    $('#speedDisplay').text(line.split(':')[1].trim());
                }
            }
        }
    };
});