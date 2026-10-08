let isExecuting = false;

async function apiCall(command) {
    try {
        const res = await fetch('/api/command', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ command })
        });
        const data = await res.json();
        return data.response;
    } catch (err) {
        console.error(err);
        return 'ERROR Server unreachable';
    }
}

function showFeedback(elementId, msg, isError = false) {
    const el = document.getElementById(elementId);
    el.textContent = msg;
    el.className = 'feedback ' + (isError ? 'error' : 'success');
}

async function updateLogs() {
    try {
        const res = await fetch('/api/logs');
        const data = await res.json();
        if (data.logs) {
            const feed = document.getElementById('log-feed');
            const wasAtBottom = feed.scrollTop >= (feed.scrollHeight - feed.clientHeight - 10);
            
            feed.innerHTML = data.logs.map(log => {
                let cls = 'log-info';
                if (log.includes('[WARN')) cls = 'log-warn';
                if (log.includes('[ERROR')) cls = 'log-error';
                return `<div class="log-entry ${cls}">${log}</div>`;
            }).join('');
            
            if (wasAtBottom) feed.scrollTop = feed.scrollHeight;
            
            document.getElementById('logger-status').textContent = 'Logger: Active';
            document.getElementById('logger-status').className = 'badge ok';
        } else {
            document.getElementById('logger-status').textContent = 'Logger: Idle';
            document.getElementById('logger-status').className = 'badge';
        }
    } catch(err) {
        document.getElementById('logger-status').textContent = 'Logger: Error';
        document.getElementById('logger-status').className = 'badge err';
    }
}

async function execIPC() {
    if (isExecuting) return;
    
    const n1 = document.getElementById('ipc-num1').value;
    const n2 = document.getElementById('ipc-num2').value;
    
    if (n1 === '' || n2 === '') {
        showFeedback('ipc-feedback', 'Please enter both numbers.', true);
        return;
    }
    
    isExecuting = true;
    document.getElementById('btn-add').disabled = true;
    document.getElementById('btn-add').style.opacity = '0.5';
    document.getElementById('ipc-status').textContent = 'Executing POSIX fork and pipe...';
    showFeedback('ipc-feedback', '');
    
    const res = await apiCall(`IPC_ADD ${n1} ${n2}`);
    
    if (res.startsWith('ERROR')) {
        showFeedback('ipc-feedback', res, true);
        document.getElementById('ipc-status').textContent = 'Failed to execute IPC.';
    } else {
        const sum = res.split(' ')[1];
        showFeedback('ipc-feedback', `Success! Computed Sum: ${sum}`);
        document.getElementById('ipc-status').textContent = `Process 1 received ${n2} from Process 2 and computed: ${n1} + ${n2} = ${sum}`;
    }
    
    isExecuting = false;
    document.getElementById('btn-add').disabled = false;
    document.getElementById('btn-add').style.opacity = '1';
}

// Initial pull and periodic refresh
updateLogs();
setInterval(updateLogs, 1000);
