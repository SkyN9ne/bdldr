
CC=gcc
CFLAGS=-O2

all: bdldr_cli bdldr_daemon

bdldr_cli: bdldr_cli.c
	$(CC) $(CFLAGS) bdldr_cli.c -ldl -o bdldr_cli

bdldr_daemon: bdldr_daemon.c
	$(CC) $(CFLAGS) bdldr_daemon.c -ldl -o bdldr_daemon

clean:
	rm bdldr_cli bdldr_daemon

