const express = require('express');
const http = require('http');
const WebSocket = require('ws');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ noServer: true });

// Serve HTML file
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});

// Handle WebSocket connections
wss.on('connection', (ws) => {
  ws.on('message', (message) => {
    // Broadcast the message to all other clients
    wss.clients.forEach((client) => {
      if (client !== ws && client.readyState === WebSocket.OPEN) {
        client.send(message);
      }
    });
  });
});

// Integrate WebSocket server with HTTP server
server.on('upgrade', (request, socket, head) => {
  wss.handleUpgrade(request, socket, head, (ws) => {
    wss.emit('connection', ws, request);
  });
});

// Start the server
const port = 3000;
server.listen(port, () => {
  console.log(`Server is listening on http://localhost:${port}`);
});
