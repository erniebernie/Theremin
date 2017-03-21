CC = g++
COMP_FLAGS = -w
LINKER_FLAGS = -lSDL2 -lSDL2_ttf  
EXE = UI
OBJS = UI.o input.o

all : $(OBJS)
	$(CC) $(OBJS) $(COMP_FLAGS) $(LINKER_FLAGS) -o $(EXE)

UI.o : UI.cpp
	$(CC) -c UI.cpp
input.o : input.cpp
	$(CC) -c input.cpp

clean : 
	rm $(EXE) *.o
run :
	./$(EXE)

#for somereason when using this makefile I get memory leaks
#g++ *.cpp -o UI -lSDL2 -lSDL2_ttf
