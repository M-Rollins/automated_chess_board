import chess
import chess.engine
import sys
from board_adapter import BoardAdapter
import time


class Game():
    
    def __init__(self, robot, w=True, b=True, cpu_time_limit=0.1):
        self.robot = robot
        self.white_is_human = w
        self.black_is_human = b
        self.cpu_time_limit = cpu_time_limit
        
        self.engine = chess.engine.SimpleEngine.popen_uci('stockfish/stockfish-windows-x86-64-sse41-popcnt.exe')
        self.board = chess.Board()
        
        
    def quitGame(self):
        self.engine.quit()
        sys.exit()
    
    def get_human_move(self):
        while True:
            player = 'white' if self.board.turn else 'black'
    
            # ask for user input until a legal move is selected
            while True:
                try:
                    m = input(player + ' to move: ')
                    if m.lower() == 'quit':
                        self.quitGame()
    
                    move = self.board.parse_san(m)
                    break
                except ValueError:
                    print('\'' + m + '\'' + ' is not a legal move. Try again.')
    
            return move
    
    def get_computer_move(self):
        result = self.engine.play(self.board, chess.engine.Limit(time=self.cpu_time_limit))
        return result.move
    
    #playing the game
    def play_game(self):
        # tell the robot that the board in in the starting position
        robot.initialize_position()
        time.sleep(1)
        
        while not self.board.is_game_over():
            move = chess.Move.null
        
            if (self.board.turn and self.white_is_human) or (not self.board.turn and self.black_is_human):
                move = self.get_human_move()
            else:
                move = self.get_computer_move()
        
        
            if move in self.board.legal_moves:
                self.board.push(move)
                self.robot.move(move.uci())
            print(move)
            # print()
            print(self.board)
            print()
            
        outcome = self.board.outcome()
        print(f'game over: {outcome}')
        self.quitGame()



if __name__ == '__main__':
    robot = BoardAdapter()
    robot.await_response('ready')
    robot.home()
    
    # let the user choose which side(s) to play
    game_params = input('game parameters: ')
    
    white_is_human = 'w' in game_params.lower()
    black_is_human = 'b' in game_params.lower()
    
    # start the game
    game = Game(robot, white_is_human, black_is_human)
    game.play_game()
    
    
    
    

