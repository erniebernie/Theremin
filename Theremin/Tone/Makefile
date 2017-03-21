CC = g++
COMP_FLAGS = -w
LINKER_FLAGS = -lSDL2 -lSDL2_mixer #-lwiringPi
EXE = tone
OBJS = tone.cpp

all : $(OBJS)
	$(CC) $(OBJS) $(COMP_FLAGS) $(LINKER_FLAGS) -o $(EXE)

#tone.o : tone.cpp
#	$(CC) -c tone.cpp

clean : 
	rm $(EXE) #*.o
run :
	./$(EXE)


