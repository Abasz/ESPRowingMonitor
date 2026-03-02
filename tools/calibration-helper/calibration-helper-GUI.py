#!/usr/bin/env python3
"""
ESP Rowing Monitor Data Visualizer

Visualizes data from ESP Rowing Monitor simulation output:
1. Delta Times Tab: Raw vs cleaned delta times from cyclic error filter
2. Handle Forces Tab: Force curves for each stroke with navigation
3. Stroke Detection Tab: Analyze stroke detection accuracy using Theil-Sen regression

Features:
- GUI file picker for selecting the data file
- Tabbed interface for different data views
- Interactive zoom and pan (use toolbar buttons or mouse)
- Handle forces: navigate through strokes with prev/next buttons and slider
- Stroke detection: identify missed or duplicate strokes

Usage:
  python calibration-helper-GUI.py
  
Controls (Delta Times):
- Zoom: Click the magnifying glass icon, then drag a rectangle to zoom
- Pan: Click the move icon, then drag to scroll
- Home: Reset to original view
- Back/Forward: Navigate zoom history
- Mouse scroll: Zoom in/out (when pan/zoom tool is active)

Controls (Handle Forces):
- Previous/Next buttons: Navigate between stroke pairs
- Slider: Jump to any stroke position
- Entry box: Type a stroke number to display on the left chart

Controls (Stroke Detection):
- Adjust window size and slope threshold to tune detection
- Navigate to anomalies using the anomaly list
"""

import gc
import os
import re
import sys
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from dataclasses import dataclass, field
from typing import Optional
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
import numpy as np
from itertools import combinations
import base64
import tempfile
import pyperclipimg

# Constants
STROKE_COUNT_THRESHOLD = 4000  # Values below this in deltaTime: X are stroke counts
MAX_DELTA_TIME_THRESHOLD = 50000  # Skip initial data above this (spin-up phase)
MIN_PHASE_DURATION_MS = 200  # Minimum duration in ms for drive/recovery phases
MAX_LOADED_FILES = 10  # Maximum number of files that can be loaded at once


class CustomNavigationToolbar(NavigationToolbar2Tk):
    """Custom navigation toolbar with overridable home function."""
    
    def __init__(self, canvas, window, home_callback=None):
        self.home_callback = home_callback
        super().__init__(canvas, window)
    
    def home(self, *args):
        """Override home to use custom callback if provided."""
        if self.home_callback:
            self.home_callback()
        else:
            super().home(*args)


@dataclass
class FileData:
    """Container for all per-file data including cached analysis results."""
    filepath: str
    display_name: str
    
    # Delta Times data
    raw_deltas: Optional[np.ndarray] = None
    clean_deltas: Optional[np.ndarray] = None
    
    # Handle Forces data
    handle_forces: list = field(default_factory=list)
    current_stroke_index: int = 0
    
    # Per-stroke metrics (parallel arrays to handle_forces)
    stroke_power: list = field(default_factory=list)  # Watts per stroke
    stroke_drag_factor: list = field(default_factory=list)  # Dimensionless per stroke
    stroke_distance: list = field(default_factory=list)  # Meters per stroke
    stroke_drive_duration: list = field(default_factory=list)  # Seconds per stroke
    stroke_recovery_duration: list = field(default_factory=list)  # Seconds per stroke
    
    # Stroke Detection data
    stroke_raw_deltas: Optional[np.ndarray] = None
    stroke_clean_deltas: Optional[np.ndarray] = None
    stroke_markers: list = field(default_factory=list)
    
    # Cached analysis results
    stroke_slopes: Optional[np.ndarray] = None
    stroke_anomalies: list = field(default_factory=list)
    current_anomaly_index: int = 0
    
    # Analysis settings (for restoring UI state)
    analysis_settings: Optional[dict] = None  # {"window_size": X, "slope_threshold": Y}
    
    # View state (zoom/pan position per chart)
    delta_view_xlim: Optional[tuple] = None  # (x_min, x_max)
    delta_view_ylim: Optional[tuple] = None  # (y_min, y_max)
    forces_view_xlim: Optional[tuple] = None
    forces_view_ylim: Optional[tuple] = None
    stroke_detection_view_xlim: Optional[tuple] = None
    stroke_detection_view_ylim: Optional[tuple] = None
    
    def has_delta(self) -> bool:
        return self.raw_deltas is not None and len(self.raw_deltas) > 0
    
    def has_forces(self) -> bool:
        return len(self.handle_forces) > 0
    
    def has_stroke_data(self) -> bool:
        return (self.stroke_raw_deltas is not None and 
                len(self.stroke_raw_deltas) > 0 and 
                len(self.stroke_markers) > 0)
    
    def has_cached_analysis(self) -> bool:
        return self.analysis_settings is not None and len(self.stroke_anomalies) >= 0
    
    def has_metrics(self) -> bool:
        """Check if per-stroke metrics are available."""
        return (len(self.stroke_power) > 0 or 
                len(self.stroke_drag_factor) > 0 or 
                len(self.stroke_distance) > 0 or 
                len(self.stroke_drive_duration) > 0 or 
                len(self.stroke_recovery_duration) > 0)
    
    @staticmethod
    def format_metric(value, default="N/A") -> str:
        """Format a metric value, returning default if None or invalid."""
        if value is None:
            return default
        if isinstance(value, (int, float)):
            if value < 0 or (isinstance(value, float) and (value != value or value == float('inf'))):
                return default
            return f"{value:.2f}" if isinstance(value, float) else str(value)
        return default


def theil_sen_slope(y_values: np.ndarray) -> float:
    """
    Calculate Theil-Sen slope estimate for a series of values.
    Assumes x values are sequential indices (0, 1, 2, ...).
    
    Args:
        y_values: Array of y values
        
    Returns:
        Median slope estimate
    """
    n = len(y_values)
    if n < 2:
        return 0.0
    
    slopes = []
    for i, j in combinations(range(n), 2):
        # slope = (y[j] - y[i]) / (j - i)
        slope = (y_values[j] - y_values[i]) / (j - i)
        slopes.append(slope)
    
    return float(np.median(slopes)) if slopes else 0.0


def parse_data(filepath: str) -> tuple[np.ndarray, np.ndarray, list[list[int]], list, list, list, list, list]:
    """
    Parse the data file and extract delta times, handle forces, and per-stroke metrics.
    
    Args:
        filepath: Path to the data file
        
    Returns:
        Tuple of (raw_deltas, clean_deltas, handle_forces_list, power_list, drag_factor_list, 
                  distance_list, drive_duration_list, recovery_duration_list)
        - raw_deltas: numpy array of raw delta times
        - clean_deltas: numpy array of cleaned delta times
        - handle_forces_list: list of force curves (each is a list of integers)
        - power_list: list of power values per stroke (Watts, int or None)
        - drag_factor_list: list of drag factor values per stroke (dimensionless, int or None)
        - distance_list: list of distance values per stroke (meters, float or None) - LAST distance before next stroke
        - drive_duration_list: list of drive durations per stroke (seconds, float or None)
        - recovery_duration_list: list of recovery durations per stroke (seconds, float or None)
    """
    # Pattern for delta times: two comma-separated floats
    delta_pattern = re.compile(r'^(\d+\.?\d*),(\d+\.?\d*)$')
    # Pattern for handle forces: handleForces: [num,num,...]
    forces_pattern = re.compile(r'^handleForces:\s*\[([\d.,\s-]+)\]')
    # Patterns for per-stroke metrics
    power_pattern = re.compile(r'^power:\s*(-?\d+)$')
    drag_factor_pattern = re.compile(r'^dragFactor:\s*(-?\d+)$')
    distance_pattern = re.compile(r'^distance:\s*(-?\d+\.?\d*)$')
    drive_duration_pattern = re.compile(r'^driveDuration:\s*(-?\d+\.?\d*)$')
    recovery_duration_pattern = re.compile(r'^recoveryDuration:\s*(-?\d+\.?\d*)$')
    
    raw_deltas = []
    clean_deltas = []
    handle_forces_list = []
    power_list = []
    drag_factor_list = []
    distance_list = []
    drive_duration_list = []
    recovery_duration_list = []
    
    # Track current stroke's metrics (will accumulate between handleForces lines)
    current_power = None
    current_drag = None
    current_distance = None
    current_drive_duration = None
    current_recovery_duration = None
    
    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            
            # Check for delta times
            delta_match = delta_pattern.match(line)
            if delta_match:
                raw_deltas.append(float(delta_match.group(1)))
                clean_deltas.append(float(delta_match.group(2)))
                continue
            
            # Check for handle forces - this marks START of a NEW stroke
            forces_match = forces_pattern.match(line)
            if forces_match:
                # Save previous stroke's metrics if we have a stroke
                if handle_forces_list and len(handle_forces_list) > len(power_list):
                    # We have unrecorded metrics from the previous stroke, save them
                    power_list.append(current_power)
                    drag_factor_list.append(current_drag)
                    distance_list.append(current_distance)
                    drive_duration_list.append(current_drive_duration)
                    recovery_duration_list.append(current_recovery_duration)
                
                # Parse new force curve
                forces_str = forces_match.group(1)
                # Parse comma-separated floats and round to int
                forces = [round(float(x.strip())) for x in forces_str.split(',') if x.strip()]
                if forces:  # Only add non-empty arrays
                    handle_forces_list.append(forces)
                
                # Reset current stroke metrics
                current_power = None
                current_drag = None
                current_distance = None
                current_drive_duration = None
                current_recovery_duration = None
                continue
            
            # Check for power
            power_match = power_pattern.match(line)
            if power_match:
                current_power = int(power_match.group(1))
                continue
            
            # Check for drag factor
            drag_match = drag_factor_pattern.match(line)
            if drag_match:
                current_drag = int(drag_match.group(1))
                continue
            
            # Check for distance - ACCUMULATE (keep last one before next stroke)
            distance_match = distance_pattern.match(line)
            if distance_match:
                current_distance = float(distance_match.group(1))
                continue
            
            # Check for drive duration
            drive_match = drive_duration_pattern.match(line)
            if drive_match:
                current_drive_duration = float(drive_match.group(1))
                continue
            
            # Check for recovery duration
            recovery_match = recovery_duration_pattern.match(line)
            if recovery_match:
                current_recovery_duration = float(recovery_match.group(1))
                continue
    
    # Don't forget the last stroke's metrics
    if handle_forces_list and len(handle_forces_list) > len(power_list):
        power_list.append(current_power)
        drag_factor_list.append(current_drag)
        distance_list.append(current_distance)
        drive_duration_list.append(current_drive_duration)
        recovery_duration_list.append(current_recovery_duration)
    
    return (np.array(raw_deltas), np.array(clean_deltas), handle_forces_list,
            power_list, drag_factor_list, distance_list, 
            drive_duration_list, recovery_duration_list)


def parse_stroke_data(filepath: str) -> tuple[np.ndarray, np.ndarray, list[tuple[int, int]]]:
    """
    Parse the data file and extract raw/clean delta times and stroke markers.
    
    Args:
        filepath: Path to the data file
        
    Returns:
        Tuple of (raw_deltas, clean_deltas, stroke_markers)
        - raw_deltas: numpy array of raw delta times (for visualization)
        - clean_deltas: numpy array of cleaned delta times (for slope detection)
        - stroke_markers: list of (index, stroke_count) tuples
    """
    # Pattern for delta times: two comma-separated floats (raw, clean)
    delta_pattern = re.compile(r'^(\d+\.?\d*),(\d+\.?\d*)$')
    # Pattern for stroke markers: deltaTime: <small_number>
    stroke_pattern = re.compile(r'^deltaTime:\s*(\d+)$')
    
    raw_deltas = []
    clean_deltas = []
    stroke_markers = []
    delta_index = 0
    
    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            
            # Check for delta times (two comma-separated values)
            delta_match = delta_pattern.match(line)
            if delta_match:
                raw_value = float(delta_match.group(1))
                clean_value = float(delta_match.group(2))
                # Collect all delta times for stroke analysis (don't filter by threshold)
                raw_deltas.append(raw_value)
                clean_deltas.append(clean_value)
                delta_index += 1
                continue
            
            # Check for stroke markers (deltaTime: X where X < threshold)
            stroke_match = stroke_pattern.match(line)
            if stroke_match:
                value = int(stroke_match.group(1))
                if value < STROKE_COUNT_THRESHOLD:
                    # This is a stroke count, not a delta time
                    # Associate it with the current delta index
                    if delta_index > 0:
                        stroke_markers.append((delta_index - 1, value))
    
    return np.array(raw_deltas), np.array(clean_deltas), stroke_markers


def calculate_rolling_slopes(data: np.ndarray, window_size: int) -> np.ndarray:
    """
    Calculate rolling Theil-Sen slopes for the data.
    
    Args:
        data: Array of delta times
        window_size: Number of points to use for slope calculation
        
    Returns:
        Array of slopes (same length as data, padded with NaN at start)
    """
    n = len(data)
    slopes = np.full(n, np.nan)
    
    for i in range(window_size - 1, n):
        window = data[i - window_size + 1:i + 1]
        slopes[i] = theil_sen_slope(window)
    
    return slopes


def find_slope_zero_crossings(slopes: np.ndarray, threshold: float = 0.0) -> list[tuple[int, str]]:
    """
    Find indices where slope crosses through zero (or threshold).
    
    Args:
        slopes: Array of slope values
        threshold: Slope threshold for considering it "zero"
        
    Returns:
        List of (index, crossing_type) where crossing_type is 'neg_to_pos' or 'pos_to_neg'
    """
    crossings = []
    
    for i in range(1, len(slopes)):
        if np.isnan(slopes[i]) or np.isnan(slopes[i-1]):
            continue
            
        prev_sign = slopes[i-1] - threshold
        curr_sign = slopes[i] - threshold
        
        if prev_sign < 0 and curr_sign >= 0:
            crossings.append((i, 'neg_to_pos'))  # Drive to recovery transition
        elif prev_sign >= 0 and curr_sign < 0:
            crossings.append((i, 'pos_to_neg'))  # Recovery to drive transition
    
    return crossings


def detect_stroke_anomalies(
    raw_deltas: np.ndarray,
    stroke_markers: list[tuple[int, int]],
    window_size: int = 8,
    slope_threshold: float = 0.0,
    min_phase_duration_ms: float = MIN_PHASE_DURATION_MS
) -> tuple[np.ndarray, list[dict]]:
    """
    Detect missed or duplicate strokes using Theil-Sen regression.
    
    Uses recovery-to-recovery cycles (slowest to slowest points, i.e., pos_to_neg crossings).
    Applies minimum duration cooldown for drive and recovery phases to avoid false detections.
    
    Args:
        raw_deltas: Array of raw delta times (in microseconds)
        stroke_markers: List of (index, stroke_count) tuples
        window_size: Window size for slope calculation
        slope_threshold: Threshold for slope zero-crossing detection
        min_phase_duration_ms: Minimum duration in ms for each phase (drive/recovery)
        
    Returns:
        Tuple of (slopes, anomalies)
        - slopes: Array of calculated slopes
        - anomalies: List of anomaly dicts with type, cycle_start, cycle_end, stroke_count
    """
    # Calculate rolling slopes
    slopes = calculate_rolling_slopes(raw_deltas, window_size)
    
    # Find zero crossings with minimum phase duration filtering
    # Convert min_phase_duration_ms to approximate sample count
    # We need to accumulate delta times to track elapsed time
    
    crossings = []
    last_crossing_idx = 0
    last_crossing_type = None
    accumulated_time_us = 0.0
    min_phase_duration_us = min_phase_duration_ms * 1000  # Convert ms to us
    
    for i in range(1, len(slopes)):
        if np.isnan(slopes[i]) or np.isnan(slopes[i-1]):
            # Still accumulate time even if slope is NaN
            if i < len(raw_deltas):
                accumulated_time_us += raw_deltas[i]
            continue
        
        # Accumulate time
        if i < len(raw_deltas):
            accumulated_time_us += raw_deltas[i]
        
        prev_sign = slopes[i-1] - slope_threshold
        curr_sign = slopes[i] - slope_threshold
        
        crossing_type = None
        if prev_sign < 0 and curr_sign >= 0:
            crossing_type = 'neg_to_pos'  # Drive to recovery transition (fastest point)
        elif prev_sign >= 0 and curr_sign < 0:
            crossing_type = 'pos_to_neg'  # Recovery to drive transition (slowest point)
        
        if crossing_type is not None:
            # Check if enough time has passed since last crossing
            time_since_last = accumulated_time_us
            
            if last_crossing_type is None or time_since_last >= min_phase_duration_us:
                crossings.append((i, crossing_type))
                last_crossing_idx = i
                last_crossing_type = crossing_type
                accumulated_time_us = 0.0  # Reset accumulated time
    
    # Filter to only pos_to_neg crossings (recovery to drive = slowest point = cycle boundary)
    # This defines recovery-to-recovery cycles
    recovery_points = [idx for idx, ctype in crossings if ctype == 'pos_to_neg']
    
    if len(recovery_points) < 2:
        return slopes, []
    
    # Convert stroke markers to a set of indices for quick lookup
    stroke_indices = {idx for idx, _ in stroke_markers}
    stroke_by_index = {idx: count for idx, count in stroke_markers}
    
    anomalies = []
    
    # Check each cycle (between consecutive recovery points - slowest to slowest)
    for i in range(len(recovery_points) - 1):
        cycle_start = recovery_points[i]
        cycle_end = recovery_points[i + 1]
        
        # Count strokes in this cycle
        strokes_in_cycle = [
            (idx, stroke_by_index[idx]) 
            for idx in stroke_indices 
            if cycle_start <= idx < cycle_end
        ]
        
        stroke_count = len(strokes_in_cycle)
        
        if stroke_count == 0:
            anomalies.append({
                'type': 'MISSED',
                'cycle_start': cycle_start,
                'cycle_end': cycle_end,
                'stroke_count': 0,
                'strokes': []
            })
        elif stroke_count > 1:
            anomalies.append({
                'type': 'DUPLICATE',
                'cycle_start': cycle_start,
                'cycle_end': cycle_end,
                'stroke_count': stroke_count,
                'strokes': strokes_in_cycle
            })
        # stroke_count == 1 is OK, no anomaly
    
    return slopes, anomalies


