#!/usr/bin/env python3
"""
HTTP OTA Upload Script for ESP32
Usage: python3 ota_upload.py <firmware.bin> [ip_address]
"""

import sys
import os
import requests
from pathlib import Path

DEFAULT_IP = "192.168.1.252"
TIMEOUT = 60

def upload_firmware(firmware_path: str, ip: str) -> bool:
    """Upload firmware to ESP32 via HTTP POST"""
    
    if not os.path.exists(firmware_path):
        print(f"❌ File not found: {firmware_path}")
        return False
    
    file_size = os.path.getsize(firmware_path)
    print(f"📦 Firmware: {firmware_path} ({file_size:,} bytes)")
    print(f"🎯 Target: http://{ip}/api/update")
    print()
    
    url = f"http://{ip}/api/update"
    
    try:
        with open(firmware_path, 'rb') as f:
            print("⬆️  Uploading...")
            files = {'firmware': (Path(firmware_path).name, f, 'application/octet-stream')}
            response = requests.post(url, files=files, timeout=TIMEOUT)
        
        if response.status_code == 200:
            print(f"✅ Success! {response.json()}")
            print("🔄 Device is rebooting...")
            return True
        else:
            print(f"❌ Failed: {response.status_code}")
            print(f"   {response.text}")
            return False
            
    except requests.exceptions.ConnectTimeout:
        print("❌ Connection timeout - device not reachable")
        return False
    except requests.exceptions.ConnectionError as e:
        print(f"❌ Connection error: {e}")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 ota_upload.py <firmware.bin> [ip_address]")
        print(f"Default IP: {DEFAULT_IP}")
        sys.exit(1)
    
    firmware_path = sys.argv[1]
    ip = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_IP
    
    success = upload_firmware(firmware_path, ip)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
