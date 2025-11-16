import serial
from serial.tools import list_ports
import time
import threading

product_id = 0x7523
end_marker = ';'# terminates a command to the board

# Castling is represented by the engine as a king move, but communicated to the
# board as a king move followed by a rook move
# Pair each castling king move with the corresponding rook move
castling_rook_moves = {'e1g1': 'h1f1', 'e1c1': 'a1d1', 'e8g8': 'h8f8', 'e8c8': 'a8d8'}

class BoardAdapter():
    '''Class for communicating with the board control hardware'''
    
    def __init__(self):
        self.connection_error = False
        self.waiting = False
        self.desired_response = None
        self.is_starting_position = False
        
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
            self.connection_error = True
            return
            
        # establish connection
        try:
            self.ser = serial.Serial(mc_port, 9600, rtscts=True)
        except Exception as e:
            print(f"Error opening serial port: {e}")
            self.connection_error = True
            return
            
        print(f'Microcontroller connected at port {mc_port}')
        print('------------------------')
        
        self.read_t = threading.Thread(target=self.read_serial, daemon=True)
        self.read_t.start()
        
        
    def read_serial(self):
        '''Monitor the serial port and print input as it is recieved'''
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
        '''Block until data is recieved over serial, or the time limit is reached'''
        if self.connection_error: return
        
        self.waiting = True
        self.desired_response=response
        stop_time = time.time() + timeout
        
        if response is None:
            print('Waiting for response')
        else:
            print(f'Waiting for response: {response}')
        
        while self.waiting:
            if time.time() > stop_time:
                print(f'\tTimeout after {timeout} secconds')
                return
            time.sleep(0.1)
        
    
    def write_serial(self, command, include_end_marker=True):
        if self.connection_error: return
        
        data = (command + end_marker) if include_end_marker else command
        self.ser.write(data.encode('utf-8'))
    
    def home(self):
        self.write_serial('H')
        self.await_response('Y axis homed')
    
    def adjust_pieces(self):
        self.write_serial('A')
        self.await_response('done', 60)
        
    def move(self, m, is_castling=False):
        '''Instruct the board to play a move in the form of a UCI string (e.g. e4e5)'''
        self.write_serial('P' + m)
        self.is_starting_position = False
        self.await_response('Move complete')
        
        # for castling, also move ethe rook to the appropriate square
        if is_castling:
            self.write_serial('P' + castling_rook_moves[m])
            self.await_response('Move complete')
            
        
    def initialize_position(self, position='starting'):
        '''Set the boaard state in memory'''
        if position == 'empty':
            self.write_serial('I0')
            self.await_response()
            self.is_starting_position = False
        elif position == 'starting':
            self.write_serial('I1')
            self.await_response()
            self.is_starting_position = True
        else:
            raise ValueError('Invalid position code')
        
            
    def setup_position(self, position='starting'):
        '''Set up the board (physically moveing the pieces)'''
        if position == 'empty':
            self.write_serial('S0')
            self.await_response('Position set', 120)
            self.is_starting_position = False
        elif position == 'starting':
            self.write_serial('S1')
            self.await_response('Position set', 120)
            self.is_starting_position = True
        else:
            raise ValueError('Invalid position code')
        
        