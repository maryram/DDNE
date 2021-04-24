#
# Makefile for non-Microsoft compilers
#

## Linux  (uncomment the 2 lines below for compilation on Linux)
CXXFLAGS += -std=c++0x -Wall
LDFLAGS += -lrt

## CygWin (uncomment the 2 lines below for compilation on CygWin)
#CXXFLAGS += -Wall
#LDFLAGS += 

MAIN = infer

opt: CXXFLAGS += -O4
opt: LDFLAGS += -O4
opt: $(MAIN) generate_Net	split_Data_Synthetic	split_INFOPATHInfNet	memeDataProcess

linux: LDFLAGS += -lrt
linux: generate_Net	split_Data_Synthetic	split_INFOPATHInfNet	memeDataProcess

opt_linux: CXXFLAGS += -O4
opt_linux: LDFLAGS += -lrt -O4
opt_linux: generate_Net	split_Data_Synthetic	split_INFOPATHInfNet	memeDataProcess

# COMPILE

processGephiFile: processGephiFile.cpp Snap.o
	g++ $(LDFLAGS) -o processGephiFile processGephiFile.cpp Snap.o -lrt -I./glib -I./snap -std=c++0x

generate_Net: generate_Net.cpp cascinf.cpp Snap.o
	g++ -o generate_Net generate_Net.cpp cascinf.cpp snap/kronecker.cpp Snap.o -I./glib -I./snap $(LDFLAGS)

split_Data_Synthetic: split_Data_Synthetic.cpp Snap.o
	g++ -o split_Data_Synthetic split_Data_Synthetic.cpp Snap.o -I./glib -I./snap $(LDFLAGS)

split_INFOPATHInfNet: split_INFOPathInfNet.cpp Snap.o
	g++ -o split_INFOPATHInfNet split_INFOPathInfNet.cpp Snap.o -I./glib -I./snap $(LDFLAGS)
	

memeDataProcess: memeDataProcess.cpp Snap.o
	g++ -o memeDataProcess memeDataProcess.cpp Snap.o -I./glib -I./snap $(LDFLAGS)

Snap.o: 
	g++ -c $(CXXFLAGS) ./snap/Snap.cpp -I./glib -I./snap

clean:
	rm -f   processGephiFile generate_Net split_Data_Synthetic split_INFOPATHInfNet memeDataProcess processGephiFile.exe generate_Net.exe split_Data_Synthetic.exe split_INFOPATHInfNet.exe memeDataProcess.exe	Snap.o 