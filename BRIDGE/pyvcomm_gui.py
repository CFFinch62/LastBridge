#!/usr/bin/env python3
"""
PyVComm GUI - Graphical interface for the Direct Virtual Null Modem
"""

import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import threading
import time
import logging
import os
import sys
from pyvcomm import DirectNullModem

class PyVCommGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("PyVComm - Virtual Null Modem")
        self.root.geometry("600x500")
        
        # Set up logging
        self.setup_logging()
        
        self.modem = DirectNullModem(self.logger)
        self.status_update_thread = None
        self.running = True
        
        self.setup_ui()
        self.start_status_updates()
        
        # Handle window close
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
    
    def setup_logging(self):
        """Set up logging to capture messages"""
        self.logger = logging.getLogger('PyVCommGUI')
        self.logger.setLevel(logging.INFO)
        
        # Create a custom handler that will send logs to our GUI
        self.log_handler = GUILogHandler(self)
        self.log_handler.setLevel(logging.INFO)
        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        self.log_handler.setFormatter(formatter)
        self.logger.addHandler(self.log_handler)
    
    def setup_ui(self):
        # Main frame
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Title
        title_label = ttk.Label(main_frame, text="PyVComm - Virtual Null Modem", 
                               font=("Arial", 14, "bold"))
        title_label.pack(pady=(0, 10))
        
        # Device configuration section
        config_frame = ttk.LabelFrame(main_frame, text="Virtual Device Configuration", padding="10")
        config_frame.pack(fill=tk.X, pady=(0, 10))
        
        # Device names
        device_grid = ttk.Frame(config_frame)
        device_grid.pack(fill=tk.X)
        
        ttk.Label(device_grid, text="Device 1:").grid(row=0, column=0, sticky=tk.W, padx=(0, 5))
        self.device1_var = tk.StringVar(value="/tmp/ttyV0")
        device1_entry = ttk.Entry(device_grid, textvariable=self.device1_var, width=20)
        device1_entry.grid(row=0, column=1, sticky=(tk.W, tk.E), padx=(0, 20))

        ttk.Label(device_grid, text="Device 2:").grid(row=0, column=2, sticky=tk.W, padx=(0, 5))
        self.device2_var = tk.StringVar(value="/tmp/ttyV1")
        device2_entry = ttk.Entry(device_grid, textvariable=self.device2_var, width=20)
        device2_entry.grid(row=0, column=3, sticky=(tk.W, tk.E))
        
        device_grid.columnconfigure(1, weight=1)
        device_grid.columnconfigure(3, weight=1)
        
        # Control buttons
        button_frame = ttk.Frame(config_frame)
        button_frame.pack(pady=(15, 0))
        
        self.start_button = ttk.Button(button_frame, text="Start Virtual Null Modem", 
                                      command=self.start_modem)
        self.start_button.pack(side=tk.LEFT, padx=(0, 10))
        
        self.stop_button = ttk.Button(button_frame, text="Stop Virtual Null Modem", 
                                     command=self.stop_modem, state=tk.DISABLED)
        self.stop_button.pack(side=tk.LEFT, padx=(0, 10))
        
        self.test_button = ttk.Button(button_frame, text="Test Communication", 
                                     command=self.test_communication, state=tk.DISABLED)
        self.test_button.pack(side=tk.LEFT)
        
        # Status section
        status_frame = ttk.LabelFrame(main_frame, text="Status", padding="10")
        status_frame.pack(fill=tk.X, pady=(0, 10))
        
        status_grid = ttk.Frame(status_frame)
        status_grid.pack(fill=tk.X)
        
        ttk.Label(status_grid, text="Status:").grid(row=0, column=0, sticky=tk.W, padx=(0, 10))
        self.status_label = ttk.Label(status_grid, text="Stopped", foreground="red")
        self.status_label.grid(row=0, column=1, sticky=tk.W)
        
        ttk.Label(status_grid, text="Active Devices:").grid(row=1, column=0, sticky=tk.W, padx=(0, 10))
        self.devices_label = ttk.Label(status_grid, text="None")
        self.devices_label.grid(row=1, column=1, sticky=tk.W)
        
        # Instructions
        instructions_frame = ttk.LabelFrame(status_frame, text="Usage Instructions", padding="5")
        instructions_frame.pack(fill=tk.X, pady=(10, 0))
        
        instructions = ("1. Click 'Start Virtual Null Modem' to create the devices\n"
                       "2. Use the device paths shown above in your applications\n"
                       "3. Data sent to one device will appear on the other\n"
                       "4. Devices are created in /tmp/ (no root privileges required)\n"
                       "5. Compatible with any program that accepts custom port paths")
        
        ttk.Label(instructions_frame, text=instructions, justify=tk.LEFT).pack(anchor=tk.W)
        
        # Log section
        log_frame = ttk.LabelFrame(main_frame, text="Log", padding="10")
        log_frame.pack(fill=tk.BOTH, expand=True)
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=8, state=tk.DISABLED)
        self.log_text.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        
        log_controls = ttk.Frame(log_frame)
        log_controls.pack(fill=tk.X)
        
        ttk.Button(log_controls, text="Clear Log", command=self.clear_log).pack(side=tk.LEFT)
    
    def start_modem(self):
        """Start the virtual null modem"""
        device1 = self.device1_var.get().strip()
        device2 = self.device2_var.get().strip()
        
        if not device1 or not device2:
            messagebox.showerror("Error", "Please enter valid device names")
            return
        
        if device1 == device2:
            messagebox.showerror("Error", "Device names must be different")
            return
        
        # Update the modem's device names
        self.modem.device1 = device1
        self.modem.device2 = device2
        
        def start_thread():
            if self.modem.create_null_modem():
                self.root.after(0, self.update_ui_started)
            else:
                self.root.after(0, lambda: messagebox.showerror("Error", 
                    "Failed to start virtual null modem. Check log for details."))
        
        threading.Thread(target=start_thread, daemon=True).start()
    
    def stop_modem(self):
        """Stop the virtual null modem"""
        def stop_thread():
            self.modem.stop()
            self.root.after(0, self.update_ui_stopped)
        
        threading.Thread(target=stop_thread, daemon=True).start()
    
    def test_communication(self):
        """Test communication between devices"""
        def test_thread():
            success = self.modem.test_communication()
            message = "✓ Communication test passed!" if success else "✗ Communication test failed!"
            self.root.after(0, lambda: messagebox.showinfo("Test Result", message))
        
        threading.Thread(target=test_thread, daemon=True).start()
    
    def update_ui_started(self):
        """Update UI when modem is started"""
        self.start_button.config(state=tk.DISABLED)
        self.stop_button.config(state=tk.NORMAL)
        self.test_button.config(state=tk.NORMAL)
        self.status_label.config(text="Running", foreground="green")
        
        device1, device2 = self.modem.get_devices()
        self.devices_label.config(text=f"{device1} ↔ {device2}")
    
    def update_ui_stopped(self):
        """Update UI when modem is stopped"""
        self.start_button.config(state=tk.NORMAL)
        self.stop_button.config(state=tk.DISABLED)
        self.test_button.config(state=tk.DISABLED)
        self.status_label.config(text="Stopped", foreground="red")
        self.devices_label.config(text="None")
    
    def clear_log(self):
        """Clear the log"""
        self.log_text.config(state=tk.NORMAL)
        self.log_text.delete(1.0, tk.END)
        self.log_text.config(state=tk.DISABLED)
    
    def add_log_message(self, message: str):
        """Add a message to the log (thread-safe)"""
        self.log_text.config(state=tk.NORMAL)
        self.log_text.insert(tk.END, f"{message}\n")
        self.log_text.see(tk.END)
        self.log_text.config(state=tk.DISABLED)
        
        # Limit log length
        lines = self.log_text.index(tk.END).split('.')[0]
        if int(lines) > 500:
            self.log_text.config(state=tk.NORMAL)
            self.log_text.delete('1.0', '100.0')
            self.log_text.config(state=tk.DISABLED)
    
    def start_status_updates(self):
        """Start the status update thread"""
        def update_loop():
            while self.running:
                # Check if modem is still running
                if self.modem.is_running():
                    # Update status if needed
                    pass
                time.sleep(1)
        
        self.status_update_thread = threading.Thread(target=update_loop, daemon=True)
        self.status_update_thread.start()
    
    def on_closing(self):
        """Handle window closing"""
        self.running = False
        if self.modem.is_running():
            self.modem.stop()
        self.root.destroy()

class GUILogHandler(logging.Handler):
    """Custom logging handler that sends logs to the GUI"""
    def __init__(self, gui):
        super().__init__()
        self.gui = gui
    
    def emit(self, record):
        msg = self.format(record)
        self.gui.root.after_idle(lambda: self.gui.add_log_message(msg))

def main():
    root = tk.Tk()
    app = PyVCommGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
