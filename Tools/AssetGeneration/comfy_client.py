"""
comfy_client.py - Minimal ComfyUI HTTP/WebSocket API client for the
Emberbound Isekai asset-generation pipeline.

Responsibilities:
  - Probe the ComfyUI server (/system_stats).
  - Discover available checkpoints via /object_info/CheckpointLoaderSimple
    and confirm a specific checkpoint is loadable.
  - Submit a workflow (API-format node graph) to /prompt.
  - Wait for completion via the /ws WebSocket feed, with a polling
    /history/{prompt_id} fallback if the socket drops or never signals.
  - Fetch the resulting PNG bytes via /view.
  - Simple timeout + one-retry-with-simplified-prompt behavior lives in
    generate_assets.py (this module just exposes the primitives + a
    convenience `generate_image` that a caller can retry itself).

No Unreal Engine dependency. No import of `unreal`. Safe to run standalone.
"""

from __future__ import annotations

import copy
import json
import time
import uuid
import urllib.request
import urllib.parse
import urllib.error
from dataclasses import dataclass, field
from typing import Any, Optional

import requests

try:
    import websocket  # websocket-client package
    HAVE_WEBSOCKET = True
except ImportError:
    HAVE_WEBSOCKET = False


class ComfyError(RuntimeError):
    """Raised for any unrecoverable ComfyUI API failure."""


@dataclass
class GenerationResult:
    ok: bool
    prompt_id: Optional[str] = None
    images: list = field(default_factory=list)  # list of {filename, subfolder, type}
    error: Optional[str] = None
    elapsed_sec: float = 0.0


class ComfyClient:
    def __init__(self, base_url: str = "http://127.0.0.1:8188", timeout: float = 120.0):
        self.base_url = base_url.rstrip("/")
        self.timeout = timeout
        self.client_id = str(uuid.uuid4())

    # ------------------------------------------------------------------
    # Server / model probing
    # ------------------------------------------------------------------

    def probe_server(self) -> dict:
        """GET /system_stats. Raises ComfyError if server is unreachable."""
        try:
            resp = requests.get(f"{self.base_url}/system_stats", timeout=10)
            resp.raise_for_status()
            return resp.json()
        except Exception as exc:  # noqa: BLE001
            raise ComfyError(f"ComfyUI server not reachable at {self.base_url}: {exc}") from exc

    def list_checkpoints(self) -> list[str]:
        """GET /object_info/CheckpointLoaderSimple -> list of checkpoint filenames."""
        try:
            resp = requests.get(f"{self.base_url}/object_info/CheckpointLoaderSimple", timeout=15)
            resp.raise_for_status()
            data = resp.json()
            ckpt_field = data["CheckpointLoaderSimple"]["input"]["required"]["ckpt_name"]
            # ComfyUI returns [[list_of_names], {meta}]
            names = ckpt_field[0]
            return list(names)
        except Exception as exc:  # noqa: BLE001
            raise ComfyError(f"Could not read checkpoint list: {exc}") from exc

    def ensure_checkpoint(self, name: str) -> str:
        """Confirm `name` is visible to CheckpointLoaderSimple. Returns the name if found."""
        names = self.list_checkpoints()
        if name in names:
            return name
        raise ComfyError(
            f"Checkpoint '{name}' not found via /object_info. Available: {names}"
        )

    # ------------------------------------------------------------------
    # Prompt submission
    # ------------------------------------------------------------------

    def queue_prompt(self, workflow: dict) -> str:
        """POST /prompt. Returns prompt_id."""
        payload = {"prompt": workflow, "client_id": self.client_id}
        data = json.dumps(payload).encode("utf-8")
        req = urllib.request.Request(
            f"{self.base_url}/prompt", data=data, headers={"Content-Type": "application/json"}
        )
        try:
            with urllib.request.urlopen(req, timeout=30) as resp:
                body = json.loads(resp.read())
        except urllib.error.HTTPError as exc:  # noqa: BLE001
            err_body = exc.read().decode("utf-8", errors="replace")
            raise ComfyError(f"/prompt HTTP {exc.code}: {err_body}") from exc
        except Exception as exc:  # noqa: BLE001
            raise ComfyError(f"/prompt submission failed: {exc}") from exc

        if "error" in body:
            raise ComfyError(f"/prompt rejected workflow: {body['error']}")
        prompt_id = body.get("prompt_id")
        if not prompt_id:
            raise ComfyError(f"/prompt response missing prompt_id: {body}")
        return prompt_id

    # ------------------------------------------------------------------
    # Waiting for completion
    # ------------------------------------------------------------------

    def wait_for_completion(self, prompt_id: str, timeout: float | None = None) -> dict:
        """
        Block until `prompt_id` finishes executing. Prefers the WebSocket
        event feed; falls back to polling /history/{prompt_id} if the
        socket is unavailable or misbehaves. Returns the /history entry
        for this prompt_id (contains 'outputs').
        """
        timeout = timeout if timeout is not None else self.timeout
        deadline = time.time() + timeout

        if HAVE_WEBSOCKET:
            try:
                return self._wait_via_websocket(prompt_id, deadline)
            except Exception:  # noqa: BLE001
                # Fall through to polling; the websocket path is best-effort.
                pass

        return self._wait_via_polling(prompt_id, deadline)

    def _wait_via_websocket(self, prompt_id: str, deadline: float) -> dict:
        ws_url = self.base_url.replace("http://", "ws://").replace("https://", "wss://")
        ws_url = f"{ws_url}/ws?clientId={self.client_id}"
        ws = websocket.create_connection(ws_url, timeout=10)
        try:
            while time.time() < deadline:
                remaining = max(1.0, deadline - time.time())
                ws.settimeout(min(remaining, 15.0))
                try:
                    raw = ws.recv()
                except Exception:  # noqa: BLE001 - timeout or transient drop
                    # Check history in case we missed the terminal message.
                    hist = self._peek_history(prompt_id)
                    if hist is not None:
                        return hist
                    continue

                if isinstance(raw, (bytes, bytearray)):
                    continue  # binary preview frame, ignore

                try:
                    msg = json.loads(raw)
                except json.JSONDecodeError:
                    continue

                mtype = msg.get("type")
                data = msg.get("data", {})

                if mtype == "execution_error" and data.get("prompt_id") == prompt_id:
                    raise ComfyError(f"Execution error: {data}")

                if mtype == "executing" and data.get("prompt_id") == prompt_id and data.get("node") is None:
                    # node == None signals this prompt_id finished executing.
                    hist = self._peek_history(prompt_id)
                    if hist is not None:
                        return hist
                    return self._wait_via_polling(prompt_id, deadline)

            raise ComfyError(f"Timed out waiting for prompt {prompt_id} via websocket")
        finally:
            try:
                ws.close()
            except Exception:  # noqa: BLE001
                pass

    def _peek_history(self, prompt_id: str) -> Optional[dict]:
        try:
            resp = requests.get(f"{self.base_url}/history/{prompt_id}", timeout=10)
            resp.raise_for_status()
            body = resp.json()
        except Exception:  # noqa: BLE001
            return None
        entry = body.get(prompt_id)
        if entry and entry.get("outputs"):
            return entry
        return None

    def _wait_via_polling(self, prompt_id: str, deadline: float) -> dict:
        while time.time() < deadline:
            entry = self._peek_history(prompt_id)
            if entry is not None:
                status = entry.get("status", {})
                if status.get("status_str") == "error":
                    raise ComfyError(f"Prompt {prompt_id} errored: {status}")
                return entry
            time.sleep(1.0)
        raise ComfyError(f"Timed out waiting for prompt {prompt_id} via polling")

    # ------------------------------------------------------------------
    # Image retrieval
    # ------------------------------------------------------------------

    def extract_images(self, history_entry: dict) -> list[dict]:
        """Pull {filename, subfolder, type} dicts out of a /history entry's outputs."""
        images = []
        outputs = history_entry.get("outputs", {})
        for _node_id, node_out in outputs.items():
            for img in node_out.get("images", []):
                images.append(img)
        return images

    def fetch_image_bytes(self, filename: str, subfolder: str = "", img_type: str = "output") -> bytes:
        params = urllib.parse.urlencode({"filename": filename, "subfolder": subfolder, "type": img_type})
        url = f"{self.base_url}/view?{params}"
        resp = requests.get(url, timeout=30)
        resp.raise_for_status()
        return resp.content

    # ------------------------------------------------------------------
    # High-level convenience
    # ------------------------------------------------------------------

    def generate_image(self, workflow: dict, timeout: float | None = None) -> GenerationResult:
        """
        Submit `workflow` and block until it completes (or errors/times
        out). Returns a GenerationResult; does not raise on generation
        failure (caller decides retry/fallback policy), but DOES raise
        ComfyError for connectivity failures during submission.
        """
        start = time.time()
        try:
            prompt_id = self.queue_prompt(workflow)
        except ComfyError as exc:
            return GenerationResult(ok=False, error=str(exc), elapsed_sec=time.time() - start)

        try:
            history_entry = self.wait_for_completion(prompt_id, timeout=timeout)
        except ComfyError as exc:
            return GenerationResult(ok=False, prompt_id=prompt_id, error=str(exc), elapsed_sec=time.time() - start)

        images = self.extract_images(history_entry)
        if not images:
            return GenerationResult(
                ok=False, prompt_id=prompt_id, error="No images in history outputs",
                elapsed_sec=time.time() - start,
            )

        return GenerationResult(ok=True, prompt_id=prompt_id, images=images, elapsed_sec=time.time() - start)


