import chess
import chess.engine
import sys
from board_adapter import BoardAdapter
import IO_manager
from collections import namedtuple
import threading
import time
import queue


# # engine skill level ranges from 0-20
# engine_skill_levels = [0, 2, 5, 10, 15, 20]

# settings for engine level (based on lichess skill settings 1, 3, 4, 5, 6, 8)
# engine_skill_levels = [-9, -1, 3, 7, 11, 20]
engine_skill_levels = [0, 1, 3, 7, 11, 20]
engine_time_limits = [.05, .15, .2, .3, .4, 1]
engine_depth = [5, 5, 5, 5, 8, 22]

# Commands are put in a queue monitored by a GameManager. When a executing a command, 
# it calls the given function with the provided argumens. If the current time is after
# the expiration time, the command is skipped
Command = namedtuple('Command', ['func', 'args', 'expiration_time'])
    

class GameManager():
    MENU = 1
    # AWAITING_MOVE = 2
    AWAITING_CPU_MOVE = 2
    AWAITING_PLAYER_MOVE = 3
    BUSY = 4

    
    def __init__(self, robot, cpu_time_limit=0.1, cpu_depth_limit=None):
        self.robot = robot
        self.white_is_cpu = False
        self.black_is_cpu = False
        self.cpu_time_limit = cpu_time_limit
        self.cpu_depth_limit = cpu_depth_limit

        # monitors switches/buttons and controls LEDs
        self.command_queue = queue.Queue()
        print('Initializing IO manager...')
        self.IO = IO_manager.IO_Manager(self.command_queue)
        self.keyboard = IO_manager.KeyboardManager(self.command_queue)
        print('\tSuccess')
        
        current_os = sys.platform
        if current_os == 'win32':
            self.engine_filepath = 'stockfish/stockfish-windows-x86-64-sse41-popcnt.exe'
        elif current_os =='linux':
            self.engine_filepath = 'stockfish/stockfish-android-armv8'
        else:
            raise ValueError(f'OS "{current_os}" not recognized')

        for i in range(5):
            try:
                self.engine = chess.engine.SimpleEngine.popen_uci(self.engine_filepath, timeout=15)
                print('Engine initialized')
                break
            except TimeoutError:
                print(f'Failed to initialize engine ({i+1}/5)')
                
        self.board = None    
        
        self.state = self.MENU
        
    
    def run(self):
        # Start threads to monitor inputs
        threading.Thread(target=self.IO.update_loop, daemon=True).start()
        threading.Thread(target=self.keyboard.update_loop, daemon=True).start()
        
        while True:
            if self.state != self.BUSY:
                # check for input from the control panel / keyboard
                while not self.command_queue.empty():
                    cmd = self.command_queue.get()
                    print(f'command: {cmd}')
                    if cmd.expiration_time is not None and cmd.expiration_time < time.time():
                        print('\tTime expired: skipping command')
                        continue
                    func = getattr(self, cmd.func)
                    # execute the command
                    if type(cmd.args) == tuple:
                        func(*cmd.args)
                    else:
                        func(cmd.args)
                
                
                # If a game is running and it is the computer's turn, query the engine
                if self.state == self.AWAITING_CPU_MOVE:
                    self.play_move(self.get_computer_move())
                        
            # match self.state:
            #     case MENU:
            #         pass
            #     case AWAITING_MOVE:
            #         pass
            #     case _:
            #         pass
        
        
        
    def configure_engine(self, level):
        '''Set the engine to one of six levels (0-5)'''
        print('\tsetting engine level to', level)
        self.engine.configure({"Skill Level": engine_skill_levels[level]})
        self.cpu_time_limit = engine_time_limits[level]
        self.cpu_depth_limit = engine_depth[level]
    
    def configure_players(self, white, black):
        print(f'Setting players: white={'cpu' if white else 'human'} black={'cpu' if black else 'human'}')
        self.white_is_cpu = white
        self.black_is_cpu = black
        if self.state == self.AWAITING_PLAYER_MOVE or self.state == self.AWAITING_CPU_MOVE:
            self.set_awaiting_state()
    
        
    def keyboard_input(self, user_input):
        '''Parse user input text commands'''
        if self.state == self.BUSY:
            print('busy')
            return        
        
        # some commands may be followed by additional arguments, separetd by spaces
        if ' ' in user_input:
            [cmd, args] = user_input.lower().split(' ', maxsplit=1)
        else:
            cmd = user_input.lower()
            args = ''
            
        match cmd:
            case 'play':    # start a game, letting the user chose which side(s) to play
                self.white_is_cpu = not 'w' in args
                self.black_is_cpu = not 'b' in args
                self.start_game()
                
            case 'resume':    # resume the game in progress, optionally changing sides
                if self.state != self.MENU:
                    print('\'resume\' command is only allowed from the menu')
                    return
                if self.board is None:
                    print('\tno game in progress')
                    return
                self.white_is_cpu = not 'w' in args
                self.black_is_cpu = not 'b' in args
                self.resume_game()
            
            case 'engine':    # set the strength of the engine (0-5)
                level = int(args)
                if(level < 0 or level > 5):
                    print('Engine level must be an integer from 0-5')
                else:
                    self.configure_engine(level) 
                
            case 'init':
                self.manual_setup('starting')
            case 'setup':
                self.setup('starting')
            case 'clear':
                self.setup('empty')
            case 'adjust':
                self.adjust_pieces()
            case 'home':
                self.home_axes()
            case 'pause':
                self.pause_game()
            case 'exit':
                self.quit()
            case _:
                if self.state == self.AWAITING_PLAYER_MOVE:
                    # Enter a move in standard algebreic notation
                    try:
                        move = self.board.parse_san(user_input)
                        self.play_move(move)
                        return
                    except chess.IllegalMoveError:
                        print('\tillegal move')
                        return
                    except chess.AmbiguousMoveError:
                        print('\tambiguous move')
                        return
                    except chess.InvalidMoveError:
                        pass
                
                print('\tinvalid command')
        
                
        
    
    
    def button_press(self, button_id):
        #TODO
        print(f'button {button_id}')
        
        match button_id:
            case 0: # start or stop game
                if self.state == self.MENU:
                    if self.board == None:
                        self.start_game()
                    else:
                        self.resume_game()
                elif self.state == self.AWAITING_PLAYER_MOVE or self.state == self.AWAITING_CPU_MOVE:
                    self.pause_game()
            case 1:
                self.home_axes()
            case 2:
                self.manual_setup('starting')
            case 3:
                self.setup('starting')
            case 4:
                pass
            case 5:
                self.adjust_pieces()
            case 6:
                self.manual_setup('empty')
            case 7:
                self.setup('empty')
            
            case _:
                raise(ValueError(f'Invalid button id: {button_id}'))

    
    # UI commands ================================================
    def start_game(self):
        if self.state != self.MENU:
            print('\'play\' command is only allowed from the menu')
            return

        # check that the board is set properly
        if not robot.is_starting_position:
            print('\tboard is not in starting position (call \'setup\' to set up automatically or \'init\' to set up manually)')
            return
        print('Starting new game')
        # start the game
        self.board = chess.Board()
        self.set_awaiting_state()
        self.IO.set_game_state(True)

    def resume_game(self):
        print('Resuming game')
        self.set_awaiting_state()
        self.IO.set_game_state(True)
    
    def pause_game(self):
        if self.state == self.MENU:
            print('no game in progress')
        else:
            print('Pausing game')
            self.state = self.MENU
            self.IO.set_game_state(False)

    
    def quit_game(self):
        print('Quitting game')
        self.board = None
        self.state = self.MENU
        self.IO.set_game_state(False)

    def home_axes(self):
        print('Homing axes')
        old_state = self.state
        self.state = self.BUSY
        robot.home()
        self.state = old_state

    def adjust_pieces(self):
        print('Adjusting pieces')
        old_state = self.state
        self.state = self.BUSY
        robot.adjust_pieces()
        self.state = old_state

    def setup(self, state):
        ''''Physically reset the board to the starting position or clear the board'''
        print(f'Setting up: {state}')
        robot.setup_position(state)
        # quit any games in progress
        self.quit_game()

    def manual_setup(self, state):
        '''Initialize the board state after manually setting up'''
        print(f'Manual setup: {state}')
        robot.initialize_position(state)
        # quit any games in progress
        self.quit_game()


    #=============================================================

    def set_awaiting_state(self):
        '''Set the current state to await a move from the engine or human player'''
        cpu_turn = self.white_is_cpu if self.board.turn else self.black_is_cpu
        self.state = self.AWAITING_CPU_MOVE if cpu_turn else self.AWAITING_PLAYER_MOVE

        # flash indicator LEDs to indicate player's turn
        if self.board.turn:
            self.IO.set_blinking(True, False)
        else:
            self.IO.set_blinking(False, True)
    

    
    def get_computer_move(self):
        '''Choose the best move in the position, according to the engine'''
        result = self.engine.play(self.board, chess.engine.Limit(time=self.cpu_time_limit, depth=self.cpu_depth_limit))
        return result.move
    
    
    def play_move(self, move):
        self.state = self.BUSY
        # TODO: handle promotion and ep captures
        self.robot.move(move.uci(), is_castling=self.board.is_castling(move))
        self.board.push(move)
        
        print(move)
        print(self.board)
        print()

        if self.board.is_game_over():
            outcome = self.board.outcome()
            print(f'game over: {outcome}')
            self.quit_game()
            return

        self.set_awaiting_state()
        
    def quit(self):
        print('closing program')
        self.engine.quit()
        sys.exit()
    


if __name__ == '__main__':
    robot = BoardAdapter()
    robot.await_response('ready')
    if robot.connection_error:
        print('Connection error: no board control')

    manager = GameManager(robot)

    robot.home()
    robot.initialize_position()
    time.sleep(1)
    
    manager.run()
