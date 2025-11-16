import chess
import chess.engine

engine = chess.engine.SimpleEngine.popen_uci('stockfish/stockfish-windows-x86-64-sse41-popcnt.exe')

board = chess.Board()
while not board.is_game_over():
    result = engine.play(board, chess.engine.Limit(time=0.1))
    board.push(result.move)
    print(board)

engine.quit()




# import asyncio
# import chess
# import chess.engine

# async def foo():
#     print('foo')

# async def main() -> None:
#     transport, engine = await chess.engine.popen_uci('stockfish/stockfish-windows-x86-64-sse41-popcnt.exe')

#     board = chess.Board()
#     while not board.is_game_over():
#         result = await engine.play(board, chess.engine.Limit(time=0.1))
#         board.push(result.move)
        
#         print(board)

#     await engine.quit()

# asyncio.run(main())