# ----------------------------------------------------------------------
# Workflow template helpers
# ----------------------------------------------------------------------

def load_workflow_template(path: str) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def build_workflow(
    template: dict,
    ckpt_name: str,
    positive: str,
    negative: str,
    width: int,
    height: int,
    seed: int,
    filename_prefix: str,
    steps: int = 25,
    cfg: float = 7.0,
    sampler_name: str = "euler",
    scheduler: str = "normal",
) -> dict:
    """Deep-copy `template` and overwrite the parameterized fields."""
    wf = copy.deepcopy(template)
    # Strip the doc-only key if present (not a valid ComfyUI node entry).
    wf.pop("_meta_note", None)

    wf["4"]["inputs"]["ckpt_name"] = ckpt_name
    wf["6"]["inputs"]["text"] = positive
    wf["7"]["inputs"]["text"] = negative
    wf["5"]["inputs"]["width"] = width
    wf["5"]["inputs"]["height"] = height
    wf["3"]["inputs"]["seed"] = seed
    wf["3"]["inputs"]["steps"] = steps
    wf["3"]["inputs"]["cfg"] = cfg
    wf["3"]["inputs"]["sampler_name"] = sampler_name
    wf["3"]["inputs"]["scheduler"] = scheduler
    wf["9"]["inputs"]["filename_prefix"] = filename_prefix
    return wf


if __name__ == "__main__":
    # Quick standalone smoke test: probe server + list checkpoints.
    client = ComfyClient()
    stats = client.probe_server()
    print("Server OK:", stats.get("system", {}).get("comfyui_version"))
    ckpts = client.list_checkpoints()
    print("Checkpoints:", ckpts)
