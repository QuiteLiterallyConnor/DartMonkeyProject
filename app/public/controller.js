const wsURL = (window.location.protocol === "https:" ? "wss://" : "ws://") + window.location.host + "/controller/ws";
const ws = new WebSocket(wsURL);
const livestreamLocation = window.location.protocol + '//' + window.location.hostname + ':6001/stream';

console.log(livestreamLocation);


$(document).ready(function() {
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

    console.log(livestreamLocation);
    $("#livestream").attr("src", livestreamLocation);

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

        console.log(line) 

        processSystemMessage(line);
        
        if (!line.includes("%%%_HEARTBEAT")) {
            $('#messages').append(`<p>${e.data}</p>`);
            $('#messages').scrollTop($('#messages')[0].scrollHeight);
        }

        
    };

    // const video = document.getElementById('video');
    // const liveButton = document.getElementById('liveButton');
    
    // const hls = new Hls({
    //     liveBackBufferLength: 3, // Keep only 3 seconds of content for back buffer
    //     maxBufferLength: 5,     // Maximum buffer length of 5 seconds
    //     maxBufferSize: 500000,  // Maximum buffer size in bytes
    //     liveSyncDurationCount: 1 // Try to stay close to live
    // });

    
    // hls.loadSource('/hls/playlist.m3u8');
    // hls.attachMedia(video);
    
    // liveButton.addEventListener('click', function() {
    //     if (video.duration) {
    //         video.currentTime = video.duration; // Move to the most recent moment
    //     }
    // });


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
        ws.send('YO5');
    });

    $('#downButton').click(function() {
        ws.send('YO-5');
    });

    $('#leftButton').click(function() {
        ws.send('XO5');
    });

    $('#rightButton').click(function() {
        ws.send('XO-5');
    });




    $('#motorASpeedUp').click(function() {
        ws.send("CO5")
    });

    $('#motorASpeedDown').click(function() {
        ws.send("CO-5")
    });

    $('#motorBSpeedUp').click(function() {
        ws.send("DO5")
    });

    $('#motorBSpeedDown').click(function() {
        ws.send("DO-5")
    });



    $('#motorServoALeft').click(function() {
        ws.send("AO5")
    });

    $('#motorServoARight').click(function() {
        ws.send("AO-5")
    });

    $('#motorServoBLeft').click(function() {
        ws.send("BO5")
    });

    $('#motorServoBRight').click(function() {
        ws.send("BO-5")
    });


    $('#pingButton').click(function() {
        ws.send('H');
    });


});

let keyInterval = {};
let keysPressed = { left: false, up: false, right: false, down: false };

document.addEventListener("keydown", function(event) {
    let key;
    switch (event.key) {
        case "ArrowLeft":
            key = "left";
            break;
        case "ArrowUp":
            key = "up";
            break;
        case "ArrowRight":
            key = "right";
            break;
        case "ArrowDown":
            key = "down";
            break;
    }

    if (key) {
        document.getElementById(key).classList.add('pressed');
        sendKeyCommand(key);
    }
});

document.addEventListener("keyup", function(event) {
    let key;
    switch (event.key) {
        case "ArrowLeft":
            key = "left";
            break;
        case "ArrowUp":
            key = "up";
            break;
        case "ArrowRight":
            key = "right";
            break;
        case "ArrowDown":
            key = "down";
            break;
    }
    if (key) {
        document.getElementById(key).classList.remove('pressed');
    }
});

function sendKeyCommand(key) {
    switch (key) {
        case "left":
            console.log("sending XO5 for left");
            ws.send("XO5");
            break;
        case "right":
            console.log("sending XO-5 for right");
            ws.send("XO-5");
            break;
        case "up":
            console.log("sending YO5 for up");
            ws.send("YO5");
            break;
        case "down":
            console.log("sending YO-5 for down");
            ws.send("YO-5");
            break;
    }
}

