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
OBJS = $(PROGRAM)/mysh.o $(MODULES)/ADTList.o

# Το εκτελέσιμο πρόγραμμα
EXEC = $(PROGRAM)/mysh

# Παράμετροι για δοκιμαστική εκτέλεση
ARGS = 

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	$(EXEC) $(ARGS)

valgrind: $(EXEC)
	valgrind --leak-check=full $(EXEC) $(ARGS)
