#!/usr/bin/env python3
"""ESPTool GUI

This tool provides a user-friendly GUI for common esptool operations including in assistance in flashing installation of ESP Rowing Monitor firmware.

Operations supported includes:
- Flash any compiled firmware to ESP32 devices (write-flash with auto-address mapping).
- Erase device flash and read device information (MAC, chip id).
- Auto-detect connected devices and probe to determine the chip variant.
- Manage precompiled ESP Rowing Monitor firmware automatically (download from a GitHub release and auto-map extracted files to the proper flash addresses).
- Support custom firmware directories (select a local directory containing compiled .bin files and auto-map them to the proper flash addresses).

The GUI is implemented with Tkinter and integrates the esptool library by
invoking `esptool.main()` in a redirected stdout/stderr context while
streaming output back into the Tk widgets for live feedback.
"""

from __future__ import annotations

import contextlib
import io
import json
import os
import re
import sys
import tempfile
import threading
import time
import tkinter as tk
import tkinter.font as tkfont
import urllib.request
import urllib.error
import zipfile
from pathlib import Path
from tkinter import filedialog, messagebox, scrolledtext, ttk
from typing import Any, Callable, Dict, List, Optional, Tuple, Set

import serial.tools.list_ports as list_ports
import esptool # type: ignore[import]
import ssl
import certifi

# ==================== CONFIGURATION CONSTANTS ====================

# Serial communication settings
DEFAULT_BAUD = 115200
DEFAULT_BAUD_FAST = 460800
AVAILABLE_BAUD_RATES = [
    "9600", "19200", "38400", "57600", "115200",
    "230400", "460800", "921600", "1500000"
]
DEFAULT_PROBE_TIMEOUT = 3.0

# UI layout settings
DEFAULT_WINDOW_WIDTH = 900
DEFAULT_WINDOW_HEIGHT = 720
SCROLL_THRESHOLD = 660  # Window height threshold for fixed vs dynamic text widget sizing
DEFAULT_TEXT_HEIGHT = 12  # Fixed lines for text widgets when height <= SCROLL_THRESHOLD
MIN_TEXT_HEIGHT = 5  # Minimum lines for text widgets when dynamically sizing
MAX_FILE_TREE_HEIGHT = 3  # Maximum files to display in tree

# Settings persistence
SETTINGS_FILENAME = ".esptool_gui_settings.json"

# Port refresh interval (milliseconds)
PORT_REFRESH_INTERVAL = 5000
INITIAL_PORT_PROBE_DELAY = 3000

# Progress bar settings
PROGRESS_UPDATE_DELAY = 100  # milliseconds

# Chip detection patterns (order matters - check more specific variants first)
CHIP_PATTERNS_ORDERED = [
    ("esp32s3", re.compile(r"ESP32[-\s]*S3|ESP32S3", re.I)),
    ("esp32s2", re.compile(r"ESP32[-\s]*S2|ESP32S2", re.I)),
    ("esp32c3", re.compile(r"ESP32[-\s]*C3|ESP32C3", re.I)),
    ("esp32",   re.compile(r"\bESP32\b", re.I)),
    ("esp8266", re.compile(r"\bESP8266\b", re.I)),
]

# Chip-specific flash configuration
KNOWN_CHIPS = {
    "esp8266": {"flash_addr": "0x00000", "flash_mode": "dio", "flash_freq": "40m"},
    "esp32":   {"flash_addr": "0x1000",  "flash_mode": "dio", "flash_freq": "40m"},
    "esp32s2": {"flash_addr": "0x0000",  "flash_mode": "dio", "flash_freq": "40m"},
    "esp32c3": {"flash_addr": "0x0000",  "flash_mode": "dio", "flash_freq": "40m"},
    "esp32s3": {"flash_addr": "0x0000",  "flash_mode": "dio", "flash_freq": "40m"},
}

# Standard firmware file offsets
ESP32_BOOTLOADER_ADDR = "0x1000"
ESP32_PARTITION_ADDR = "0x8000"
ESP32_APP_ADDR = "0x10000"

ESP32_VARIANT_BOOTLOADER_ADDR = "0x0000"  # For S2, S3, C3
ESP32_VARIANT_PARTITION_ADDR = "0x8000"
ESP32_VARIANT_APP_ADDR = "0x10000"

# Priority keywords for auto-selecting serial ports
PORT_PRIORITY_KEYWORDS = [
    "usb-serial", "cp210", "silicon labs", "cp210x",
    "ftdi", "ch340", "usb2.0-serial"
]

# Common firmware file naming patterns
BOOTLOADER_FILE_NAMES = ["bootloader.bin", "boot.bin"]
PARTITION_FILE_NAMES = ["partitions.bin"]
APP_FILE_NAMES = ["firmware.bin", "app.bin", "application.bin", "ota.bin"]
FIRMWARE_FILE_PRIORITY = [
    "bootloader", "boot", "partition", "partitions",
    "firmware", "app", "application", "ota"
]

# GitHub API settings
GITHUB_API_ENDPOINT = "https://api.github.com/repos/Abasz/ESPRowingMonitor/releases/latest"
GITHUB_REQUEST_TIMEOUT = 10  # seconds

# Firmware source modes
FIRMWARE_MODE_PRECOMPILED = "precompiled"
FIRMWARE_MODE_CUSTOM = "custom"

# ==================== LOW-LEVEL UTILITY FUNCTIONS ====================

def list_serial_ports() -> List[Tuple[str, str, str]]:
    """List all available serial ports.
    
    Returns:
        List of tuples containing (device_path, description, hardware_id).
    """
    ports: List[Tuple[str, str, str]] = []
    for p in list_ports.comports():
        ports.append((p.device, p.description or "", p.hwid or ""))
    return ports


def probe_port_for_chip(
    port: str, 
    baud: int = DEFAULT_BAUD
) -> Optional[str]:
    """Probe a serial port to detect the connected chip type.
    
    Args:
        port: Serial port device path
        baud: Baud rate for communication
        
    Returns:
        Raw output from esptool chip_id command, or None if probe failed
    """
    args = ["--port", port, "--baud", str(baud), "--after", "no-reset", "chip-id"]
    try:
        _, output = run_esptool(args)
        return output if output else None
    except Exception:
        return None


def detect_chip_from_output(output: Optional[str]) -> Optional[str]:
    """Parse esptool output to identify chip type.
    
    Args:
        output: Raw output from esptool command
        
    Returns:
        Chip identifier (e.g., "esp32", "esp32s3") or None if not detected
    """
    if not output:
        return None
    
    for key, pattern in CHIP_PATTERNS_ORDERED:
        if pattern.search(output):
            return key
    return None


def _strip_quotes(s: str) -> str:
    """Remove surrounding quotes from a string.
    
    Args:
        s: Input string that may be quoted
        
    Returns:
        String with surrounding quotes removed if present
    """
    s2 = s.strip()
    if len(s2) >= 2 and ((s2[0] == s2[-1] == '"') or (s2[0] == s2[-1] == "'")):
        return s2[1:-1].strip()
    return s2

def build_flash_entries_from_dir(dirpath: str, chip_hint: Optional[str] = None) -> List[str]:
    """Build flash entries from firmware directory based on chip type.
    
    Scans a directory for common firmware files (bootloader, partitions, app) and
    maps them to appropriate flash addresses based on the chip type.
    
    Args:
        dirpath: Path to directory containing firmware files
        chip_hint: Optional chip type hint (e.g., "esp32", "esp32s3")
        
    Returns:
        List of flash entries in "address:filepath" format
        
    Raises:
        ValueError: If dirpath is invalid or directory doesn't exist
    """
    dirpath = _strip_quotes(dirpath)
    p = Path(os.path.expanduser(dirpath))
    if not p.exists() or not p.is_dir():
        raise ValueError(f"Provided firmware directory does not exist or is not a directory: {dirpath}")

    files = {f.name.lower(): f for f in p.iterdir() if f.is_file()}
    entries: List[str] = []
    chip = (chip_hint or "esp32").lower()

    # Use chip-specific addresses
    if chip == "esp32":
        bootloader_addr = ESP32_BOOTLOADER_ADDR
        partition_addr = ESP32_PARTITION_ADDR
        app_addr = ESP32_APP_ADDR
    else:
        bootloader_addr = ESP32_VARIANT_BOOTLOADER_ADDR
        partition_addr = ESP32_VARIANT_PARTITION_ADDR
        app_addr = ESP32_VARIANT_APP_ADDR

    # Map bootloader file
    for boot_name in BOOTLOADER_FILE_NAMES:
        if boot_name in files:
            entries.append(f"{bootloader_addr}:{os.path.abspath(str(files[boot_name]))}")
            break
    
    # Map partition file
    for part_name in PARTITION_FILE_NAMES:
        if part_name in files:
            entries.append(f"{partition_addr}:{os.path.abspath(str(files[part_name]))}")
            break
    
    # Map application file
    for app_name in APP_FILE_NAMES:
        if app_name in files:
            entries.append(f"{app_addr}:{os.path.abspath(str(files[app_name]))}")
            break

    # Fallback: if no files matched, try to map .bin files by priority
    if not entries:
        bin_files = [f for k, f in files.items() if k.endswith(".bin")]
        if bin_files:
            def key_fn(x: Path):
                name = x.name.lower()
                for idx, token in enumerate(FIRMWARE_FILE_PRIORITY):
                    if token in name:
                        return idx
                return len(FIRMWARE_FILE_PRIORITY)
            
            sorted_bins = sorted(bin_files, key=key_fn)
            if len(sorted_bins) >= 1:
                entries.append(f"{app_addr}:{os.path.abspath(str(sorted_bins[0]))}")
            if len(sorted_bins) >= 2:
                entries.append(f"{partition_addr}:{os.path.abspath(str(sorted_bins[1]))}")
            if len(sorted_bins) >= 3:
                entries.append(f"{bootloader_addr}:{os.path.abspath(str(sorted_bins[2]))}")
    
    return entries

