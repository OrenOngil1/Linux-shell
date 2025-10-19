CC = gcc

CFLAGS = -Wall -g -o

OBJECTS = src/operator.o src/stack.o src/queue.o src/scanner.o src/ast.o src/process.o src/history.o src/sig.o src/executor.o src/shell.o

all: $(OBJECTS)
	$(CC) $(CFLAGS) shell $(OBJECTS)
	rm -f src/*.o

parser_test: test.o src/operator.o src/stack.o src/scanner.o src/ast.o
	$(CC) $(CFLAGS) parser_test test.o src/operator.o src/stack.o src/scanner.o src/ast.o

src/shell.o: src/shell.c
	$(CC) -c $(CFLAGS) src/shell.o src/shell.c

src/executor.o: src/executor.c
	$(CC) -c $(CFLAGS) src/executor.o src/executor.c

src/sig.o:
	$(CC) -c $(CFLAGS) src/sig.o src/sig.c

src/process.o: src/process.c
	$(CC) -c $(CFLAGS) src/process.o src/process.c

src/history.o: src/history.c
	$(CC) -c $(CFLAGS) src/history.o src/history.c

src/ast.o: src/ast.c
	$(CC) -c $(CFLAGS) src/ast.o src/ast.c

src/scanner.o: src/scanner.c
	$(CC) -c $(CFLAGS) src/scanner.o src/scanner.c

src/queue.o: src/queue.c
	$(CC) -c $(CFLAGS) src/queue.o src/queue.c

src/stack.o: src/stack.c
	$(CC) -c $(CFLAGS) src/stack.o src/stack.c

src/operator.o: src/operator.c
	$(CC) -c $(CFLAGS) src/operator.o src/operator.c

test.o:
	$(CC) -c $(CFLAGS) test.o test.c

.PHONY: clean
clean:
	rm -f src/*.o shell