import serial
from serial.tools import list_ports
import sys
import time
import threading

product_id = 0x7523
end_marker = ';'# terminates a command to the board


class BoardAdapter():
    '''Class for communicating with the board control hardware'''
    
    def __init__(self):
        self.waiting = False
        self.desired_response=None
        
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
            self.ser = serial.Serial(mc_port, 9600, rtscts=True)
        except Exception as e:
            print(f"Error opening serial port: {e}")
            sys.exit()
            
        print(f'Microcontroller connected at port {mc_port}')
        
        
        self.read_t = threading.Thread(target=self.read_serial, daemon=True)
        self.read_t.start()
        
        
    def read_serial(self):
        '''monitor the serial port for a length of time and print input'''
        while True:
            try:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if not line: continue
                    print(f'IN:\t{line}')
                    
                    if self.waiting and (self.desired_response is None or line == self.desired_response):
                        self.waiting = False
                            
            except Exception as e:
                print(f"Error reading serial: {e}")
                
    def await_response(self, response=None, timeout=10):
        '''block until data is recieved over serial, or the time limit is reached'''
        self.waiting = True
        self.desired_response=response
        stop_time = time.time() + timeout
        
        if response is None:
            print('Waiting for response')
        else:
            print(f'Waiting for response: {response}')
        
        while self.waiting:
            if time.time() > stop_time:
                print('\tTimeout after {timeout} secconds')
            time.sleep(0.1)
        
    
    def write_serial(self, command):
        data = command + end_marker
        self.ser.write(data.encode('utf-8'))
    
    def home(self):
        self.write_serial('H')
        self.await_response('Y axis homed')
        
    def move(self, m):
        '''instruct the board to play a move in the form of a UCI string (e.g. e4e5)'''
        #TODO: identify and handle castling
        self.write_serial('P' + m)
        self.await_response('Move complete')
        
    def initialize_position(self, position='starting'):
        if position == 'empty':
            self.write_serial('I0')
        elif position == 'starting':
            self.write_serial('I1')
        else:
            raise ValueError('Invalid position code')
        