def parse_file_list(entries: List[str]) -> List[str]:
    """Parse a list of flash entries into esptool argument format.
    
    Converts entries like "0x1000:bootloader.bin" into a flat list suitable
    for passing to esptool: ["0x1000", "/abs/path/to/bootloader.bin", ...]
    
    Args:
        entries: List of strings in "address:filepath" format
        
    Returns:
        Flat list alternating between addresses and absolute file paths
        
    Raises:
        ValueError: If entry format is invalid
    """
    out: List[str] = []
    for e in entries:
        s = e.strip()
        if not s:
            continue
        s = _strip_quotes(s)
        
        # Try parsing as "address:path"
        if re.match(r'^\s*(0x[0-9a-fA-F]+|\d+)\s*:', s):
            left, right = s.split(":", 1)
        else:
            if ":" not in s:
                raise ValueError(f"File entry must include address and path separated by ':' -> {e}")
            left, right = s.rsplit(":", 1)
        
        left = left.strip()
        right = right.strip()
        
        # Determine which is address and which is path
        if left.startswith("0x") or re.fullmatch(r"\d+", left):
            addr = left
            path = right
        else:
            addr = right
            path = left
        
        path_abs = os.path.abspath(os.path.expanduser(_strip_quotes(path)))
        out.append(addr)
        out.append(path_abs)
    return out

def run_esptool(args: List[str], output_callback: Optional[Callable[[str], None]] = None) -> Tuple[int, str]:
    """Execute esptool with given arguments and capture output.
    
    Args:
        args: Command line arguments to pass to esptool
        output_callback: Optional callback function to receive output line by line
        
    Returns:
        Tuple of (return_code, combined_output_string)
    """
    pretty = " ".join(f'"{a}"' if " " in a else a for a in ["esptool"] + args)
    print("Running:", pretty)
    
    # Custom stream class that captures output AND calls callback in real-time
    class CallbackStream:
        def __init__(self, callback: Optional[Callable[[str], None]] = None) -> None:
            self.buffer = io.StringIO()
            self.callback: Optional[Callable[[str], None]] = callback
            self.line_buffer: str = ""
        
        def write(self, text: str) -> int:
            self.buffer.write(text)
            if self.callback:
                self.line_buffer += text
                while '\n' in self.line_buffer or '\r' in self.line_buffer:
                    # Handle both \n and \r (esptool uses \r for progress updates)
                    if '\r' in self.line_buffer and '\n' not in self.line_buffer.split('\r')[0]:
                        # Progress line with \r but no \n yet
                        parts = self.line_buffer.split('\r')
                        if len(parts) > 1:
                            for part in parts[:-1]:
                                if part:
                                    self.callback(part)
                            self.line_buffer = parts[-1]
                    else:
                        # Regular line with \n
                        if '\n' in self.line_buffer:
                            line, self.line_buffer = self.line_buffer.split('\n', 1)
                            if line:
                                self.callback(line)
                        else:
                            break
            return len(text)
        
        def flush(self):
            # Ensure underlying buffer is flushed (StringIO.flush exists but is a no-op)
            try:
                self.buffer.flush()
            except Exception:
                pass
            # If we have any partial content accumulated, deliver it to the callback now
            if self.callback and self.line_buffer:
                try:
                    self.callback(self.line_buffer)
                except Exception:
                    pass
                finally:
                    self.line_buffer = ""
        
        def getvalue(self):
            return self.buffer.getvalue()
    
    stdout_stream = CallbackStream(output_callback)
    stderr_stream = CallbackStream(output_callback)
    output_lines: List[str] = []
    
    try:
        # Save original sys.argv to restore later
        original_argv = sys.argv[:]
        
        # Set sys.argv as esptool expects it (with script name as first arg)
        sys.argv = ["esptool"] + args
        
        # Redirect stdout and stderr to our custom streaming handler
        with contextlib.redirect_stdout(stdout_stream), contextlib.redirect_stderr(stderr_stream):  # type: ignore[arg-type]
            try:
                # Call esptool main function
                esptool.main()
                rc: int = 0
            except SystemExit as e:
                rc = int(e.code) if e.code is not None and isinstance(e.code, int) else 0
            except Exception as e:
                stderr_stream.write(f"Runtime error: {e}\n")
                rc = 2
        
        # Restore original sys.argv
        sys.argv = original_argv
        
        # Flush any remaining buffered content
        if output_callback:
            if stdout_stream.line_buffer:
                output_callback(stdout_stream.line_buffer)
            if stderr_stream.line_buffer:
                output_callback(stderr_stream.line_buffer)
        
        # Combine stdout and stderr output
        stdout_output = stdout_stream.getvalue()
        stderr_output = stderr_stream.getvalue()
        combined_output = stdout_output + stderr_output
        
        # Collect output lines for return value
        if combined_output:
            lines = combined_output.split('\n')
            for line in lines:
                if line.strip():
                    output_lines.append(line)
        
        return rc, '\n'.join(output_lines)
        
    except Exception as e:
        err = f"Failed to run esptool: {e}"
        if output_callback:
            output_callback(err)
        return 2, err

def auto_select_port(preferred: Optional[str] = None, try_probe: bool = True) -> Tuple[Optional[str], Dict[str, Any]]:
    """Automatically select the most appropriate serial port.
    
    Selection priority:
    1. Preferred port if specified and available
    2. Port with detected chip (if probing enabled)
    3. Port matching priority keywords (USB-Serial, CP210x, etc.)
    4. First available port
    
    Args:
        preferred: Preferred port device path to use if available
        try_probe: Whether to probe ports for chip detection
        
    Returns:
        Tuple of (selected_port_or_None, metadata_dict)
        metadata includes all_ports list and auto_chip if detected
    """
    ports = list_serial_ports()
    metadata: Dict[str, Any] = {"all_ports": ports, "auto_chip": None}
    
    # Try preferred port first
    if preferred:
        for dev, _, _ in ports:
            if dev == preferred:
                if try_probe:
                    out = probe_port_for_chip(dev)
                    chip = detect_chip_from_output(out)
                    metadata["auto_chip"] = chip
                return dev, metadata
        return preferred, metadata

    # No ports available
    if not ports:
        return None, metadata

    # Single port - use it
    if len(ports) == 1:
        dev = ports[0][0]
        if try_probe:
            out = probe_port_for_chip(dev)
            chip = detect_chip_from_output(out)
            metadata["auto_chip"] = chip
        return dev, metadata

    # Multiple ports - probe for chip detection
    if try_probe:
        for dev, _, _ in ports:
            out = probe_port_for_chip(dev)
            chip = detect_chip_from_output(out)
            if chip:
                metadata["auto_chip"] = chip
                return dev, metadata

    # No chip detected - use priority keywords
    for dev, desc, hwid in ports:
        combined = (desc + " " + hwid).lower()
        if any(k in combined for k in PORT_PRIORITY_KEYWORDS):
            return dev, metadata

    # Default to first port
    return ports[0][0], metadata

def fetch_github_release(api_url: str = GITHUB_API_ENDPOINT, max_retries: int = 3) -> Tuple[Optional[Dict[str, Any]], Optional[str]]:
    """Fetch latest release metadata from GitHub API with retry mechanism.
    
    Args:
        api_url: GitHub API endpoint URL
        max_retries: Maximum number of retry attempts
        
    Returns:
        Tuple of (release_metadata_dict_or_None, error_message_or_None)
    """
    ssl_context = ssl.create_default_context(cafile=certifi.where())

    for attempt in range(max_retries):
        try:
            req = urllib.request.Request(api_url)
            req.add_header("Accept", "application/vnd.github.v3+json")
            with urllib.request.urlopen(req, timeout=GITHUB_REQUEST_TIMEOUT, context=ssl_context) as response:
                if response.status == 200:
                    return json.loads(response.read().decode()), None
                else:
                    error_msg = f"HTTP {response.status}: {response.reason}"
                    print(error_msg, flush=True)
                    if attempt < max_retries - 1:
                        time.sleep(3)
                        continue
                    return None, error_msg
        except urllib.error.HTTPError as e:
            error_msg = f"Attempt {attempt + 1}/{max_retries} failed: HTTP {e.code}: {e.reason}"
            print(error_msg, flush=True)
            if attempt < max_retries - 1:
                time.sleep(3)
                continue
            return None, error_msg
        except Exception as e:
            error_msg = f"Attempt {attempt + 1}/{max_retries} failed: Unexpected error: {type(e).__name__}: {e}"
            print(error_msg, flush=True)
            if attempt < max_retries - 1:
                time.sleep(3)
                continue
            return None, error_msg

    return None, "Max retries exceeded"

