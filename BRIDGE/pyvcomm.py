#!/usr/bin/env python3
"""
Direct Virtual Null Modem Solution
Creates devices that serial programs will actually detect by using the right approach
"""

import os
import sys
import subprocess
import threading
import time
import signal
import logging
from typing import Optional, Tuple

class DirectNullModem:
    def __init__(self, logger: Optional[logging.Logger] = None):
        self.logger = logger or logging.getLogger(__name__)
        self.socat_process: Optional[subprocess.Popen] = None
        self.device1: str = "/tmp/ttyV0"
        self.device2: str = "/tmp/ttyV1"
        self.running = False
        
    def create_null_modem(self) -> bool:
        """Create a null modem using socat with persistent device names"""
        if self.running:
            self.logger.error("Null modem is already running")
            return False
        
        # Check if socat is available
        try:
            subprocess.run(['socat', '-V'], capture_output=True, check=True)
        except (FileNotFoundError, subprocess.CalledProcessError):
            self.logger.error("socat not found. Install with: sudo apt-get install socat")
            return False
        
        # Remove existing devices if they exist
        for device in [self.device1, self.device2]:
            if os.path.exists(device):
                try:
                    os.unlink(device)
                except PermissionError:
                    self.logger.error(f"Cannot remove existing device {device}. Run with sudo.")
                    return False
        
        try:
            # Create the socat command that creates persistent device links
            cmd = [
                'socat',
                '-d', '-d',  # Verbose output
                f'pty,raw,echo=0,link={self.device1}',
                f'pty,raw,echo=0,link={self.device2}'
            ]
            
            self.logger.info(f"Starting socat: {' '.join(cmd)}")
            
            # Start socat in the background
            self.socat_process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            # Wait for devices to be created
            for _ in range(50):  # Wait up to 5 seconds
                if os.path.exists(self.device1) and os.path.exists(self.device2):
                    break
                time.sleep(0.1)
            else:
                self.logger.error("Devices not created within timeout")
                self.stop()
                return False
            
            # Check if socat is still running
            if self.socat_process.poll() is not None:
                stdout, stderr = self.socat_process.communicate()
                self.logger.error(f"socat failed: {stderr}")
                return False
            
            # Try to set proper permissions
            try:
                os.chmod(self.device1, 0o666)
                os.chmod(self.device2, 0o666)
            except PermissionError:
                self.logger.warning("Could not set device permissions (run with sudo for better compatibility)")
            
            self.running = True
            self.logger.info(f"Created null modem: {self.device1} <-> {self.device2}")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to create null modem: {e}")
            self.stop()
            return False
    
    def stop(self):
        """Stop the null modem"""
        if not self.running:
            return
        
        self.logger.info("Stopping null modem...")
        self.running = False
        
        # Stop socat process
        if self.socat_process:
            try:
                self.socat_process.terminate()
                try:
                    self.socat_process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    self.socat_process.kill()
                    self.socat_process.wait()
            except Exception as e:
                self.logger.warning(f"Error stopping socat: {e}")
            
            self.socat_process = None
        
        # Clean up device links (they should disappear automatically, but just in case)
        for device in [self.device1, self.device2]:
            if os.path.exists(device):
                try:
                    os.unlink(device)
                except Exception:
                    pass
        
        self.logger.info("Null modem stopped")
    
    def get_devices(self) -> Tuple[str, str]:
        """Get the device names"""
        return self.device1, self.device2
    
    def is_running(self) -> bool:
        """Check if the null modem is running"""
        if not self.running:
            return False
        
        # Check if socat process is still alive
        if self.socat_process and self.socat_process.poll() is not None:
            self.logger.warning("socat process died unexpectedly")
            self.running = False
            return False
        
        # Check if devices still exist
        if not (os.path.exists(self.device1) and os.path.exists(self.device2)):
            self.logger.warning("Devices disappeared")
            self.running = False
            return False
        
        return True
    
    def test_communication(self) -> bool:
        """Test communication between the devices"""
        if not self.is_running():
            return False
        
        try:
            import serial
            
            # Open both devices
            ser1 = serial.Serial(self.device1, 9600, timeout=1)
            ser2 = serial.Serial(self.device2, 9600, timeout=1)
            
            # Test message
            test_msg = b"Hello null modem!"
            
            # Send from device1 to device2
            ser1.write(test_msg)
            ser1.flush()
            
            time.sleep(0.1)
            
            received = ser2.read(len(test_msg))
            
            ser1.close()
            ser2.close()
            
            success = received == test_msg
            if success:
                self.logger.info("✓ Communication test passed")
            else:
                self.logger.error(f"✗ Communication test failed: sent {test_msg}, received {received}")
            
            return success
            
        except Exception as e:
            self.logger.error(f"Communication test failed: {e}")
            return False
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop()

def check_serial_program_detection():
    """Check what serial programs can detect after creating devices"""
    print("\n=== Checking Serial Program Detection ===")
    
    # Check pyserial detection
    try:
        import serial.tools.list_ports
        ports = [port.device for port in serial.tools.list_ports.comports()]
        print(f"PySerial detected ports: {ports}")
        
        # Check if our devices are detected
        our_devices = ["/dev/ttyV0", "/dev/ttyV1"]
        detected = [d for d in our_devices if d in ports]
        if detected:
            print(f"✓ Our devices detected by PySerial: {detected}")
        else:
            print(f"✗ Our devices NOT detected by PySerial")
            
    except ImportError:
        print("PySerial not available")

def main():
    """Test the direct null modem"""
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
    
    print("Direct Virtual Null Modem Solution")
    print("=" * 40)
    
    def signal_handler(signum, frame):
        print(f"\nReceived signal {signum}, shutting down...")
        modem.stop()
        sys.exit(0)
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    with DirectNullModem() as modem:
        print(f"Creating null modem...")
        
        if modem.create_null_modem():
            device1, device2 = modem.get_devices()
            
            print(f"✓ Created devices: {device1}, {device2}")
            
            # Check detection
            check_serial_program_detection()
            
            # Test communication
            print("\nTesting communication...")
            if modem.test_communication():
                print("✓ Communication works!")
            else:
                print("✗ Communication failed")
            
            print(f"\nDevices created:")
            print(f"  {device1}")
            print(f"  {device2}")
            print()
            print("Now test with your serial terminal programs:")
            print("1. Open cutecom")
            print("2. Open moserial") 
            print("3. Open serial port assistant")
            print()
            print("IMPORTANT: If the devices don't appear in the dropdown lists,")
            print("it means these programs are filtering them out based on device")
            print("properties or subsystem information.")
            print()
            print("You can still test the devices directly with:")
            print(f"  python3 -c \"import serial; s=serial.Serial('{device1}', 9600); s.write(b'test'); s.close()\"")
            print()
            print("Press Ctrl+C to stop...")
            
            try:
                while modem.is_running():
                    time.sleep(1)
            except KeyboardInterrupt:
                pass
        else:
            print("✗ Failed to create null modem")
            print("\nTroubleshooting:")
            print("1. Install socat: sudo apt-get install socat")
            print("2. Run with sudo for better device permissions")
            print("3. Check that /dev/ is writable")

if __name__ == "__main__":
    main()
