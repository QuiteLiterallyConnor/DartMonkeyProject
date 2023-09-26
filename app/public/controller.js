$(document).ready(function() {
    const wsURL = (window.location.protocol === "https:" ? "wss://" : "ws://") + window.location.host + "/controller/ws";
    const ws = new WebSocket(wsURL);
    let intervalID; // To store the interval ID

    // Limits
    const MAX_ANGLE = 90;
    const MIN_ANGLE = -90;
    const MAX_SPEED = 100;
    const MIN_SPEED = 0;

    ws.onopen = function() {
        console.log('Connected to the WebSocket');
    };

    ws.onclose = function() {
        console.log('WebSocket connection closed');

        // Clear the interval when the WebSocket disconnects
        clearInterval(intervalID);
    };

    let lastHeartbeatTime = Date.now();
    let heartbeatInterval;

    function updateHeartbeatTimer() {
        const currentTime = Date.now();
        const timeDiff = Math.floor((currentTime - lastHeartbeatTime) / 1000);
        $('#heartbeatTimer').text(timeDiff + 's');

        // If no heartbeat is received for more than 10 seconds, consider it disconnected
        if (timeDiff > 10) {
            $('#connectionStatus').removeClass('bg-success').addClass('bg-danger');
            $('#connectionLabel').text('Disconnected');
        }
    }

    // Start the heartbeat timer
    heartbeatInterval = setInterval(updateHeartbeatTimer, 1000);

    // When a message is received, check if it's a heartbeat
    ws.onmessage = function(e) {
        line = e.data

        console.log("Got message: ")
        console.log(line)

        if (line.startsWith('%%')) {
            processSystemMessage(line);
        } else {
            $('#messages').append(`<p>${e.data}</p>`);
            $('#messages').scrollTop($('#messages')[0].scrollHeight);
        }
        
    };

    function processSystemMessage(line) {
        lastHeartbeatTime = Date.now();
        $('#connectionStatus').removeClass('bg-danger').addClass('bg-success');
        $('#connectionLabel').text('Connected');


        if (line.startsWith('%%%_X_SERVO_POS:')) {
            $('#xDisplay').text(line.split(':')[1].trim()+ '°');
        } else if (line.startsWith('%%%_Y_SERVO_POS:')) {
            $('#yDisplay').text(line.split(':')[1].trim()+ '°');
        } else if (line.startsWith('%%%_MOTOR_A_SERVO_POS:')) {
            $('#servoADisplay').text(line.split(':')[1].trim() + '°');
        } else if (line.startsWith('%%%_MOTOR_B_SERVO_POS:')) {
            $('#servoBDisplay').text(line.split(':')[1].trim() + '°');
        } else if (line.startsWith('%%%_MOTOR_A_SPEED:')) {
            $('#motorADisplay').text(line.split(':')[1].trim());
        } else if (line.startsWith('%%%_MOTOR_B_SPEED:')) {
            $('#motorBDisplay').text(line.split(':')[1].trim());
        } else {
            // Do something with non specified system messages
        }

    }

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

    // Gamepad Controls
    $('#upButton').click(function() {
        if (parseInt($('#yDisplay').text()) + 5 <= MAX_ANGLE) {
            ws.send('YO5');
        }
    });

    $('#downButton').click(function() {
        if (parseInt($('#yDisplay').text()) - 5 >= MIN_ANGLE) {
            ws.send('YO-5');
        }
    });

    $('#leftButton').click(function() {
        if (parseInt($('#xDisplay').text()) - 5 >= MIN_ANGLE) {
            ws.send('XO-5');
        }
    });

    $('#rightButton').click(function() {
        if (parseInt($('#xDisplay').text()) + 5 <= MAX_ANGLE) {
            ws.send('XO5');
        }
    });

    $('#powerUp').click(function() {
        ws.send('SO10');
    });

    $('#powerDown').click(function() {
        ws.send('SO-10');
    });

    $('#servoSlider').on('input', function() {
        const value = $(this).val();
        ws.send('AS' + value);
        ws.send('BS' + value);
    });

    $('#setAngleButton').click(function() {
        const angle = $('#angleInput').val();
        if (angle) {
            ws.send('AO' + angle);
            ws.send('BO' + angle);
        }
    });

    $('#pingButton').click(function() {
        ws.send('H');
    });


});