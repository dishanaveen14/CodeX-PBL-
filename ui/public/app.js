/**
 * Client-side Controller for POSIX IPC Simulator.
 * Synchronizes user inputs and streams execution logs vertically.
 */

let lastLogCount = 0;
let isInitialLoad = true;

async function runMech(mech, arg1Id, arg2Id) {
    const statEl = document.getElementById(`stat-${mech}`);
    if (statEl) {
        statEl.textContent = 'Running...';
        statEl.className = 'status-indicator running';
    }

    const val1 = arg1Id ? document.getElementById(arg1Id)?.value || '' : '';
    const val2 = arg2Id ? document.getElementById(arg2Id)?.value || '' : '';

    try {
        const res = await fetch('/api/run', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                mechanism: mech,
                arg1: val1,
                arg2: val2
            })
        });

        const data = await res.json();
        if (data.output) {
            appendTerminalOutput(data.output);
        }

        if (statEl) {
            statEl.textContent = 'Completed';
            statEl.className = 'status-indicator success';
            setTimeout(() => {
                statEl.textContent = 'Ready';
                statEl.className = 'status-indicator';
            }, 3000);
        }
    } catch (err) {
        console.error('Request error:', err);
        appendTerminalOutput(`[-] Failed to connect to server: ${err.message}`);
        if (statEl) {
            statEl.textContent = 'Error';
            statEl.className = 'status-indicator error';
        }
    }
}

async function runAllTests() {
    appendTerminalOutput("\n========================================================");
    appendTerminalOutput(">>> DISPATCHING ALL 6 IPC DEMONSTRATIONS SEQUENTIALLY <<<");
    appendTerminalOutput("========================================================");
    await runMech('ALL');
}

function appendTerminalOutput(text) {
    const feed = document.getElementById('terminal-feed');
    if (!feed) return;

    const lines = text.split('\n');
    lines.forEach(line => {
        if (!line.trim()) return;
        const div = document.createElement('div');
        div.className = 'terminal-line';
        if (line.includes('===') || line.includes('>>>')) div.classList.add('terminal-header');
        else if (line.includes('[+]') || line.includes('SUCCESS') || line.includes('CORRECT') || line.includes('COMPLETE')) div.classList.add('terminal-success');
        else if (line.includes('[-]') || line.includes('CORRUPTION') || line.includes('ERROR') || line.includes('RACE CONDITION')) div.classList.add('terminal-error');
        else if (line.includes('[!]') || line.includes('WARN')) div.classList.add('terminal-warn');
        div.textContent = line;
        feed.appendChild(div);
    });

    feed.scrollTop = feed.scrollHeight;
}

async function clearLogs() {
    try {
        await fetch('/api/logs/clear', { method: 'POST' });
        const feed = document.getElementById('terminal-feed');
        if (feed) feed.innerHTML = '<div class="terminal-line text-muted">[System] Log cleared. Ready for next IPC run.</div>';
        lastLogCount = 0;
    } catch (err) {
        console.error(err);
    }
}

async function pollLogs() {
    try {
        const res = await fetch('/api/logs');
        const data = await res.json();
        const feed = document.getElementById('terminal-feed');

        if (data.logs && data.logs.length > 0) {
            document.getElementById('backend-status').textContent = 'C Engine: Active';
            document.getElementById('backend-status').className = 'badge live-badge';

            if (isInitialLoad) {
                feed.innerHTML = '';
                data.logs.forEach(line => {
                    const div = document.createElement('div');
                    div.className = 'terminal-line';
                    if (line.includes('===') || line.includes('>>>')) div.classList.add('terminal-header');
                    else if (line.includes('[+]') || line.includes('SUCCESS') || line.includes('CORRECT') || line.includes('COMPLETE')) div.classList.add('terminal-success');
                    else if (line.includes('[-]') || line.includes('CORRUPTION') || line.includes('ERROR') || line.includes('RACE CONDITION')) div.classList.add('terminal-error');
                    else if (line.includes('[!]') || line.includes('WARN')) div.classList.add('terminal-warn');
                    div.textContent = line;
                    feed.appendChild(div);
                });
                feed.scrollTop = feed.scrollHeight;
                isInitialLoad = false;
                lastLogCount = data.logs.length;
            }
        }
    } catch (err) {
        const status = document.getElementById('backend-status');
        if (status) {
            status.textContent = 'Backend: Standby';
            status.className = 'badge';
        }
    }
}

// Initial pull on page load
pollLogs();
setInterval(pollLogs, 3000);
