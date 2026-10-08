import http.server
import socketserver
import json
import subprocess
import threading
import urllib.parse
import os
import time

PORT = 8080
UI_DIR = os.path.join(os.path.dirname(__file__), 'public')
CORE_PATH = os.path.join(os.path.dirname(__file__), '..', 'build', 'core', 'core')
LOG_PATH = os.path.join(os.path.dirname(__file__), '..', 'sim.log')

# Start Core process
try:
    core_proc = subprocess.Popen(
        [CORE_PATH],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1  # line buffered
    )
except FileNotFoundError:
    print(f"Error: Could not find core executable at {CORE_PATH}. Did you run 'make'?")
    exit(1)

core_lock = threading.Lock()

def send_core_command(cmd):
    with core_lock:
        if core_proc.poll() is not None:
            return "ERROR Core process died"
        core_proc.stdin.write(cmd + "\n")
        core_proc.stdin.flush()
        response = core_proc.stdout.readline().strip()
        return response

class APIHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=UI_DIR, **kwargs)

    def do_GET(self):
        if self.path == '/api/state':
            res = send_core_command("GET_STATE")
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"state": res}).encode())
        elif self.path == '/api/logs':
            logs = []
            if os.path.exists(LOG_PATH):
                with open(LOG_PATH, 'r') as f:
                    # tail last 50 lines
                    all_lines = f.readlines()
                    logs = [line.strip() for line in all_lines[-50:]]
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"logs": logs}).encode())
        else:
            super().do_GET()

    def do_POST(self):
        if self.path == '/api/command':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data)
            cmd = data.get('command', '')
            
            res = send_core_command(cmd)
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"response": res}).encode())
        else:
            self.send_response(404)
            self.end_headers()

def run_server():
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), APIHandler) as httpd:
        print(f"UI Server serving at http://localhost:{PORT}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass
        finally:
            print("Shutting down UI Server and Core...")
            core_proc.terminate()

if __name__ == '__main__':
    run_server()