def parse_firmware_asset_name(asset_name: str) -> Optional[Dict[str, str]]:
    """Parse firmware asset filename into components.
    
    Expected format: firmware_{rowerProfile}-{boardProfile}_{ChipID}.zip
    Example: firmware_genericAir-FIREBEETLE2_ESP32S3.zip
    
    Args:
        asset_name: Asset filename from GitHub release
        
    Returns:
        Dict with 'rower', 'board', 'chip' keys or None if parse failed
    """
    name = asset_name
    if name.lower().endswith(".zip"):
        name = name[:-4]
    
    # Match pattern: firmware_{rower}-{board}_{chip}
    pattern = r'^firmware_([^-]+)-([^_]+)_(.+)$'
    match = re.match(pattern, name)
    if not match:
        return None
    
    return {
        "rower": match.group(1),
        "board": match.group(2),
        "chip": match.group(3).lower()
    }

def format_profile_name(profile_name: str) -> str:
    """Format camelCase profile names to human-readable format.
    
    Examples:
        genericAir -> Generic Air
        kayakFirst -> Kayak First
        oldDanube -> Old Danube
        
    Args:
        profile_name: CamelCase profile name
        
    Returns:
        Formatted profile name with proper spacing and capitalization
    """
    # Add spaces before uppercase letters (handle transitions)
    with_spaces = re.sub(r'([a-z0-9])([A-Z])', r'\1 \2', profile_name)
    with_spaces = re.sub(r'([A-Z])([A-Z][a-z])', r'\1 \2', with_spaces)
    
    # Capitalize first letter of each word
    return ' '.join(word.capitalize() for word in with_spaces.split())

def get_compatible_boards(assets: List[Dict[str, Any]], chip: str, rower: str) -> List[str]:
    """Get list of board profiles compatible with detected chip and selected rower.
    
    Args:
        assets: List of asset dicts from GitHub release
        chip: Detected chip ID (e.g., "esp32s3", "Unknown")
        rower: Selected rower profile
        
    Returns:
        List of compatible board profile names
    """
    if chip == "Unknown":
        return []
    
    boards: Set[str] = set()
    for asset in assets:
        parsed = parse_firmware_asset_name(asset.get("name", ""))
        if parsed and parsed["chip"] == chip.lower() and parsed["rower"] == rower:
            boards.add(parsed["board"])
    return sorted(list(boards))

def download_and_extract_firmware(
    url: str,
    dest_dir: str,
    progress_callback: Callable[[int, int], None],
    status_callback: Callable[[str], None],
    extraction_start_callback: Callable[[], None],
) -> Optional[str]:
    """Download firmware zip from URL and extract to directory.
    
    Args:
        url: Download URL for firmware zip
        dest_dir: Destination directory for extraction
        progress_callback: Optional callback(bytes_downloaded, total_bytes) for download progress
        status_callback: Optional callback(status_message) for status updates
        extraction_start_callback: Optional callback() called when extraction begins (for indeterminate progress)
        
    Returns:
        Path to extraction directory or None if failed
    """
    try:
        # Download to temp file
        with tempfile.NamedTemporaryFile(delete=False, suffix=".zip") as tmp:
            print(f"Downloading firmware from {url}...")
            
            status_callback("Downloading...")
            
            # Use certifi CA bundle for TLS verification
            ctx = ssl.create_default_context(cafile=certifi.where())
            with urllib.request.urlopen(url, timeout=30, context=ctx) as response:
                # Get total size from headers
                total_size = int(response.headers.get('Content-Length', 0))
                downloaded = 0
                chunk_size = 8192
                
                while True:
                    chunk = response.read(chunk_size)
                    if not chunk:
                        break
                    tmp.write(chunk)
                    downloaded += len(chunk)
                    
                    # Report progress
                    if total_size > 0:
                        progress_callback(downloaded, total_size)
            
            tmp_path = tmp.name
        
        # Extract zip
        extract_path = os.path.join(dest_dir, "firmware_extracted")
        os.makedirs(extract_path, exist_ok=True)
        
        print(f"Extracting firmware to {extract_path}...")
        
        # Signal extraction starting (switch to indeterminate mode)
        extraction_start_callback()
        
        status_callback("Extracting...")

        with zipfile.ZipFile(tmp_path, 'r') as zf:
            zf.extractall(extract_path)
        
        status_callback("Ready")

        # Clean up temp file
        os.unlink(tmp_path)
        
        return extract_path
    except Exception as e:
        print(f"Failed to download/extract firmware: {e}")
        status_callback(f"Error: {e}")
        
        return None

# ==================== GUI APPLICATION ====================

