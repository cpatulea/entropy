CXXFLAGS = -W -Wall -O2 -g -ggdb -march=pentium4 -mfpmath=sse

.PHONY: all clean
all: entropy

clean:
	rm -f entropy
