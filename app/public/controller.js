const wsURL = (window.location.protocol === "https:" ? "wss://" : "ws://") + window.location.host + "/controller/ws";
const ws = new WebSocket(wsURL);
// const livestreamLocation = window.location.protocol + '//' + window.location.hostname + ':6001/stream';
const livestreamLocation = "/stream";

// document.addEventListener('DOMContentLoaded', function () {
//     var video = document.getElementById('video');
//     var videoSrc = '/hls/playlist.m3u8';
//     if (Hls.isSupported()) {
//         var hls = new Hls();
//         hls.loadSource(videoSrc);
//         hls.attachMedia(video);
//         hls.on(Hls.Events.MANIFEST_PARSED, function () {
//             video.play();
//         });
//     } else if (video.canPlayType('application/vnd.apple.mpegurl')) {
//         video.src = videoSrc;
//         video.addEventListener('loadedmetadata', function () {
//             video.play();
//         });
//     }
// });

$(document).ready(function() {
    let intervalID;
    getServerData();

    ws.onopen = function() {
        console.log('Connected to the WebSocket');
    };

    ws.onclose = function() {
        console.log('WebSocket connection closed');
        clearInterval(intervalID);
    };

    $("#livestreamMainCamera").attr("src", livestreamLocation + "1");
    $("#livestreamSecondaryCamera").attr("src", livestreamLocation + "2");


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

    let lastMessage;
    let consecutiveDuplicateCount = 0;
    let inHeartbeatBlock = false;
    
    ws.onmessage = function(e) {
        const dataObj = JSON.parse(e.data);
        processMessage(dataObj);
    };

    function processMessage(dataObj) {
        const sender = dataObj.sender;
        const message = dataObj.message;

        // console.log("sender: " + sender + ", msg: " + message);

        processSystemMessage(message);
    
        if (message.includes("%%%_ACK:H") || message.includes("HEARTBEAT_TAG")) {
            return;
        }
    
        if (message.includes("%%%_HEARTBEAT_START")) {
            inHeartbeatBlock = true;
            return; // Ignore and don't append anything yet
        }
    
        if (message.includes("%%%_HEARTBEAT_END")) {
            inHeartbeatBlock = false;
            if (lastMessage === "Heartbeat") {
                consecutiveDuplicateCount++;
                setLastLine(sender, `Heartbeat${consecutiveDuplicateCount > 0 ? ` (${consecutiveDuplicateCount + 1})` : ''}`);
            } else {
                consecutiveDuplicateCount = 0;
                appendToConsoleTextBox(sender, "Heartbeat");
            }
            lastMessage = "Heartbeat";
            return; // Stop processing, as we've finished the heartbeat block
        }
    
        if (!inHeartbeatBlock) {
            if (lastMessage === message) {
                consecutiveDuplicateCount++;
                setLastLine(sender, `${message} (${consecutiveDuplicateCount + 1})`);
            } else {
                consecutiveDuplicateCount = 0;
                appendToConsoleTextBox(sender, message);
            }
            lastMessage = message;
        }
    }
    
    
    function getServerData() {
        $.ajax({
            url: '/data',
            method: 'GET',
            success: function(data) {
                console.log(data);

                $('#xDisplay').text(data.system_states.X_SERVO_POS.toString().trim()+ '°');
                $('#yDisplay').text(data.system_states.Y_SERVO_POS.toString().trim()+ '°');
                $('#servoADisplay').text(data.system_states.MOTOR_A_SERVO_POS.toString().trim() + '°');
                $('#servoBDisplay').text(data.system_states.MOTOR_B_SERVO_POS.toString().trim() + '°');
                $('#motorADisplay').text(data.system_states.MOTOR_A_SPEED.toString().trim());
                $('#motorBDisplay').text(data.system_states.MOTOR_B_SPEED.toString().trim());

                if (data.is_connected) {
                    $('#connectionLabel').text('Connected');
                    updateHeartbeatTimer();
                }

                $('#valueBox').val(data.heartbeat_interval);

                $.each(data.serial_buffer, function(i, d) {
                    processMessage(d);
                });

            }
        });
    }

    function setLastLine(sender, text) {
        const senderColor = hashToColor(stringToHash(sender));
        const messageColor = "rgb(0, 255, 0)";
        $('#messages p:last-child').html(`<span style="color:${senderColor}">${sender}:</span> <span style="color:${messageColor}">${text}</span>`);
    }
    
    function appendToConsoleTextBox(sender, message) {
        const senderColor = hashToColor(stringToHash(sender));
        const messageColor = "rgb(0, 255, 0)";
        $('#messages').append(`<p><span style="color:${senderColor}">${sender}:</span> <span style="color:${messageColor}">${message}</span></p>`);
        $('#messages').scrollTop($('#messages')[0].scrollHeight);
    }
    
    
    $('#increaseFont').click(function() {
        let currentSize = parseInt($('#fontSize').val());
        let newSize = currentSize + 1;
        $('#fontSize').val(newSize + 'px');
        $('#messages').css('font-size', newSize + 'px');
    });

    $('#decreaseFont').click(function() {
        let currentSize = parseInt($('#fontSize').val());
        if (currentSize > 1) {
            let newSize = currentSize - 1;
            $('#fontSize').val(newSize + 'px');
            $('#messages').css('font-size', newSize + 'px');
        }
    });

    function stringToHash(str) {
        let hash = 0;
        for (let i = 0; i < str.length; i++) {
            const char = str.charCodeAt(i);
            hash = ((hash << 5) - hash) + char;
            hash = hash & hash;
        }
        return hash;
    }

    function hashToColor(hash) {
        const red = (hash & 0xFF0000) >> 16;
        const green = (hash & 0x00FF00) >> 8;
        const blue = hash & 0x0000FF;
        return `#${red.toString(16).padStart(2, '0')}${green.toString(16).padStart(2, '0')}${blue.toString(16).padStart(2, '0')}`;
    }
    


    function processSystemMessage(line) {
        lastHeartbeatTime = Date.now();
        $('#connectionStatus').removeClass('bg-danger').removeClass('bg-warning').addClass('bg-success');
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
        } else if (line.startsWith('%%%_MOTOR_A_RELAY_STATE:')) {
            if (line.split(':')[1].trim() === "ON") {
                setButtonColorForMotorA("green");
            } else if (line.split(':')[1].trim() === "OFF") {
                setButtonColorForMotorA("red");
            }
        } else if (line.startsWith('%%%_MOTOR_B_RELAY_STATE:')) {
            if (line.split(':')[1].trim() === "ON") {
                setButtonColorForMotorB("green");
            } else if (line.split(':')[1].trim() === "OFF") {
                setButtonColorForMotorB("red");
            }
        }

    }

    $('#incrementButton').click(function() {
        let valueBox = $('#valueBox');
        valueBox.val(parseInt(valueBox.val()) + 1);
    });

    $('#decrementButton').click(function() {
        let valueBox = $('#valueBox');
        let currentValue = parseInt(valueBox.val());
        if(currentValue > 0) { 
            valueBox.val(currentValue - 1);
        }
    });

    $('#setButton').click(function() {
        let value = parseInt($('#valueBox').val());
        ws.send("%%%_SERVER:HEARTBEAT_INTERVAL:" + value);
    });


    $('#sendButton').click(function() {
        const command = $('#command').val().trim();
        if (command) {
            ws.send(command);
            $('#command').val('');
        }
    });

    $('#uploadButton').click(async function() {
        const file = $('#gcodeFile')[0].files[0];
        if (file) {
            console.debug("File selected for processing.");
            const reader = new FileReader();
            reader.onload = async function(e) {
                const lines = e.target.result.split('\n');
                await sendLinesToServer(lines);
            };
            reader.readAsText(file);
        } else {
            console.debug("No file selected.");
        }
    });
    
    async function sendLinesToServer(lines) {
        for (let line of lines) {
            line = line.trim();
    
            if (line.startsWith("#") || line.startsWith("//")) {
                console.debug(`Skipping comment line: ${line}`);
                continue; // Skip comments
            }
    
            // Check if line matches "D[value]" pattern
            const delayMatch = line.match(/^D(\d+)$/);
            if (delayMatch) {
                const delayValue = parseInt(delayMatch[1], 10);
                console.debug(`Delay detected: ${delayValue}ms. Delaying...`);
                await delay(delayValue);
                console.debug(`Delay for ${delayValue}ms completed.`);
            } else if (line) {
                console.debug(`Sending line to server: ${line}`);
                ws.send(line);
            }
        }
    }
    
    function delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
    

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
        ws.send('EH');
    });


    $('[data-motor="A"]').click(function() {
        ws.send('FT');
    });

    $('[data-motor="B"]').click(function() {
        ws.send('GT');
    });



});

