# https://github.com/ultraslayyy/I-am-bored/tree/info/code-runner/src/sdks/runner-sdk.py
# Please don't delete the above line, to credit me. But I can't stop you.
import socket, json, subprocess, sys, threading
from typing import Optional, List, Dict, Any, Union

TestCase = Dict[str, Any]
TestCaseOptions = Dict[str, Any]
RunResult = Dict[str, Any]

class RunnerClient:
    def __init__(self, host: str = 'localhost', port: int = 4000):
        self.host = host
        self.port = port
        self._server_process: subprocess.Popen | None = None

    def start_server(self, file_path: str = "runner-server.js"):
        if self._server_process is not None and self._server_process.poll() is None:
            print("Server is already running.")
            return
        
        self._server_process = subprocess.Popen(
            ["node", file_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            shell=False
        )

        def _print_stream(stream, is_err=False):
            for line in stream:
                if line:
                    print(line, end='', file=sys.stderr if is_err else sys.stdout)
        
        threading.Thread(target=_print_stream, args=(self._server_process.stdout,), daemon=True).start()
        threading.Thread(target=_print_stream, args=(self._server_process.stderr, True), daemon=True).start()

        print(f"Started server with PID {self._server_process.pid}")
    
    def stop_server(self):
        if self._server_process is not None and self._server_process.poll() is None:
            self._server_process.terminate()
            self._server_process.wait()
            print("Server stopped.")
        self._server_process = None

    def _send_request(self, payload: Dict[str, Any]) -> Union[RunResult, List[RunResult]]:
        with socket.create_connection((self.host, self.port)) as sock:
            sock.sendall((json.dumps(payload) + '\n').encode())
            data = b''
            while True:
                chunk = sock.recv(4096)
                if not chunk:
                    break
                data += chunk
                if b'\n' in data:
                    break
            response = json.loads(data.decode().strip())
            if not response.get("success", False):
                raise RuntimeError(response.get("error", "Unknown error"))
            return response["result"]

    def run_code_local(self, code: str, language: str, filenamePrefix: str, input: Optional[str] = '', test_cases: Optional[TestCaseOptions] = None) -> Union[RunResult, List[RunResult]]:
        payload = {
            "runner": "local",
            "code": code,
            "language": language
        }

        if test_cases:
            payload["testCases"] = test_cases
        else:
            payload["input"] = input
        
        if filenamePrefix:
            payload["filenamePrefix"] = filenamePrefix

        return self._send_request(payload)

    def run_code_docker(self, code: str, language: str, filenamePrefix: str, input: Optional[str] = '', test_cases: Optional[TestCaseOptions] = None) -> Union[RunResult, List[RunResult]]:
        payload = {
            "runner": "docker",
            "code": code,
            "language": language
        }

        if test_cases:
            payload["testCases"] = test_cases
        else:
            payload["input"] = input

        if filenamePrefix:
            payload["filenamePrefix"] = filenamePrefix

        return self._send_request(payload)