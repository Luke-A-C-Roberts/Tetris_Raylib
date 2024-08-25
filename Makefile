COMPILER := clang
FLAGS   := -Wall -Wextra -Wpedantic -g -Og
LIBS    := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

default:
	mkdir -p ./build
	$(COMPILER) $(FLAGS) -shared -fPIC -o ./build/libgame.so ./src/game.c $(LIBS)
	$(COMPILER) $(FLAGS) -o tetris ./src/main.c $(LIBS)

game:
	mkdir -p ./build
	$(COMPILER) $(FLAGS) -shared -fPIC -o ./build/libgame.so ./src/game.c $(LIBS)
