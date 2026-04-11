import chess
import chess.engine
import sys

current_os = sys.platform
if current_os == 'win32':
    engine_path = 'stockfish/stockfish-windows-x86-64-sse41-popcnt.exe'
elif current_os =='linux':
    engine_path = 'stockfish/stockfish-android-armv8'
else:
    raise ValueError(f'OS "{current_os}" not recognized')

print(f'OS is {current_os}\nOpening engine at {engine_path}')

engine = chess.engine.SimpleEngine.popen_uci(engine_path, timeout=15)

print('Playing test game')
board = chess.Board()
while not board.is_game_over():
    result = engine.play(board, chess.engine.Limit(time=0.01))
    board.push(result.move)
    print(board)

engine.quit()
print('Engine test successful')
