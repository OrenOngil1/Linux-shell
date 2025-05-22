CFLAGS = -Wall -g
TESTFLAGS = -Iunity/src/ -Isrc
UNITY = unity/src/unity.c
PARSER = build/parser.o
PROCESS = build/process.o
STRQUEUE = build/strQueue.o
HISTORY = build/history.o

AUXILIARY = $(PARSER) $(PROCESS) $(STRQUEUE) $(HISTORY)
MYSHELL = build/shell.o

all: shell

shell: $(MYSHELL) $(AUXILIARY)
	gcc $(CFLAGS) -o build/shell $(MYSHELL) $(AUXILIARY)

parserTest: build/parserTest.o $(PARSER)
	gcc $(CFLAGS) -o build/parserTest build/parserTest.o $(PARSER) $(UNITY)

build/parserTest.o: test/parserTest.c
	gcc $(CFLAGS) $(TESTFLAGS) -c test/parserTest.c -o build/parserTest.o

$(MYSHELL): src/shell.c
	gcc $(CFLAGS) -c src/shell.c -o $(MYSHELL)

$(PARSER): src/parser.c
	gcc $(CFLAGS) -c src/parser.c -o $(PARSER)

$(PROCESS): src/process.c
	gcc $(CFLAGS) -c src/process.c -o $(PROCESS)

$(STRQUEUE): src/strQueue.c
	gcc $(CFLAGS) -c src/strQueue.c -o $(STRQUEUE)

$(HISTORY): src/history.c
	gcc $(CFLAGS) -c src/history.c -o $(HISTORY)


.PHONY: clean
clean:
	rm -f build/*