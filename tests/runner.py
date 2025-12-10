# testlib.py
import socket
import time
import logging
import threading
from typing import Optional
import subprocess

class QEMUConnectionError(Exception):
    """Raised when QEMU connection fails"""
    pass


class KernelTestTimeoutError(Exception):
    """Raised when kernel test times out"""
    pass

class TestRunner:
    def __init__(self, serial_host: str = "localhost", serial_port: int = 5555,
                 monitor_host: str = "localhost", monitor_port: int = 4444,
                 default_timeout: float = 3, retry_attempts: int = 3):
        
        self.serial_host = serial_host
        self.serial_port = serial_port
        self.monitor_host = monitor_host
        self.monitor_port = monitor_port
        self.default_timeout = default_timeout
        self.retry_attempts = retry_attempts
        
        self._serial_socket: Optional[socket.socket] = None
        self._monitor_socket: Optional[socket.socket] = None
        self._lock = threading.Lock()
        
        # Setup logging
        self.logger = logging.getLogger(f"{__name__}.TestRunner")
        # debug mode logging
        # self.logger.setLevel(logging.DEBUG)

    def setup(self):
        self.logger.info("Connecting to the VM")
        
        try:
            self._connect_serial()
            self._connect_monitor()
            
        except Exception as e:
            self.logger.error(f"Failed to setup TestRunner: {e}")
            self.cleanup()
            raise QEMUConnectionError(f"Setup failed: {e}")
        
    def cleanup(self):
        """Cleanup connections"""
        self.logger.info("Cleaning up")
        
        with self._lock:
            if self._serial_socket:
                try:
                    self._serial_socket.close()
                except:
                    pass
                self._serial_socket = None
            
            if self._monitor_socket:
                try:
                    self._monitor_socket.close()
                except:
                    pass
                self._monitor_socket = None
        
        # subprocess.run(["pkill", "-f", "qemu-system-i386"])

    def _connect_serial(self):
        """Establish serial connection to kernel"""
        for attempt in range(self.retry_attempts):
            try:
                self._serial_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self._serial_socket.settimeout(self.default_timeout)
                self._serial_socket.connect((self.serial_host, self.serial_port))
                self.logger.info(f"Connected to serial at port {self.serial_port}")
                return
            except Exception as e:
                self.logger.warning(f"Serial connection attempt {attempt + 1} failed: {e}")
                if self._serial_socket:
                    self._serial_socket.close()
                    self._serial_socket = None
                if attempt == self.retry_attempts - 1:
                    raise
                time.sleep(1)
    
    def _connect_monitor(self):
        """Establish connection to QEMU monitor"""
        for attempt in range(self.retry_attempts):
            try:
                self._monitor_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self._monitor_socket.settimeout(self.default_timeout)
                self._monitor_socket.connect((self.monitor_host, self.monitor_port))
                
                # Read QEMU monitor welcome message
                welcome = self._monitor_socket.recv(1024).decode('utf-8', errors='ignore')
                self.logger.info(f"Monitor connection established")
                return
            except Exception as e:
                self.logger.warning(f"Monitor connection attempt {attempt + 1} failed: {e}")
                if self._monitor_socket:
                    self._monitor_socket.close()
                    self._monitor_socket = None
                if attempt == self.retry_attempts - 1:
                    raise
                time.sleep(1)
    
    def send_serial(self, command: str, timeout: Optional[float] = None) -> str:
        """
        Send command to kernel testing system via serial and read response
        
        Args:
            command: Test command to send to kernel
            timeout: Timeout for response (uses default if None)
        
        Returns:
            Raw response string from kernel (including the end marker)
        """
        if not self._serial_socket:
            raise QEMUConnectionError("Serial connection not established")
        
        timeout = timeout or self.default_timeout
        
        try:
            with self._lock:
                # Send command
                command_bytes = f"{command}\n".encode('ascii')
                self._serial_socket.send(command_bytes)
                self.logger.debug(f"Sent command: {command}")
                
                # Read response until end marker
                response = self._read_until_end_marker( marker="*", timeout=timeout )
                self.logger.debug(f"Received response ({len(response)} chars)")
                
                return response
                
        except socket.timeout:
            raise KernelTestTimeoutError(f"'{command}' timed out after {timeout}s")
        except Exception as e:
            raise QEMUConnectionError(f"Failed to send '{command}': {e}")

    def _read_until_end_marker(self, marker: str ,timeout: float) -> str:
        """Read from serial socket until [END] marker is found"""
        self._serial_socket.settimeout(timeout)
        response = ""
        
        while True:
            try:
                data = self._serial_socket.recv(1024).decode('ascii', errors='ignore')
                if not data:
                    break
                
                response += data
                
                # Check if we've received the end marker
                if response.endswith(marker):
                    return response
                    
            except socket.timeout:
                raise
        
        # If we get here, connection was closed without [END] marker
        raise QEMUConnectionError("Connection closed before receiving [END] marker")

    def send_monitor(self, command: str, timeout: Optional[float] = None) -> str:
        """
        Send command to QEMU monitor
        
        Args:
            command: QEMU monitor command
            timeout: Timeout for response
        
        Returns:
            Raw response string from monitor
        """
        if not self._monitor_socket:
            raise QEMUConnectionError("Monitor connection not established")
        
        timeout = timeout or self.default_timeout
        
        try:
            with self._lock:
                # Send command
                command_bytes = f"{command}\n".encode('ascii')
                self._monitor_socket.send(command_bytes)
                self.logger.debug(f"Sent monitor command: {command}")
                time.sleep(0.01)  # Give QEMU some time to process
                
                # Read response
                response = self._read_monitor_response(timeout)
                self.logger.debug(f"Monitor response received")
                
                return response
                
        except socket.timeout:
            raise KernelTestTimeoutError(f"Monitor command '{command}' timed out after {timeout}s")
        except Exception as e:
            raise QEMUConnectionError(f"Failed to send monitor command '{command}': {e}")

    def _read_monitor_response(self, timeout: float) -> str:
        """Read response from QEMU monitor"""
        self._monitor_socket.settimeout(timeout)
        response = ""
        
        while True:
            try:
                data = self._monitor_socket.recv(1024).decode('ascii', errors='ignore')
                if not data:
                    break
                response += data
                
                # QEMU monitor typically ends responses with (qemu) prompt
                if response.endswith("(qemu) "):
                    return response.strip()
                    
            except socket.timeout:
                break
        
        return response.strip()

