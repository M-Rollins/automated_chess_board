

# from chess import Board, engine, Move
# # import chess.engine
# # import chess.svg

import chess
import chess.engine

# print("hello world")

whiteIsHuman = True
blackIsHuman = True
cpuTimeLimit = .1

# engine = engine.SimpleEngine.popen_uci("stockfish.exe")
engine = chess.engine.SimpleEngine.popen_uci("stockfish\stockfish-windows-x86-64-sse41-popcnt.exe")


def quitGame():
    engine.quit()
    quit()

def get_human_move():
    while True:
        player = "white" if board.turn else "black"

        #ask for user input until a legal move is selected
        while True:
            try:
                m = input(player + " to move: ")
                if m == 'quit':
                    quitGame()

                move = board.parse_san(m)
                break
            except ValueError:
                print('\'' + m + '\'' + ' is not a legal move. Try again.')

        return move


def get_computer_move():

    result = engine.play(board, chess.engine.Limit(time=cpuTimeLimit))
    return result.move


board = chess.Board()


#playing the game

while not board.is_game_over():
    move = chess.Move.null
    # if board.turn:
    #     if whiteIsHuman:
    #         move = get_human_move()
    #     else:
    #         move = get_computer_move()
    # else:
    #     if blackIsHuman:
    #         move = get_human_move()
    #     else:
    #         move = get_computer_move()

    if (board.turn and whiteIsHuman) or (not board.turn and blackIsHuman):
        move = get_human_move()
    else:
        move = get_computer_move()


    if move in board.legal_moves:
        board.push(move)
    print(move)
    # print()
    print(board)
    print()

# outcome = board.outcome()
print("game over")
quitGame()

# chess.svg.board(board, size=350)










# See PyCharm help at https://www.jetbrains.com/help/pycharm/
