import serial
import time
from datetime import datetime

# Configuration
BLUETOOTH_PORT = 'COM3'  # Change to your Bluetooth port (COM3, COM4, etc. on Windows)
BAUD_RATE = 9600
OUTPUT_FILE = 'sensor_data.csv'

def connect_bluetooth(port, baudrate):
    """
    Connect to HC-05 Bluetooth module
    """
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"[✓] Connected to {port} at {baudrate} baud")
        return ser
    except serial.SerialException as e:
        print(f"[✗] Failed to connect: {e}")
        return None

def parse_sensor_data(data_string):
    """
    Parse sensor data from Arduino
    Format: FLEX1,FLEX2,...,FLEX10,ACCEL_X,ACCEL_Y,ACCEL_Z,GYRO_X,GYRO_Y,GYRO_Z
    """
    try:
        values = data_string.strip().split(',')
        if len(values) != 16:
            return None
        
        return {
            'timestamp': datetime.now().isoformat(),
            'flex_1': int(values[0]),
            'flex_2': int(values[1]),
            'flex_3': int(values[2]),
            'flex_4': int(values[3]),
            'flex_5': int(values[4]),
            'flex_6': int(values[5]),
            'flex_7': int(values[6]),
            'flex_8': int(values[7]),
            'flex_9': int(values[8]),
            'flex_10': int(values[9]),
            'accel_x': float(values[10]),
            'accel_y': float(values[11]),
            'accel_z': float(values[12]),
            'gyro_x': float(values[13]),
            'gyro_y': float(values[14]),
            'gyro_z': float(values[15])
        }
    except (ValueError, IndexError):
        return None

def save_to_csv(data, filename):
    """
    Save sensor data to CSV file
    """
    try:
        with open(filename, 'a') as f:
            line = f"{data['timestamp']},{data['flex_1']},{data['flex_2']},{data['flex_3']},{data['flex_4']},{data['flex_5']},{data['flex_6']},{data['flex_7']},{data['flex_8']},{data['flex_9']},{data['flex_10']},{data['accel_x']},{data['accel_y']},{data['accel_z']},{data['gyro_x']},{data['gyro_y']},{data['gyro_z']}\n"
            f.write(line)
    except IOError as e:
        print(f"[✗] Error writing to CSV: {e}")

def initialize_csv(filename):
    """
    Create CSV file with headers if it doesn't exist
    """
    try:
        with open(filename, 'w') as f:
            headers = "Timestamp,Flex1,Flex2,Flex3,Flex4,Flex5,Flex6,Flex7,Flex8,Flex9,Flex10,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ\n"
            f.write(headers)
        print(f"[✓] CSV file initialized: {filename}")
    except IOError as e:
        print(f"[✗] Error creating CSV: {e}")

def main():
    """
    Main loop - receive and process sensor data
    """
    # Initialize CSV file
    initialize_csv(OUTPUT_FILE)
    
    # Connect to Bluetooth
    ser = connect_bluetooth(BLUETOOTH_PORT, BAUD_RATE)
    if not ser:
        return
    
    print("[*] Receiving sensor data... (Press Ctrl+C to stop)")
    print("-" * 120)
    
    data_count = 0
    
    try:
        while True:
            if ser.in_waiting:
                # Read line from Arduino
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if line:
                    # Check for gesture recognition
                    if 'GESTURE:' in line:
                        print(f"[!] {line}")
                        continue
                    
                    # Parse sensor data
                    parsed_data = parse_sensor_data(line)
                    if parsed_data:
                        # Save to CSV
                        save_to_csv(parsed_data, OUTPUT_FILE)
                        data_count += 1
                        
                        # Display in console
                        if data_count % 10 == 0:  # Print every 10th reading
                            print(f"[{data_count}] Flex: {parsed_data['flex_1']:3d} {parsed_data['flex_2']:3d} {parsed_data['flex_3']:3d} {parsed_data['flex_4']:3d} {parsed_data['flex_5']:3d} | "
                                  f"Accel: {parsed_data['accel_x']:6.2f} {parsed_data['accel_y']:6.2f} {parsed_data['accel_z']:6.2f}")
                    else:
                        print(f"[✗] Failed to parse: {line}")
            
            time.sleep(0.01)
    
    except KeyboardInterrupt:
        print("\n" + "-" * 120)
        print(f"[*] Stopped. Total readings: {data_count}")
        print(f"[✓] Data saved to: {OUTPUT_FILE}")
    
    finally:
        ser.close()
        print("[✓] Bluetooth connection closed")

if __name__ == "__main__":
    main()