class ESPToolGUI:
    """Main GUI application for ESPTool operations.
    
    Provides a user-friendly interface for flashing ESP32/ESP8266 devices,
    including auto-detection of chip types, firmware file management, and
    progress tracking.
    """
    
    # UI calculation constants
    _TAB_HEADER_HEIGHT_PX = 130  # Estimated height for tab headers
    _SAFETY_MARGIN_PX = 22  # Safety margin for scroll calculations
    
    def __init__(self, root: tk.Tk) -> None:
        """Initialize the ESPTool GUI application.
        
        Args:
            root: The Tkinter root window
        """
        self.root = root
        self.root.title("ESPTool GUI")
        self.root.iconbitmap(os.path.join(sys._MEIPASS, "ESPTools-GUI.ico")) # type: ignore[arg-type]
        self.root.geometry(f"{DEFAULT_WINDOW_WIDTH}x{DEFAULT_WINDOW_HEIGHT}")

        self.settings_file = Path.home() / SETTINGS_FILENAME

        # Initialize Tkinter variables
        self.port_var = tk.StringVar()
        self.baud_var = tk.StringVar(value=str(DEFAULT_BAUD_FAST))
        self.fw_dir_var = tk.StringVar()
        self.flash_mode_var = tk.StringVar(value="dio")
        self.flash_freq_var = tk.StringVar(value="40m")

        # Application state
        self.detected_chip: Optional[str] = None
        self.flash_entries: List[str] = []
        self.last_selected_port: Optional[str] = None
        self.progress_var = tk.DoubleVar()
        self._last_progress_line: Optional[str] = None
        
        # GitHub release state
        self.release_data: Optional[Dict[str, Any]] = None
        self.available_rowers: List[str] = []
        self.available_boards: List[str] = []
        self.firmware_mode_var = tk.StringVar(value=FIRMWARE_MODE_PRECOMPILED)
        self.rower_var = tk.StringVar()
        self.board_var = tk.StringVar()
        self.rower_name_map: Dict[str, str] = {"Custom": "custom"}  # Formatted -> original mapping

        # -- scrolling container setup --
        # Outer container
        self.container = ttk.Frame(self.root)
        self.container.pack(fill=tk.BOTH, expand=True)

        self.canvas = tk.Canvas(self.container, highlightthickness=0)
        self.vscroll = ttk.Scrollbar(self.container, orient=tk.VERTICAL, command=self.canvas.yview)  # type: ignore[arg-type]
        self.canvas.configure(yscrollcommand=self.vscroll.set)

        # Inner frame that will contain the full UI
        self.inner = ttk.Frame(self.canvas)
        self.inner_id = self.canvas.create_window((0, 0), window=self.inner, anchor="nw")

        self.vscroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Bind to make scrollregion responsive and to adjust inner width to canvas
        self.inner.bind("<Configure>", self._on_inner_configure)
        self.canvas.bind("<Configure>", self._on_canvas_configure)
        # Bind root window resize to trigger scroll logic update
        self.root.bind("<Configure>", self._on_root_configure)
        # Mousewheel support
        self.canvas.bind_all("<MouseWheel>", self._on_mousewheel)       # Windows / macOS
        self.canvas.bind_all("<Button-4>", self._on_mousewheel_linux)   # Linux scroll up
        self.canvas.bind_all("<Button-5>", self._on_mousewheel_linux)   # Linux scroll down

        # Build UI inside the inner frame (preserve original layout)
        self.create_widgets(parent=self.inner)
        self.load_settings()

        # Use auto_select_port to pick initial port without blocking UI for probe
        preferred = self.last_selected_port or (self.port_var.get().split(" - ", 1)[0] if self.port_var.get() else None)
        # First detect available ports and choose candidate without probing (non-blocking)
        selected_port, _ = auto_select_port(preferred=preferred, try_probe=False)
        ports = list_serial_ports()
        port_list = [f"{dev} - {desc}" for dev, desc, _ in ports]
        self.port_combo['values'] = port_list
        if selected_port:
            match = next((p for p in port_list if p.startswith(selected_port)), None)
            if match:
                self.port_combo.set(match)
                self.port_var.set(match)
                # Start an asynchronous probe so we can show 'Detecting...' immediately
                def _auto_probe_worker(dev: Optional[str] = selected_port) -> None:
                    try:
                        baud = int(self.baud_var.get() or DEFAULT_BAUD)
                        if dev:
                            out = probe_port_for_chip(dev, baud=baud)
                        else:
                            out = None
                        chip = detect_chip_from_output(out)
                        self.root.after(0, self.on_chip_detected, chip, out)
                    except Exception as e:
                        self.root.after(0, self.on_chip_detect_error, str(e))

                # Provide immediate UI feedback while probing
                try:
                    self.chip_label.config(text="Detecting...", foreground="orange")
                    # force an update so label is visible before probe finishes
                    self.root.update_idletasks()
                except Exception:
                    pass

                threading.Thread(target=_auto_probe_worker, daemon=True).start()
        else:
            self.refresh_ports()

        # ensure scroll state is correct right after layout
        self.root.after(100, self._on_inner_configure)

        self.root.after(3000, self.periodic_refresh)

    # -- scrolling helpers --
    def _on_inner_configure(self, _: Optional[Any] = None) -> None:
        """Handle inner frame configuration changes to manage scrolling behavior.
        
        Text widgets have:
        - Minimum: 6 lines (MIN_TEXT_HEIGHT)
        - Fixed 12 lines when window height <= threshold (DEFAULT_TEXT_HEIGHT)
        - Dynamic expansion to fill bottom space when window height > threshold
        """
        # Update scrollregion to bounding box of all content
        bbox = self.canvas.bbox("all")
        if not bbox:
            return

        # Use the actual visible canvas height
        canvas_h = self.canvas.winfo_height()
        # If canvas hasn't been laid out yet, defer and retry
        if canvas_h <= 1:
            self.root.after(80, self._on_inner_configure)
            return

        # Always set scrollregion to full content (scrollbar auto-disables if not needed)
        self.canvas.configure(scrollregion=bbox)
        
        # Determine threshold based on firmware mode
        base_threshold = SCROLL_THRESHOLD
        if self.firmware_mode_var.get() == FIRMWARE_MODE_PRECOMPILED:
            threshold = base_threshold - 100  # 100px less for precompiled mode
        else:
            threshold = base_threshold
        
        # Determine if we should use fixed or dynamic sizing
        if canvas_h <= threshold:
            # Small window: use fixed DEFAULT_TEXT_HEIGHT (12 lines)
            self._set_text_widget_size(DEFAULT_TEXT_HEIGHT)
        else:
            # Large window: calculate dynamic size to fill available space
            self._calculate_and_set_dynamic_size(canvas_h)

    def _set_text_widget_size(self, lines: int):
        """Set text widgets to a specific number of lines.
        
        Args:
            lines: Number of lines to display
        """
        try:
            self.command_output_text.config(height=lines)
            self.output_text.config(height=lines)
        except Exception:
            pass

    def _calculate_and_set_dynamic_size(self, canvas_h: int):
        """Calculate and set dynamic text widget size based on available space.
        
        Args:
            canvas_h: Height of the canvas in pixels
        """
        try:
            # Ensure geometry info is up-to-date
            self.inner.update_idletasks()
            self.tab_parent.update_idletasks()

            # Sum heights of all widgets except the tab parent (includes visible custom controls)
            fixed_h = 0
            for c in self.inner.winfo_children():
                if c is self.tab_parent:
                    continue
                try:
                    # Only count if widget is actually visible/mapped
                    if c.winfo_ismapped():
                        fixed_h += c.winfo_reqheight()
                except Exception:
                    pass

            # Get line height from font metrics
            try:
                font = tkfont.Font(font=self.command_output_text['font'])
                line_h = font.metrics('linespace') or 14
            except Exception:
                line_h = 14

            # Calculate available space for tabs (with margins and tab headers)
            margin = 8
            tab_overhead = self._TAB_HEADER_HEIGHT_PX + self._SAFETY_MARGIN_PX
            avail_for_text = canvas_h - fixed_h - margin - tab_overhead

            # Calculate lines, ensuring minimum of MIN_TEXT_HEIGHT (6 lines)
            lines = max(MIN_TEXT_HEIGHT, int(avail_for_text / line_h))

            # Apply the computed line count
            self._set_text_widget_size(lines)

            # Force layout recalculation
            self.tab_parent.update_idletasks()
            self.inner.update_idletasks()
        except Exception as e:
            # Fallback to minimum if calculation fails
            print(f"Error calculating text size: {e}")
            self._set_text_widget_size(MIN_TEXT_HEIGHT)

    def _on_canvas_configure(self, event: Any) -> None:
        """Handle canvas resize events."""
        # Ensure inner frame width matches canvas width so widgets wrap nicely
        try:
            self.canvas.itemconfigure(self.inner_id, width=event.width)
        except Exception:
            pass
        # After resizing, update scrollregion and check scroll state
        self._on_inner_configure()

    def _on_root_configure(self, event: Any) -> None:
        """Handle root window resize events."""
        # Only respond to root window resize events, not child widget events
        if event.widget == self.root:
            self.root.after_idle(self._on_inner_configure)

    def _on_mousewheel(self, event: Any) -> None:
        """Handle mouse wheel scrolling on Windows/macOS."""
        try:
            # Determine threshold based on firmware mode
            base_threshold = SCROLL_THRESHOLD
            if self.firmware_mode_var.get() == FIRMWARE_MODE_PRECOMPILED:
                threshold = base_threshold - 100
            else:
                threshold = base_threshold
            # Disable scrolling when window height > threshold
            if self.canvas.winfo_height() > threshold:
                return
            delta = int(-1 * (event.delta / 120))
            if delta:
                self.canvas.yview_scroll(delta, "units")
        except Exception:
            pass

    def _on_mousewheel_linux(self, event: Any) -> None:
        """Handle mouse wheel scrolling on Linux."""
        # Determine threshold based on firmware mode
        base_threshold = SCROLL_THRESHOLD
        if self.firmware_mode_var.get() == FIRMWARE_MODE_PRECOMPILED:
            threshold = base_threshold - 100
        else:
            threshold = base_threshold
        # Disable scrolling when window height > threshold
        if self.canvas.winfo_height() > threshold:
            return
        # Button-4 = up, Button-5 = down
        if event.num == 4:
            self.canvas.yview_scroll(-1, "units")
        elif event.num == 5:
            self.canvas.yview_scroll(1, "units")

    # ---------- Original UI creation (same as your v3) ----------
    def create_widgets(self, parent: tk.Widget) -> None:
        top_frame = ttk.Frame(parent, padding=8)
        top_frame.pack(fill=tk.X)

        ttk.Label(top_frame, text="Port:").grid(row=0, column=0, sticky=tk.W, padx=(0, 6))
        self.port_combo = ttk.Combobox(top_frame, textvariable=self.port_var, width=40)
        self.port_combo.grid(row=0, column=1, sticky=tk.W)
        self.port_combo.bind("<<ComboboxSelected>>", self.on_port_selected)
        ttk.Button(top_frame, text="Refresh", command=self.refresh_ports).grid(row=0, column=2, padx=6)

        ttk.Label(top_frame, text="Baud:").grid(row=0, column=3, sticky=tk.W, padx=(10, 6))
        self.baud_combo = ttk.Combobox(top_frame, textvariable=self.baud_var, width=12,
                                       values=AVAILABLE_BAUD_RATES)
        self.baud_combo.grid(row=0, column=4, sticky=tk.W)

        ttk.Label(top_frame, text="Chip:").grid(row=1, column=0, sticky=tk.W, pady=(8, 0))
        self.chip_label = ttk.Label(top_frame, text="Unknown", foreground="gray")
        self.chip_label.grid(row=1, column=1, sticky=tk.W, pady=(8, 0))
        ttk.Button(top_frame, text="Detect Chip", command=self.detect_chip).grid(row=1, column=2, padx=6, pady=(8, 0))

        # Firmware selection frame (rower dropdown toggles between precompiled/custom)
        fw_select_frame = ttk.LabelFrame(parent, text="Firmware Selection", padding=8)
        fw_select_frame.pack(fill=tk.X, padx=8, pady=(8, 0))

        profiles_row = ttk.Frame(fw_select_frame)
        profiles_row.pack(fill=tk.X, pady=2)
        ttk.Label(profiles_row, text="Rower:").pack(side=tk.LEFT, padx=(0, 4))
        self.rower_combo = ttk.Combobox(profiles_row, textvariable=self.rower_var, state="readonly", width=20)
        self.rower_combo.pack(side=tk.LEFT, padx=(0, 10))
        self.rower_combo.bind("<<ComboboxSelected>>", self.on_rower_changed)
        
        ttk.Label(profiles_row, text="Board:").pack(side=tk.LEFT, padx=(0, 4))
        self.board_combo = ttk.Combobox(profiles_row, textvariable=self.board_var, state="readonly", width=20)
        self.board_combo.pack(side=tk.LEFT)

        # Status label for profile loading - on the same line as dropdowns
        self.profile_status_label = ttk.Label(profiles_row, text="", foreground="gray")
        self.profile_status_label.pack(side=tk.RIGHT, padx=(10, 0))

        # Custom firmware directory (visible when custom mode is active)
        self.fw_frame = ttk.LabelFrame(parent, text="Firmware directory", padding=8)
        ttk.Entry(self.fw_frame, textvariable=self.fw_dir_var, width=80).grid(row=0, column=0, sticky=tk.W, padx=(0, 6))
        ttk.Button(self.fw_frame, text="Browse...", command=self.browse_fw_dir).grid(row=0, column=1)
        ttk.Button(self.fw_frame, text="Auto-map", command=self.update_file_mapping).grid(row=0, column=2, padx=(6, 0))

        self.mapping_frame = ttk.LabelFrame(parent, text="File Mapping (double-click address to edit)", padding=8)
        columns = ("addr", "file")
        # Limit to MAX_FILE_TREE_HEIGHT rows as there will never be more than that many files
        self.mapping_tree = ttk.Treeview(self.mapping_frame, columns=columns, show="headings", height=MAX_FILE_TREE_HEIGHT)
        self.mapping_tree.heading("addr", text="Address")
        self.mapping_tree.heading("file", text="File path")
        self.mapping_tree.column("addr", width=120, anchor=tk.W)
        self.mapping_tree.column("file", width=620, anchor=tk.W)
        self.mapping_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.mapping_tree.bind("<Double-1>", self.on_tree_click)
        scrollbar = ttk.Scrollbar(self.mapping_frame, orient=tk.VERTICAL, command=self.mapping_tree.yview)  # type: ignore[arg-type]
        self.mapping_tree.configure(yscroll=scrollbar.set)  # type: ignore[call-overload]
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        # Store reference to progress frame so we can insert custom controls before it
        self.progress_frame = ttk.Frame(parent, padding=8)
        progress_frame = self.progress_frame
        progress_frame.pack(fill=tk.X, padx=8, pady=(2, 0))
        self.progress_bar = ttk.Progressbar(progress_frame, variable=self.progress_var, maximum=100)
        self.progress_bar.pack(fill=tk.X)
        self.progress_label = ttk.Label(progress_frame, text="Ready")
        self.progress_label.pack(anchor=tk.W, pady=(4, 0))

        btn_frame = ttk.Frame(parent, padding=8)
        btn_frame.pack(fill=tk.X, padx=8, pady=(8, 0))
        ttk.Button(btn_frame, text="Erase Flash", command=self.erase_flash).pack(side=tk.LEFT)
        self.download_flash_btn = ttk.Button(
            btn_frame, 
            text="Download & Flash Firmware", 
            command=self.download_and_flash_precompiled
        )
        self.download_flash_btn.pack(side=tk.RIGHT)
        self.flash_custom_btn = ttk.Button(btn_frame, text="Flash Firmware", command=self.flash_firmware)
        self.flash_custom_btn.pack(side=tk.RIGHT, padx=(0, 0))

        self.tab_parent = ttk.Notebook(parent)
        self.tab_parent.pack(fill=tk.BOTH, expand=True, padx=8, pady=(8, 8))

        tools_tab = ttk.Frame(self.tab_parent)
        self.tab_parent.add(tools_tab, text="Tools")
        ttk.Button(tools_tab, text="Read MAC", command=self.read_mac).pack(anchor=tk.W, padx=8, pady=4)
        ttk.Button(tools_tab, text="Chip ID", command=self.chip_id).pack(anchor=tk.W, padx=8, pady=4)
        # Store reference to command output text for dynamic sizing
        self.command_output_text = scrolledtext.ScrolledText(tools_tab, height=DEFAULT_TEXT_HEIGHT)
        self.command_output_text.pack(fill=tk.BOTH, expand=True, padx=8, pady=(5, 8))

        log_tab = ttk.Frame(self.tab_parent)
        self.tab_parent.add(log_tab, text="Log")
        # Store reference to output text for dynamic sizing
        self.output_text = scrolledtext.ScrolledText(log_tab, height=DEFAULT_TEXT_HEIGHT) 
        self.output_text.pack(fill=tk.BOTH, expand=True, padx=8, pady=(4, 8))
        
        # Initialize firmware selection after all UI elements are created
        self.root.after(100, self.initialize_firmware_selection)

    # ---------- Logging helpers ----------
    def log_output(self, message: str) -> None:
        ts = time.strftime("%H:%M:%S")
        self.output_text.insert(tk.END, f"[{ts}] {message}\n")
        # Do not auto-scroll when adding debug/log lines; keep view where user left it.

    def command_output(self, message: str) -> None:
        ts = time.strftime("%H:%M:%S")
        self.command_output_text.insert(tk.END, f"[{ts}] {message}\n")
        # Do not auto-scroll when adding command output; parse_and_update_progress
        # will update progress but should not force scroll.

    def esptool_output(self, message: str) -> None:
        def _insert() -> None:
            # Preserve carriage-return progress updates on a single line.
            raw = message
            # Remove common ANSI escape sequences (cursor movement, erase line, colors)
            try:
                raw = re.sub(r'\x1B\[[0-?]*[ -/]*[@-~]', '', raw)
            except Exception:
                pass
            if '\r' in raw:
                # take last segment after carriage return (esptool uses CR to update the same line)
                msg = raw.split('\r')[-1].strip()
            else:
                msg = raw

            # Determine whether this is a progress-like message
            is_progress = False
            try:
                if '\r' in raw:
                    is_progress = True
                else:
                    # reuse the same regex used in parse_and_update_progress
                    if re.search(r"(\d{1,3}(?:\.\d+)?)%\s*([0-9,]+)/([0-9]+)\s*bytes", msg):
                        is_progress = True
            except Exception:
                is_progress = False

            try:
                if is_progress:
                    # Update or insert a single progress line
                    if not getattr(self, '_last_progress_line', None):
                        # Append a new progress line and remember its start index
                        self.command_output_text.insert(tk.END, f"{msg}")
                        try:
                            self._last_progress_line = self.command_output_text.index("end-1c linestart")
                        except Exception:
                            self._last_progress_line = None
                    elif self._last_progress_line:
                        # Replace the previous progress line with the new content
                        try:
                            # Delete only the previous progress line (from its start to its lineend)
                            self.command_output_text.delete(self._last_progress_line, f"{self._last_progress_line} lineend")
                            # Insert new progress line at the same position
                            self.command_output_text.insert(self._last_progress_line, f"{msg}")
                            # Update stored start index to the new line
                            self._last_progress_line = self.command_output_text.index(f"{self._last_progress_line}")
                        except Exception:
                            # If anything goes wrong, fallback to appending
                            self.command_output_text.insert(tk.END, f"{msg}\n")
                else:
                    # Non-progress lines - append normally and clear any tracked progress line
                    self.command_output_text.insert(tk.END, f"{msg}\n")
                    self._last_progress_line = None
            except Exception:
                # Best-effort fallback
                try:
                    self.command_output_text.insert(tk.END, f"{msg}\n")
                except Exception:
                    pass

            # Drive progress parsing for percent/bytes extraction
            self.parse_and_update_progress(msg)
        self.root.after(0, _insert)

    def parse_and_update_progress(self, message: str) -> None:
        m = re.search(r'(\d{1,3}(?:\.\d+)?)%\s*([0-9,]+)/([0-9,]+)\s*bytes', message)
        if m:
            pct = float(m.group(1))
            try:
                self.progress_var.set(pct)
                cur = int(m.group(2).replace(",", ""))
                tot = int(m.group(3).replace(",", ""))
                self.progress_label.config(text=f"{pct:.1f}% — {cur:,}/{tot:,} bytes")
            except Exception:
                self.progress_label.config(text=f"{pct:.1f}%")
            return
        if "100.0%" in message or "Flash complete" in message:
            self.progress_var.set(100.0)
            self.progress_label.config(text="Flash complete!")

    def set_progress_determinate(self):
        """Switch progress bar to determinate mode (shows percentage)."""
        def _set():
            self.progress_bar.config(mode='determinate')
            self.progress_var.set(0.0)
        self.root.after(0, _set)

    def set_progress_indeterminate(self):
        """Switch progress bar to indeterminate mode (shows activity animation)."""
        def _set():
            self.progress_bar.config(mode='indeterminate')
            self.progress_bar.start(10)  # 10ms interval for animation
        self.root.after(0, _set)

    def stop_progress_indeterminate(self):
        """Stop indeterminate progress animation."""
        def _stop():
            self.progress_bar.stop()
            self.progress_bar.config(mode='determinate')
        self.root.after(0, _stop)

    def update_download_progress(self, downloaded: int, total: int):
        """Update progress bar and label for download progress."""
        def _update():
            if total > 0:
                pct = (downloaded / total) * 100
                self.progress_var.set(pct)
                self.progress_label.config(text=f"Downloading... {pct:.1f}% — {downloaded:,}/{total:,} bytes")
        self.root.after(0, _update)

    def update_status_message(self, message: str):
        """Update the progress label with a status message."""
        def _update():
            self.progress_label.config(text=message)
        self.root.after(0, _update)

    # ---------- Firmware mode management ----------
    def initialize_firmware_selection(self):
        """Initialize firmware selection - fetch releases asynchronously with UI feedback."""
        # Show loading status
        self.profile_status_label.config(text="Loading profiles...", foreground="orange")
        self.root.update_idletasks()
        
        def fetch_worker():
            try:
                self.log_output("Fetching GitHub release data...")
                self.release_data, error_msg = fetch_github_release()
                
                if not self.release_data:
                    if error_msg:
                        self.root.after(0, lambda: self.profile_status_label.config(text="Loading profiles failed...", foreground="red"))
                        self.log_output(f"WARNING: Failed to fetch release data: {error_msg}")
                    else:
                        self.root.after(0, lambda: self.profile_status_label.config(text="Loading profiles failed...", foreground="red"))
                        self.log_output("WARNING: Failed to fetch release data - unknown error")
                    self.log_output("Falling back to custom mode only")
                    self.available_rowers = ["custom"]
                    self.root.after(0, lambda: self.rower_combo.config(values=["Custom"]))
                    self.rower_name_map = {"Custom": "custom"}
                    self.root.after(0, lambda: self.rower_var.set("Custom"))
                    self.root.after(0, self.on_rower_changed, None)
                    return
                
                # Extract unique rower profiles from assets
                rowers: Set[str] = set()
                for asset in self.release_data.get("assets", []):
                    parsed = parse_firmware_asset_name(asset["name"])
                    if parsed:
                        rowers.add(parsed["rower"])
                
                # Always add "custom" option - selecting it switches to custom mode
                self.available_rowers = sorted(list(rowers)) + ["custom"]
                # Format profile names for display
                formatted_rowers = [format_profile_name(r) if r != "custom" else "Custom" for r in self.available_rowers]
                self.root.after(0, lambda: self.rower_combo.config(values=formatted_rowers))
                # Store mapping of formatted names to original names
                self.rower_name_map = {formatted_rowers[i]: self.available_rowers[i] for i in range(len(self.available_rowers))}
                
                # Default to custom mode
                self.root.after(0, lambda: self.rower_var.set("Custom"))
                self.root.after(0, self.on_rower_changed, None)
                
                # Hide status label on success
                self.root.after(0, lambda: self.profile_status_label.config(text=""))
                
                tag = self.release_data.get('tag_name', 'unknown') if self.release_data else 'unknown'
                self.root.after(0, lambda: self.log_output(f"Found {len(rowers)} rower profiles in release {tag}"))
            except Exception as e:
                self.root.after(0, lambda: self.profile_status_label.config(text="Loading profiles failed...", foreground="red"))
                self.root.after(0, lambda: self.log_output(f"ERROR: Exception while fetching releases: {e}"))
                # Fallback to custom mode only
                self.available_rowers = ["custom"]
                self.root.after(0, lambda: self.rower_combo.config(values=["Custom"]))
                self.rower_name_map = {"Custom": "custom"}
                self.root.after(0, lambda: self.rower_var.set("Custom"))
                self.root.after(0, self.on_rower_changed, None)
        
        threading.Thread(target=fetch_worker, daemon=True).start()

    def on_rower_changed(self, _: Optional[Any] = None) -> None:
        """Update board dropdown and UI mode when rower selection changes."""
        rower_display = self.rower_var.get()
        # Get actual rower name from mapping
        rower = self.rower_name_map.get(rower_display, rower_display.lower())
        
        if rower == "custom":
            # Switch to custom mode - show firmware directory and file mapping before tabs
            self.firmware_mode_var.set(FIRMWARE_MODE_CUSTOM)
            self.board_combo["values"] = []
            self.board_var.set("")
            self.board_combo.config(state="disabled")
            
            # Show custom controls right after firmware selection and before progress
            self.fw_frame.pack(fill=tk.X, padx=8, pady=(8, 0), before=self.progress_frame)
            self.mapping_frame.pack(fill=tk.BOTH, expand=False, padx=8, pady=(8, 0), before=self.progress_frame)
            
            # Show custom flash button, hide download button
            self.download_flash_btn.pack_forget()
            self.flash_custom_btn.pack(side=tk.RIGHT)
            
            # Disable flash button if no files mapped
            if len(self.flash_entries) == 0:
                self.flash_custom_btn.config(state="disabled")
            else:
                self.flash_custom_btn.config(state="normal")
            
            self.log_output("Switched to custom firmware mode")
            
            # Force scroll calculation update after layout changes
            self.root.after(50, self._on_inner_configure)
        else:
            # Switch to precompiled mode - hide custom controls, show board selection
            self.firmware_mode_var.set(FIRMWARE_MODE_PRECOMPILED)
            self.fw_frame.pack_forget()
            self.mapping_frame.pack_forget()
            self.board_combo.config(state="readonly")
            
            # Show download button, hide custom flash button
            self.flash_custom_btn.pack_forget()
            self.download_flash_btn.pack(side=tk.RIGHT)
            
            # Force scroll calculation update after layout changes
            self.root.after(50, self._on_inner_configure)
            
            if not self.release_data:
                self.log_output("ERROR: No release data available")
                return
            
            detected_chip = self.chip_label.cget("text")
            
            # Filter boards by rower and chip
            self.available_boards = get_compatible_boards(self.release_data.get("assets", []), detected_chip, rower)
            self.board_combo["values"] = self.available_boards
            
            if self.available_boards:
                self.board_var.set(self.available_boards[0])
                self.download_flash_btn.config(state="normal")
            else:
                self.board_var.set("")
                self.download_flash_btn.config(state="disabled")
                if detected_chip != "Unknown":
                    self.log_output(f"WARNING: No boards found for rower '{rower}' and chip '{detected_chip}'")
            
            self.log_output(f"Switched to precompiled mode: {rower}")

    def download_and_flash_precompiled(self) -> None:
        """Download firmware from GitHub and flash to device."""
        # Map displayed profile names back to internal keys used in asset filenames
        rower_display = self.rower_var.get()
        rower = self.rower_name_map.get(rower_display, (rower_display or "").lower())
        board = self.board_var.get()
        detected_chip = self.chip_label.cget("text")
        chip_key = (detected_chip or "").lower()
        
        if not rower or not board:
            self.log_output("ERROR: Please select rower and board profiles")
            return
        
        if detected_chip == "Unknown":
            self.log_output("ERROR: Please detect chip first")
            return
        
        if not self.release_data:
            self.log_output("ERROR: No release data available")
            return
        
        # Show user-facing rower name in logs
        self.log_output(f"Downloading firmware for {rower_display}/{board}...")
        
        # Find matching asset
        asset_name = None
        download_url: Optional[str] = None
        for asset in self.release_data.get("assets", []):
            parsed = parse_firmware_asset_name(asset["name"])
            if parsed and parsed["rower"] == rower and parsed["board"] == board and parsed["chip"] == chip_key:
                asset_name = asset["name"]
                download_url = asset["browser_download_url"]
                break
        
        if not asset_name:
            # Show the user-facing names in the error, but include the keys attempted for debugging
            self.log_output(f"ERROR: No firmware found for {rower_display}/{board}/{detected_chip} (searched key: {rower}/{board}/{chip_key})")
            return
        
        # Start download in background thread with progress callbacks
        def _download_thread() -> None:
            try:
                # Switch to determinate mode for download
                self.set_progress_determinate()
                
                # Download and extract with progress callbacks
                # Narrow download_url for the type checker
                assert download_url is not None
                fw_dir = download_and_extract_firmware(
                    download_url,
                    tempfile.gettempdir(),
                    progress_callback=self.update_download_progress,
                    status_callback=self.update_status_message,
                    extraction_start_callback=self.set_progress_indeterminate
                )
                
                if not fw_dir:
                    self.log_output("ERROR: Failed to download firmware")
                    self.update_status_message("Download failed")
                    self.stop_progress_indeterminate()
                    return
                
                self.log_output(f"Downloaded firmware to {fw_dir}")
                
                # Update firmware directory and auto-map files on UI thread
                def _complete_and_flash():
                    self.stop_progress_indeterminate()
                    self.fw_dir_var.set(fw_dir)
                    self.update_file_mapping()
                    
                    # Flash firmware
                    self.log_output("Starting flash process...")
                    self.flash_firmware()
                
                self.root.after(0, _complete_and_flash)
                
            except Exception as e:
                self.log_output(f"ERROR: Failed to download/flash firmware: {e}")
                self.update_status_message(f"Error: {e}")
                self.stop_progress_indeterminate()
        
        # Start download in background
        download_thread = threading.Thread(target=_download_thread, daemon=True)
        download_thread.start()

    # ---------- Port & chip helpers (unchanged logic) ----------
    def refresh_ports(self) -> None:
        ports = list_serial_ports()
        port_list = [f"{dev} - {desc}" for dev, desc, _ in ports]
        self.port_combo['values'] = port_list

        # Get currently selected device (if any)
        current_value = self.port_var.get()
        available_ports = [p.split(" - ", 1)[0] for p in port_list]

        # If current selection exists and still available, keep it as-is
        if current_value:
            curr_dev = current_value.split(" - ", 1)[0] if " - " in current_value else current_value
            if curr_dev in available_ports:
                # Current selection is still available - keep it
                return
            else:
                # Current selection is no longer available - clear it
                self.port_var.set("")
                self.port_combo.set("")
                # Clear detected chip state/UI when the selected port disappears
                try:
                    self.detected_chip = None
                    self.chip_label.config(text="Unknown", foreground="gray")
                except Exception:
                    pass
                self.log_output(f"Previously selected port {curr_dev} is no longer available")

        # Check if the last_selected_port (from settings) has come back online
        if self.last_selected_port:
            last_dev = self.last_selected_port.split(" - ", 1)[0] if " - " in self.last_selected_port else self.last_selected_port
            if last_dev in available_ports:
                # Last selected port is back - reselect it
                match = next((p for p in port_list if p.startswith(last_dev)), None)
                if match:
                    self.port_combo.set(match)
                    self.port_var.set(match)
                    self.log_output(f"Previously selected port {last_dev} is back online - reselecting")
                    # Trigger the same detection flow as when a user selects a port
                    try:
                        # This will set the UI to 'Detecting...' and start the probe thread
                        self.on_port_selected()
                    except Exception:
                        pass
                    return

        # Otherwise, leave selection empty (no auto-selection to first port)
        self.log_output(f"{len(port_list)} serial ports found")

    def periodic_refresh(self) -> None:
        self.refresh_ports()
        self.root.after(5000, self.periodic_refresh)

    def get_selected_port(self) -> Optional[str]:
        v = self.port_var.get()
        if not v:
            return None
        return v.split(" - ", 1)[0] if " - " in v else v

    def _close_previous_serial(self, port: Optional[str] = None) -> None:
        """Attempt to close any lingering serial connections.

        If `port` is provided, try to close only connections matching that port.
        If `port` is None, attempt to close all serial-like objects we can find
        (used when the app is shutting down).

        This is defensive: esptool or other code paths may leave a Serial-like object open. We try a few strategies:
        - Inspect the esptool module attributes for objects that look like serial ports (have .close and .port attributes) and close those matching the previous port.
        - Inspect globals for pyserial Serial instances and close those matching the port.
        - Log actions for visibility.
        """
        prev = port or self.last_selected_port
        # If no specific port provided and none recorded, we'll try to close everything
        prev_dev = prev.split(" - ", 1)[0] if prev and " - " in prev else prev
        # Try closing objects attached to the esptool module
        try:
            for name in dir(esptool):
                try:
                    attr = getattr(esptool, name)
                except Exception:
                    continue
                if not hasattr(attr, "close") or not hasattr(attr, "port"):
                    continue
                try:
                    port_attr = getattr(attr, "port", None)
                    if not prev_dev or (port_attr and str(port_attr) == str(prev_dev)):
                        try:
                            attr.close()
                            self.log_output(f"Closed serial port on esptool.{name} ({prev_dev})")
                        except Exception as e:
                            self.log_output(f"Warning: failed to close esptool.{name}: {e}")
                except Exception:
                    continue
        except Exception:
            pass

        # Try closing any pyserial Serial objects found in global variables
        try:
            import serial as _serial_mod
            SerialType = getattr(_serial_mod, 'Serial', None)
            if SerialType:
                for gname, gval in list(globals().items()):
                    try:
                        if isinstance(gval, SerialType):
                            port_attr = getattr(gval, 'port', None)
                            if not prev_dev or (port_attr and str(port_attr) == str(prev_dev)):
                                try:
                                    gval.close()
                                    self.log_output(f"Closed global Serial object '{gname}' for {prev_dev}")
                                except Exception as e:
                                    self.log_output(f"Warning: failed to close global Serial '{gname}': {e}")
                    except Exception:
                        continue
        except Exception:
            # If pyserial is not available or something else fails, ignore silently
            pass

    def on_port_selected(self, _: Optional[Any] = None) -> None:
        # Close previous connection (if any) before switching to new port
        try:
            self._close_previous_serial()
        except Exception:
            pass

        selected = self.get_selected_port()
        if selected:
            self.last_selected_port = selected
        # Reset detected chip state when user changes port and show probing state
        self.detected_chip = None
        try:
            self.chip_label.config(text="Detecting...", foreground="orange")
            # Ensure label updates before the probe starts
            try:
                self.root.update_idletasks()
            except Exception:
                pass
        except Exception:
            # Fallback to Unknown if updating fails
            try:
                self.chip_label.config(text="Unknown", foreground="gray")
            except Exception:
                pass
        self.save_settings()
        if selected:
            def worker() -> None:
                try:
                    baud = int(self.baud_var.get() or DEFAULT_BAUD)
                    if selected:
                        out = probe_port_for_chip(selected, baud=baud)
                        chip = detect_chip_from_output(out)
                        self.root.after(0, self.on_chip_detected, chip, out)
                except Exception as e:
                    self.root.after(0, self.on_chip_detect_error, str(e))
            threading.Thread(target=worker, daemon=True).start()

    def detect_chip(self) -> None:
        """Probe the currently selected port for chip identification in a worker thread."""
        port = self.get_selected_port()
        if not port:
            messagebox.showerror("Error", "Please select a serial port first")
            return
        # Provide immediate UI feedback
        self.chip_label.config(text="Detecting...", foreground="orange")

        def worker() -> None:
            try:
                baud = int(self.baud_var.get() or DEFAULT_BAUD)
                out = probe_port_for_chip(port, baud=baud)
                chip = detect_chip_from_output(out)
                self.root.after(0, self.on_chip_detected, chip, out)
            except Exception as e:
                self.root.after(0, self.on_chip_detect_error, str(e))

        threading.Thread(target=worker, daemon=True).start()

    def on_chip_detected(self, chip: Optional[str], raw_output: Optional[str]) -> None:
        if chip:
            self.detected_chip = chip
            self.chip_label.config(text=chip.upper(), foreground="green")
            self.log_output(f"Detected chip: {chip}")
            if chip in KNOWN_CHIPS:
                settings = KNOWN_CHIPS[chip]
                self.flash_mode_var.set(settings.get("flash_mode", "dio"))
                self.flash_freq_var.set(settings.get("flash_freq", "40m"))
                if chip.startswith("esp32") and int(self.baud_var.get() or 0) <= DEFAULT_BAUD:
                    self.baud_var.set(str(DEFAULT_BAUD_FAST))
            if self.fw_dir_var.get():
                self.update_file_mapping()
            # Update board dropdown when chip is detected (if not in custom mode)
            if self.rower_var.get() and self.rower_var.get() != "custom":
                self.on_rower_changed(None)
        else:
            self.chip_label.config(text="Detection failed", foreground="red")
            self.log_output("Chip detection failed")
            if raw_output:
                excerpt = raw_output if len(raw_output) < 1000 else raw_output[:1000] + "..."
                self.log_output(excerpt)

    def on_chip_detect_error(self, err: str) -> None:
        self.chip_label.config(text="Error", foreground="red")
        self.log_output(f"Chip detection error: {err}")

    # ---------- File mapping, flashing and other functions unchanged ----------
    def browse_fw_dir(self) -> None:
        d = filedialog.askdirectory(title="Select firmware directory")
        if d:
            self.fw_dir_var.set(d)
            self.update_file_mapping()
            self.save_settings()

    def update_file_mapping(self) -> None:
        fw_dir = self.fw_dir_var.get()
        for it in self.mapping_tree.get_children():
            self.mapping_tree.delete(it)
        if not fw_dir:
            self.log_output("No firmware directory selected")
            return
        if not os.path.isdir(fw_dir):
            self.log_output(f"Directory does not exist: {fw_dir}")
            return
        try:
            entries = build_flash_entries_from_dir(fw_dir, chip_hint=self.detected_chip)
            self.flash_entries = entries[:]
            for entry in entries:
                if ":" in entry:
                    addr, path = entry.split(":", 1)
                    self.mapping_tree.insert("", "end", values=(addr, path))
            self.log_output(f"Mapped {len(entries)} file(s) from: {fw_dir}")

            # Enable/disable flash button based on file count
            if self.firmware_mode_var.get() == FIRMWARE_MODE_CUSTOM:
                if len(entries) > 0:
                    self.flash_custom_btn.config(state="normal")
                else:
                    self.flash_custom_btn.config(state="disabled")
        except Exception as e:
            self.log_output(f"Error mapping files: {e}")
            messagebox.showerror("Error", f"Failed to map files: {e}")

    def on_tree_click(self, event: Any) -> None:
        region = self.mapping_tree.identify_region(event.x, event.y)
        if region != "cell":
            return
        col = self.mapping_tree.identify_column(event.x)
        row = self.mapping_tree.identify_row(event.y)
        if col != "#1":
            return
        item = row
        values = list(self.mapping_tree.item(item, "values"))
        if not values:
            return
        cur_addr = values[0]
        bbox = self.mapping_tree.bbox(item, col)
        if not bbox:
            return
        x, y, width, height = bbox
        entry = tk.Entry(self.mapping_tree)
        entry.insert(0, cur_addr)
        entry.place(x=x, y=y, width=width, height=height)
        entry.focus_set()
        def finish(_: Optional[Any] = None) -> None:
            new_addr = entry.get().strip()
            if not (new_addr.startswith("0x") or new_addr.isdigit()):
                messagebox.showerror("Invalid address", "Address must be hex (0x...) or decimal")
                entry.destroy()
                return
            values[0] = new_addr
            self.mapping_tree.item(item, values=values)
            entry.destroy()
            self.update_flash_entries_from_tree()
        entry.bind("<Return>", finish)
        entry.bind("<FocusOut>", finish)
        entry.bind("<Escape>", lambda e: entry.destroy())

    def update_flash_entries_from_tree(self) -> None:
        fw_dir = self.fw_dir_var.get()
        new_entries: List[str] = []
        for it in self.mapping_tree.get_children():
            addr, path = self.mapping_tree.item(it, "values")
            if not os.path.isabs(path):
                full = os.path.join(fw_dir, os.path.basename(path)) if fw_dir else path
            else:
                full = path
            if not os.path.exists(full):
                self.log_output(f"Warning: mapped file not found: {full}")
            new_entries.append(f"{addr}:{full}")
        self.flash_entries = new_entries
        
        # Enable/disable flash button based on file count
        if self.firmware_mode_var.get() == FIRMWARE_MODE_CUSTOM:
            if len(new_entries) > 0:
                self.flash_custom_btn.config(state="normal")
            else:
                self.flash_custom_btn.config(state="disabled")

    def flash_firmware(self) -> None:
        port = self.get_selected_port()
        if not port:
            messagebox.showerror("Error", "Please select a serial port first")
            return
        if not self.flash_entries:
            messagebox.showerror("Error", "No files mapped to flash (use Auto-map or add files)")
            return
        if not messagebox.askyesno("Confirm", f"Flash {len(self.flash_entries)} file(s) to {port}?"):
            return
        def worker() -> None:
            try:
                baud = int(self.baud_var.get() or DEFAULT_BAUD)
                flash_args = parse_file_list(self.flash_entries)
                args: List[str] = ["--port", port, "--baud", str(baud), "write-flash"]
                if self.flash_mode_var.get():
                    args += ["--flash-mode", self.flash_mode_var.get()]
                if self.flash_freq_var.get():
                    args += ["--flash-freq", self.flash_freq_var.get()]
                args += flash_args
                preview = " ".join(f'"{a}"' if " " in a else a for a in ["esptool"] + args)
                self.root.after(0, self.command_output, f"Running: {preview}")
                rc, _ = run_esptool(args, output_callback=self.esptool_output)
                if rc == 0:
                    self.root.after(0, self.command_output, "Flash finished successfully")
                    self.root.after(0, messagebox.showinfo, "Success", "Flash completed")
                else:
                    self.root.after(0, self.command_output, f"Flash failed with code {rc}")
                    self.root.after(0, messagebox.showerror, "Error", f"Flash failed (rc={rc})")
            except Exception as e:
                self.root.after(0, self.command_output, f"Flash error: {e}")
        self.progress_var.set(0.0)
        self.progress_label.config(text="Starting flash...")
        threading.Thread(target=worker, daemon=True).start()

    def erase_flash(self) -> None:
        port = self.get_selected_port()
        if not port:
            messagebox.showerror("Error", "Please select a serial port first")
            return
        if not messagebox.askyesno("Confirm erase", f"Erase entire flash on {port}?"):
            return
        def start_indeterminate() -> None:
            self.progress_bar.config(mode='indeterminate')
            try:
                self.progress_bar.start(10)
            except Exception:
                pass
            self.progress_label.config(text="Erasing flash...")
        def stop_indeterminate() -> None:
            try:
                self.progress_bar.stop()
            except Exception:
                pass
            self.progress_bar.config(mode='determinate')
            self.progress_var.set(0.0)
            self.progress_label.config(text="Ready")
        start_indeterminate()
        def worker() -> None:
            try:
                baud = int(self.baud_var.get() or DEFAULT_BAUD)
                args = ["--port", port, "--baud", str(baud), "erase-flash"]
                self.root.after(0, self.command_output, f"Running: esptool {' '.join(args)}")
                rc, _ = run_esptool(args, output_callback=self.esptool_output)
                self.root.after(0, stop_indeterminate)
                if rc == 0:
                    self.root.after(0, self.command_output, "Erase completed")
                    self.root.after(0, messagebox.showinfo, "Success", "Erase completed")
                else:
                    self.root.after(0, self.command_output, f"Erase failed (rc={rc})")
                    self.root.after(0, messagebox.showerror, "Error", f"Erase failed (rc={rc})")
            except Exception as e:
                self.root.after(0, stop_indeterminate)
                self.root.after(0, self.command_output, f"Erase error: {e}")
        threading.Thread(target=worker, daemon=True).start()

    def read_mac(self) -> None:
        port = self.get_selected_port()
        if not port:
            messagebox.showerror("Error", "Please select a serial port first")
            return
        def worker() -> None:
            try:
                baud = int(self.baud_var.get() or DEFAULT_BAUD)
                args = ["--port", port, "--baud", str(baud), "--after", "no-reset", "read-mac"]
                rc, _ = run_esptool(args, output_callback=self.esptool_output)
                if rc == 0:
                    self.root.after(0, self.command_output, "Read MAC finished")
                else:
                    self.root.after(0, self.command_output, f"Read MAC failed (rc={rc})")
            except Exception as e:
                self.root.after(0, self.command_output, f"Read MAC error: {e}")
        threading.Thread(target=worker, daemon=True).start()

    def chip_id(self) -> None:
        port = self.get_selected_port()
        if not port:
            messagebox.showerror("Error", "Please select a serial port first")
            return
        def worker() -> None:
            try:
                baud = int(self.baud_var.get() or DEFAULT_BAUD)
                args = ["--port", port, "--baud", str(baud), "--after", "no-reset", "chip-id"]
                rc, _ = run_esptool(args, output_callback=self.esptool_output)
                if rc == 0:
                    self.root.after(0, self.command_output, "chip-id finished")
                else:
                    self.root.after(0, self.command_output, f"chip-id failed (rc={rc})")
            except Exception as e:
                self.root.after(0, self.command_output, f"chip-id error: {e}")
        threading.Thread(target=worker, daemon=True).start()

    def save_settings(self) -> None:
        settings: Dict[str, Any] = {
            "port": self.port_var.get(),
            "baud": self.baud_var.get(),
            "fw_dir": self.fw_dir_var.get(),
            "flash_mode": self.flash_mode_var.get(),
            "flash_freq": self.flash_freq_var.get(),
            "last_selected_port": self.last_selected_port,
        }
        try:
            with open(self.settings_file, "w", encoding="utf-8") as f:
                json.dump(settings, f, indent=2)
        except Exception as e:
            self.log_output(f"Failed to save settings: {e}")

    def load_settings(self) -> None:
        if not self.settings_file.exists():
            return
        try:
            with open(self.settings_file, "r", encoding="utf-8") as f:
                s = json.load(f)
            if s.get("port"):
                self.port_var.set(s.get("port"))
            if s.get("baud"):
                self.baud_var.set(str(s.get("baud")))
            if s.get("fw_dir"):
                self.fw_dir_var.set(s.get("fw_dir"))
            if s.get("flash_mode"):
                self.flash_mode_var.set(s.get("flash_mode"))
            if s.get("flash_freq"):
                self.flash_freq_var.set(s.get("flash_freq"))
            self.last_selected_port = s.get("last_selected_port")
            if self.fw_dir_var.get():
                self.root.after(200, self.update_file_mapping)
        except Exception as e:
            self.log_output(f"Failed to load settings: {e}")

    def on_closing(self) -> None:
        try:
            self._close_previous_serial()
        except Exception:
            pass
        self.save_settings()
        self.root.destroy()

def main() -> None:
    root = tk.Tk()
    app = ESPToolGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    try:
        root.mainloop()
    except KeyboardInterrupt:
        app.on_closing()

if __name__ == "__main__":
    main()