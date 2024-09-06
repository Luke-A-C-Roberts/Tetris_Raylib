COMPILER := clang
FLAGS   := -Wall -Wextra -Wpedantic -g -Og
LIBS    := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

default:
	make game
	$(COMPILER) $(FLAGS) -o tetris ./src/load.c ./src/debug.c ./src/main.c $(LIBS)

game:
	mkdir -p ./build
	$(COMPILER) $(FLAGS) -shared -fPIC -o ./build/libgame.so ./src/game.c ./src/display.c $(LIBS)

clear:
	rm ./build -rf
	rm ./tetris
