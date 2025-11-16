import serial
from serial.tools import list_ports
import sys
import time

# vendor_id = 0x1a86
product_id = 0x7523

def read_serial(ser, duration):
    '''monitor the serial port for a length of time and print input'''
    start_time = time.time()
    while time.time() - start_time < duration:
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if not line: continue
                print('\t', line)
                        
        except Exception as e:
            print(f"Error reading serial: {e}")
            

if __name__ == '__main__':
    # query connceted devices
    print('Detected USB devices:')
    ports = list_ports.comports()
    mc_port = None
    for p in ports:
        print(f'\t{p.device}: {p.description}\n\t\tpid: {p.pid}')
        if p.pid == product_id:
            mc_port = p.device
    
    # check whether the device was found
    if mc_port is None:
        print('Microcontroller not detected')
        sys.exit()
        
    # establish connection
    try:
        ser = serial.Serial(mc_port, 9600, rtscts=True)
    except Exception as e:
        print(f"Error opening serial port: {e}")
        sys.exit()
        
    print(f'Microcontroller connected at port {mc_port}')
    read_serial(ser, 3)
    
    # send data
    message = 'H;'
    print(f'Sending data: {message}')
    ser.write(message.encode('utf-8'))
    
    # read serial
    read_serial(ser, 10)
    
    # close the port when finished
    ser.close()
    print('port closed')
        
        
        