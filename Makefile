CFLAGS = `pkg-config --cflags --libs opencv4`
#LIBS = `pkg-config --libs opencv4`

all: miku

% : %.cpp
	g++ $< $(CFLAGS) $(LIBS) -o $@ 

