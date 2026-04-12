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
    AWAITING_MOVE = 2
    BUSY = 3

    
    def __init__(self, robot, cpu_time_limit=0.1, cpu_depth_limit=None):
        self.robot = robot
        self.white_is_cpu = False
        self.black_is_cpu = False
        self.cpu_time_limit = cpu_time_limit
        self.cpu_depth_limit = cpu_depth_limit
        
        current_os = sys.platform
        if current_os == 'win32':
            self.engine_filepath = 'stockfish/stockfish-windows-x86-64-sse41-popcnt.exe'
        elif current_os =='linux':
            self.engine_filepath = 'stockfish/stockfish-android-armv8'
        else:
            raise ValueError(f'OS "{current_os}" not recognized')

        self.engine = chess.engine.SimpleEngine.popen_uci(self.engine_filepath, timeout=15)
        self.board = chess.Board()
        # self.game_over = False
        
        self.command_queue = queue.Queue()
        
        self.game_in_progress = False
        self.state = self.MENU
        # monitors switches/buttons and controls LEDs
        print('Initializing IO manager...')
        self.IO = IO_manager.IO_Manager(self.command_queue)
        self.keyboard = IO_manager.KeyboardManager(self.command_queue)
        print('\tSuccess')
        
    
    def run(self):
        # Start threads to monitor inputs
        threading.Thread(target=self.IO.update_loop, daemon=True).start()
        threading.Thread(target=self.keyboard.update_loop, daemon=True).start()
        
        while True:
            # check for input from the control panel / keyboard
            while not self.command_queue.empty():
                cmd = self.command_queue.get()
                print(f'command: {cmd}')
                if cmd.expiration_time is not None and cmd.expiration_time < time.time():
                    print('/tTime expired: skipping command')
                    continue
                func = getattr(self, cmd.func)
                # execute the command
                if type(cmd.args) == tuple:
                    # cmd.func(*cmd.args)
                    func(*cmd.args)
                else:
                    # cmd.func(cmd.args)
                    func(cmd.args)
            
            
            # If a game is running and it is the computer's turn, query the engine
            if self.game_in_progress:
                cpu_turn = self.white_is_cpu if self.board.turn else self.black_is_cpu
                if cpu_turn:
                    self.play_move(self.get_computer_move())
                    if self.board.is_game_over():
                        outcome = self.board.outcome()
                        print(f'game over: {outcome}')
                        self.quit_game()
                        
            # match self.state:
            #     case MENU:
            #         pass
            #     case AWAITING_MOVE:
            #         pass
            #     case _:
            #         pass
        
        
    
    # def set_engine_strength(self, s):
    #     self.engine_strength = s
        
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
                if self.state != self.MENU:
                    print('\'play\' command is only allowed from the menu')
                    return

                # check that the board is set properly
                if not robot.is_starting_position:
                    print('\tboard is not in starting position (call \'setup\' to set up automatically or \'init\' to set up manually)')
                    return
                
                white_is_human = 'w' in args
                black_is_human = 'b' in args
                self.configure_players(not white_is_human, not black_is_human)
                # start the game
                self.board = chess.Board()
                self.state = self.AWAITING_MOVE
                
            case 'resume':    # resume the game in progress, optionally changing sides
                if self.state != self.MENU:
                    print('\'resume\' command is only allowed from the menu')
                    return
                if not self.game_in_progress:
                    print('\tno game in progress')
                    return
                
                # update which side(s) the computer plays
                white_is_human = 'w' in args
                black_is_human = 'b' in args
                self.configure_players(not white_is_human, not black_is_human)
                # reume the game
                self.state = self.AWAITING_MOVE
            
            case 'engine':    # set the strength of the engine (0-5)
                try:
                    level = int(args)
                    if(level < 0 or level > 5):
                        print('Engine level must be an integer from 0-5')
                    else:
                        self.configure_engine(level) 
                except ValueError:
                    print('Engine level must be an integer from 0-5')
                
            case 'init':    # initialize the board state to the starting position after manually setting up
                robot.initialize_position()
                self.game_in_progress = False
                self.state = self.MENU
            case 'setup':    # physically reset the board to the starting position
                robot.setup_position('starting')
                self.game_in_progress = False
                self.state = self.MENU
            case 'clear':    # physically remove all pieces from the board
                robot.setup_position('empty')
                self.game_in_progress = False
                self.state = self.MENU
            case 'adjust':
                robot.adjust_pieces()
            case 'home':
                robot.home()
            case 'exit':
                sys.exit()
            case _:
                print('\tinvalid command')
        
    
    
    def button_press(self, button_id):
        #TODO
        print(f'button {button_id}')
    
        
    def resume(self):
        '''Start up a game again after quitting'''
        self.engine = chess.engine.SimpleEngine.popen_uci(self.engine_filepath)
        self.game_over = False
        self.play_game()
        
    def quit_game(self):
        self.engine.quit()
        # self.game_over = True
        self.game_in_progress = False
    
    def get_human_move(self):
        '''Allow a human player to choose a move. Stops the game if the "quit" command is recieved'''
        while True:
            player = 'white' if self.board.turn else 'black'
    
            # ask for user input until a legal move is selected
            while True:
                m = input(player + ' to move: ')
                if m.lower() == 'quit':
                    self.quitGame()
                    return
                try:
                    move = self.board.parse_san(m)
                    break
                except ValueError:
                    print('\'' + m + '\'' + ' is not a legal move. Try again.')
    
            return move
    
    def get_computer_move(self):
        '''Choose the best move in the position, according to the engine'''
        result = self.engine.play(self.board, chess.engine.Limit(time=self.cpu_time_limit, depth=self.cpu_depth_limit))
        return result.move
    
    
    def play_move(self, move):
        if move in self.board.legal_moves:
            # TODO: handle promotion and ep captures
            self.robot.move(move.uci(), is_castling=self.board.is_castling(move))
            self.board.push(move)
            
        print(move)
        print(self.board)
        print()
        
        
    # def play_game(self):
    #     '''Play a game, ending when the game is over or the user enters the "quit" command'''
    #     while not self.board.is_game_over():
    #         move = chess.Move.null
        
    #         if (self.board.turn and self.white_is_human) or (not self.board.turn and self.black_is_human):
    #             move = self.get_human_move()
    #             if self.game_over:
    #                 print('game ended by user')
    #                 return
    #         else:
    #             move = self.get_computer_move()
        
    #         if move in self.board.legal_moves:
    #             # TODO: handle promotion and ep captures
    #             self.robot.move(move.uci(), is_castling=self.board.is_castling(move))
    #             self.board.push(move)
                
    #         print(move)
    #         print(self.board)
    #         print()
            
    #     outcome = self.board.outcome()
    #     print(f'game over: {outcome}')
    #     self.quitGame()


if __name__ == '__main__':
    robot = BoardAdapter()
    if robot.connection_error:
        print('Connection error: no board control')
        
    robot.await_response('ready')
    robot.home()
    robot.initialize_position()
    time.sleep(1)
    
    GameManager(robot).run()
