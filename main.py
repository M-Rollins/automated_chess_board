

from Chess import Board, engine, Move
# import chess.engine
# import chess.svg

# print("hello world")

whiteIsHuman = True
blackIsHuman = False
cpuTimeLimit = .1

engine = engine.SimpleEngine.popen_uci("stockfish.exe")

def quitGame():
    engine.quit()
    quit()

def get_human_move():
    while True:
        if board.turn:
            player = "white"
        else:
            player = "black"

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

    result = engine.play(board, engine.Limit(time=cpuTimeLimit))
    return result.move


board = Board()


#playing the game

while not board.is_game_over():
    move = Move.null
    if board.turn:
        if whiteIsHuman:
            move = get_human_move()
        else:
            move = get_computer_move()
    else:
        if blackIsHuman:
            move = get_human_move()
        else:
            move = get_computer_move()


    if move in board.legal_moves:
        board.push(move)
    print(move)
    # print()
    print(board)
    print()


quitGame()

    # chess.svg.board(board, size=350)










# See PyCharm help at https://www.jetbrains.com/help/pycharm/
