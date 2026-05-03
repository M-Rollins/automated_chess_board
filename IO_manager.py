import gpiozero
import math
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
                b.update_led()
                if val != -1:
                    self.command_queue.put(Command('button_press', val, time.time() + 1))

    def set_game_state(self, playing):
        '''Change led states based on whetehr a game is in progress'''
        self.buttons[0].led_default = playing
        if not playing:
            # Only flash lights when a game is in progress
            self.set_blinking(False, False)

    def set_blinking(self, white=False, black=False):
        '''Flash LEDs associated with the toggle switches'''
        self.white_toggle.set_blinking(white)
        self.black_toggle.set_blinking(black)

                
class KeyboardManager():
    def __init__(self, command_queue):
        self.command_queue = command_queue
    
    def update_loop(self):
        while(True):
            # if self.command_queue.empty()
            try:
                s = input('')
                self.command_queue.put(Command('keyboard_input', s, time.time() + 0.5))
            except Exception:
                pass
   
        
   
class MyButton():
    '''Class for controlling a pushbutton with integrated LED indicator. val1 is a value retuned on a short press, val2 on a long press. The LED is ltoggled while one of the functions is in progress'''
    def __init__(self, sw_pin, led_pin, val1, val2, led_default_state=False, long_press_threshold=0.5):
        self.button = gpiozero.Button(sw_pin)
        self.led = gpiozero.LED(led_pin)
        self.led_default = led_default_state
        self.val1 = val1
        self.val2 = val2
        self.long_press_threshold = long_press_threshold
        self.sw_state = False
        self.press_detected = False    # indicate when a long press has been triggered but the button hasn't been released
        self.sw_press_time = 0
        
        self.update_led()
    
    def update(self):
        '''Read switch state and return val1 when a short press is detected, val2 when a long press is detected, and -1 otherwise'''
        if self.sw_state:
            duration = time.time() - self.sw_press_time

            # button is released
            if not self.button.is_pressed:
                short_press = not self.press_detected
                self.sw_state = False
                self.press_detected = False
                
                # if the button wasn't held, register a short press
                if short_press:
                    return self.val1
                
            elif duration > self.long_press_threshold and not self.press_detected:
                self.press_detected = True
                return self.val2
                
        else:
            if self.button.is_pressed:
                self.sw_state = True
                self.sw_press_time = time.time()
        return -1
                
    def update_led(self):
        if self.sw_state == self.led_default:
            self.led.off()
        else:
            self.led.on()
            
            
class ToggleSwitch():
    '''Toggle switch with associated LED'''
    BLINK_PERIOD = 0.75
    BLINK_DC = 0.5      # duty cycle (time off / total time)

    def __init__(self, sw_pin, led_pin, default_state=False):
        self.button = gpiozero.Button(sw_pin)
        self.led = gpiozero.PWMLED(led_pin)
        self.default_state = default_state
        self.blinking = False
        
        self.read()
        
    def read(self):
        self.sw_state = self.button.is_pressed ^ self.default_state
        self._set_led(self.sw_state)
        return self.sw_state
    
    def set_blinking(self, blinkstate):
        self.blinking = blinkstate
        if blinkstate: self.blink_start_time = time.time()
    
    def _set_led(self, state):
        # To blink, periodically invert led state
        if self.blinking:
            t = ((time.time() - self.blink_start_time) / self.BLINK_PERIOD) % 1
            if t < self.BLINK_DC:
                theta = t / self.BLINK_DC * 2 * math.pi
                val = (math.cos(theta) + 1) / 2
            else:
                val = 1
            self.led.value = val if state else 1 - val
        else:
            self.led.value = 1 if state else 0
            
            
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
    
