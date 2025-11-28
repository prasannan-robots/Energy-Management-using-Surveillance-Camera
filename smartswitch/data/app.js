// ESP32 Smart Zone Switch - Web Application
// Canvas zone drawing, WebSocket communication, and UI management

// Global state
let zones = [];
let relays = [];
let config = {};
let ws = null;
let isDrawing = false;
let drawStart = { x: 0, y: 0 };
let drawCurrent = { x: 0, y: 0 };
let currentZoneId = 1;
let editingZone = null;
let detections = [];

// Canvas and context
const canvas = document.getElementById('video-canvas');
const ctx = canvas.getContext('2d');
const cameraSnapshot = document.getElementById('camera-snapshot');

// Zone colors
const zoneColors = ['#3b82f6', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6', '#ec4899'];

// Initialize application
document.addEventListener('DOMContentLoaded', () => {
    console.log('Smart Zone Switch initializing...');
    
    // Setup canvas event listeners
    setupCanvas();
    
    // Setup tab switching
    setupTabs();
    
    // Setup sliders
    setupSliders();
    
    // Load initial data
    loadConfig().then(() => {
        // Load camera snapshot after config is loaded
        updateCameraSnapshot();
    });
    loadZones();
    loadRelays();
    loadStatistics();
    loadSystemInfo();
    
    // Connect WebSocket
    connectWebSocket();
    
    // Start drawing loop
    setInterval(drawCanvas, 50); // 20 FPS redraw
    
    // Auto-refresh data
    setInterval(loadRelays, 2000);
    setInterval(loadStatistics, 5000);
    setInterval(loadSystemInfo, 10000);
    setInterval(checkCameraStatus, 3000); // Check camera status every 3 seconds
});

// WebSocket connection
function connectWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws`;
    
    ws = new WebSocket(wsUrl);
    
    ws.onopen = () => {
        console.log('WebSocket connected');
        updateStreamStatus(true);
    };
    
    ws.onclose = () => {
        console.log('WebSocket disconnected');
        updateStreamStatus(false);
        // Attempt reconnect after 3 seconds
        setTimeout(connectWebSocket, 3000);
    };
    
    ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        updateStreamStatus(false);
    };
    
    ws.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            handleWebSocketMessage(data);
        } catch (e) {
            console.error('Failed to parse WebSocket message:', e);
        }
    };
}

// Handle WebSocket messages
function handleWebSocketMessage(data) {
    if (data.type === 'detections') {
        detections = data.detections || [];
        document.getElementById('detection-count').textContent = data.count || 0;
    } else if (data.zones) {
        zones = data.zones;
        renderZoneList();
    }
}

// Canvas setup
function setupCanvas() {
    canvas.addEventListener('mousedown', onCanvasMouseDown);
    canvas.addEventListener('mousemove', onCanvasMouseMove);
    canvas.addEventListener('mouseup', onCanvasMouseUp);
    
    // Touch support
    canvas.addEventListener('touchstart', (e) => {
        e.preventDefault();
        const touch = e.touches[0];
        const rect = canvas.getBoundingClientRect();
        const mouseEvent = new MouseEvent('mousedown', {
            clientX: touch.clientX,
            clientY: touch.clientY
        });
        canvas.dispatchEvent(mouseEvent);
    });
    
    canvas.addEventListener('touchmove', (e) => {
        e.preventDefault();
        const touch = e.touches[0];
        const mouseEvent = new MouseEvent('mousemove', {
            clientX: touch.clientX,
            clientY: touch.clientY
        });
        canvas.dispatchEvent(mouseEvent);
    });
    
    canvas.addEventListener('touchend', (e) => {
        e.preventDefault();
        canvas.dispatchEvent(new MouseEvent('mouseup'));
    });
    
    // Add zone button
    document.getElementById('add-zone-btn').addEventListener('click', () => {
        isDrawing = false;
        openAddZoneModal();
    });
    
    document.getElementById('clear-drawing-btn').addEventListener('click', () => {
        isDrawing = false;
        drawCanvas();
    });
}

// Canvas mouse events
function onCanvasMouseDown(e) {
    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    
    drawStart = {
        x: (e.clientX - rect.left) * scaleX,
        y: (e.clientY - rect.top) * scaleY
    };
    
    // Check if clicking on existing zone
    const clickedZone = zones.find(zone => {
        return drawStart.x >= zone.x && drawStart.x <= zone.x + zone.width &&
               drawStart.y >= zone.y && drawStart.y <= zone.y + zone.height;
    });
    
    if (clickedZone) {
        // Edit existing zone
        editingZone = clickedZone;
        openEditZoneModal(clickedZone);
    } else {
        // Start drawing new zone
        isDrawing = true;
        drawCurrent = { ...drawStart };
    }
}

function onCanvasMouseMove(e) {
    if (!isDrawing) return;
    
    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    
    drawCurrent = {
        x: (e.clientX - rect.left) * scaleX,
        y: (e.clientY - rect.top) * scaleY
    };
}

function onCanvasMouseUp(e) {
    if (!isDrawing) return;
    
    isDrawing = false;
    
    // Calculate zone dimensions
    const x = Math.min(drawStart.x, drawCurrent.x);
    const y = Math.min(drawStart.y, drawCurrent.y);
    const width = Math.abs(drawCurrent.x - drawStart.x);
    const height = Math.abs(drawCurrent.y - drawStart.y);
    
    // Only create zone if it has meaningful size
    if (width > 20 && height > 20) {
        // Open modal to configure zone
        openAddZoneModal(x, y, width, height);
    }
}

// Draw canvas
function drawCanvas() {
    // Clear canvas
    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    
    // Draw placeholder for video feed
    ctx.fillStyle = '#333';
    ctx.font = '24px Arial';
    ctx.textAlign = 'center';
    ctx.fillText('Live Feed Placeholder', canvas.width / 2, canvas.height / 2);
    ctx.font = '14px Arial';
    ctx.fillText('(MJPEG stream will display here)', canvas.width / 2, canvas.height / 2 + 30);
    
    // Draw existing zones
    zones.forEach((zone, index) => {
        const color = zoneColors[index % zoneColors.length];
        
        // Zone rectangle
        ctx.strokeStyle = zone.active ? '#4ade80' : color;
        ctx.lineWidth = zone.active ? 4 : 2;
        ctx.strokeRect(zone.x, zone.y, zone.width, zone.height);
        
        // Zone fill
        ctx.fillStyle = zone.active ? 'rgba(74, 222, 128, 0.2)' : `${color}33`;
        ctx.fillRect(zone.x, zone.y, zone.width, zone.height);
        
        // Zone label
        ctx.fillStyle = '#fff';
        ctx.font = '14px Arial';
        ctx.textAlign = 'left';
        ctx.fillText(zone.name, zone.x + 5, zone.y + 20);
    });
    
    // Draw detections
    detections.forEach(det => {
        const x = det.x * canvas.width;
        const y = det.y * canvas.height;
        const w = det.width * canvas.width;
        const h = det.height * canvas.height;
        
        ctx.strokeStyle = '#4ade80';
        ctx.lineWidth = 2;
        ctx.strokeRect(x, y, w, h);
        
        // Confidence label
        ctx.fillStyle = '#4ade80';
        ctx.font = '12px Arial';
        ctx.fillText(`${(det.confidence * 100).toFixed(0)}%`, x + 2, y - 5);
    });
    
    // Draw current drawing rectangle
    if (isDrawing) {
        const x = Math.min(drawStart.x, drawCurrent.x);
        const y = Math.min(drawStart.y, drawCurrent.y);
        const width = Math.abs(drawCurrent.x - drawStart.x);
        const height = Math.abs(drawCurrent.y - drawStart.y);
        
        ctx.strokeStyle = '#f59e0b';
        ctx.lineWidth = 2;
        ctx.setLineDash([5, 5]);
        ctx.strokeRect(x, y, width, height);
        ctx.setLineDash([]);
        
        ctx.fillStyle = 'rgba(245, 158, 11, 0.2)';
        ctx.fillRect(x, y, width, height);
    }
}

// Tab switching
function setupTabs() {
    const tabs = document.querySelectorAll('.tab');
    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            const tabName = tab.dataset.tab;
            
            // Deactivate all tabs
            tabs.forEach(t => t.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            
            // Activate selected tab
            tab.classList.add('active');
            document.getElementById(`${tabName}-tab`).classList.add('active');
        });
    });
}

// Slider setup
function setupSliders() {
    const thresholdSlider = document.getElementById('detection-threshold');
    const thresholdValue = document.getElementById('threshold-value');
    
    thresholdSlider.addEventListener('input', (e) => {
        thresholdValue.textContent = parseFloat(e.target.value).toFixed(2);
    });
    
    const timeoutSlider = document.getElementById('global-timeout');
    const timeoutValue = document.getElementById('timeout-value');
    
    timeoutSlider.addEventListener('input', (e) => {
        timeoutValue.textContent = `${e.target.value}s`;
    });
}

// API: Load configuration
async function loadConfig() {
    try {
        const response = await fetch('/api/config');
        config = await response.json();
        
        document.getElementById('cctv-ip').value = config.cctvIP || '';
        document.getElementById('cctv-port').value = config.cctvPort || 8080;
        document.getElementById('stream-path').value = config.streamPath || '';
        document.getElementById('detection-threshold').value = config.detectionThreshold || 0.5;
        document.getElementById('threshold-value').textContent = (config.detectionThreshold || 0.5).toFixed(2);
        document.getElementById('global-timeout').value = config.globalTimeout || 5;
        document.getElementById('timeout-value').textContent = `${config.globalTimeout || 5}s`;
        document.getElementById('auto-relay-control').checked = config.autoRelayControl !== false;
        
        console.log('Configuration loaded');
    } catch (error) {
        console.error('Failed to load configuration:', error);
    }
}

// Update camera snapshot (refreshes every minute)
let snapshotInterval;

function updateCameraSnapshot() {
    const cameraSnapshot = document.getElementById('camera-snapshot');
    
    // Clear existing interval
    if (snapshotInterval) {
        clearInterval(snapshotInterval);
    }
    
    if (config.cctvIP && config.cctvPort) {
        // Function to fetch and display snapshot
        const fetchSnapshot = async () => {
            try {
                const timestamp = new Date().getTime();
                const snapshotUrl = `/api/camera/snapshot?t=${timestamp}`;
                console.log('Fetching camera snapshot...');
                
                // Test if the image loads
                const testImg = new Image();
                testImg.onload = () => {
                    cameraSnapshot.src = snapshotUrl;
                    cameraSnapshot.style.display = 'block';
                    console.log('Camera snapshot loaded successfully');
                };
                testImg.onerror = () => {
                    console.error('Failed to load camera snapshot');
                    cameraSnapshot.style.display = 'none';
                };
                testImg.src = snapshotUrl;
                
            } catch (error) {
                console.error('Error fetching camera snapshot:', error);
                cameraSnapshot.style.display = 'none';
            }
        };
        
        // Fetch initial snapshot
        fetchSnapshot();
        
        // Set up interval to refresh every minute (60000 ms)
        snapshotInterval = setInterval(fetchSnapshot, 60000);
        
    } else {
        cameraSnapshot.style.display = 'none';
        console.log('No camera configured');
    }
}

// API: Save configuration
async function saveSettings() {
    try {
        config.cctvIP = document.getElementById('cctv-ip').value;
        config.cctvPort = parseInt(document.getElementById('cctv-port').value);
        config.streamPath = document.getElementById('stream-path').value;
        config.detectionThreshold = parseFloat(document.getElementById('detection-threshold').value);
        config.globalTimeout = parseInt(document.getElementById('global-timeout').value);
        config.autoRelayControl = document.getElementById('auto-relay-control').checked;
        
        const response = await fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(config)
        });
        
        if (response.ok) {
            alert('✓ Settings saved successfully!');
            // Update camera snapshot with new settings
            updateCameraSnapshot();
        } else {
            alert('✗ Failed to save settings');
        }
    } catch (error) {
        console.error('Failed to save settings:', error);
        alert('✗ Error saving settings');
    }
}

// API: Load zones
async function loadZones() {
    try {
        const response = await fetch('/api/zones');
        const data = await response.json();
        zones = data.zones || [];
        renderZoneList();
        console.log(`Loaded ${zones.length} zones`);
    } catch (error) {
        console.error('Failed to load zones:', error);
    }
}

// Render zone list
function renderZoneList() {
    const list = document.getElementById('zone-list');
    list.innerHTML = '';
    
    if (zones.length === 0) {
        list.innerHTML = '<p style="text-align: center; color: #666;">No zones configured. Click "Draw Zone" to add one.</p>';
        return;
    }
    
    zones.forEach(zone => {
        const item = document.createElement('div');
        item.className = `zone-item ${zone.active ? 'active' : ''}`;
        
        const relayText = zone.relayPins.join(', ');
        
        item.innerHTML = `
            <div class="zone-info">
                <div class="zone-name">${zone.name}</div>
                <div class="zone-details">
                    Position: ${zone.x},${zone.y} | Size: ${zone.width}x${zone.height}<br>
                    Relays: GPIO ${relayText} | Timeout: ${zone.timeout}s
                </div>
            </div>
            <div class="zone-actions">
                <button class="btn-secondary" onclick="editZone(${zone.id})">Edit</button>
                <button class="btn-danger" onclick="deleteZone(${zone.id})">Delete</button>
            </div>
        `;
        
        list.appendChild(item);
    });
}

// API: Load relays
async function loadRelays() {
    try {
        const response = await fetch('/api/relays');
        const data = await response.json();
        relays = data.relays || [];
        renderRelayGrid();
        
        // Update active relay count
        const activeCount = relays.filter(r => r.active).length;
        document.getElementById('relay-count').textContent = activeCount;
        
        // Update GPIO summary
        const gpioSummary = document.getElementById('gpio-summary');
        if (activeCount === 0) {
            gpioSummary.textContent = 'All LOW';
            gpioSummary.style.color = '#6b7280';
        } else if (activeCount === relays.length) {
            gpioSummary.textContent = 'All HIGH';
            gpioSummary.style.color = '#ef4444';
        } else {
            const highPins = relays.filter(r => r.active).map(r => r.pin).join(', ');
            gpioSummary.textContent = `${highPins} HIGH`;
            gpioSummary.style.color = '#f59e0b';
        }
    } catch (error) {
        console.error('Failed to load relays:', error);
    }
}

// Render relay grid
function renderRelayGrid() {
    const grid = document.getElementById('relay-grid');
    grid.innerHTML = '';
    
    if (relays.length === 0) {
        grid.innerHTML = '<p style="text-align: center; color: #666;">No relays configured</p>';
        return;
    }
    
    relays.forEach(relay => {
        const card = document.createElement('div');
        card.className = 'relay-card';
        
        // Determine physical GPIO state (HIGH/LOW)
        const isHigh = relay.active; // Assuming relayActiveHigh=true
        
        card.innerHTML = `
            <div class="gpio-status ${isHigh ? 'gpio-high' : 'gpio-low'}">
                ${isHigh ? 'HIGH' : 'LOW'}
            </div>
            <div class="relay-status ${relay.active ? 'active' : ''}">
                ${relay.active ? '⚡' : '○'}
            </div>
            <div><strong>GPIO ${relay.pin}</strong></div>
            <div style="font-size: 0.9rem; color: #666; margin: 0.5rem 0;">
                ${relay.active ? 'ACTIVE' : 'INACTIVE'}
            </div>
            <button class="btn-${relay.active ? 'danger' : 'success'}" 
                    onclick="toggleRelay(${relay.pin}, ${!relay.active})" 
                    style="width: 100%;">
                ${relay.active ? 'Turn OFF' : 'Turn ON'}
            </button>
        `;
        
        grid.appendChild(card);
    });
}

// API: Toggle relay
async function toggleRelay(pin, state) {
    try {
        const response = await fetch(`/api/relays/set?pin=${pin}&state=${state}`, {
            method: 'POST'
        });
        
        if (response.ok) {
            loadRelays(); // Refresh relay states
        } else {
            alert('Failed to toggle relay');
        }
    } catch (error) {
        console.error('Failed to toggle relay:', error);
        alert('Error toggling relay');
    }
}

// API: Emergency stop
async function emergencyStop() {
    if (!confirm('⚠️ This will immediately turn OFF all relays. Continue?')) {
        return;
    }
    
    try {
        const response = await fetch('/api/emergency-stop', {
            method: 'POST'
        });
        
        if (response.ok) {
            alert('✓ All relays disabled');
            loadRelays();
        } else {
            alert('✗ Emergency stop failed');
        }
    } catch (error) {
        console.error('Emergency stop failed:', error);
        alert('✗ Error during emergency stop');
    }
}

// API: Load statistics
async function loadStatistics() {
    try {
        const response = await fetch('/api/statistics');
        const data = await response.json();
        
        document.getElementById('total-detections').textContent = data.totalDetections || 0;
        document.getElementById('total-zones').textContent = zones.filter(z => z.active).length;
        
        // Zone statistics
        const statsDiv = document.getElementById('zone-stats');
        statsDiv.innerHTML = '';
        
        if (data.zones && data.zones.length > 0) {
            data.zones.forEach(zone => {
                const item = document.createElement('div');
                item.style.cssText = 'display: flex; justify-content: space-between; padding: 0.5rem 0; border-bottom: 1px solid #e5e5e5;';
                item.innerHTML = `
                    <span>${zone.name}</span>
                    <strong>${zone.detections} detections</strong>
                `;
                statsDiv.appendChild(item);
            });
        } else {
            statsDiv.innerHTML = '<p style="text-align: center; color: #666;">No statistics yet</p>';
        }
    } catch (error) {
        console.error('Failed to load statistics:', error);
    }
}

// API: Reset statistics
async function resetStatistics() {
    if (!confirm('Reset all statistics?')) {
        return;
    }
    
    try {
        const response = await fetch('/api/statistics/reset', {
            method: 'POST'
        });
        
        if (response.ok) {
            alert('✓ Statistics reset');
            loadStatistics();
        }
    } catch (error) {
        console.error('Failed to reset statistics:', error);
    }
}

// API: Load system info
async function loadSystemInfo() {
    try {
        const response = await fetch('/api/system');
        const data = await response.json();
        
        document.getElementById('sys-ip').textContent = data.ipAddress || '-';
        document.getElementById('sys-heap').textContent = `${(data.freeHeap / 1024).toFixed(1)} KB`;
        document.getElementById('sys-wifi').textContent = data.wifiRSSI || '-';
        document.getElementById('sys-model').textContent = data.modelInfo || '-';
    } catch (error) {
        console.error('Failed to load system info:', error);
    }
}

// API: Test CCTV connection
async function testConnection() {
    try {
        const statusDiv = document.getElementById('camera-status');
        statusDiv.style.display = 'block';
        statusDiv.style.background = '#fef3c7';
        statusDiv.style.color = '#92400e';
        statusDiv.textContent = '⏳ Testing connection...';
        
        const response = await fetch('/api/test-connection');
        const data = await response.json();
        
        if (data.success) {
            statusDiv.style.background = '#d1fae5';
            statusDiv.style.color = '#065f46';
            statusDiv.textContent = '✓ Connection successful!';
            setTimeout(() => { statusDiv.style.display = 'none'; }, 3000);
        } else {
            statusDiv.style.background = '#fee2e2';
            statusDiv.style.color = '#991b1b';
            statusDiv.textContent = `✗ Connection failed: ${data.message}`;
        }
    } catch (error) {
        console.error('Connection test failed:', error);
        const statusDiv = document.getElementById('camera-status');
        statusDiv.style.display = 'block';
        statusDiv.style.background = '#fee2e2';
        statusDiv.style.color = '#991b1b';
        statusDiv.textContent = '✗ Error testing connection';
    }
}

// Start camera streaming
async function startCamera() {
    try {
        const statusDiv = document.getElementById('camera-status');
        statusDiv.style.display = 'block';
        statusDiv.style.background = '#fef3c7';
        statusDiv.style.color = '#92400e';
        statusDiv.textContent = '⏳ Starting camera...';
        
        const response = await fetch('/api/camera/start', { method: 'POST' });
        const data = await response.json();
        
        if (data.success) {
            statusDiv.style.background = '#d1fae5';
            statusDiv.style.color = '#065f46';
            statusDiv.textContent = '✓ Camera started!';
            
            document.getElementById('start-camera-btn').style.display = 'none';
            document.getElementById('stop-camera-btn').style.display = 'block';
            
            // Reload camera snapshot
            updateCameraSnapshot();
            
            setTimeout(() => { statusDiv.style.display = 'none'; }, 3000);
        } else {
            statusDiv.style.background = '#fee2e2';
            statusDiv.style.color = '#991b1b';
            statusDiv.textContent = `✗ Failed: ${data.message}`;
        }
    } catch (error) {
        console.error('Failed to start camera:', error);
        const statusDiv = document.getElementById('camera-status');
        statusDiv.style.display = 'block';
        statusDiv.style.background = '#fee2e2';
        statusDiv.style.color = '#991b1b';
        statusDiv.textContent = '✗ Error starting camera';
    }
}

// Stop camera streaming
async function stopCamera() {
    try {
        const statusDiv = document.getElementById('camera-status');
        statusDiv.style.display = 'block';
        statusDiv.style.background = '#fef3c7';
        statusDiv.style.color = '#92400e';
        statusDiv.textContent = '⏳ Stopping camera...';
        
        const response = await fetch('/api/camera/stop', { method: 'POST' });
        const data = await response.json();
        
        if (data.success) {
            statusDiv.style.background = '#d1fae5';
            statusDiv.style.color = '#065f46';
            statusDiv.textContent = '✓ Camera stopped';
            
            document.getElementById('start-camera-btn').style.display = 'block';
            document.getElementById('stop-camera-btn').style.display = 'none';
            
            // Hide MJPEG stream
            mjpegStream.style.display = 'none';
            
            setTimeout(() => { statusDiv.style.display = 'none'; }, 3000);
        } else {
            statusDiv.style.background = '#fee2e2';
            statusDiv.style.color = '#991b1b';
            statusDiv.textContent = `✗ Failed: ${data.message}`;
        }
    } catch (error) {
        console.error('Failed to stop camera:', error);
        const statusDiv = document.getElementById('camera-status');
        statusDiv.style.display = 'block';
        statusDiv.style.background = '#fee2e2';
        statusDiv.style.color = '#991b1b';
        statusDiv.textContent = '✗ Error stopping camera';
    }
}

// Check camera status periodically
async function checkCameraStatus() {
    try {
        const response = await fetch('/api/camera/status');
        const data = await response.json();
        
        if (data.connected) {
            document.getElementById('start-camera-btn').style.display = 'none';
            document.getElementById('stop-camera-btn').style.display = 'block';
            updateCameraSnapshot();
        } else {
            document.getElementById('start-camera-btn').style.display = 'block';
            document.getElementById('stop-camera-btn').style.display = 'none';
            cameraSnapshot.style.display = 'none';
        }
    } catch (error) {
        console.error('Failed to check camera status:', error);
    }
}

// Modal management
function openAddZoneModal(x = 10, y = 10, width = 100, height = 100) {
    editingZone = { x, y, width, height };
    document.getElementById('zone-name').value = `Zone ${currentZoneId}`;
    document.getElementById('zone-relays').value = '12';
    document.getElementById('zone-timeout').value = config.globalTimeout || 5;
    document.getElementById('zone-modal').classList.add('active');
}

function openEditZoneModal(zone) {
    editingZone = zone;
    document.getElementById('zone-name').value = zone.name;
    document.getElementById('zone-relays').value = zone.relayPins.join(',');
    document.getElementById('zone-timeout').value = zone.timeout;
    document.getElementById('zone-modal').classList.add('active');
}

function closeZoneModal() {
    document.getElementById('zone-modal').classList.remove('active');
    editingZone = null;
}

// API: Save zone
async function saveZone() {
    const name = document.getElementById('zone-name').value.trim();
    const relayText = document.getElementById('zone-relays').value.trim();
    const timeout = parseInt(document.getElementById('zone-timeout').value);
    
    if (!name || !relayText) {
        alert('Please fill in all fields');
        return;
    }
    
    const relayPins = relayText.split(',').map(s => parseInt(s.trim())).filter(n => !isNaN(n));
    
    if (relayPins.length === 0) {
        alert('Please enter at least one valid GPIO pin number');
        return;
    }
    
    const zone = {
        id: editingZone.id || currentZoneId++,
        name,
        x: Math.round(editingZone.x),
        y: Math.round(editingZone.y),
        width: Math.round(editingZone.width),
        height: Math.round(editingZone.height),
        relayPins,
        timeout
    };
    
    try {
        const endpoint = editingZone.id ? '/api/zones/update' : '/api/zones/add';
        const response = await fetch(endpoint, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(zone)
        });
        
        if (response.ok) {
            alert('✓ Zone saved!');
            closeZoneModal();
            loadZones();
        } else {
            alert('✗ Failed to save zone');
        }
    } catch (error) {
        console.error('Failed to save zone:', error);
        alert('✗ Error saving zone');
    }
}

// Edit zone
function editZone(zoneId) {
    const zone = zones.find(z => z.id === zoneId);
    if (zone) {
        openEditZoneModal(zone);
    }
}

// API: Delete zone
async function deleteZone(zoneId) {
    if (!confirm('Delete this zone?')) {
        return;
    }
    
    try {
        const response = await fetch(`/api/zones/delete?id=${zoneId}`, {
            method: 'DELETE'
        });
        
        if (response.ok) {
            alert('✓ Zone deleted');
            loadZones();
        } else {
            alert('✗ Failed to delete zone');
        }
    } catch (error) {
        console.error('Failed to delete zone:', error);
        alert('✗ Error deleting zone');
    }
}

// Update status indicators
function updateStreamStatus(connected) {
    const statusDot = document.getElementById('stream-status');
    const statusText = document.getElementById('stream-text');
    
    if (connected) {
        statusDot.classList.remove('offline');
        statusText.textContent = 'Connected';
    } else {
        statusDot.classList.add('offline');
        statusText.textContent = 'Disconnected';
    }
}
