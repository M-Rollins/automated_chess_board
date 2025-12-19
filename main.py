import chess
import chess.engine
import sys
from board_adapter import BoardAdapter
import time


class Game():
    
    def __init__(self, robot, w=True, b=True, cpu_time_limit=0.1, engine_strength=20):
        self.robot = robot
        self.white_is_human = w
        self.black_is_human = b
        self.cpu_time_limit = cpu_time_limit
        self.engine_strength = engine_strength
        
        self.engine = chess.engine.SimpleEngine.popen_uci('stockfish/stockfish-windows-x86-64-sse41-popcnt.exe')
        self.engine.configure({"Skill Level": self.engine_strength})
        self.board = chess.Board()
        self.game_over = False
    
    def set_engine_strength(self, s):
        self.engine_strength = s
        
    def resume(self):
        '''Start up a game again after quitting'''
        self.engine = chess.engine.SimpleEngine.popen_uci('stockfish/stockfish-windows-x86-64-sse41-popcnt.exe')
        self.engine.configure({"Skill Level": self.engine_strength})
        self.game_over = False
        self.play_game()
        
    def quitGame(self):
        self.engine.quit()
        self.game_over = True
    
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
        result = self.engine.play(self.board, chess.engine.Limit(time=self.cpu_time_limit))
        return result.move
    
    
    def play_game(self):
        '''Play a game, ending when the game is over or the user enters the "quit" command'''
        while not self.board.is_game_over():
            move = chess.Move.null
        
            if (self.board.turn and self.white_is_human) or (not self.board.turn and self.black_is_human):
                move = self.get_human_move()
                if self.game_over:
                    print('game ended by user')
                    return
            else:
                move = self.get_computer_move()
        
            if move in self.board.legal_moves:
                # TODO: handle ep captures
                self.robot.move(move.uci(), is_castling=self.board.is_castling(move))
                self.board.push(move)
                
            print(move)
            print(self.board)
            print()
            
        outcome = self.board.outcome()
        print(f'game over: {outcome}')
        self.quitGame()


if __name__ == '__main__':
    robot = BoardAdapter()
    if robot.connection_error:
        print('Connection error: no board control')
        
    robot.await_response('ready')
    robot.home()
    robot.initialize_position()
    time.sleep(1)
        
    game = None
    engine_strength = 20
    
    while True:
        # let the user choose which side(s) to play
        user_input = input('command(\'exit\' to quit): ')
        if ' ' in user_input:
            [cmd, args] = user_input.lower().split(' ', maxsplit=1)
        else:
            cmd = user_input.lower()
            args = ''
        
        match cmd:
            case 'play':
                # start a game, letting the user chose which side(s) to play
                
                # check that the board is set properly
                if not robot.is_starting_position:
                    print('\tboard is not in starting position (call \'setup\' to set up automatically or \'init\' to set up manually)')
                    continue
                
                white_is_human = 'w' in args
                black_is_human = 'b' in args
                # start the game
                game = Game(robot, white_is_human, black_is_human, engine_strength=engine_strength)
                game.play_game()
                
            case 'resume':
                # resume the game in progress, optionally changing sides
                if game is None:
                    print('\tno game in progress')
                    continue
                
                # update which side(s) the computer plays
                game.white_is_human = 'w' in args
                game.black_is_human = 'b' in args
                
                game.resume()
            
            case 'engine':
                # set the strength of the engine (1-20)
                try:
                    s = int(args)
                    if(s < 0 or s > 20):
                        print('Engine strength must be an integer from 0-20')
                    else:
                        engine_strength = s
                        if game is not None: game.set_engine_strength(s)
                        print('\tsetting engine strength to', s)
                except ValueError:
                    print('Engine strength must be an integer from 0-20')
                
                
            case 'init':
                # initialize the board state to the starting position after manually setting up
                robot.initialize_position()
                game = None
            case 'setup':
                # physically reset the board to the starting position
                robot.setup_position('starting')
                game = None
            case 'clear':
                # physically remove all pieces from the board
                robot.setup_position('empty')
                game = None
            case 'adjust':
                robot.adjust_pieces()
            case 'home':
                robot.home()
            case 'exit':
                sys.exit()
            case _:
                print('\tinvalid command')
