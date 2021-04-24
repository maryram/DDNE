#
# Makefile for non-Microsoft compilers
#

## Linux  (uncomment the 2 lines below for compilation on Linux)
CXXFLAGS += -std=c++0x -Wall -ggdb
LDFLAGS += -lrt

## CygWin (uncomment the 2 lines below for compilation on CygWin)
#CXXFLAGS += -Wall
#LDFLAGS += 

MAIN = DDNE

all: $(MAIN) extract_Rank extract_ProbabilityParams

opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: $(MAIN) extract_Rank extract_ProbabilityParams

# COMPILE
$(MAIN): $(MAIN).cpp Snap.o 
	g++ $(LDFLAGS) -o DDNE $(MAIN).cpp dne.cpp Snap.o -lrt -I./glib -I./snap -std=c++0x

extract_Rank: extract_Rank.cpp Snap.o
	g++ $(LDFLAGS) -o extract_Rank extract_Rank.cpp dne.cpp Snap.o -lrt -I./glib -I./snap -std=c++0x
	
extract_ProbabilityParams: extract_ProbabilityParams.cpp Snap.o
	g++ $(LDFLAGS) -o extract_ProbabilityParams extract_ProbabilityParams.cpp dne.cpp Snap.o -lrt -I./glib -I./snap -std=c++0x

Snap.o: 
	g++ -c $(CXXFLAGS) ./snap/Snap.cpp -I./glib -I./snap -std=c++0x

clean:
	rm -f   $(MAIN) extract_Rank extract_ProbabilityParams $(MAIN).exe extract_Rank.exe extract_ProbabilityParams.exe

