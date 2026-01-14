CXX = g++
CXXFLAGS = -Wall -Werror -Wextra -O2 -std=c++17 -march=native -flto -fno-plt -mtune=native -g

TARGET = quoridor
SRCS = quoridor.cpp bitboard.cpp movegen.cpp position.cpp search.cpp

OBJDIR = build
OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Pattern rule for objects in build/
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build directory if missing (Linux mkdir works fine)
$(OBJDIR):
	mkdir -p $(OBJDIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJDIR)/*.o
	rm -rf $(OBJDIR)
	rm -f $(TARGET)