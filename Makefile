# paths
INCLUDE = ./include
MODULES = ./modules
PROGRAM = ./program
# compiler
CC = gcc

# Compile options. Το -I<dir> λέει στον compiler να αναζητήσει εκεί include files
CFLAGS = -Wall  -g -I$(INCLUDE)
LDFLAGS = 

# Αρχεία .o
OBJS = $(PROGRAM)/mysh.o $(MODULES)/ADTList.o $(MODULES)/helping_funcs.o $(MODULES)/bash_interface.o $(MODULES)/ADTVector.o $(MODULES)/ADTMap.o

# Το εκτελέσιμο πρόγραμμα
EXEC = $(PROGRAM)/mysh

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJS) $(EXEC) \
		*.txt

run: $(EXEC)
	$(EXEC)

valgrind: $(EXEC)
	valgrind --leak-check=full --track-origins=yes $(EXEC)
