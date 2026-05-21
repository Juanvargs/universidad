import argparse
import json
import mimetypes
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import unquote

from dashboard.controller import DashboardController


ROOT = Path(__file__).resolve().parent
STATIC_ROOT = ROOT / "static"


class DashboardRequestHandler(BaseHTTPRequestHandler):
    controller = DashboardController()

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            self._send_file(STATIC_ROOT / "index.html")
            return
        if self.path == "/api/status":
            self._send_json(self.controller.status_payload())
            return
        if self.path == "/api/snapshot":
            self._send_json(self.controller.snapshot_payload())
            return
        if self.path.startswith("/static/"):
            relative_path = unquote(self.path.removeprefix("/static/"))
            self._send_file(STATIC_ROOT / relative_path)
            return
        self._send_error(404, "Ruta no encontrada.")

    def do_HEAD(self):
        if self.path in ("/", "/index.html"):
            self._send_file(STATIC_ROOT / "index.html", include_body=False)
            return
        if self.path == "/api/status":
            self._send_json(self.controller.status_payload(), include_body=False)
            return
        if self.path == "/api/snapshot":
            self._send_json(self.controller.snapshot_payload(), include_body=False)
            return
        if self.path.startswith("/static/"):
            relative_path = unquote(self.path.removeprefix("/static/"))
            self._send_file(STATIC_ROOT / relative_path, include_body=False)
            return
        self._send_error(404, "Ruta no encontrada.", include_body=False)

    def do_POST(self):
        try:
            if self.path == "/api/start":
                self._send_json(self.controller.start(self._read_json()))
                return
            if self.path == "/api/stop":
                self._send_json(self.controller.stop())
                return
            self._send_error(404, "Ruta no encontrada.")
        except Exception as exc:
            self._send_json({"ok": False, "error": str(exc)}, status=400)

    def log_message(self, fmt, *args):
        print(f"[dashboard] {self.address_string()} - {fmt % args}")

    def _read_json(self):
        length = int(self.headers.get("Content-Length", "0"))
        if length <= 0:
            return {}
        raw = self.rfile.read(length).decode("utf-8")
        return json.loads(raw)

    def _send_json(self, payload, status=200, include_body=True):
        data = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        if include_body:
            self.wfile.write(data)

    def _send_file(self, path, include_body=True):
        try:
            resolved = path.resolve()
            if STATIC_ROOT.resolve() not in resolved.parents and resolved != STATIC_ROOT.resolve():
                self._send_error(403, "Acceso denegado.")
                return
            if not resolved.is_file():
                self._send_error(404, "Archivo no encontrado.")
                return
            data = resolved.read_bytes()
            content_type = mimetypes.guess_type(str(resolved))[0] or "application/octet-stream"
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            if include_body:
                self.wfile.write(data)
        except OSError as exc:
            self._send_error(500, str(exc))

    def _send_error(self, status, message, include_body=True):
        self._send_json({"ok": False, "error": message}, status=status, include_body=include_body)


class DashboardHTTPServer(ThreadingHTTPServer):
    allow_reuse_address = True


def run_server(host="0.0.0.0", port=8000):
    server = DashboardHTTPServer((host, port), DashboardRequestHandler)
    print(f"Dashboard SDR disponible en http://localhost:{port}")
    print("Presiona Ctrl+C para detener el servidor.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        DashboardRequestHandler.controller.shutdown()
        server.server_close()


def main():
    parser = argparse.ArgumentParser(description="Servidor local para el dashboard RTL-SDR.")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8000)
    args = parser.parse_args()
    run_server(args.host, args.port)


if __name__ == "__main__":
    main()
