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
    setTimeout(() => { if(el.textContent === msg) el.textContent = ''; }, 3000);
}

async function updateState() {
    try {
        const res = await fetch('/api/state');
        const data = await res.json();
        if (data.state && data.state.startsWith('OK')) {
            document.getElementById('core-status').textContent = 'Core: Connected';
            document.getElementById('core-status').className = 'badge ok';
            
            // Parse state: OK R0=10 R1=3 R2=13 R3=7 STACK_DEPTH=0 QUEUE_COUNT=0
            const parts = data.state.substring(3).split(' ');
            parts.forEach(part => {
                const [k, v] = part.split('=');
                if (k === 'R2') document.getElementById('r2-val').textContent = v;
                if (k === 'R3') document.getElementById('r3-val').textContent = v;
                if (k === 'STACK_DEPTH') document.getElementById('stack-count').textContent = 'Depth: ' + v;
                if (k === 'QUEUE_COUNT') document.getElementById('queue-count').textContent = 'Count: ' + v;
            });
        } else {
            document.getElementById('core-status').textContent = 'Core: Offline';
            document.getElementById('core-status').className = 'badge err';
        }
    } catch(err) {
        document.getElementById('core-status').textContent = 'Core: Disconnected';
        document.getElementById('core-status').className = 'badge err';
    }
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

async function updateRegisters() {
    const r0 = document.getElementById('r0').value || 0;
    const r1 = document.getElementById('r1').value || 0;
    await apiCall(`SET_REG 0 ${r0}`);
    await apiCall(`SET_REG 1 ${r1}`);
    showFeedback('cpu-feedback', 'R0 and R1 updated');
    updateState();
}

async function execCPU(op, dst, srcA, srcB) {
    const res = await apiCall(`CPU ${op} ${dst} ${srcA} ${srcB}`);
    if (res.startsWith('ERROR')) {
        showFeedback('cpu-feedback', res, true);
    } else {
        showFeedback('cpu-feedback', `${op} executed successfully`);
        updateState();
    }
}

async function execMem(op) {
    const addr = document.getElementById('mem-addr').value || 0;
    const val = document.getElementById('mem-val').value || 0;
    
    let cmd = op === 'WRITE' ? `MEM_WRITE ${addr} ${val}` : `MEM_READ ${addr}`;
    const res = await apiCall(cmd);
    
    if (res.startsWith('ERROR')) {
        showFeedback('mem-feedback', res, true);
    } else {
        if (op === 'READ') {
            const readVal = res.split(' ')[1];
            showFeedback('mem-feedback', `Read Value: ${readVal}`);
        } else {
            showFeedback('mem-feedback', 'Write successful');
        }
    }
}

async function execDS(op) {
    let cmd = op;
    if (op === 'STACK_PUSH') cmd += ' ' + (document.getElementById('stack-val').value || 0);
    if (op === 'QUEUE_ENQ') cmd += ' ' + (document.getElementById('queue-val').value || 0);
    
    const res = await apiCall(cmd);
    if (res.startsWith('ERROR')) {
        showFeedback('ds-feedback', res, true);
    } else {
        if (op === 'STACK_POP' || op === 'QUEUE_DEQ') {
            const val = res.split(' ')[1];
            showFeedback('ds-feedback', `Dequeued/Popped: ${val}`);
        } else {
            showFeedback('ds-feedback', 'Operation successful');
        }
        updateState();
    }
}

async function execIPC() {
    const n1 = document.getElementById('ipc-num1').value || 7;
    const n2 = document.getElementById('ipc-num2').value || 5;
    document.getElementById('ipc-status').textContent = 'Executing fork/pipe...';
    
    const res = await apiCall(`IPC_ADD ${n1} ${n2}`);
    if (res.startsWith('ERROR')) {
        showFeedback('ipc-feedback', res, true);
        document.getElementById('ipc-status').textContent = 'Failed.';
    } else {
        showFeedback('ipc-feedback', 'IPC Demo completed. Check logs.');
        document.getElementById('ipc-status').textContent = `Sent inputs: ${n1} and ${n2}. See system log for result.`;
    }
}

// Initial pull and periodic refresh
updateState();
updateLogs();
setInterval(updateState, 2000);
setInterval(updateLogs, 1000);