const FIRE_gcode = "FO;GO;D1500;AS0;BS20;CS100;DS100;D5000;AS20;BS0;D1000;CS0;DS0;AS0;BS20;FF;GF"

function setButtonColorForMotorA(color) {
    const btnA = $('[data-motor="A"]');
    if (color === 'green') {
        setGreen(btnA);
    } else if (color === 'red') {
        setRed(btnA);
    }
}

function setButtonColorForMotorB(color) {
    const btnB = $('[data-motor="B"]');
    if (color === 'green') {
        setGreen(btnB);
    } else if (color === 'red') {
        setRed(btnB);
    }
}

function setGreen(btn) {
    btn.css('background-color', 'green');
}

function setRed(btn) {
    btn.css('background-color', 'red');
}

let keysPressed = { left: false, up: false, right: false, down: false };

document.addEventListener("keydown", function(event) {
    let key;
    switch (event.key) {
        case "a":  // WASD - Left
            key = "left";
            break;
        case "w":  // WASD - Up
            key = "up";
            break;
        case "d":  // WASD - Right
            key = "right";
            break;
        case "s":  // WASD - Down
            key = "down";
            break;
    }

    if (key && !keysPressed[key]) {
        keysPressed[key] = true; // Mark the key as pressed
        document.getElementById(key).classList.add('pressed');
        sendKeyCommand(key);
    }
});

document.addEventListener("keyup", function(event) {
    let key;
    switch (event.key) {
        case "a":  // WASD - Left
            key = "left";
            break;
        case "w":  // WASD - Up
            key = "up";
            break;
        case "d":  // WASD - Right
            key = "right";
            break;
        case "s":  // WASD - Down
            key = "down";
            break;
    }

    if (key) {
        keysPressed[key] = false; // Reset the key state to not pressed
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
