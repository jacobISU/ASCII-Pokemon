CC = gcc
CXX = g++
ECHO = echo
RM = del /Q

TERM = "S2023"

# Update include path and library path for ncurses
CFLAGS = -Wall -Werror -ggdb -funroll-loops -DTERM=$(TERM) -IC:/MinGW/include/ncursesw
CXXFLAGS = -Wall -Werror -ggdb -funroll-loops -DTERM=$(TERM) -IC:/MinGW/include/ncursesw

LDFLAGS = -LC:/MinGW/lib -lncursesw

BIN = poke327
OBJS = poke327.o heap.o character.o io.o

all: $(BIN)

$(BIN): $(OBJS)
	@$(ECHO) Linking $@
	@$(CXX) $^ -o $@ $(LDFLAGS)

-include $(OBJS:.o=.d)

%.o: %.c
	@$(ECHO) Compiling $<
	@$(CC) $(CFLAGS) -MMD -MF $*.d -c $<

%.o: %.cpp
	@$(ECHO) Compiling $<
	@$(CXX) $(CXXFLAGS) -MMD -MF $*.d -c $<

.PHONY: all clean clobber

clean:
	@$(ECHO) Removing all generated files
	@$(RM) *.o $(BIN) *.d TAGS core vgcore.* gmon.out

clobber: clean
	@$(ECHO) Removing backup files
	@$(RM) *~ \#* *pgm