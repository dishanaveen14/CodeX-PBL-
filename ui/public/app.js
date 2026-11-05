/**
 * Client-side Controller for POSIX IPC Simulator.
 * Synchronizes user inputs and streams execution logs vertically.
 */

let lastLogCount = 0;
let isInitialLoad = true;

const PROCESS_ROLES = {
    PIPES: "[PROCESS ROLES & DATA FLOW: POSIX PIPES]\n• Process A (Parent): Creates pipe, forks child, closes write fd[1], reads sum from fd[0].\n• Process B (Child): Closes read fd[0], transmits integer payload into write fd[1], exits.\n• IPC Channel: Unidirectional kernel FIFO byte-stream pipe ring-buffer.",
    MQ: "[PROCESS ROLES & DATA FLOW: POSIX MESSAGE QUEUES]\n• Process A (Sender): Connects to /posix_ipc_prio_queue, sets priority, calls mq_send().\n• Process B (Receiver): Listens on queue, calls mq_receive(), dequeues highest-priority message first.\n• IPC Channel: POSIX kernel priority queue structure.",
    SOCKETS: "[PROCESS ROLES & DATA FLOW: STREAM SOCKETS]\n• Process A (TCP Server): Binds 127.0.0.1:9099, accepts client connection, returns ACK.\n• Process B (TCP Client): Transmits query stream over TCP socket, receives response.\n• IPC Channel: Full-duplex loopback stream socket (AF_INET / SOCK_STREAM).",
    SHM: "[PROCESS ROLES & DATA FLOW: POSIX SHARED MEMORY]\n• Process A (Writer): Creates shm object via shm_open(), resizes with ftruncate(), maps with mmap(), writes data.\n• Process B (Reader): Opens shm object with shm_open(), maps with mmap(), reads directly from physical RAM.\n• IPC Channel: Zero-copy physical RAM pages shared across address spaces.",
    SEM: "[PROCESS ROLES & DATA FLOW: POSIX SEMAPHORES]\n• Concurrent Threads: 4 worker threads performing 25,000 parallel increments on shared counter.\n• Synchronization: sem_wait() (lock) before critical section; sem_post() (unlock) on exit.\n• IPC Channel: POSIX counting semaphore (sem_t) kernel primitive.",
    MUTEX: "[PROCESS ROLES & DATA FLOW: POSIX MUTEX LOCKS]\n• Concurrent Threads: 4 bank transaction threads performing parallel deposits & withdrawals.\n• Synchronization: pthread_mutex_lock() ensures mutual exclusion, preventing balance corruption.\n• IPC Channel: POSIX mutual exclusion lock (pthread_mutex_t)."
};

async function runMech(mech, arg1Id, arg2Id) {
    const statEl = document.getElementById(`stat-${mech}`);
    if (statEl) {
        statEl.textContent = 'Running...';
        statEl.className = 'status-indicator running';
    }

    if (PROCESS_ROLES[mech]) {
        appendTerminalOutput(`\n${PROCESS_ROLES[mech]}`);
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
