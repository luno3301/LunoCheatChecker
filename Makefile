CXX      = C:/msys64/mingw64/bin/g++.exe
CXXFLAGS = -std=c++17 -I. -IC:/msys64/mingw64/include -IC:/msys64/mingw64/include/postgresql
LDFLAGS  = -LC:/msys64/mingw64/lib -lpq -lbcrypt -liphlpapi -lws2_32

SRCS = main.cpp logger.cpp database.cpp system.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = app.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
