# from gpiozero import PWMLED
import gpiozero
import threading
import time
from main import GameManager, Command


# pin definitions
button_pins = [11, 4, 10, 27]
button_led_pins = [5, 17, 9, 22]
toggle_sw_pins = [3, 13]
toggle_led_pins = [2, 6]
rot_sw_pins = [19, 26, 21, 20, 16, 12]

# class InputData:
#     def __init__(self):
#         self.last_button = None
#         self.rot_sw_value = 0
#         self.keyboard_input = None

class IO_Manager():
    '''Class for controling LEDs, reading switch and button states, and communicating control inputs to the main program'''
    def __init__(self, command_queue):
        self.command_queue = command_queue
        
        numbuttons = 4
        self.buttons = [None] * numbuttons
        for i in range(numbuttons):
            self.buttons[i] = MyButton(button_pins[i], button_led_pins[i], i, i + numbuttons)
            
        
        self.white_toggle = ToggleSwitch(toggle_sw_pins[0], toggle_led_pins[0], default_state=True)
        self.black_toggle = ToggleSwitch(toggle_sw_pins[1], toggle_led_pins[1], default_state=True)
        self.toggle_states = (False, False)
        
        self.rot_sw = RotarySwitch(rot_sw_pins)
        self.rot_sw_value = -2


    def update_loop(self):
        while True:
            # read rotary switch
            val = self.rot_sw.read()
            if val != self.rot_sw_value:
                self.rot_sw_value = val
                if val == -1:
                    print("Error reading rotary switch: could not detect position")
                else:
                    self.command_queue.put(Command('configure_engine', val, None))
                
            # read toggle switches
            new_toggle_states = (self.white_toggle.read(), self.black_toggle.read())
            if new_toggle_states != self.toggle_states:
                self.toggle_states = new_toggle_states
                self.command_queue.put(Command('configure_players', self.toggle_states, None))
            
            # read pushbuttons
            for b in self.buttons:
                val = b.update()
                if val != -1:
                    self.command_queue.put(Command('button_press', val, time.time() + 0.1))
                
class KeyboardManager():
    def __init__(self, command_queue):
        self.command_queue = command_queue
    
    def update_loop(self):
        # pass
        while(True):
            # if self.command_queue.empty()
            s = input('> ')
            self.command_queue.put(Command('keyboard_input', s, time.time() + 0.1))
   
        
   
class MyButton():
    '''Class for controlling a pushbutton with integrated LED indicator. val1 is a value retuned on a short press, fn2 on a long press. The LED is ltoggled while one of the functions is in progress'''
    def __init__(self, sw_pin, led_pin, val1, val2, led_default_state=False, long_press_threshold=0.5):
        self.button = gpiozero.Button(sw_pin)
        self.led = gpiozero.LED(led_pin)
        self.led_default = led_default_state
        self.val1 = val1
        self.val2 = val2
        self.sw_state = False
        self.press_detected = False    # indicate when a long press has been triggered but the button hasn't been released
        self.sw_press_time = 0
        
        self.set_led(self.led_default)
    
    def update(self):
        '''Read switch state and return val1 when a short press is detected, val2 when a long press is detected, and -1 otherwise'''
        if self.sw_state:
            duration = time.time() - self.sw_press_time
            
            
            # # retrun LED to default after the command finishes
            # self.set_led(self.led_default)
            
            if not self.button.is_pressed:
                self.sw_state = False
                self.press_detected = False
                
                # if the button wasn't held, register a short press
                if not self.press_detected:
                    return self.val1
                
            elif duration > self.long_press_threshold:
                self.press_detected = True
                return self.val2
                
        else:
            if self.button.is_pressed:
                self.sw_state = True
                self.sw_press_time = time.time()
                # toggle LED when the button is pressed
                self.set_led(not self.led_default)
        return -1
                
    def set_led(self, state):
        if state:
            self.led.on()
        else:
            self.led.off()
            
            
class ToggleSwitch():
    '''Toggle switch with associated LED'''
    def __init__(self, sw_pin, led_pin, default_state=False):
        self.button = gpiozero.Button(sw_pin)
        self.led = gpiozero.LED(led_pin)
        self.default_state = default_state
        
        self.read()
        
    def read(self):
        self.sw_state = self.button.is_pressed ^ self.default_state
        self.set_led(self.sw_state)
        return self.sw_state
    
    def set_led(self, state):
        if state:
            self.led.on()
        else:
            self.led.off()
            
            
class RotarySwitch():
    '''Reads state of multi-position rotary switch (each position connects a different pin to ground)'''
    def __init__(self, pins):
        self.num_positions = len(pins)
        self.switches =[gpiozero.Button(p) for p in pins]
        
        
    def read(self):
        '''Return switch position in a range of 0-(n-1) where n is the number of positions'''
        for i in range(self.num_positions):
            if self.switches[i].is_pressed:
                
                return i
        
        # error state: none of the switches is closed
        return -1
    
