import asyncio
import csv
import time
from bleak import BleakClient

# Use the address found by your scanner script [cite: 64]
DEVICE_ADDRESS = "3795D2F2-3400-2CCB-1FE0-DE367315789E" 
CHAR_UUID = "12345678-1234-1234-1234-1234567890ac"
OUTPUT_FILE = "solar_policy2.csv"

collected_data = []

def handle_notify(handle, data_bytes):
    try:
     
        text = data_bytes.decode("utf-8").strip()
        v, i, p, s = map(float, text.split(","))
        timestamp = time.time()
        
        collected_data.append((timestamp, v, i, p, s))
   
        print(f"V: {v:.3f}V | I: {i:.4f}A | P: {v*i:.4f}W | State: {int(s)}%")
    except Exception as e:
        print(f"Parsing Error: {e}")

async def main():
    print(f"Connecting to {DEVICE_ADDRESS}...")
    try:
        async with BleakClient(DEVICE_ADDRESS) as client:
            print(f"Connected: {client.is_connected}")
            
            await client.start_notify(CHAR_UUID, handle_notify)
            print("Logging for 250 seconds... Please wait.")

            # comment out between 60 and 250: 250 for fan testing and 60 for motor testing
            #await asyncio.sleep(250) 
            await asyncio.sleep(600)  
            
            await client.stop_notify(CHAR_UUID)
    except Exception as e:
        print(f"Connection failed: {e}")
        return

    with open(OUTPUT_FILE, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Timestamp", "Voltage_V", "Current_A", "Power_W", "State_Percent"])
        writer.writerows(collected_data)
    
    print(f"Finished! Data saved to {OUTPUT_FILE}")

if __name__ == "__main__":
    asyncio.run(main())