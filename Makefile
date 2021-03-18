# make
# make clean
# make run procs=4

CXX=mpic++
DEP_FLAGS=-MMD
DEP_FLAGS+=-MP
CXXFLAGS+=$(DEP_FLAGS)
MAIN=Executable
SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)
PROCS=$(procs)

all: $(MAIN)

$(MAIN): $(OBJ)
		$(CXX) $? -o $@

$(OBJ): $(SRC)
		$(CXX) $(CXXFLAGS) -c $*.cpp -o $@

.PHONY: clean run backup cB

clean:
	rm -f $(MAIN) $(OBJ) $(DEP)

run: $(MAIN)
	mpiexec -np $(PROCS) ./Executable

backup:
	if [ -d "./backup" ]; then rm -r backup; fi
	mkdir backup
	cp $(SRC) backup
	cp *.h backup

cB:
	rm -r backup

-include $(DEP)