class DataVisualizer:
    """Main application window for visualizing ESP Rowing Monitor data."""

    def __init__(self):
        self.root = tk.Tk()
        self.root.title("ESP Rowing Monitor Data Visualizer")
        try:
            icon_path = os.path.join(
                getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__))),
                "calibration-helper-GUI.ico"
            )
            if sys.platform == 'win32':
                self.root.iconbitmap(icon_path)
            else:
                # tkinter on Linux/macOS does not support .ico via iconbitmap;
                # use iconphoto with PIL instead
                from PIL import Image, ImageTk  # noqa: PLC0415
                _img = Image.open(icon_path)
                self._icon_photo = ImageTk.PhotoImage(_img)  # keep reference alive
                self.root.iconphoto(True, self._icon_photo)
        except Exception as e:
            print(f"[Warning] failed to load app icon: {e}")
            pass  # icon is cosmetic – never crash on failure
        self.root.geometry("1400x900")
        
        # Multi-file storage
        self.loaded_files: dict[str, FileData] = {}  # key = filepath
        self.current_file: Optional[FileData] = None
        
        # Handle forces UI elements
        self.forces_fig = None
        self.forces_axes = None
        self.forces_canvas = None
        self.stroke_slider = None
        self.stroke_entry = None
        self.stroke_label = None
        self.forces_stats_labels = {}  # Dict to store stat label widgets
        self.forces_stats_data = None  # Cached stats data
        self.forces_stats_tooltip = None  # Tooltip label for hover
        self.summary_expanded = False  # State for collapsible summary section (start collapsed)
        self.summary_frame = None  # Reference to summary section
        self.summary_toggle_btn = None  # Reference to toggle button
        self.summary_content_frame = None  # Reference to content frame
        self.stroke_metrics_text = None  # Text widget for per-stroke metrics in chart
        
        # Delta times UI elements
        self.delta_fig = None
        self.delta_ax = None
        self.delta_canvas = None
        self.delta_toolbar = None
        self.delta_x_data = None
        self.delta_x_scrollbar = None
        self.delta_y_min_entry = None
        self.delta_y_max_entry = None
        self.delta_y_min = None
        self.delta_y_max = None
        
        # Stroke detection UI elements
        self.sd_fig = None
        self.sd_ax = None
        self.sd_canvas = None
        self.sd_window_size = None
        self.sd_slope_threshold = None
        self.sd_anomaly_listbox = None
        self.sd_stats_label = None
        self.stroke_line_positions = []
        self.sd_hover_annotation = None
        
        # File selector UI elements
        self.file_selector_frame = None
        self.file_dropdown = None
        self.open_file_btn = None
        self.close_file_btn = None
        self.close_all_btn = None
        self.reload_btn = None
        self.reload_all_btn = None
        
        # Summary tab UI elements
        self.summary_tab_frame = None
        self.summary_tree = None
        self.summary_tooltip = None
        
        # Sync view state memory (remembers which files were synced per tab)
        self.sync_state_memory: dict[str, set[str]] = {
            'delta': set(),
            'stroke_detection': set()
        }
        
        # Handle window close properly
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
        
        # Bind Ctrl+C to copy active chart
        self.root.bind('<Control-c>', self._on_copy_chart)
        
        # Create UI
        self._create_menu()
        self._create_main_frame()
    
    def _clear_figures(self):
        """Clear matplotlib figures to free memory (but keep file data)."""
        # Close all matplotlib figures
        if self.delta_fig is not None:
            plt.close(self.delta_fig)
            self.delta_fig = None
        if self.forces_fig is not None:
            plt.close(self.forces_fig)
            self.forces_fig = None
        if self.sd_fig is not None:
            plt.close(self.sd_fig)
            self.sd_fig = None
        
        # Clear UI references
        self.delta_ax = None
        self.delta_canvas = None
        self.delta_toolbar = None
        self.delta_x_data = None
        self.delta_x_scrollbar = None
        self.delta_y_min_entry = None
        self.delta_y_max_entry = None
        self.forces_axes = None
        self.forces_canvas = None
        self.stroke_slider = None
        self.stroke_entry = None
        self.stroke_label = None
        self.forces_stats_labels = {}
        self.forces_stats_data = None
        self.forces_stats_tooltip = None
        self.sd_ax = None
        self.sd_canvas = None
        self.sd_window_size = None
        self.sd_slope_threshold = None
        self.sd_anomaly_listbox = None
        self.sd_stats_label = None
        self.stroke_line_positions = []
        self.sd_hover_annotation = None
        
        # Force garbage collection
        gc.collect()
    
    def _update_file_dropdown(self):
        """Update the file dropdown with currently loaded files."""
        if self.file_dropdown is None:
            return
        
        # Get display names
        display_names = [fd.display_name for fd in self.loaded_files.values()]
        self.file_dropdown['values'] = display_names
        
        # Set current selection
        if self.current_file:
            self.file_dropdown.set(self.current_file.display_name)
        elif display_names:
            self.file_dropdown.set(display_names[0])
        else:
            self.file_dropdown.set('')
        
        # Update button states
        self._update_button_states()
    
    def _update_button_states(self):
        """Update enabled/disabled state of file management buttons."""
        num_files = len(self.loaded_files)
        
        # Disable Open if at max files
        if self.open_file_btn:
            if num_files >= MAX_LOADED_FILES:
                self.open_file_btn.config(state=tk.DISABLED)
            else:
                self.open_file_btn.config(state=tk.NORMAL)
        
        # Enable/disable close buttons based on loaded files
        if self.close_file_btn:
            self.close_file_btn.config(state=tk.NORMAL if num_files > 0 else tk.DISABLED)
        if self.close_all_btn:
            self.close_all_btn.config(state=tk.NORMAL if num_files > 0 else tk.DISABLED)
        if self.reload_btn:
            self.reload_btn.config(state=tk.NORMAL if num_files > 0 else tk.DISABLED)
        if self.reload_all_btn:
            self.reload_all_btn.config(state=tk.NORMAL if num_files > 0 else tk.DISABLED)
        
    def _create_menu(self):
        """Create the application menu bar."""
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Open...", command=self._open_file, accelerator="Ctrl+O")
        file_menu.add_command(label="Reload", command=self._reload_current_file, accelerator="Ctrl+R")
        file_menu.add_separator()
        file_menu.add_command(label="Close Current", command=self._close_current_file)
        file_menu.add_command(label="Close All", command=self._close_all_files)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self._on_close)
        
        # Bind keyboard shortcuts
        self.root.bind('<Control-o>', lambda e: self._open_file())
        self.root.bind('<Control-r>', lambda e: self._reload_current_file())
        
    def _create_main_frame(self):
        """Create the main content frame with tabs."""
        self.main_frame = tk.Frame(self.root)
        self.main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # File selector frame at top
        self.file_selector_frame = tk.Frame(self.main_frame)
        self.file_selector_frame.pack(side=tk.TOP, fill=tk.X, pady=5)
        
        tk.Label(
            self.file_selector_frame, 
            text="Active File:", 
            font=('Arial', 10)
        ).pack(side=tk.LEFT, padx=5)
        
        self.file_dropdown = ttk.Combobox(
            self.file_selector_frame,
            state="readonly",
            width=60
        )
        self.file_dropdown.pack(side=tk.LEFT, padx=5)
        self.file_dropdown.bind('<<ComboboxSelected>>', self._on_file_selected)
        
        # File management buttons
        self.open_file_btn = tk.Button(
            self.file_selector_frame,
            text="Open File",
            command=self._open_file,
            font=('Arial', 9)
        )
        self.open_file_btn.pack(side=tk.LEFT, padx=5)
        
        self.close_file_btn = tk.Button(
            self.file_selector_frame,
            text="Close File",
            command=self._close_current_file,
            font=('Arial', 9),
            state=tk.DISABLED
        )
        self.close_file_btn.pack(side=tk.LEFT, padx=5)
        
        self.close_all_btn = tk.Button(
            self.file_selector_frame,
            text="Close All",
            command=self._close_all_files,
            font=('Arial', 9),
            state=tk.DISABLED
        )
        self.close_all_btn.pack(side=tk.LEFT, padx=5)
        
        self.reload_btn = tk.Button(
            self.file_selector_frame,
            text="Reload",
            command=self._reload_current_file,
            font=('Arial', 9),
            state=tk.DISABLED
        )
        self.reload_btn.pack(side=tk.LEFT, padx=5)
        
        self.reload_all_btn = tk.Button(
            self.file_selector_frame,
            text="Reload All",
            command=self._reload_all_files,
            font=('Arial', 9),
            state=tk.DISABLED
        )
        self.reload_all_btn.pack(side=tk.LEFT, padx=5)
        
        # Files loaded indicator
        self.files_count_label = tk.Label(
            self.file_selector_frame,
            text=f"(0/{MAX_LOADED_FILES} files)",
            font=('Arial', 9)
        )
        self.files_count_label.pack(side=tk.RIGHT, padx=10)
        
        # Info label
        self.info_label = tk.Label(
            self.main_frame, 
            text="Use File → Open or 'Open File' button to load a data file",
            font=('Arial', 12)
        )
        self.info_label.pack(pady=20)
        
        # Placeholder for notebook (tabs)
        self.notebook = None
        self.delta_frame = None
        self.forces_frame = None
    
    def _on_file_selected(self, event=None):
        """Handle file selection from dropdown."""
        selected_name = self.file_dropdown.get()
        if not selected_name:
            return
        
        # Find the file with this display name
        for filepath, file_data in self.loaded_files.items():
            if file_data.display_name == selected_name:
                if self.current_file and filepath == self.current_file.filepath:
                    return  # Already selected
                
                # Save current state before switching
                self._save_current_state()
                
                # Switch to new file
                self.current_file = file_data
                
                # Clear figures and rebuild UI
                self._clear_figures()
                self._refresh_ui()
                break
    
    def _save_current_state(self):
        """Save current UI state back to the current FileData."""
        if self.current_file is None:
            return
        
        # Save navigation indices
        if hasattr(self, 'stroke_slider') and self.stroke_slider:
            try:
                self.current_file.current_stroke_index = int(self.stroke_slider.get())
            except:
                pass
        
        # Save analysis settings if controls exist
        if self.sd_window_size and self.sd_slope_threshold:
            try:
                self.current_file.analysis_settings = {
                    'window_size': int(self.sd_window_size.get()),
                    'slope_threshold': float(self.sd_slope_threshold.get())
                }
            except:
                pass
        
        # Save view state (zoom/pan position) for each chart
        if self.delta_ax is not None:
            try:
                self.current_file.delta_view_xlim = self.delta_ax.get_xlim()
                self.current_file.delta_view_ylim = self.delta_ax.get_ylim()
            except:
                pass
        
        if self.forces_axes is not None:
            try:
                self.current_file.forces_view_xlim = self.forces_axes[0].get_xlim()
                self.current_file.forces_view_ylim = self.forces_axes[0].get_ylim()
            except:
                pass
        
        if self.sd_ax is not None:
            try:
                self.current_file.stroke_detection_view_xlim = self.sd_ax.get_xlim()
                self.current_file.stroke_detection_view_ylim = self.sd_ax.get_ylim()
            except:
                pass
    
    def _refresh_ui(self):
        """Refresh the UI with current file's data."""
        if self.current_file is None:
            self.info_label.config(text="Use File → Open or 'Open File' button to load a data file")
            if self.notebook:
                self.notebook.destroy()
                self.notebook = None
            return
        
        # Update info label
        info_parts = []
        if self.current_file.has_delta():
            info_parts.append(f"{len(self.current_file.raw_deltas):,} delta time points")
        if self.current_file.has_forces():
            info_parts.append(f"{len(self.current_file.handle_forces):,} force curves")
        if self.current_file.has_stroke_data():
            info_parts.append(f"{len(self.current_file.stroke_markers):,} stroke markers")
        
        self.info_label.config(
            text=f"Loaded: {self.current_file.display_name} - " + ", ".join(info_parts)
        )
        
        # Create tabs
        self._create_tabs(
            self.current_file.has_delta(),
            self.current_file.has_forces(),
            self.current_file.has_stroke_data()
        )
    
    def _close_current_file(self):
        """Close the currently selected file."""
        if self.current_file is None:
            return
        
        filepath = self.current_file.filepath
        
        # Remove from loaded files
        if filepath in self.loaded_files:
            del self.loaded_files[filepath]
        
        # Clear current file
        self.current_file = None
        
        # Clear figures
        self._clear_figures()
        
        # Select another file if available
        if self.loaded_files:
            # Select first available file
            first_filepath = next(iter(self.loaded_files))
            self.current_file = self.loaded_files[first_filepath]
            self._refresh_ui()
        else:
            # No files left
            if self.notebook:
                self.notebook.destroy()
                self.notebook = None
            self.info_label.config(text="Use File → Open or 'Open File' button to load a data file")
        
        # Update dropdown
        self._update_file_dropdown()
        self._update_files_count()
        # Refresh summary view when files change
        try:
            self._refresh_summary_view()
        except Exception:
            pass
        gc.collect()
    
    def _close_all_files(self):
        """Close all loaded files."""
        # Clear all figures
        self._clear_figures()
        
        # Clear all data
        self.loaded_files.clear()
        self.current_file = None
        
        # Update UI
        if self.notebook:
            self.notebook.destroy()
            self.notebook = None
        
        self.info_label.config(text="Use File → Open or 'Open File' button to load a data file")
        self._update_file_dropdown()
        self._update_files_count()
        # Refresh summary view after closing all files
        try:
            self._refresh_summary_view()
        except Exception:
            pass
        gc.collect()
    
    def _reload_current_file(self):
        """Reload the current file from disk, preserving zoom state."""
        if self.current_file is None:
            return
        
        filepath = self.current_file.filepath
        
        # Check if file still exists
        if not os.path.exists(filepath):
            messagebox.showerror("Error", f"File no longer exists:\n{filepath}")
            return
        
        # Save current view state before reloading
        self._save_current_state()
        
        # Store the view state we want to preserve
        saved_delta_xlim = self.current_file.delta_view_xlim
        saved_delta_ylim = self.current_file.delta_view_ylim
        saved_stroke_xlim = self.current_file.stroke_detection_view_xlim
        saved_stroke_ylim = self.current_file.stroke_detection_view_ylim
        saved_stroke_index = self.current_file.current_stroke_index
        saved_analysis_settings = self.current_file.analysis_settings
        display_name = self.current_file.display_name
        
        try:
            # Show progress in info label
            self.info_label.config(text=f"⏳ Reloading {filepath}...")
            self.root.update()
            
            # Re-parse file data
            (raw_deltas, clean_deltas, handle_forces, 
             power_list, drag_factor_list, distance_list,
             drive_duration_list, recovery_duration_list) = parse_data(filepath)
            stroke_raw_deltas, stroke_clean_deltas, stroke_markers = parse_stroke_data(filepath)
            
            # Create new FileData with preserved state
            file_data = FileData(
                filepath=filepath,
                display_name=display_name,
                raw_deltas=raw_deltas,
                clean_deltas=clean_deltas,
                handle_forces=handle_forces,
                stroke_power=power_list,
                stroke_drag_factor=drag_factor_list,
                stroke_distance=distance_list,
                stroke_drive_duration=drive_duration_list,
                stroke_recovery_duration=recovery_duration_list,
                stroke_raw_deltas=stroke_raw_deltas,
                stroke_clean_deltas=stroke_clean_deltas,
                stroke_markers=stroke_markers,
                # Preserve view state
                delta_view_xlim=saved_delta_xlim,
                delta_view_ylim=saved_delta_ylim,
                stroke_detection_view_xlim=saved_stroke_xlim,
                stroke_detection_view_ylim=saved_stroke_ylim,
                current_stroke_index=min(saved_stroke_index, len(handle_forces) - 1) if handle_forces else 0,
                analysis_settings=saved_analysis_settings
            )
            
            if not file_data.has_delta() and not file_data.has_forces() and not file_data.has_stroke_data():
                messagebox.showerror("Error", "No valid data found after reload.")
                return
            
            # Replace in loaded files and set as current
            self.loaded_files[filepath] = file_data
            self.current_file = file_data
            
            # Clear figures and refresh UI
            self._clear_figures()
            self._refresh_ui()
            # Refresh summary view after reload
            try:
                self._refresh_summary_view()
            except Exception:
                pass
            
            self.info_label.config(
                text=f"✓ Reloaded: {display_name}"
            )
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to reload file:\n{str(e)}")
            self.info_label.config(text=f"Loaded: {display_name}")
    
    def _reload_all_files(self):
        """Reload all loaded files from disk, preserving current selection and view states."""
        if not self.loaded_files:
            messagebox.showinfo("No Files", "No files are currently loaded.")
            return
        
        # Save current file path
        current_filepath = self.current_file.filepath if self.current_file else None
        
        try:
            num_reloaded = 0
            failed_files = []
            total_files = len(self.loaded_files)
            file_list = list(self.loaded_files.keys())
            
            # Reload each file with progress indicator
            for idx, filepath in enumerate(file_list, 1):
                # Update progress
                progress_text = f"⏳ Reloading files... ({idx}/{total_files}) {os.path.basename(filepath)}"
                self.info_label.config(text=progress_text)
                self.root.update()
                
                if not os.path.exists(filepath):
                    failed_files.append(os.path.basename(filepath))
                    continue
                
                try:
                    # Parse the file again
                    (raw_deltas, clean_deltas, handle_forces, 
                     power_list, drag_list, distance_list, 
                     drive_duration_list, recovery_duration_list) = parse_data(filepath)
                    
                    stroke_raw_deltas, stroke_clean_deltas, stroke_markers = parse_stroke_data(filepath)
                    
                    # Update file data
                    file_data = self.loaded_files[filepath]
                    file_data.raw_deltas = raw_deltas
                    file_data.clean_deltas = clean_deltas
                    file_data.handle_forces = handle_forces
                    file_data.stroke_power = power_list
                    file_data.stroke_drag_factor = drag_list
                    file_data.stroke_distance = distance_list
                    file_data.stroke_drive_duration = drive_duration_list
                    file_data.stroke_recovery_duration = recovery_duration_list
                    file_data.stroke_raw_deltas = stroke_raw_deltas
                    file_data.stroke_clean_deltas = stroke_clean_deltas
                    file_data.stroke_markers = stroke_markers
                    file_data.stroke_slopes = None  # Clear cached analysis
                    file_data.stroke_anomalies = []
                    
                    num_reloaded += 1
                    
                except Exception as e:
                    failed_files.append(os.path.basename(filepath))
                    continue
            
            # Restore current file selection if it still exists
            if current_filepath and current_filepath in self.loaded_files:
                self.current_file = self.loaded_files[current_filepath]
            elif self.loaded_files:
                # Select first available file
                self.current_file = next(iter(self.loaded_files.values()))
            else:
                self.current_file = None
            
            # Refresh UI
            self._update_file_dropdown()
            if self.current_file:
                self._clear_figures()
                self._refresh_ui()
            
            # Update summary view
            try:
                self._refresh_summary_view()
            except Exception:
                pass
            
            # Show summary message
            message = f"✓ Reloaded {num_reloaded} file(s)."
            if failed_files:
                message += f"\n\nFailed to reload: {', '.join(failed_files)}"
            
            messagebox.showinfo("Reload Complete", message)
            
        except Exception as e:
            messagebox.showerror("Error", f"Error reloading files: {str(e)}")
    
    def _update_files_count(self):
        """Update the files count label."""
        if self.files_count_label:
            self.files_count_label.config(text=f"({len(self.loaded_files)}/{MAX_LOADED_FILES} files)")
        
    def _open_file(self):
        """Open file dialog and load the selected data file(s)."""
        # Check if at max files
        if len(self.loaded_files) >= MAX_LOADED_FILES:
            messagebox.showwarning(
                "Maximum Files Reached",
                f"You can only have {MAX_LOADED_FILES} files open at once.\n"
                "Please close some files before opening new ones."
            )
            return
        
        filepaths = filedialog.askopenfilenames(
            parent=self.root,
            title="Select Data File(s)",
            filetypes=[
                ("Text files", "*.txt"),
                ("All files", "*.*")
            ]
        )
        
        if not filepaths:
            return
        
        # Process all selected files
        loaded_new_files = []
        skipped_files = []
        already_loaded = []
        
        for filepath in filepaths:
            # Check if at max files
            if len(self.loaded_files) >= MAX_LOADED_FILES:
                skipped_files.append(os.path.basename(filepath))
                continue
            
            # Check if already loaded
            if filepath in self.loaded_files:
                already_loaded.append(os.path.basename(filepath))
                continue
            
            try:
                self.info_label.config(text=f"Loading {os.path.basename(filepath)}...")
                self.root.update()
                
                # Parse file data
                (raw_deltas, clean_deltas, handle_forces,
                 power_list, drag_factor_list, distance_list,
                 drive_duration_list, recovery_duration_list) = parse_data(filepath)
                stroke_raw_deltas, stroke_clean_deltas, stroke_markers = parse_stroke_data(filepath)
                
                # Create FileData object
                display_name = os.path.basename(filepath)
                
                # Handle duplicate display names by adding a suffix
                existing_names = {fd.display_name for fd in self.loaded_files.values()}
                if display_name in existing_names:
                    counter = 2
                    base_name = display_name
                    while display_name in existing_names:
                        display_name = f"{base_name} ({counter})"
                        counter += 1
                
                file_data = FileData(
                    filepath=filepath,
                    display_name=display_name,
                    raw_deltas=raw_deltas,
                    clean_deltas=clean_deltas,
                    handle_forces=handle_forces,
                    stroke_power=power_list,
                    stroke_drag_factor=drag_factor_list,
                    stroke_distance=distance_list,
                    stroke_drive_duration=drive_duration_list,
                    stroke_recovery_duration=recovery_duration_list,
                    stroke_raw_deltas=stroke_raw_deltas,
                    stroke_clean_deltas=stroke_clean_deltas,
                    stroke_markers=stroke_markers
                )
                
                if not file_data.has_delta() and not file_data.has_forces() and not file_data.has_stroke_data():
                    messagebox.showerror(
                        "Error", 
                        f"No valid data found in {os.path.basename(filepath)}.\n\n"
                        "Expected formats:\n"
                        "- Delta times: two comma-separated numbers per line (e.g., 19436.00,19436.95)\n"
                        "- Handle forces: handleForces: [num,num,...]\n"
                        "- Stroke markers: deltaTime: <number>"
                    )
                    continue
                
                # Add to loaded files
                self.loaded_files[filepath] = file_data
                loaded_new_files.append(os.path.basename(filepath))
                
            except Exception as e:
                skipped_files.append(f"{os.path.basename(filepath)}: {str(e)}")
        
        # If we loaded new files, update UI
        if loaded_new_files:
            # Save current state before switching
            self._save_current_state()
            
            # Set first newly loaded file as current
            first_new_filepath = [fp for fp in filepaths if os.path.basename(fp) in loaded_new_files][0]
            self.current_file = self.loaded_files[first_new_filepath]
            
            # Clear figures and refresh UI
            self._clear_figures()
            self._update_file_dropdown()
            self._update_files_count()
            self._refresh_ui()
            # Ensure summary tab is up-to-date
            try:
                self._refresh_summary_view()
            except Exception:
                pass
        
        # Show summary message if there were any issues
        if skipped_files or already_loaded:
            message_parts = []
            if loaded_new_files:
                message_parts.append(f"Loaded {len(loaded_new_files)} file(s):\n" + "\n".join(f"  • {f}" for f in loaded_new_files))
            if already_loaded:
                message_parts.append(f"\nAlready loaded ({len(already_loaded)}):\n" + "\n".join(f"  • {f}" for f in already_loaded))
            if skipped_files:
                message_parts.append(f"\nSkipped ({len(skipped_files)}):\n" + "\n".join(f"  • {f}" for f in skipped_files))
            
            messagebox.showinfo("File Loading Summary", "\n".join(message_parts))
            
    def _create_tabs(self, has_delta: bool, has_forces: bool, has_stroke_data: bool = False):
        """Create the tabbed interface."""
        # Remember current tab index before destroying
        current_tab_index = 0
        if self.notebook:
            try:
                current_tab_index = self.notebook.index(self.notebook.select())
            except:
                current_tab_index = 0
            self.notebook.destroy()
            
        self.notebook = ttk.Notebook(self.main_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True)
        
        # Track tab names for restoring selection
        tab_names = []
        
        # Create Delta Times tab
        if has_delta:
            self.delta_frame = tk.Frame(self.notebook)
            self.notebook.add(self.delta_frame, text="Delta Times")
            tab_names.append("Delta Times")
            self._create_delta_chart()
        
        # Create Handle Forces tab
        if has_forces:
            self.forces_frame = tk.Frame(self.notebook)
            self.notebook.add(self.forces_frame, text="Handle Forces")
            tab_names.append("Handle Forces")
            self._create_forces_view()
        
        # Create Stroke Detection tab
        if has_stroke_data:
            self.stroke_detection_frame = tk.Frame(self.notebook)
            self.notebook.add(self.stroke_detection_frame, text="Stroke Detection")
            tab_names.append("Stroke Detection")
            self._create_stroke_detection_view()
        
        # Create Summary tab (only when files are loaded)
        if len(self.loaded_files) > 0:
            self.summary_tab_frame = tk.Frame(self.notebook)
            self.notebook.add(self.summary_tab_frame, text="Summary")
            tab_names.append("Summary")
            self._create_summary_view()
        
        # Restore previous tab selection if possible
        if tab_names and current_tab_index < len(tab_names):
            self.notebook.select(current_tab_index)
    
    def _create_summary_view(self):
        """Create the summary tab with a table showing stats for all loaded files."""
        if self.summary_tab_frame is None:
            return
        
        # Clear any existing widgets in the frame
        for widget in self.summary_tab_frame.winfo_children():
            widget.destroy()
        
        # Toolbar at top
        toolbar_frame = tk.Frame(self.summary_tab_frame)
        toolbar_frame.pack(side=tk.TOP, fill=tk.X, padx=10, pady=5)
        
        tk.Label(
            toolbar_frame,
            text="Workout Summary - All Open Files",
            font=('Arial', 12, 'bold')
        ).pack(side=tk.LEFT, padx=5)
        
        copy_csv_btn = tk.Button(
            toolbar_frame,
            text="Export CSV",
            command=self._export_summary_csv_to_file,
            font=('Arial', 10),
            bg='#4CAF50',
            fg='white',
            padx=15,
            pady=5
        ).pack(side=tk.RIGHT, padx=5)
        
        # Treeview for the table
        tree_frame = tk.Frame(self.summary_tab_frame)
        tree_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        # Scrollbars
        vsb = tk.Scrollbar(tree_frame, orient="vertical")
        hsb = tk.Scrollbar(tree_frame, orient="horizontal")
        
        # Define columns
        columns = (
            'file', 'strokes', 'min_dp', 'max_dp', 'avg_dp', 'std_dp',
            'distance', 'avg_power', 'avg_spm', 'avg_drag'
        )
        
        self.summary_tree = ttk.Treeview(
            tree_frame,
            columns=columns,
            show='headings',
            yscrollcommand=vsb.set,
            xscrollcommand=hsb.set
        )
        
        vsb.config(command=self.summary_tree.yview)
        hsb.config(command=self.summary_tree.xview)
        
        # Column headings
        self.summary_tree.heading('file', text='File')
        self.summary_tree.heading('strokes', text='Stroke Count')
        self.summary_tree.heading('min_dp', text='Min DataPoints')
        self.summary_tree.heading('max_dp', text='Max DataPoints')
        self.summary_tree.heading('avg_dp', text='Avg DataPoints')
        self.summary_tree.heading('std_dp', text='StdDev DataPoints')
        self.summary_tree.heading('distance', text='Distance (m)')
        self.summary_tree.heading('avg_power', text='Avg Power (W)')
        self.summary_tree.heading('avg_spm', text='Avg SPM')
        self.summary_tree.heading('avg_drag', text='Avg Drag')
        
        # Column widths: first column stretches, others fixed
        self.summary_tree.column('file', width=300, anchor='w', stretch=True)
        self.summary_tree.column('strokes', width=100, anchor='center', stretch=False)
        self.summary_tree.column('min_dp', width=110, anchor='center', stretch=False)
        self.summary_tree.column('max_dp', width=110, anchor='center', stretch=False)
        self.summary_tree.column('avg_dp', width=110, anchor='center', stretch=False)
        self.summary_tree.column('std_dp', width=130, anchor='center', stretch=False)
        self.summary_tree.column('distance', width=110, anchor='e', stretch=False)
        self.summary_tree.column('avg_power', width=110, anchor='e', stretch=False)
        self.summary_tree.column('avg_spm', width=90, anchor='e', stretch=False)
        self.summary_tree.column('avg_drag', width=90, anchor='e', stretch=False)
        
        # Pack
        self.summary_tree.grid(row=0, column=0, sticky='nsew')
        vsb.grid(row=0, column=1, sticky='ns')
        hsb.grid(row=1, column=0, sticky='ew')
        tree_frame.grid_rowconfigure(0, weight=1)
        tree_frame.grid_columnconfigure(0, weight=1)
        
        # Create tooltip for hover (reuse forces_stats_tooltip style)
        self.summary_tooltip = tk.Label(
            self.root,
            text="",
            font=('Arial', 9),
            bg='#FFFACD',
            relief=tk.SOLID,
            borderwidth=1,
            padx=5,
            pady=2
        )
        
        # Bind hover events
        self.summary_tree.bind('<Motion>', self._on_summary_hover)
        self.summary_tree.bind('<Leave>', self._on_summary_leave)
        
        # Populate rows
        self._refresh_summary_view()
    
    def _compute_summary_row(self, file_data: 'FileData') -> dict:
        """Compute summary statistics for a single file.
        
        Args:
            file_data: FileData instance
            
        Returns:
            Dict with keys matching summary table columns
        """
        # Get summary metrics
        summary = {
            'total_distance': None,
            'avg_power': None,
            'avg_drag_factor': None,
            'stroke_count': 0,
            'total_time': None,
            'avg_stroke_rate': None
        }
        
        if file_data.has_forces():
            stroke_count = len(file_data.handle_forces)
            summary['stroke_count'] = stroke_count
            
            # Distance
            if file_data.stroke_distance:
                valid_distances = [d for d in file_data.stroke_distance if d is not None and d >= 0]
                if valid_distances:
                    summary['total_distance'] = valid_distances[-1]
            
            # Power
            if file_data.stroke_power:
                valid_power = [p for p in file_data.stroke_power if p is not None and p >= 0]
                if valid_power:
                    summary['avg_power'] = sum(valid_power) / len(valid_power)
            
            # Drag factor
            if file_data.stroke_drag_factor:
                valid_drag = [d for d in file_data.stroke_drag_factor if d is not None and d >= 0]
                if valid_drag:
                    summary['avg_drag_factor'] = sum(valid_drag) / len(valid_drag)
            
            # SPM
            if file_data.stroke_drive_duration and file_data.stroke_recovery_duration:
                valid_pairs = [
                    (drive, recovery)
                    for drive, recovery in zip(file_data.stroke_drive_duration,
                                              file_data.stroke_recovery_duration)
                    if drive is not None and recovery is not None and drive >= 0 and recovery >= 0
                ]
                if valid_pairs:
                    total_time = sum(drive + recovery for drive, recovery in valid_pairs)
                    if total_time > 0:
                        summary['avg_stroke_rate'] = (len(valid_pairs) / total_time) * 60
        
        # Datapoint stats
        dp_stats = {'min': None, 'max': None, 'avg': None, 'std': None,
                   'min_strokes': [], 'max_strokes': []}
        if file_data.has_forces() and file_data.handle_forces:
            counts = [len(forces) for forces in file_data.handle_forces]
            if counts:
                dp_stats['min'] = min(counts)
                dp_stats['max'] = max(counts)
                dp_stats['avg'] = float(np.mean(counts))
                dp_stats['std'] = float(np.std(counts))
                
                # Find which strokes have min/max
                dp_stats['min_strokes'] = [i + 1 for i, c in enumerate(counts) if c == dp_stats['min']]
                dp_stats['max_strokes'] = [i + 1 for i, c in enumerate(counts) if c == dp_stats['max']]
        
        return {
            'file': file_data.display_name,
            'strokes': summary['stroke_count'],
            'min_dp': dp_stats['min'],
            'max_dp': dp_stats['max'],
            'avg_dp': dp_stats['avg'],
            'std_dp': dp_stats['std'],
            'distance': summary['total_distance'],
            'avg_power': summary['avg_power'],
            'avg_spm': summary['avg_stroke_rate'],
            'avg_drag': summary['avg_drag_factor'],
            'min_strokes': dp_stats['min_strokes'],
            'max_strokes': dp_stats['max_strokes']
        }
    
    def _refresh_summary_view(self):
        """Refresh the summary table with current loaded files."""
        if self.summary_tree is None:
            return
        
        # Clear existing rows
        for item in self.summary_tree.get_children():
            self.summary_tree.delete(item)
        
        # Add row for each loaded file
        for filepath, file_data in self.loaded_files.items():
            row = self._compute_summary_row(file_data)
            
            # Format values
            values = (
                row['file'],
                row['strokes'],
                row['min_dp'] if row['min_dp'] is not None else 'N/A',
                row['max_dp'] if row['max_dp'] is not None else 'N/A',
                f"{row['avg_dp']:.1f}" if row['avg_dp'] is not None else 'N/A',
                f"{row['std_dp']:.2f}" if row['std_dp'] is not None else 'N/A',
                f"{row['distance']:.2f}" if row['distance'] is not None else 'N/A',
                f"{row['avg_power']:.1f}" if row['avg_power'] is not None else 'N/A',
                f"{row['avg_spm']:.1f}" if row['avg_spm'] is not None else 'N/A',
                f"{row['avg_drag']:.1f}" if row['avg_drag'] is not None else 'N/A'
            )
            
            # Insert row into tree
            item_id = self.summary_tree.insert('', 'end', values=values)
        # After inserting rows, populate is complete
        pass
    def _on_summary_hover(self, event):
        """Show tooltip on hover over min/max datapoint cells."""
        if self.summary_tooltip is None or self.summary_tree is None:
            return
        
        # Identify what's under the cursor
        item = self.summary_tree.identify_row(event.y)
        column = self.summary_tree.identify_column(event.x)
        
        if not item or not column:
            self.summary_tooltip.place_forget()
            return
        
        # Column index (column is like '#3')
        col_idx = int(column.replace('#', '')) - 1
        col_names = ['file', 'strokes', 'min_dp', 'max_dp', 'avg_dp', 'std_dp',
                    'distance', 'avg_power', 'avg_spm', 'avg_drag']
        
        if col_idx < 0 or col_idx >= len(col_names):
            self.summary_tooltip.place_forget()
            return
        
        col_name = col_names[col_idx]
        
        # Only show tooltip for min_dp and max_dp columns
        if col_name not in ['min_dp', 'max_dp']:
            self.summary_tooltip.place_forget()
            return
        
        # Get the filepath stored in item
        values = self.summary_tree.item(item, 'values')
        if not values:
            self.summary_tooltip.place_forget()
            return
        
        file_name = values[0]
        
        # Find the FileData
        file_data = None
        for fp, fd in self.loaded_files.items():
            if fd.display_name == file_name:
                file_data = fd
                break
        
        if file_data is None:
            self.summary_tooltip.place_forget()
            return
        
        # Compute row to get stroke lists
        row = self._compute_summary_row(file_data)
        
        strokes = row['min_strokes'] if col_name == 'min_dp' else row['max_strokes']
        
        if not strokes:
            self.summary_tooltip.place_forget()
            return
        
        # Format tooltip text
        if len(strokes) <= 5:
            stroke_text = ", ".join([f"#{s}" for s in strokes])
        else:
            stroke_text = ", ".join([f"#{s}" for s in strokes[:5]]) + f"... (+{len(strokes) - 5} more)"
        
        tooltip_text = f"Stroke(s): {stroke_text}"
        self.summary_tooltip.config(text=tooltip_text)
        
        # Position tooltip
        x = event.x_root + 10
        y = event.y_root + 10
        self.summary_tooltip.lift()
        self.summary_tooltip.place(x=x, y=y)
    
    def _on_summary_leave(self, event=None):
        """Hide summary tooltip."""
        if self.summary_tooltip:
            self.summary_tooltip.place_forget()
    
    def _export_summary_csv_to_file(self):
        """Export the summary table to a CSV file."""
        if self.summary_tree is None:
            return
        
        # Ask user for file path
        filepath = filedialog.asksaveasfilename(
            parent=self.root,
            title="Export Summary as CSV",
            defaultextension=".csv",
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")]
        )
        
        if not filepath:
            return  # User cancelled
        
        # Build CSV
        lines = []
        
        # Header
        headers = ['File', 'Stroke Count', 'Min DataPoints', 'Max DataPoints',
                  'Avg DataPoints', 'StdDev DataPoints', 'Distance (m)',
                  'Avg Power (W)', 'Avg SPM', 'Avg Drag']
        lines.append(','.join(headers))
        
        # Rows
        for item in self.summary_tree.get_children():
            values = self.summary_tree.item(item, 'values')
            # Escape any commas and quotes in values
            escaped_values = []
            for v in values:
                s = str(v)
                if ',' in s or '"' in s:
                    s = f'"{s.replace('"', '""')}"'
                escaped_values.append(s)
            lines.append(','.join(escaped_values))
        
        csv_text = '\n'.join(lines)
        
        # Write to file
        try:
            with open(filepath, 'w', newline='', encoding='utf-8') as f:
                f.write(csv_text)
            self._show_temporary_status(f"Summary exported to {os.path.basename(filepath)}!")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to export CSV:\n{str(e)}")
            
    def _create_delta_chart(self):
        """Create the delta times chart."""
        if self.current_file is None:
            return
        
        raw_deltas = self.current_file.raw_deltas
        clean_deltas = self.current_file.clean_deltas
        
        # Calculate y-axis range from data
        # Default: min = lowest clean delta - 2000 (padding), max = MAX_DELTA_TIME_THRESHOLD
        min_clean = float(np.min(clean_deltas))
        max_clean = float(np.max(clean_deltas))
        
        # If all data is above the threshold, adjust the max to be above the data
        if min_clean > MAX_DELTA_TIME_THRESHOLD:
            self.delta_y_min = min_clean - 2000
            self.delta_y_max = max_clean + 2000
        else:
            # Normal case: use threshold as max, but ensure it's at least above the data
            self.delta_y_min = min_clean - 2000
            self.delta_y_max = max(MAX_DELTA_TIME_THRESHOLD, max_clean + 2000)
        
        # Create figure with larger size for better visibility
        self.delta_fig, self.delta_ax = plt.subplots(figsize=(12, 6), dpi=100)
        
        # Create x-axis (sample indices)
        self.delta_x_data = np.arange(len(raw_deltas))
        
        # Plot both lines
        self.delta_ax.plot(self.delta_x_data, raw_deltas, 'b-', linewidth=1.5, label='Raw Delta Time', alpha=0.8)
        self.delta_ax.plot(self.delta_x_data, clean_deltas, 'r-', linewidth=1.5, label='Clean Delta Time', alpha=0.8)
        
        # Configure axes
        self.delta_ax.set_xlabel('Sample Index', fontsize=10)
        self.delta_ax.set_ylabel('Delta Time (μs)', fontsize=10)
        self.delta_ax.set_title('Cyclic Error Filter: Raw vs Clean Delta Times', fontsize=12)
        self.delta_ax.legend(loc='upper right')
        self.delta_ax.grid(True, alpha=0.3)
        
        # Set default Y limits
        self.delta_ax.set_ylim(self.delta_y_min, self.delta_y_max)
        
        # Enable tight layout
        self.delta_fig.tight_layout()
        
        # Embed in tkinter
        self.delta_canvas = FigureCanvasTkAgg(self.delta_fig, master=self.delta_frame)
        self.delta_canvas.draw()
        
        # Add navigation toolbar for zoom/pan with custom home
        toolbar_frame = tk.Frame(self.delta_frame)
        toolbar_frame.pack(side=tk.TOP, fill=tk.X)
        self.delta_toolbar = CustomNavigationToolbar(
            self.delta_canvas, 
            toolbar_frame,
            home_callback=self._reset_delta_view
        )
        self.delta_toolbar.update()
        
        # Add stats frame first (at bottom, packed first so it reserves space)
        stats_frame = tk.Frame(self.delta_frame)
        stats_frame.pack(side=tk.BOTTOM, fill=tk.X, pady=5)
        
        # Create x_scroll_frame before it's used
        x_scroll_frame = tk.Frame(self.delta_frame)
        
        stats_text = (
            f"Data points: {len(raw_deltas):,} | "
            f"Raw: min={raw_deltas.min():.2f}, max={raw_deltas.max():.2f}, "
            f"mean={raw_deltas.mean():.2f} | "
            f"Clean: min={clean_deltas.min():.2f}, max={clean_deltas.max():.2f}, "
            f"mean={clean_deltas.mean():.2f}"
        )
        stats_label = tk.Label(stats_frame, text=stats_text, font=('Arial', 9))
        stats_label.pack()
        
        # Add Y-axis min/max control frame
        y_control_frame = tk.Frame(self.delta_frame)
        y_control_frame.pack(side=tk.BOTTOM, fill=tk.X, pady=2)
        
        tk.Label(y_control_frame, text="Y Min:", font=('Arial', 9)).pack(side=tk.LEFT, padx=5)
        self.delta_y_min_entry = tk.Entry(y_control_frame, width=10, font=('Arial', 9))
        self.delta_y_min_entry.pack(side=tk.LEFT, padx=2)
        self.delta_y_min_entry.insert(0, f"{self.delta_y_min:.0f}")
        self.delta_y_min_entry.bind('<Return>', self._on_delta_y_limits_change)
        
        tk.Label(y_control_frame, text="Y Max:", font=('Arial', 9)).pack(side=tk.LEFT, padx=(15, 5))
        self.delta_y_max_entry = tk.Entry(y_control_frame, width=10, font=('Arial', 9))
        self.delta_y_max_entry.pack(side=tk.LEFT, padx=2)
        self.delta_y_max_entry.insert(0, f"{self.delta_y_max:.0f}")
        self.delta_y_max_entry.bind('<Return>', self._on_delta_y_limits_change)
        
        apply_y_btn = tk.Button(
            y_control_frame,
            text="Apply Y",
            command=self._on_delta_y_limits_change,
            font=('Arial', 9)
        )
        apply_y_btn.pack(side=tk.LEFT, padx=10)
        
        # Add Reset View button
        reset_btn = tk.Button(
            y_control_frame,
            text="Reset View",
            command=self._reset_delta_view,
            font=('Arial', 9)
        )
        reset_btn.pack(side=tk.LEFT, padx=10)
        
        # Add Sync View State button
        sync_btn = tk.Button(
            y_control_frame,
            text="Sync View State",
            command=lambda: self._show_sync_view_dialog('delta'),
            font=('Arial', 9),
            bg='#2196F3',
            fg='white'
        )
        sync_btn.pack(side=tk.LEFT, padx=10)
        
        # Add horizontal scrollbar for x-axis navigation
        x_scroll_frame.pack(side=tk.BOTTOM, fill=tk.X, pady=2)
        
        tk.Label(x_scroll_frame, text="Scroll X:", font=('Arial', 9)).pack(side=tk.LEFT, padx=5)
        
        self.delta_x_scrollbar = tk.Scale(
            x_scroll_frame,
            from_=0,
            to=max(0, len(raw_deltas) - 1),
            orient=tk.HORIZONTAL,
            command=self._on_delta_x_scroll,
            showvalue=True
        )
        self.delta_x_scrollbar.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=10)
        
        # Pack canvas last so it fills remaining space
        self.delta_canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        
        # Restore saved view state (zoom/pan position) if available
        if self.current_file and self.current_file.delta_view_xlim:
            self.delta_ax.set_xlim(self.current_file.delta_view_xlim)
        if self.current_file and self.current_file.delta_view_ylim:
            self.delta_ax.set_ylim(self.current_file.delta_view_ylim)
            # Update entry boxes to reflect restored limits
            y_min, y_max = self.current_file.delta_view_ylim
            self.delta_y_min_entry.delete(0, tk.END)
            self.delta_y_min_entry.insert(0, f"{y_min:.0f}")
            self.delta_y_max_entry.delete(0, tk.END)
            self.delta_y_max_entry.insert(0, f"{y_max:.0f}")
        
        # Final draw
        self.delta_canvas.draw()
        
    def _on_delta_x_scroll(self, value):
        """Handle delta times X scrollbar change - scroll to position while keeping current zoom width."""
        if self.delta_ax is None or self.delta_canvas is None:
            return
        
        # Get the current view width (respects zoom level)
        current_xlim = self.delta_ax.get_xlim()
        current_width = current_xlim[1] - current_xlim[0]
        
        # Calculate new x limits centered on scrollbar position
        center = int(float(value))
        half_width = current_width / 2
        
        x_min = center - half_width
        x_max = center + half_width
        
        # Clamp to data bounds
        data_len = len(self.current_file.raw_deltas) if self.current_file else 0
        if x_min < 0:
            x_min = 0
            x_max = current_width
        if x_max > data_len:
            x_max = data_len
            x_min = max(0, x_max - current_width)
        
        # Set new x limits
        self.delta_ax.set_xlim(x_min, x_max)
        self.delta_canvas.draw_idle()
        
    def _on_delta_y_limits_change(self, event=None):
        """Handle Y-axis limits change from text entries."""
        if self.delta_ax is None or self.delta_canvas is None:
            return
        
        try:
            y_min = float(self.delta_y_min_entry.get())
            y_max = float(self.delta_y_max_entry.get())
            
            if y_min >= y_max:
                messagebox.showwarning("Invalid Range", "Y Min must be less than Y Max")
                return
            
            # Set new y limits
            self.delta_ax.set_ylim(y_min, y_max)
            self.delta_canvas.draw()
            
        except ValueError:
            messagebox.showwarning("Invalid Input", "Please enter valid numbers for Y Min and Y Max")
    
    def _reset_delta_view(self, *args):
        """Reset delta chart view to initial state (full X range, default Y limits)."""
        if self.delta_ax is None or self.delta_canvas is None or self.current_file is None:
            return
        
        # Reset to initial view: full X range
        data_len = len(self.current_file.raw_deltas)
        self.delta_ax.set_xlim(0, data_len)
        
        # Reset Y to defaults using the same logic as initial creation
        min_clean = float(np.min(self.current_file.clean_deltas))
        max_clean = float(np.max(self.current_file.clean_deltas))
        
        # If all data is above the threshold, adjust the max to be above the data
        if min_clean > MAX_DELTA_TIME_THRESHOLD:
            y_min = min_clean - 2000
            y_max = max_clean + 2000
        else:
            # Normal case: use threshold as max, but ensure it's at least above the data
            y_min = min_clean - 2000
            y_max = max(MAX_DELTA_TIME_THRESHOLD, max_clean + 2000)
        
        self.delta_ax.set_ylim(y_min, y_max)
        
        # Update entry boxes
        self.delta_y_min_entry.delete(0, tk.END)
        self.delta_y_min_entry.insert(0, f"{y_min:.0f}")
        self.delta_y_max_entry.delete(0, tk.END)
        self.delta_y_max_entry.insert(0, f"{y_max:.0f}")
        
        # Reset X scrollbar
        if self.delta_x_scrollbar:
            self.delta_x_scrollbar.set(0)
        
        self.delta_canvas.draw()
    
    def _show_sync_view_dialog(self, tab_type: str):
        """Show dialog to select files to sync view state to.
        
        Args:
            tab_type: Either 'delta' for Delta Times tab or 'stroke_detection' for Stroke Detection tab
        """
        if self.current_file is None:
            messagebox.showwarning("No File", "No file is currently open")
            return
        
        if len(self.loaded_files) < 2:
            messagebox.showinfo("Single File", "Only one file is loaded. Open more files to sync view state.")
            return
        
        # Save current view state first
        self._save_current_state()
        
        # Get current view limits based on tab type
        if tab_type == 'delta':
            if self.delta_ax is None:
                messagebox.showwarning("No Chart", "Delta times chart is not available")
                return
            current_xlim = self.delta_ax.get_xlim()
            current_ylim = self.delta_ax.get_ylim()
            title = "Sync Delta Times View State"
            description = f"Current view: X=[{current_xlim[0]:.0f}, {current_xlim[1]:.0f}], Y=[{current_ylim[0]:.0f}, {current_ylim[1]:.0f}]"
        elif tab_type == 'stroke_detection':
            if self.sd_ax is None:
                messagebox.showwarning("No Chart", "Stroke detection chart is not available")
                return
            current_xlim = self.sd_ax.get_xlim()
            current_ylim = self.sd_ax.get_ylim()
            title = "Sync Stroke Detection View State"
            description = f"Current view: X=[{current_xlim[0]:.0f}, {current_xlim[1]:.0f}], Y=[{current_ylim[0]:.0f}, {current_ylim[1]:.0f}]"
        else:
            return
        
        # Create dialog
        dialog = tk.Toplevel(self.root)
        dialog.title(title)
        dialog.geometry("500x400")
        dialog.transient(self.root)
        dialog.grab_set()
        
        # Center the dialog
        dialog.update_idletasks()
        x = self.root.winfo_x() + (self.root.winfo_width() - 500) // 2
        y = self.root.winfo_y() + (self.root.winfo_height() - 400) // 2
        dialog.geometry(f"+{x}+{y}")
        
        # Description label
        tk.Label(
            dialog,
            text=f"Sync view state from: {self.current_file.display_name}",
            font=('Arial', 10, 'bold'),
            wraplength=480
        ).pack(padx=10, pady=(10, 5))
        
        tk.Label(
            dialog,
            text=description,
            font=('Arial', 9),
            fg='#666666'
        ).pack(padx=10, pady=(0, 10))
        
        tk.Label(
            dialog,
            text="Select files to apply this view state to:",
            font=('Arial', 10)
        ).pack(padx=10, pady=5)
        
        # Frame for checkboxes with scrollbar (fixed height)
        list_frame = tk.Frame(dialog, height=200)
        list_frame.pack(fill=tk.X, padx=10, pady=5)
        list_frame.pack_propagate(False)  # Prevent frame from shrinking
        
        canvas = tk.Canvas(list_frame, height=180)
        scrollbar = tk.Scrollbar(list_frame, orient="vertical", command=canvas.yview)
        scrollable_frame = tk.Frame(canvas)
        
        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )
        
        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        
        # Bind mouse wheel scrolling
        def on_mousewheel(event):
            canvas.yview_scroll(int(-1*(event.delta/120)), "units")
        
        def on_mousewheel_linux(event):
            if event.num == 4:
                canvas.yview_scroll(-1, "units")
            elif event.num == 5:
                canvas.yview_scroll(1, "units")
        
        # Bind for Windows/Mac
        canvas.bind("<MouseWheel>", on_mousewheel)
        # Bind for Linux
        canvas.bind("<Button-4>", on_mousewheel_linux)
        canvas.bind("<Button-5>", on_mousewheel_linux)
        
        # Also bind to scrollable_frame so scrolling works when hovering over checkboxes
        scrollable_frame.bind("<MouseWheel>", on_mousewheel)
        scrollable_frame.bind("<Button-4>", on_mousewheel_linux)
        scrollable_frame.bind("<Button-5>", on_mousewheel_linux)
        
        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Helper function to check if two names share at least 2/3 prefix
        def names_are_similar(name1: str, name2: str) -> bool:
            """Check if names share at least 2/3 of their length from the start."""
            # Remove file extension for comparison
            base1 = os.path.splitext(name1)[0].lower()
            base2 = os.path.splitext(name2)[0].lower()
            
            # Calculate minimum prefix length needed (2/3 of shorter string)
            min_len = min(len(base1), len(base2))
            if min_len == 0:
                return False
            required_prefix_len = (min_len * 2) // 3
            if required_prefix_len < 3:  # At least 3 chars must match
                required_prefix_len = min(3, min_len)
            
            # Check if first required_prefix_len characters match
            return base1[:required_prefix_len] == base2[:required_prefix_len]
        
        # Get previously synced files for this tab type (memory)
        previously_synced = self.sync_state_memory.get(tab_type, set())
        
        # Create checkboxes for each file (except current)
        file_vars = {}
        for filepath, file_data in self.loaded_files.items():
            if filepath == self.current_file.filepath:
                continue  # Skip current file
            
            # Determine initial check state:
            # 1. If we have memory of previous sync, use that
            # 2. Otherwise, check if names share at least 2/3 prefix
            if previously_synced:
                # Use memory: check if this file was previously synced
                initial_checked = filepath in previously_synced
            else:
                # No memory: use prefix similarity matching
                initial_checked = names_are_similar(
                    self.current_file.display_name, 
                    file_data.display_name
                )
            
            var = tk.BooleanVar(value=initial_checked)
            file_vars[filepath] = var
            
            cb = tk.Checkbutton(
                scrollable_frame,
                text=file_data.display_name,
                variable=var,
                font=('Arial', 10),
                anchor='w'
            )
            cb.pack(fill=tk.X, padx=5, pady=2)
        
        # Select All / Deselect All buttons
        select_frame = tk.Frame(dialog)
        select_frame.pack(fill=tk.X, padx=10, pady=5)
        
        def select_all():
            for var in file_vars.values():
                var.set(True)
        
        def deselect_all():
            for var in file_vars.values():
                var.set(False)
        
        tk.Button(
            select_frame,
            text="Select All",
            command=select_all,
            font=('Arial', 9),
            pady=3
        ).pack(side=tk.LEFT, padx=5)
        
        tk.Button(
            select_frame,
            text="Deselect All",
            command=deselect_all,
            font=('Arial', 9),
            pady=3
        ).pack(side=tk.LEFT, padx=5)
        
        # OK / Cancel buttons
        button_frame = tk.Frame(dialog)
        button_frame.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=15)
        
        def apply_sync():
            selected_files = [fp for fp, var in file_vars.items() if var.get()]
            if not selected_files:
                messagebox.showwarning("No Selection", "No files selected for sync")
                return
            
            synced_files = []
            skipped_files = []
            
            # Apply view state to selected files (with validation)
            for filepath in selected_files:
                file_data = self.loaded_files.get(filepath)
                if file_data is None:
                    continue
                
                # Validate that target file has enough data for the view range
                can_sync = False
                if tab_type == 'delta':
                    if file_data.has_delta() and file_data.raw_deltas is not None:
                        data_len = len(file_data.raw_deltas)
                        # Check if at least part of the X range overlaps with data
                        if current_xlim[0] < data_len:
                            can_sync = True
                elif tab_type == 'stroke_detection':
                    if file_data.has_stroke_data() and file_data.stroke_raw_deltas is not None:
                        data_len = len(file_data.stroke_raw_deltas)
                        # Check if at least part of the X range overlaps with data
                        if current_xlim[0] < data_len:
                            can_sync = True
                
                if can_sync:
                    if tab_type == 'delta':
                        file_data.delta_view_xlim = current_xlim
                        file_data.delta_view_ylim = current_ylim
                    elif tab_type == 'stroke_detection':
                        file_data.stroke_detection_view_xlim = current_xlim
                        file_data.stroke_detection_view_ylim = current_ylim
                    synced_files.append(file_data.display_name)
                else:
                    skipped_files.append(file_data.display_name)
            
            # Remember selected files for next time (save to memory)
            self.sync_state_memory[tab_type] = set(selected_files)
            
            dialog.destroy()
            
            # Only show warning if some files were skipped
            if skipped_files:
                messagebox.showwarning(
                    "Partial Sync",
                    f"Synced to {len(synced_files)} file(s).\n\n"
                    f"Could not sync to {len(skipped_files)} file(s) "
                    "(view range exceeds data bounds):\n• " + "\n• ".join(skipped_files)
                )
        
        tk.Button(
            button_frame,
            text="Apply Sync",
            command=apply_sync,
            font=('Arial', 10, 'bold'),
            bg='#4CAF50',
            fg='white',
            padx=20,
            pady=5
        ).pack(side=tk.RIGHT, padx=5)
        
        tk.Button(
            button_frame,
            text="Cancel",
            command=dialog.destroy,
            font=('Arial', 10),
            padx=20,
            pady=5
        ).pack(side=tk.RIGHT, padx=5)
    
    def _copy_delta_chart_to_clipboard(self):
        """Copy the current delta times chart to the system clipboard."""
        if self.delta_fig is None or self.delta_canvas is None:
            messagebox.showwarning("No Chart", "No chart available to copy")
            return
        self._copy_figure_to_clipboard(self.delta_fig)
    
    def _copy_forces_chart_to_clipboard(self):
        """Copy the current forces chart to the system clipboard."""
        if self.forces_fig is None or self.forces_canvas is None:
            messagebox.showwarning("No Chart", "No chart available to copy")
            return
        self._copy_figure_to_clipboard(self.forces_fig)
    
    def _copy_stroke_detection_chart_to_clipboard(self):
        """Copy the current stroke detection chart to the system clipboard."""
        if self.sd_fig is None or self.sd_canvas is None:
            messagebox.showwarning("No Chart", "No chart available to copy")
            return
        self._copy_figure_to_clipboard(self.sd_fig)

    def _copy_figure_to_clipboard(self, fig):
        """Copy a Matplotlib figure to the system clipboard using pyperclipimg.

        Centralized version used by all copy helpers.
        """
        try:
            from PIL import Image
            import io
            
            # Draw the figure to a buffer
            buffer = io.BytesIO()
            fig.savefig(buffer, format='png', dpi=100, bbox_inches='tight')
            buffer.seek(0)
            
            # Load as PIL Image
            image = Image.open(buffer)
            
            # Copy to clipboard using pyperclipimg (cross-platform)
            pyperclipimg.copy(image)
            self._show_temporary_status("Chart copied to clipboard!")
                    
        except ImportError as e:
            error_msg = "Missing clipboard dependencies.\n\n"
            if 'pyperclipimg' in str(e):
                error_msg += "Install with: pip install pyperclipimg\n\n"
            if 'PIL' in str(e) or 'Pillow' in str(e):
                error_msg += "Install with: pip install Pillow\n\n"
            
            # Add platform-specific instructions
            if os.name == 'nt':
                error_msg += "Windows: pip install pywin32"
            elif os.name == 'posix':
                if 'darwin' in os.uname().sysname.lower():
                    error_msg += "macOS: pip install pyobjc-framework-quartz"
                else:
                    error_msg += "Linux: sudo apt install xclip or sudo apt install wl-clipboard"
            
            error_msg += f"\n\nError: {str(e)}"
            messagebox.showerror("Clipboard Error", error_msg)
        except Exception as e:
            error_msg = f"Failed to copy chart to clipboard:\n{str(e)}\n\n"
            
            # Provide helpful hints based on the error
            error_str = str(e).lower()
            if 'xclip' in error_str or 'command not found' in error_str:
                error_msg += "Linux: Install xclip with: sudo apt install xclip"
            elif 'pyobjc' in error_str:
                error_msg += "macOS: Install Quartz with: pip install pyobjc-framework-quartz"
            elif 'win32' in error_str:
                error_msg += "Windows: Install pywin32 with: pip install pywin32"
            
            messagebox.showerror("Clipboard Error", error_msg)
    
    def _update_stat_labels(self):
        """Update all stat labels with current data."""
        if not self.forces_stats_data or not self.forces_stats_labels:
            return
        
        # Update each label with fresh values
        if 'max' in self.forces_stats_labels:
            self.forces_stats_labels['max'].config(
                text=f"Data Point Statistics (hover for strokes) | Max: {self.forces_stats_data['max']}"
            )
        
        if 'min' in self.forces_stats_labels:
            self.forces_stats_labels['min'].config(
                text=f"Min: {self.forces_stats_data['min']}"
            )
        
        if 'median' in self.forces_stats_labels:
            self.forces_stats_labels['median'].config(
                text=f"Median: {self.forces_stats_data['median']:.1f}"
            )
        
        if 'avg' in self.forces_stats_labels:
            self.forces_stats_labels['avg'].config(
                text=f"Avg: {self.forces_stats_data['avg']:.1f}"
            )
    
    def _calculate_summary_metrics(self) -> dict:
        """Calculate summary metrics from per-stroke data.
        
        Returns:
            Dict with keys: total_distance, avg_power, avg_drag_factor, stroke_count,
            total_time, avg_stroke_rate
        """
        if self.current_file is None or not self.current_file.has_forces():
            return {
                'total_distance': None,
                'avg_power': None,
                'avg_drag_factor': None,
                'stroke_count': 0,
                'total_time': None,
                'avg_stroke_rate': None
            }
        
        stroke_count = len(self.current_file.handle_forces)
        
        # Calculate total distance
        total_distance = None
        if self.current_file.stroke_distance:
            valid_distances = [d for d in self.current_file.stroke_distance if d is not None and d >= 0]
            if valid_distances:
                # Distance is cumulative, so use the last value
                total_distance = valid_distances[-1]
        
        # Calculate average power
        avg_power = None
        if self.current_file.stroke_power:
            valid_power = [p for p in self.current_file.stroke_power if p is not None and p >= 0]
            if valid_power:
                avg_power = sum(valid_power) / len(valid_power)
        
        # Calculate average drag factor
        avg_drag_factor = None
        if self.current_file.stroke_drag_factor:
            valid_drag = [d for d in self.current_file.stroke_drag_factor if d is not None and d >= 0]
            if valid_drag:
                avg_drag_factor = sum(valid_drag) / len(valid_drag)
        
        # Calculate total time and average stroke rate
        total_time = None
        avg_stroke_rate = None
        if (self.current_file.stroke_drive_duration and 
            self.current_file.stroke_recovery_duration):
            valid_pairs = [
                (drive, recovery) 
                for drive, recovery in zip(self.current_file.stroke_drive_duration,
                                          self.current_file.stroke_recovery_duration)
                if drive is not None and recovery is not None and drive >= 0 and recovery >= 0
            ]
            if valid_pairs:
                total_time = sum(drive + recovery for drive, recovery in valid_pairs)
                if total_time > 0:
                    avg_stroke_rate = (len(valid_pairs) / total_time) * 60  # strokes per minute
        
        return {
            'total_distance': total_distance,
            'avg_power': avg_power,
            'avg_drag_factor': avg_drag_factor,
            'stroke_count': stroke_count,
            'total_time': total_time,
            'avg_stroke_rate': avg_stroke_rate
        }
    
    def _calculate_datapoint_stats(self, handle_forces: list) -> dict:
        """Calculate statistics about datapoint counts across all strokes.
        
        Args:
            handle_forces: List of force curves (each is a list of integers)
            
        Returns:
            Dict with keys: max, min, median, avg (values), and max_strokes, min_strokes (lists of stroke indices)
        """
        if not handle_forces:
            return {}
        
        # Calculate datapoint count for each stroke
        counts = [len(forces) for forces in handle_forces]
        
        max_count = max(counts)
        min_count = min(counts)
        median_count = float(np.median(counts))
        avg_count = float(np.mean(counts))
        
        # Find which strokes have max/min counts
        max_strokes = [i + 1 for i, c in enumerate(counts) if c == max_count]  # 1-indexed
        min_strokes = [i + 1 for i, c in enumerate(counts) if c == min_count]
        
        # Find strokes closest to median
        median_strokes = []
        min_diff = float('inf')
        for i, c in enumerate(counts):
            diff = abs(c - median_count)
            if diff < min_diff:
                min_diff = diff
                median_strokes = [i + 1]
            elif diff == min_diff:
                median_strokes.append(i + 1)
        
        # Find strokes closest to average
        avg_strokes = []
        min_diff = float('inf')
        for i, c in enumerate(counts):
            diff = abs(c - avg_count)
            if diff < min_diff:
                min_diff = diff
                avg_strokes = [i + 1]
            elif diff == min_diff:
                avg_strokes.append(i + 1)
        
        return {
            'max': max_count,
            'min': min_count,
            'median': median_count,
            'avg': avg_count,
            'max_strokes': max_strokes,
            'min_strokes': min_strokes,
            'median_strokes': median_strokes,
            'avg_strokes': avg_strokes,
            'all_counts': counts
        }
    
    def _get_datapoint_distribution(self, handle_forces: list) -> list:
        """Get distribution of datapoint counts (how many strokes have each count).
        
        Args:
            handle_forces: List of force curves
            
        Returns:
            List of (datapoint_count, frequency) tuples, sorted by datapoint_count
        """
        if not handle_forces:
            return []
        
        counts = [len(forces) for forces in handle_forces]
        
        # Count frequency of each datapoint count
        from collections import Counter
        freq = Counter(counts)
        
        # Sort by datapoint count
        distribution = sorted(freq.items())
        
        return distribution
    
    def _create_forces_view(self):
        """Create the handle forces visualization with navigation."""
        if self.current_file is None:
            return
        
        handle_forces = self.current_file.handle_forces
        
        # Summary metrics section - always visible, single line
        self.summary_frame = tk.Frame(self.forces_frame, relief=tk.RIDGE, borderwidth=2, bg='#e8f5e9')
        self.summary_frame.pack(side=tk.TOP, fill=tk.X, padx=20, pady=5)
        
        # Calculate summary metrics
        summary = self._calculate_summary_metrics()
        
        # Create all summary labels in one frame (single line)
        summary_grid = tk.Frame(self.summary_frame, bg='#e8f5e9')
        summary_grid.pack(side=tk.TOP, fill=tk.X, padx=5, pady=2)
        
        # Session Summary label on the left
        tk.Label(
            summary_grid,
            text="Session Summary:",
            font=('Arial', 10, 'bold'),
            bg='#e8f5e9',
            fg='#2e7d32'
        ).grid(row=0, column=0, padx=5, pady=3, sticky='w')
        
        # All metrics in one row
        tk.Label(
            summary_grid,
            text=f"Total Distance: {FileData.format_metric(summary['total_distance'])} m",
            font=('Arial', 11),
            bg='#e8f5e9',
            fg='#1b5e20'
        ).grid(row=0, column=1, padx=15, pady=3, sticky='w')
        
        time_str = FileData.format_metric(summary['total_time'])
        if summary['total_time'] is not None:
            # Convert seconds to MM:SS format
            total_sec = int(summary['total_time'])
            minutes = total_sec // 60
            seconds = total_sec % 60
            time_str = f"{minutes}:{seconds:02d}"
        
        tk.Label(
            summary_grid,
            text=f"Total Time: {time_str}",
            font=('Arial', 11),
            bg='#e8f5e9',
            fg='#1b5e20'
        ).grid(row=0, column=2, padx=15, pady=3, sticky='w')
        
        tk.Label(
            summary_grid,
            text=f"Stroke Count: {summary['stroke_count']}",
            font=('Arial', 11),
            bg='#e8f5e9',
            fg='#1b5e20'
        ).grid(row=0, column=3, padx=15, pady=3, sticky='w')
        
        tk.Label(
            summary_grid,
            text=f"Avg Power: {FileData.format_metric(summary['avg_power'])} W",
            font=('Arial', 11),
            bg='#e8f5e9',
            fg='#1b5e20'
        ).grid(row=0, column=4, padx=15, pady=3, sticky='w')
        
        tk.Label(
            summary_grid,
            text=f"Avg Drag: {FileData.format_metric(summary['avg_drag_factor'])}",
            font=('Arial', 11),
            bg='#e8f5e9',
            fg='#1b5e20'
        ).grid(row=0, column=5, padx=15, pady=3, sticky='w')
        
        tk.Label(
            summary_grid,
            text=f"Avg Rate: {FileData.format_metric(summary['avg_stroke_rate'])} SPM",
            font=('Arial', 11),
            bg='#e8f5e9',
            fg='#1b5e20'
        ).grid(row=0, column=6, padx=15, pady=3, sticky='w')
        
        # Store reference for potential future use
        self.summary_content_frame = summary_grid
        
        # Data point statistics frame
        stats_frame = tk.Frame(self.forces_frame, relief=tk.RIDGE, borderwidth=2, bg='#f0f0f0')
        stats_frame.pack(side=tk.TOP, fill=tk.X, padx=20, pady=5)
        
        # Calculate statistics
        self.forces_stats_data = self._calculate_datapoint_stats(handle_forces)
        
        # Create grid for statistics
        stats_grid = tk.Frame(stats_frame, bg='#f0f0f0')
        stats_grid.pack(side=tk.TOP, pady=(5, 5), anchor='w')
        
        # Row 0: Title + Max and Min (inlined title in max label)
        self.forces_stats_labels['max'] = tk.Label(
            stats_grid,
            text=f"📊 Data Point Statistics (hover for strokes) | Max: {self.forces_stats_data['max']}",
            font=('Arial', 11),
            bg='#f0f0f0',
            fg='#1565C0',
            cursor='hand2'
        )
        self.forces_stats_labels['max'].grid(row=0, column=0, columnspan=2, padx=10, pady=2, sticky='w')
        
        self.forces_stats_labels['min'] = tk.Label(
            stats_grid,
            text=f"Min: {self.forces_stats_data['min']}",
            font=('Arial', 11),
            bg='#f0f0f0',
            fg='#E53935',
            cursor='hand2'
        )
        self.forces_stats_labels['min'].grid(row=0, column=2, padx=10, pady=2, sticky='w')
        self.forces_stats_labels['min'].stat_value = self.forces_stats_data['min']
        
        self.forces_stats_labels['median'] = tk.Label(
            stats_grid,
            text=f"Median: {self.forces_stats_data['median']:.1f}",
            font=('Arial', 11),
            bg='#f0f0f0',
            fg='#43A047',
            cursor='hand2'
        )
        self.forces_stats_labels['median'].grid(row=0, column=3, padx=10, pady=2, sticky='w')
        self.forces_stats_labels['median'].stat_value = self.forces_stats_data['median']
        
        self.forces_stats_labels['avg'] = tk.Label(
            stats_grid,
            text=f"Avg: {self.forces_stats_data['avg']:.1f}",
            font=('Arial', 11),
            bg='#f0f0f0',
            fg='#FB8C00',
            cursor='hand2'
        )
        self.forces_stats_labels['avg'].grid(row=0, column=4, padx=10, pady=2, sticky='w')
        self.forces_stats_labels['avg'].stat_value = self.forces_stats_data['avg']
        
        # Update label texts with fresh data
        self._update_stat_labels()
        
        # Create tooltip label (hidden by default, uses place() for floating positioning)
        self.forces_stats_tooltip = tk.Label(
            self.root,
            text="",
            font=('Arial', 9),
            bg='#FFFACD',
            relief=tk.SOLID,
            borderwidth=1,
            padx=5,
            pady=2
        )
        
        # Bind hover events to stat labels
        for stat_name, label in self.forces_stats_labels.items():
            label.bind('<Enter>', lambda e, sn=stat_name: self._on_stat_hover(sn, e))
            label.bind('<Leave>', lambda e: self._on_stat_leave())
        
        # Slider
        slider_frame = tk.Frame(self.forces_frame)
        slider_frame.pack(side=tk.TOP, fill=tk.X, padx=20, pady=5)
        
        tk.Label(slider_frame, text="Navigate strokes:", font=('Arial', 9)).pack(side=tk.LEFT)
        
        max_index = max(0, len(handle_forces) - 2)
        self.stroke_slider = tk.Scale(
            slider_frame,
            from_=0,
            to=max_index,
            orient=tk.HORIZONTAL,
            length=800,
            command=self._on_slider_change,
            showvalue=False
        )
        self.stroke_slider.set(self.current_file.current_stroke_index)
        self.stroke_slider.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=10)
        
        # Control panel MOVED HERE (directly above chart) - includes nav buttons + entry + distribution
        control_frame = tk.Frame(self.forces_frame)
        control_frame.pack(side=tk.TOP, fill=tk.X, pady=10, padx=20)
        
        # Navigation buttons on left
        nav_frame = tk.Frame(control_frame)
        nav_frame.pack(side=tk.LEFT, padx=5)
        
        prev_btn = tk.Button(
            nav_frame, 
            text="◀ Previous", 
            command=self._prev_stroke,
            font=('Arial', 10),
            padx=10
        )
        prev_btn.pack(side=tk.LEFT, padx=5)
        
        next_btn = tk.Button(
            nav_frame, 
            text="Next ▶", 
            command=self._next_stroke,
            font=('Arial', 10),
            padx=10
        )
        next_btn.pack(side=tk.LEFT, padx=5)
        
        # Stroke entry section
        entry_frame = tk.Frame(control_frame)
        entry_frame.pack(side=tk.LEFT, padx=20)
        
        tk.Label(entry_frame, text="Left chart stroke #:", font=('Arial', 10)).pack(side=tk.LEFT)
        
        self.stroke_entry = tk.Entry(entry_frame, width=8, font=('Arial', 10))
        self.stroke_entry.pack(side=tk.LEFT, padx=5)
        self.stroke_entry.insert(0, str(self.current_file.current_stroke_index + 1))
        self.stroke_entry.bind('<Return>', self._on_entry_change)
        
        go_btn = tk.Button(
            entry_frame, 
            text="Go", 
            command=self._on_entry_change,
            font=('Arial', 10)
        )
        go_btn.pack(side=tk.LEFT, padx=5)
        
        # Data Point Distribution button
        dist_btn = tk.Button(
            entry_frame,
            text="Data Point Distribution",
            command=self._show_datapoint_distribution_dialog,
            font=('Arial', 10),
            bg='#2196F3',
            fg='white',
            padx=10
        )
        dist_btn.pack(side=tk.LEFT, padx=10)
        
        # Stroke info label on right
        self.stroke_label = tk.Label(
            control_frame, 
            text=f"Showing strokes 1 and 2 of {len(handle_forces)}",
            font=('Arial', 10)
        )
        self.stroke_label.pack(side=tk.RIGHT, padx=5)
        
        # Chart frame
        chart_frame = tk.Frame(self.forces_frame)
        chart_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        
        # Create figure with two side-by-side subplots
        self.forces_fig, self.forces_axes = plt.subplots(1, 2, figsize=(14, 7.5), dpi=100)
        self.forces_fig.tight_layout(pad=4.0)
        
        # Embed in tkinter
        self.forces_canvas = FigureCanvasTkAgg(self.forces_fig, master=chart_frame)
        self.forces_canvas.draw()
        
        # Add navigation toolbar
        toolbar_frame = tk.Frame(chart_frame)
        toolbar_frame.pack(side=tk.TOP, fill=tk.X)
        toolbar = NavigationToolbar2Tk(self.forces_canvas, toolbar_frame)
        toolbar.update()
        
        self.forces_canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        
        # Bind keyboard shortcuts for navigation
        self.root.bind('<Left>', lambda e: self._prev_stroke())
        self.root.bind('<Right>', lambda e: self._next_stroke())
        
        # Initial plot
        self._update_forces_plot()
        
    def _update_forces_plot(self):
        """Update the handle forces plot with current stroke pair."""
        if self.current_file is None or not self.current_file.handle_forces or self.forces_axes is None:
            return
        
        handle_forces = self.current_file.handle_forces
            
        # Clear previous plots
        for ax in self.forces_axes:
            ax.clear()
        
        # Get current and next stroke
        idx1 = self.current_file.current_stroke_index
        idx2 = self.current_file.current_stroke_index + 1
        
        # Plot first stroke (left chart)
        if idx1 < len(handle_forces):
            forces1 = handle_forces[idx1]
            x1 = np.arange(len(forces1))
            self.forces_axes[0].plot(x1, forces1, 'b-', linewidth=2)
            self.forces_axes[0].fill_between(x1, forces1, alpha=0.3)
            self.forces_axes[0].set_title(f'Stroke #{idx1 + 1}', fontsize=12)
            self.forces_axes[0].set_ylabel('Force (N)', fontsize=10)
            self.forces_axes[0].grid(True, alpha=0.3)
            
            # Add stats
            max_force = max(forces1)
            avg_force = sum(forces1) / len(forces1)
            self.forces_axes[0].text(
                0.02, 0.98, 
                f'Max: {max_force} N\nAvg: {avg_force:.0f} N\nSamples: {len(forces1)}',
                transform=self.forces_axes[0].transAxes,
                verticalalignment='top',
                fontsize=9,
                bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5)
            )
            
            # Add per-stroke metrics (right corner)
            if self.current_file.has_metrics():
                metrics_text = self._format_stroke_metrics(idx1)
                self.forces_axes[0].text(
                    0.98, 0.98,
                    metrics_text,
                    transform=self.forces_axes[0].transAxes,
                    verticalalignment='top',
                    horizontalalignment='right',
                    fontsize=9,
                    bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.5, pad=0.3)
                )
        
        # Plot second stroke (right chart)
        if idx2 < len(handle_forces):
            forces2 = handle_forces[idx2]
            x2 = np.arange(len(forces2))
            self.forces_axes[1].plot(x2, forces2, 'r-', linewidth=2)
            self.forces_axes[1].fill_between(x2, forces2, alpha=0.3, color='red')
            self.forces_axes[1].set_title(f'Stroke #{idx2 + 1}', fontsize=12)
            self.forces_axes[1].set_ylabel('Force (N)', fontsize=10)
            self.forces_axes[1].grid(True, alpha=0.3)
            
            # Add stats
            max_force = max(forces2)
            avg_force = sum(forces2) / len(forces2)
            self.forces_axes[1].text(
                0.02, 0.98, 
                f'Max: {max_force} N\nAvg: {avg_force:.0f} N\nSamples: {len(forces2)}',
                transform=self.forces_axes[1].transAxes,
                verticalalignment='top',
                fontsize=9,
                bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5)
            )
            
            # Add per-stroke metrics (right corner)
            if self.current_file.has_metrics():
                metrics_text = self._format_stroke_metrics(idx2)
                self.forces_axes[1].text(
                    0.98, 0.98,
                    metrics_text,
                    transform=self.forces_axes[1].transAxes,
                    verticalalignment='top',
                    horizontalalignment='right',
                    fontsize=9,
                    bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.5, pad=0.3)
                )
        else:
            self.forces_axes[1].text(
                0.5, 0.5, 
                'No more strokes',
                transform=self.forces_axes[1].transAxes,
                horizontalalignment='center',
                verticalalignment='center',
                fontsize=14
            )
            self.forces_axes[1].set_title('N/A', fontsize=12)
        
        # Update label
        if idx2 < len(handle_forces):
            self.stroke_label.config(
                text=f"Showing strokes {idx1 + 1} and {idx2 + 1} of {len(handle_forces)}"
            )
        else:
            self.stroke_label.config(
                text=f"Showing stroke {idx1 + 1} of {len(handle_forces)}"
            )
        
        # Update entry box
        self.stroke_entry.delete(0, tk.END)
        self.stroke_entry.insert(0, str(idx1 + 1))
        
        self.forces_canvas.draw()
    
    def _format_stroke_metrics(self, stroke_idx: int) -> str:
        """Format per-stroke metrics for display in chart.
        
        Args:
            stroke_idx: Index of the stroke (0-based)
            
        Returns:
            Formatted string with metrics
        """
        if not self.current_file:
            return "Metrics: N/A"
        
        if not self.current_file.has_metrics():
            return "Metrics: N/A"
        
        metrics = []
        
        # Power
        if stroke_idx < len(self.current_file.stroke_power):
            power = self.current_file.stroke_power[stroke_idx]
            metrics.append(f"Power: {FileData.format_metric(power)} W")
        
        # Drag Factor
        if stroke_idx < len(self.current_file.stroke_drag_factor):
            drag = self.current_file.stroke_drag_factor[stroke_idx]
            metrics.append(f"Drag: {FileData.format_metric(drag)}")
        
        # Distance
        if stroke_idx < len(self.current_file.stroke_distance):
            dist = self.current_file.stroke_distance[stroke_idx]
            metrics.append(f"Dist: {FileData.format_metric(dist)} m")
        
        # Drive Duration
        if stroke_idx < len(self.current_file.stroke_drive_duration):
            drive = self.current_file.stroke_drive_duration[stroke_idx]
            metrics.append(f"Drive: {FileData.format_metric(drive)} s")
        
        # Recovery Duration
        if stroke_idx < len(self.current_file.stroke_recovery_duration):
            recovery = self.current_file.stroke_recovery_duration[stroke_idx]
            metrics.append(f"Recovery: {FileData.format_metric(recovery)} s")
        
        # Calculate stroke rate if both durations available
        if (stroke_idx < len(self.current_file.stroke_drive_duration) and
            stroke_idx < len(self.current_file.stroke_recovery_duration)):
            drive = self.current_file.stroke_drive_duration[stroke_idx]
            recovery = self.current_file.stroke_recovery_duration[stroke_idx]
            if drive is not None and recovery is not None and drive >= 0 and recovery >= 0:
                total_time = drive + recovery
                if total_time > 0:
                    stroke_rate = 60 / total_time
                    metrics.append(f"Rate: {stroke_rate:.1f} SPM")
        
        # Calculate 500m pace
        if (stroke_idx < len(self.current_file.stroke_distance) and
            stroke_idx < len(self.current_file.stroke_drive_duration) and
            stroke_idx < len(self.current_file.stroke_recovery_duration)):
            
            dist_cumulative = self.current_file.stroke_distance[stroke_idx]
            drive = self.current_file.stroke_drive_duration[stroke_idx]
            recovery = self.current_file.stroke_recovery_duration[stroke_idx]
            
            if dist_cumulative is not None and drive is not None and recovery is not None:
                # Calculate distance covered in this stroke
                if stroke_idx == 0:
                    dist_per_stroke = dist_cumulative
                else:
                    prev_dist = self.current_file.stroke_distance[stroke_idx - 1]
                    if prev_dist is not None:
                        dist_per_stroke = dist_cumulative - prev_dist
                    else:
                        dist_per_stroke = None
                
                if dist_per_stroke is not None and dist_per_stroke > 0:
                    stroke_time = drive + recovery
                    if stroke_time > 0:
                        pace_500m_seconds = (500 / dist_per_stroke) * stroke_time
                        # Round seconds to integer, handle rollover
                        total_seconds = int(round(pace_500m_seconds))
                        pace_minutes = total_seconds // 60
                        pace_seconds = total_seconds % 60
                        pace_str = f"500m Pace: {pace_minutes}:{pace_seconds:02d}"
                        metrics.append(pace_str)
        
        result = "\n".join(metrics) if metrics else "Metrics: N/A"
        return result
    
    def _toggle_summary(self):
        """Toggle visibility of summary section."""
        self.summary_expanded = not self.summary_expanded
        
        if self.summary_expanded:
            self.summary_content_frame.pack(side=tk.TOP, fill=tk.X, padx=10, pady=5)
            self.summary_toggle_btn.config(text="▼ Session Summary")
        else:
            self.summary_content_frame.pack_forget()
            self.summary_toggle_btn.config(text="▶ Session Summary")
        
    def _prev_stroke(self):
        """Navigate to previous stroke pair."""
        if self.current_file is None:
            return
        if self.current_file.current_stroke_index > 0:
            self.current_file.current_stroke_index -= 1
            if self.stroke_slider:
                self.stroke_slider.set(self.current_file.current_stroke_index)
            self._update_forces_plot()
            
    def _next_stroke(self):
        """Navigate to next stroke pair."""
        if self.current_file is None:
            return
        if self.current_file.current_stroke_index < len(self.current_file.handle_forces) - 1:
            self.current_file.current_stroke_index += 1
            if self.stroke_slider:
                self.stroke_slider.set(self.current_file.current_stroke_index)
            self._update_forces_plot()
            
    def _on_slider_change(self, value):
        """Handle slider value change."""
        if self.current_file is None:
            return
        new_index = int(float(value))
        if new_index != self.current_file.current_stroke_index:
            self.current_file.current_stroke_index = new_index
            self._update_forces_plot()
            
    def _on_entry_change(self, event=None):
        """Handle stroke entry change."""
        if self.current_file is None:
            return
        try:
            stroke_num = int(self.stroke_entry.get())
            # Convert to 0-based index
            new_index = stroke_num - 1
            if 0 <= new_index < len(self.current_file.handle_forces):
                self.current_file.current_stroke_index = new_index
                if self.stroke_slider:
                    self.stroke_slider.set(new_index)
                self._update_forces_plot()
            else:
                messagebox.showwarning(
                    "Invalid Stroke Number",
                    f"Please enter a number between 1 and {len(self.current_file.handle_forces)}"
                )
        except ValueError:
            messagebox.showwarning(
                "Invalid Input",
                "Please enter a valid stroke number"
            )
    
    def _on_stat_hover(self, stat_name: str, event):
        """Handle hover over a statistic label to show which strokes have that value."""
        if not self.forces_stats_data or not self.forces_stats_tooltip:
            return
        
        strokes_key = f"{stat_name}_strokes"
        if strokes_key not in self.forces_stats_data:
            return
        
        strokes = self.forces_stats_data[strokes_key]
        
        # Format stroke numbers
        if len(strokes) <= 5:
            stroke_text = ", ".join([f"#{s}" for s in strokes])
        else:
            stroke_text = ", ".join([f"#{s}" for s in strokes[:5]]) + f"... (+{len(strokes) - 5} more)"
        
        tooltip_text = f"Stroke(s): {stroke_text}"
        self.forces_stats_tooltip.config(text=tooltip_text)
        
        # Position tooltip relative to mouse, with offset to avoid covering the label
        # Get absolute screen position from event
        x = event.x_root + 10
        y = event.y_root + 10
        
        # Raise tooltip to be on top
        self.forces_stats_tooltip.lift()
        self.forces_stats_tooltip.place(x=x, y=y)
    
    def _on_stat_leave(self):
        """Handle mouse leaving a statistic label to hide tooltip."""
        if self.forces_stats_tooltip:
            self.forces_stats_tooltip.place_forget()
    
    def _show_datapoint_distribution_dialog(self):
        """Show a dialog with an interactive chart of datapoint distribution."""
        if self.current_file is None or not self.current_file.handle_forces:
            return
        
        # Create dialog window
        dialog = tk.Toplevel(self.root)
        dialog.title(f"Data Point Distribution - {self.current_file.display_name}")
        dialog.geometry("900x600")
        
        # Get distribution data
        distribution = self._get_datapoint_distribution(self.current_file.handle_forces)
        
        if not distribution:
            tk.Label(dialog, text="No data available", font=('Arial', 12)).pack(pady=20)
            return
        
        # Create matplotlib figure
        fig, ax = plt.subplots(figsize=(10, 6), dpi=100)
        
        # Extract x and y values
        x_values = [item[0] for item in distribution]  # datapoint counts
        y_values = [item[1] for item in distribution]  # frequencies
        
        # Create bar chart
        bars = ax.bar(x_values, y_values, color='#2196F3', alpha=0.7, edgecolor='#1565C0', linewidth=1.5)
        
        # Also add a line plot
        ax.plot(x_values, y_values, 'ro-', linewidth=2, markersize=6, alpha=0.8)
        
        # Configure axes
        ax.set_xlabel('Data Points per Stroke', fontsize=11)
        ax.set_ylabel('Number of Strokes', fontsize=11)
        ax.set_title('Distribution of Data Points Across Strokes', fontsize=13, fontweight='bold')
        ax.grid(True, alpha=0.3, linestyle='--')
        
        # Add value labels on bars
        for bar, y_val in zip(bars, y_values):
            height = bar.get_height()
            if height > 0:  # Only show label if bar has height
                ax.text(
                    bar.get_x() + bar.get_width() / 2,
                    height,
                    f'{int(y_val)}',
                    ha='center',
                    va='bottom',
                    fontsize=8
                )
        
        # Calculate and display summary statistics
        stats = self._calculate_datapoint_stats(self.current_file.handle_forces)
        summary_text = (
            f"Total Strokes: {len(self.current_file.handle_forces)} | "
            f"Min: {stats['min']} | Max: {stats['max']} | "
            f"Median: {stats['median']:.1f} | Avg: {stats['avg']:.1f}"
        )
        
        ax.text(
            0.5, 0.98,
            summary_text,
            transform=ax.transAxes,
            ha='center',
            va='top',
            fontsize=10,
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8)
        )
        
        # Create hover annotation
        annot = ax.annotate(
            '', xy=(0, 0), xytext=(10, 10),
            textcoords='offset points',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='yellow', alpha=0.9),
            fontsize=9,
            visible=False
        )
        
        # Hover event handler
        def on_hover(event):
            if event.inaxes != ax:
                if annot.get_visible():
                    annot.set_visible(False)
                    fig.canvas.draw_idle()
                return
            
            # Check if hovering over a bar or point
            for i, (bar, x_val, y_val) in enumerate(zip(bars, x_values, y_values)):
                cont_bar, _ = bar.contains(event)
                if cont_bar:
                    annot.xy = (x_val, y_val)
                    text = f"{int(x_val)} data points\n{int(y_val)} stroke(s)\n({y_val / len(self.current_file.handle_forces) * 100:.1f}%)"
                    
                    # If fewer than 4 strokes, include stroke numbers
                    if int(y_val) < 4:
                        stroke_indices = [i + 1 for i, forces in enumerate(self.current_file.handle_forces) if len(forces) == int(x_val)]
                        stroke_numbers = ", ".join([f"#{s}" for s in stroke_indices])
                        text += f"\nStroke(s): {stroke_numbers}"
                    
                    annot.set_text(text)
                    annot.set_visible(True)
                    fig.canvas.draw_idle()
                    return
            
            if annot.get_visible():
                annot.set_visible(False)
                fig.canvas.draw_idle()
        
        fig.canvas.mpl_connect('motion_notify_event', on_hover)
        
        fig.tight_layout()
        
        # Embed in tkinter
        canvas = FigureCanvasTkAgg(fig, master=dialog)
        canvas.draw()
        
        # Add navigation toolbar
        toolbar_frame = tk.Frame(dialog)
        toolbar_frame.pack(side=tk.TOP, fill=tk.X)
        toolbar = NavigationToolbar2Tk(canvas, toolbar_frame)
        toolbar.update()
        
        # Pack canvas
        canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        # Bind Ctrl+C in this dialog to copy the distribution chart
        def _dialog_copy(event=None):
            self._copy_figure_to_clipboard(fig)
        dialog.bind('<Control-c>', _dialog_copy)
        dialog.bind('<Control-C>', _dialog_copy)
        # Set focus so Ctrl+C works immediately
        dialog.focus_set()

        # Close button
        close_btn = tk.Button(
            dialog,
            text="Close",
            command=dialog.destroy,
            font=('Arial', 10),
            padx=20,
            pady=5
        )
        close_btn.pack(side=tk.BOTTOM, pady=10)
    
    # ==================== Stroke Detection Tab ====================
    
    def _create_stroke_detection_view(self):
        """Create the stroke detection analysis view."""
        # Main container with left panel and right chart
        main_container = tk.Frame(self.stroke_detection_frame)
        main_container.pack(fill=tk.BOTH, expand=True)
        
        # Left control panel
        control_panel = tk.Frame(main_container, width=300)
        control_panel.pack(side=tk.LEFT, fill=tk.Y, padx=5, pady=5)
        control_panel.pack_propagate(False)
        
        # Parameters frame
        params_frame = tk.LabelFrame(control_panel, text="Detection Parameters", font=('Arial', 10, 'bold'))
        params_frame.pack(fill=tk.X, padx=5, pady=5)
        
        # Window size
        tk.Label(params_frame, text="Window Size:", font=('Arial', 9)).grid(row=0, column=0, sticky='w', padx=5, pady=2)
        self.sd_window_size = tk.Scale(
            params_frame,
            from_=3,
            to=20,
            orient=tk.HORIZONTAL,
            length=150
        )
        self.sd_window_size.set(8)
        self.sd_window_size.grid(row=0, column=1, padx=5, pady=2)
        
        # Slope threshold
        tk.Label(params_frame, text="Slope Threshold:", font=('Arial', 9)).grid(row=1, column=0, sticky='w', padx=5, pady=2)
        self.sd_slope_threshold = tk.Scale(
            params_frame,
            from_=-50,
            to=50,
            orient=tk.HORIZONTAL,
            length=150,
            resolution=0.5
        )
        self.sd_slope_threshold.set(0)
        self.sd_slope_threshold.grid(row=1, column=1, padx=5, pady=2)
        
        # Analyze button
        analyze_btn = tk.Button(
            params_frame,
            text="Analyze",
            command=self._run_stroke_analysis,
            font=('Arial', 10, 'bold'),
            bg='#4CAF50',
            fg='white',
            padx=20,
            pady=5
        )
        analyze_btn.grid(row=2, column=0, columnspan=2, pady=10)
        
        # Stats frame
        stats_frame = tk.LabelFrame(control_panel, text="Analysis Results", font=('Arial', 10, 'bold'))
        stats_frame.pack(fill=tk.X, padx=5, pady=5)
        
        self.sd_stats_label = tk.Label(
            stats_frame,
            text="Click 'Analyze' to detect anomalies",
            font=('Arial', 9),
            justify=tk.LEFT,
            wraplength=280
        )
        self.sd_stats_label.pack(padx=5, pady=5, anchor='w')
        
        # Anomalies frame
        anomalies_frame = tk.LabelFrame(control_panel, text="Anomalies", font=('Arial', 10, 'bold'))
        anomalies_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Anomaly listbox with scrollbar
        list_container = tk.Frame(anomalies_frame)
        list_container.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        scrollbar = tk.Scrollbar(list_container)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.sd_anomaly_listbox = tk.Listbox(
            list_container,
            font=('Arial', 9),
            yscrollcommand=scrollbar.set,
            selectmode=tk.SINGLE
        )
        self.sd_anomaly_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.config(command=self.sd_anomaly_listbox.yview)
        
        # Bind double-click to navigate
        self.sd_anomaly_listbox.bind('<Double-Button-1>', self._on_anomaly_select)
        
        # Navigation buttons
        nav_frame = tk.Frame(anomalies_frame)
        nav_frame.pack(fill=tk.X, padx=5, pady=5)
        
        prev_btn = tk.Button(
            nav_frame,
            text="◀ Prev",
            command=self._prev_anomaly,
            font=('Arial', 9)
        )
        prev_btn.pack(side=tk.LEFT, padx=2)
        
        goto_btn = tk.Button(
            nav_frame,
            text="Go To",
            command=self._goto_anomaly,
            font=('Arial', 9)
        )
        goto_btn.pack(side=tk.LEFT, padx=2)
        
        next_btn = tk.Button(
            nav_frame,
            text="Next ▶",
            command=self._next_anomaly,
            font=('Arial', 9)
        )
        next_btn.pack(side=tk.LEFT, padx=2)
        
        # Sync View State button
        sync_frame = tk.Frame(control_panel)
        sync_frame.pack(fill=tk.X, padx=5, pady=10)
        
        sync_btn = tk.Button(
            sync_frame,
            text="Sync View State",
            command=lambda: self._show_sync_view_dialog('stroke_detection'),
            font=('Arial', 10),
            bg='#2196F3',
            fg='white',
            padx=15,
            pady=5
        )
        sync_btn.pack(fill=tk.X)
        
        # Right chart panel
        chart_panel = tk.Frame(main_container)
        chart_panel.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # Create figure (single axis, no slope)
        self.sd_fig, self.sd_ax = plt.subplots(figsize=(12, 6), dpi=100)
        
        # Embed in tkinter
        self.sd_canvas = FigureCanvasTkAgg(self.sd_fig, master=chart_panel)
        self.sd_canvas.draw()
        
        # Add navigation toolbar
        toolbar_frame = tk.Frame(chart_panel)
        toolbar_frame.pack(side=tk.TOP, fill=tk.X)
        toolbar = NavigationToolbar2Tk(self.sd_canvas, toolbar_frame)
        toolbar.update()
        
        # Pack canvas
        self.sd_canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        
        # Initial plot (without analysis)
        self._plot_stroke_detection_data()
        
        # Restore cached analysis if available
        if self.current_file and self.current_file.has_cached_analysis():
            self._restore_stroke_analysis()
        
        # Restore saved view state (zoom/pan position) if available
        if self.current_file and self.current_file.stroke_detection_view_xlim:
            self.sd_ax.set_xlim(self.current_file.stroke_detection_view_xlim)
            self.sd_canvas.draw_idle()
        if self.current_file and self.current_file.stroke_detection_view_ylim:
            self.sd_ax.set_ylim(self.current_file.stroke_detection_view_ylim)
            self.sd_canvas.draw_idle()
    
    def _restore_stroke_analysis(self):
        """Restore cached stroke analysis from current file."""
        if self.current_file is None or not self.current_file.has_cached_analysis():
            return
        
        # Restore settings
        if self.current_file.analysis_settings:
            if self.sd_window_size:
                self.sd_window_size.set(self.current_file.analysis_settings.get('window_size', 8))
            if self.sd_slope_threshold:
                self.sd_slope_threshold.set(self.current_file.analysis_settings.get('slope_threshold', 0))
        
        # Update stats display
        missed = sum(1 for a in self.current_file.stroke_anomalies if a['type'] == 'MISSED')
        duplicate = sum(1 for a in self.current_file.stroke_anomalies if a['type'] == 'DUPLICATE')
        total_strokes = len(self.current_file.stroke_markers)
        
        # Calculate num cycles from cached slopes
        if self.current_file.stroke_slopes is not None:
            slope_threshold = self.current_file.analysis_settings.get('slope_threshold', 0) if self.current_file.analysis_settings else 0
            crossings = find_slope_zero_crossings(self.current_file.stroke_slopes, slope_threshold)
            recovery_points = [idx for idx, ctype in crossings if ctype == 'pos_to_neg']
            num_cycles = max(0, len(recovery_points) - 1)
        else:
            num_cycles = 0
        
        stats_text = (
            f"Total stroke markers: {total_strokes}\n"
            f"Detected cycles (recovery-to-recovery): {num_cycles}\n"
            f"Missed strokes: {missed}\n"
            f"Duplicate strokes: {duplicate}\n"
            f"Anomaly rate: {(missed + duplicate) / max(1, num_cycles) * 100:.1f}%"
        )
        if self.sd_stats_label:
            self.sd_stats_label.config(text=stats_text)
        
        # Update anomaly listbox
        if self.sd_anomaly_listbox:
            self.sd_anomaly_listbox.delete(0, tk.END)
            for i, anomaly in enumerate(self.current_file.stroke_anomalies):
                atype = anomaly['type']
                start = anomaly['cycle_start']
                end = anomaly['cycle_end']
                count = anomaly['stroke_count']
                self.sd_anomaly_listbox.insert(
                    tk.END,
                    f"[{i+1}] {atype} @ {start}-{end} ({count} strokes)"
                )
                if atype == 'MISSED':
                    self.sd_anomaly_listbox.itemconfig(i, {'fg': 'red'})
                else:
                    self.sd_anomaly_listbox.itemconfig(i, {'fg': 'orange'})
        
        # Update plot
        self._plot_stroke_detection_data()
    
    def _run_stroke_analysis(self):
        """Run the stroke detection analysis with current parameters."""
        if self.current_file is None:
            return
        if self.current_file.stroke_clean_deltas is None or len(self.current_file.stroke_clean_deltas) == 0:
            return
        
        window_size = int(self.sd_window_size.get())
        slope_threshold = float(self.sd_slope_threshold.get())
        
        # Run detection using CLEAN deltas for slope analysis
        self.current_file.stroke_slopes, self.current_file.stroke_anomalies = detect_stroke_anomalies(
            self.current_file.stroke_clean_deltas,
            self.current_file.stroke_markers,
            window_size=window_size,
            slope_threshold=slope_threshold
        )
        
        # Cache the analysis settings
        self.current_file.analysis_settings = {
            'window_size': window_size,
            'slope_threshold': slope_threshold
        }
        
        # Update stats
        missed = sum(1 for a in self.current_file.stroke_anomalies if a['type'] == 'MISSED')
        duplicate = sum(1 for a in self.current_file.stroke_anomalies if a['type'] == 'DUPLICATE')
        total_strokes = len(self.current_file.stroke_markers)
        
        # Calculate number of cycles (recovery-to-recovery, i.e., pos_to_neg crossings)
        slopes = self.current_file.stroke_slopes
        crossings = find_slope_zero_crossings(slopes, slope_threshold)
        recovery_points = [idx for idx, ctype in crossings if ctype == 'pos_to_neg']
        num_cycles = max(0, len(recovery_points) - 1)
        
        stats_text = (
            f"Total stroke markers: {total_strokes}\n"
            f"Detected cycles (recovery-to-recovery): {num_cycles}\n"
            f"Missed strokes: {missed}\n"
            f"Duplicate strokes: {duplicate}\n"
            f"Anomaly rate: {(missed + duplicate) / max(1, num_cycles) * 100:.1f}%"
        )
        self.sd_stats_label.config(text=stats_text)
        
        # Update anomaly listbox
        self.sd_anomaly_listbox.delete(0, tk.END)
        for i, anomaly in enumerate(self.current_file.stroke_anomalies):
            atype = anomaly['type']
            start = anomaly['cycle_start']
            end = anomaly['cycle_end']
            count = anomaly['stroke_count']
            self.sd_anomaly_listbox.insert(
                tk.END,
                f"[{i+1}] {atype} @ {start}-{end} ({count} strokes)"
            )
            # Color code
            if atype == 'MISSED':
                self.sd_anomaly_listbox.itemconfig(i, {'fg': 'red'})
            else:
                self.sd_anomaly_listbox.itemconfig(i, {'fg': 'orange'})
        
        # Update plot
        self._plot_stroke_detection_data()
    
    def _plot_stroke_detection_data(self):
        """Plot the stroke detection data."""
        if self.sd_ax is None or self.current_file is None or self.current_file.stroke_raw_deltas is None:
            return
        
        # Clear axes
        self.sd_ax.clear()
        
        # Plot raw and clean delta times
        x = np.arange(len(self.current_file.stroke_raw_deltas))
        self.sd_ax.plot(x, self.current_file.stroke_raw_deltas, 'b-', linewidth=1.5, label='Raw Delta Time', alpha=0.8)
        
        # Plot clean delta times if available
        if self.current_file.stroke_clean_deltas is not None and len(self.current_file.stroke_clean_deltas) == len(self.current_file.stroke_raw_deltas):
            self.sd_ax.plot(x, self.current_file.stroke_clean_deltas, 'r-', linewidth=1.5, label='Clean Delta Time', alpha=0.8)
        
        # Store stroke line x-positions for hover detection
        self.stroke_line_positions = []
        
        # Plot recorded stroke markers as vertical lines (no text labels for performance)
        for stroke_num, (idx, _) in enumerate(self.current_file.stroke_markers, start=1):
            if idx < len(self.current_file.stroke_raw_deltas):
                self.sd_ax.axvline(x=idx, color='purple', linewidth=1.5, alpha=0.5)
                self.stroke_line_positions.append((idx, stroke_num))
        
        # Highlight anomalies only (errors from detection algorithm)
        for anomaly in self.current_file.stroke_anomalies:
            start = anomaly['cycle_start']
            end = anomaly['cycle_end']
            if anomaly['type'] == 'MISSED':
                self.sd_ax.axvspan(start, end, alpha=0.3, color='red', label='_nolegend_')
            else:  # DUPLICATE
                self.sd_ax.axvspan(start, end, alpha=0.3, color='orange', label='_nolegend_')
        
        # Configure axes
        self.sd_ax.set_xlabel('Sample Index', fontsize=10)
        self.sd_ax.set_ylabel('Delta Time (μs)', fontsize=10, color='blue')
        self.sd_ax.set_title('Stroke Detection Analysis (hover over stroke lines for number)', fontsize=12)
        self.sd_ax.grid(True, alpha=0.3)
        
        # Legend
        self.sd_ax.legend(loc='upper right')
        
        # Create hover annotation
        self.sd_hover_annotation = self.sd_ax.annotate(
            '', xy=(0, 0), xytext=(10, 10),
            textcoords='offset points',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='yellow', alpha=0.9),
            fontsize=9,
            visible=False
        )
        
        # Connect hover event
        self.sd_fig.canvas.mpl_connect('motion_notify_event', self._on_stroke_hover)
        
        self.sd_fig.tight_layout()
        self.sd_canvas.draw()
    
    def _on_stroke_hover(self, event):
        """Handle mouse hover to show stroke number tooltip."""
        if event.inaxes != self.sd_ax or not hasattr(self, 'stroke_line_positions'):
            if self.sd_hover_annotation is not None and self.sd_hover_annotation.get_visible():
                self.sd_hover_annotation.set_visible(False)
                self.sd_canvas.draw_idle()
            return
        
        # Find if we're near a stroke line (within tolerance)
        x = event.xdata
        if x is None:
            return
        
        # Get current view limits to calculate appropriate tolerance
        xlim = self.sd_ax.get_xlim()
        tolerance = (xlim[1] - xlim[0]) * 0.005  # 0.5% of visible x-range
        
        nearest_stroke = None
        min_dist = float('inf')
        
        for idx, stroke_num in self.stroke_line_positions:
            dist = abs(idx - x)
            if dist < tolerance and dist < min_dist:
                min_dist = dist
                nearest_stroke = (idx, stroke_num)
        
        if nearest_stroke is not None and self.current_file is not None:
            idx, stroke_num = nearest_stroke
            y_value = self.current_file.stroke_raw_deltas[idx] if idx < len(self.current_file.stroke_raw_deltas) else 0
            self.sd_hover_annotation.xy = (idx, y_value)
            self.sd_hover_annotation.set_text(f'Stroke #{stroke_num}')
            self.sd_hover_annotation.set_visible(True)
            self.sd_canvas.draw_idle()
        elif self.sd_hover_annotation is not None and self.sd_hover_annotation.get_visible():
            self.sd_hover_annotation.set_visible(False)
            self.sd_canvas.draw_idle()
    
    def _on_anomaly_select(self, event=None):
        """Handle anomaly selection from listbox."""
        self._goto_anomaly()
    
    def _goto_anomaly(self):
        """Navigate to the selected anomaly."""
        if self.current_file is None:
            return
            
        selection = self.sd_anomaly_listbox.curselection()
        if not selection or not self.current_file.stroke_anomalies:
            return
        
        idx = selection[0]
        self.current_file.current_anomaly_index = idx
        anomaly = self.current_file.stroke_anomalies[idx]
        
        # Calculate view window
        center = (anomaly['cycle_start'] + anomaly['cycle_end']) / 2
        margin = max(100, (anomaly['cycle_end'] - anomaly['cycle_start']) * 3)
        
        self.sd_ax.set_xlim(center - margin, center + margin)
        
        # Auto-adjust y limits for visible data
        x_min, x_max = int(max(0, center - margin)), int(min(len(self.current_file.stroke_raw_deltas), center + margin))
        if x_min < x_max:
            visible_data = self.current_file.stroke_raw_deltas[x_min:x_max]
            y_min, y_max = np.min(visible_data), np.max(visible_data)
            padding = (y_max - y_min) * 0.1
            self.sd_ax.set_ylim(y_min - padding, y_max + padding)
        
        self.sd_canvas.draw()
    
    def _prev_anomaly(self):
        """Navigate to previous anomaly."""
        if self.current_file is None or not self.current_file.stroke_anomalies:
            return
        
        self.current_file.current_anomaly_index = max(0, self.current_file.current_anomaly_index - 1)
        self.sd_anomaly_listbox.selection_clear(0, tk.END)
        self.sd_anomaly_listbox.selection_set(self.current_file.current_anomaly_index)
        self.sd_anomaly_listbox.see(self.current_file.current_anomaly_index)
        self._goto_anomaly()
    
    def _next_anomaly(self):
        """Navigate to next anomaly."""
        if self.current_file is None or not self.current_file.stroke_anomalies:
            return
        
        self.current_file.current_anomaly_index = min(len(self.current_file.stroke_anomalies) - 1, self.current_file.current_anomaly_index + 1)
        self.sd_anomaly_listbox.selection_clear(0, tk.END)
        self.sd_anomaly_listbox.selection_set(self.current_file.current_anomaly_index)
        self.sd_anomaly_listbox.see(self.current_file.current_anomaly_index)
        self._goto_anomaly()
        
    def _show_temporary_status(self, message: str, duration_ms: int = 2000, original_text: str = None):
        """Show a temporary status message that auto-dismisses.
        
        Args:
            message: Status message to display
            duration_ms: How long to show the message (default 2 seconds)
            original_text: Text to restore after the message disappears (if None, uses current loaded file info)
        """
        if self.info_label is None:
            return
        
        # Store the original text if not provided
        if original_text is None:
            if self.current_file:
                original_text = f"Loaded: {self.current_file.display_name}"
            else:
                original_text = "Use File → Open or 'Open File' button to load a data file"
        
        # Set the status message with a visual indicator
        self.info_label.config(text=f"✓ {message}", fg='#2E7D32')  # Green color
        
        # Schedule restoration of original text
        self.root.after(duration_ms, lambda: self.info_label.config(text=original_text, fg='#000000'))
        
    def _on_copy_chart(self, event=None):
        """Handle Ctrl+C to copy the currently active chart."""
        if self.notebook is None or self.current_file is None:
            return
        
        # Get current tab index
        active_tab_index = self.notebook.index(self.notebook.select())
        
        # Get tab name to determine which chart to copy
        try:
            tab_name = self.notebook.tab(active_tab_index, 'text')
            
            if tab_name == 'Delta Times':
                self._copy_delta_chart_to_clipboard()
            elif tab_name == 'Handle Forces':
                self._copy_forces_chart_to_clipboard()
            elif tab_name == 'Stroke Detection':
                self._copy_stroke_detection_chart_to_clipboard()
        except:
            pass
    
    def _on_close(self):
        """Handle window close event properly."""
        plt.close('all')
        self.root.quit()
        self.root.destroy()
        
    def run(self):
        """Start the application main loop."""
        self.root.mainloop()


def main():
    """Application entry point."""
    app = DataVisualizer()
    app.run()


if __name__ == '__main__':
    main()
