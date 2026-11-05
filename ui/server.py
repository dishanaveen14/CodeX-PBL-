"""
UI Server and REST Bridge for POSIX IPC Mechanism Demonstrator.
Serves static frontend assets and routes commands to the integrated C binary.
"""

import http.server
import socketserver
import json
import subprocess
import os
import sys

PORT = 8080
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
UI_DIR = os.path.join(os.path.dirname(__file__), 'public')
LOG_PATH = os.path.join(BASE_DIR, 'sim.log')

# Detect binary name
BIN_NAME = 'ipc_project.exe' if os.name == 'nt' else 'ipc_project'
BIN_PATH = os.path.join(BASE_DIR, BIN_NAME)

def append_to_log(text):
    try:
        with open(LOG_PATH, 'a', encoding='utf-8') as f:
            f.write(text + "\n")
    except Exception as e:
        print(f"Log write error: {e}", file=sys.stderr)

def execute_ipc_command(mech, *args):
    if not os.path.exists(BIN_PATH):
        err = f"Error: Compiled executable not found at {BIN_PATH}. Please compile with 'make' or gcc."
        append_to_log(err)
        return err

    cmd = [BIN_PATH, "--cmd", mech] + list(args)
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=10
        )
        output = proc.stdout if proc.stdout else proc.stderr
        append_to_log(output.strip())
        return output.strip()
    except Exception as e:
        err = f"Execution failed: {str(e)}"
        append_to_log(err)
        return err

class APIHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=UI_DIR, **kwargs)

    def do_GET(self):
        if self.path == '/api/logs':
            logs = []
            if os.path.exists(LOG_PATH):
                with open(LOG_PATH, 'r', encoding='utf-8', errors='ignore') as f:
                    all_lines = f.readlines()
                    logs = [line.rstrip() for line in all_lines[-80:]]
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({"logs": logs}).encode('utf-8'))
        else:
            super().do_GET()

    def do_POST(self):
        if self.path == '/api/logs/clear':
            if os.path.exists(LOG_PATH):
                with open(LOG_PATH, 'w', encoding='utf-8') as f:
                    f.truncate(0)
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"status": "cleared"}).encode('utf-8'))

        elif self.path == '/api/run':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)
            try:
                data = json.loads(post_data.decode('utf-8'))
            except Exception:
                data = {}

            mech = data.get('mechanism', 'ALL')
            arg1 = str(data.get('arg1', ''))
            arg2 = str(data.get('arg2', ''))

            output = execute_ipc_command(mech, arg1, arg2)

            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({
                "mechanism": mech,
                "output": output
            }).encode('utf-8'))
        else:
            self.send_response(404)
            self.end_headers()

def run_server():
    socketserver.TCPServer.allow_reuse_address = True
    print(f"[+] Starting UI Server at http://localhost:{PORT}")
    print(f"[+] Connected to IPC C-binary at {BIN_PATH}")
    with socketserver.TCPServer(("", PORT), APIHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n[-] UI Server stopping.")

if __name__ == '__main__':
    run_server()
