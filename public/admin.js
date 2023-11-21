$(document).ready(function() {
    const wsURL = (window.location.protocol === "https:" ? "wss://" : "ws://") + window.location.host + "/admin/ws";
    const ws = new WebSocket(wsURL);

    ws.onopen = function() {
        console.log("WebSocket connection opened");
    };

    ws.onmessage = function(event) {
        const data = JSON.parse(event.data);
        const tbody = $("#connectionsList tbody");
        
        // Clear existing rows
        tbody.empty();
        
        // Iterate through the clientInfos map by IP addresses
        for (let ip in data.clients) {
            const client = data.clients[ip];
            const row = `
                <tr>
                    <td>${client.ip}</td>
                    <td>${client.windowLocation}</td>
                    <td>${client.sessions}</td>
                    <td>${client.device}</td>
                    <td>${client.browser}</td>
                    <td>${client.os}</td>
                    <td>${client.country}</td>
                    <td>${client.city}</td>
                    <td><button class="btn btn-primary">Placeholder</button></td>
                </tr>
            `;
            tbody.append(row);
        }
    };
    

    ws.onerror = function(error) {
        console.log(`WebSocket Error: ${error}`);
    };

    ws.onclose = function() {
        console.log("WebSocket connection closed");
    };

});
